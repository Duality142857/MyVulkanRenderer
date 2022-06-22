# Vulkan Synchronization

## vulkan隊列
先只考慮單隊列同步，多隊列同步只需要稍加擴展。
Vulkan隊列是一個GPU處理被提交的commandbuffers的抽象。

### 糾正誤區
command buffer沒有邊界，對於同步來說，所有提交給隊列的東西都只是命令的線性流。任何同步機制都全局作用于VkQueue上，沒有所謂command buffer內部的同步機制。

### command overlap
官方標準的說法是：命令按順序開始執行，但是未必按順序完成。不要被這種說法誤導，實際上如果不自己添加同步機制，所有queue中的命令都是亂序執行的。順序重排可能發生在command buffers甚至vkQueueSubmits之間！
注：D3D12可能禁止了queue submits之間的overlap
注：一個renderpass 內部的frame buffer操作按照api調用順序進行， 這是一個例外。

### Pipeline stages
每個提交給vulkan的命令都經過一系列stages，由VK_PIPELINE_STAGEW enum表示。當我們進行同步時，實際上是在對這些pipeline stages的整體進行同步，而不是對具體的單個命令進行同步。 draw calls, copy commands和compute dispatchs都依次經過pipeline stages.

#### the mysterious TOP_OF_PIPE and BOTTOM_OF_PIPE stages
他們實際上是"helper" stages, 實際上不做任何工作。每個命令都會先執行top of pipe stage， 基本上是gpu命令處理器在解析命令。而 bottom of pipe則是命令retire的地方。

### in-queue execution barriers
處理memory barrier之前，先理解execution barrier, 後者是前者的子集。vulkan中引入的主要的execution barrier機制是pipeline barrier.

```c++
void vkCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers);
```
srcStageMask 和 dstStageMask是Vulkan synchronization 模型的核心， 我們使用barrier將一個command stream分成兩個，everything before和everything after，這兩部分通過某種方式被同步。

#### srcStageMask
表示需要等待的東西。vulkan不支持單個命令之間的細粒度依賴。需要考慮特定pipeline stages的所有工作。例如，如果想要提交如下一系列命令：
```c++
vkCmdDispatch (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
vkCmdCopyBuffer (VK_PIPELINE_STAGE_TRANSFER_BIT)
vkCmdDispatch (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
vkCmdPipelineBarrier (srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
```

即使分別用vkQueueSubmit提交四次，仍然是放在一起考慮同步的，我們等待的工作是所有被提交到queue的命令，包括任何之前錄製到commandbuffer的命令。而srcStageMask限制了我們等待的範圍，在這個例子中，是發生在COMPUTE_SHADER_BIT stage的那些工作。由於是mask形式，所以可以同時等待compute和transfer這兩個stage。
ALL_COMMANDS_BIT則包括queue中的所有工作。ALL_GRAPHICS_BIT指renderpasses中的那些。
如果設爲TOP_OF_PIPE，則指不等待任何東西。

#### dstStageMask
barrier之後的已提交工作需要等待srcStageMask描述的工作，但是只有dstStageMask描述的那些會受此限制。例如，如果dstStageMask是FRAGMENT_SHADER_BIT, 那麼未來的命令只要等待到達FRAGMENT_SHADER_BIT，即可提前開始工作。
注： 如果dstStageMask爲BOTTOM_OF_PIPE，代表barrier之後沒有工作需要去等待。

### A crude example
```c++
vkCmdDispatch
vkCmdDispatch
vkCmdDispatch
vkCmdPipelineBarrier(srcStageMask = COMPUTE, dstStageMask = COMPUTE)
vkCmdDispatch
vkCmdDispatch
vkCmdDispatch

```
一種可能的執行順序爲321765，因爲vkCmdDispatch在compute stage.

### Events aka. split barriers
Vulkan provides a way to get overlapping work in-between barriers. The idea of VkEvent is to get some unrelated commands in-between the “before” and “after” set of commands
```c++
vkCmdDispatch
vkCmdDispatch
vkCmdSetEvent(event, srcStageMask = COMPUTE)
vkCmdDispatch
vkCmdWaitEvent(event, dstStageMask = COMPUTE)
vkCmdDispatch
vkCmdDispatch
```
命令4不被同步機制影響，能夠在123榨乾gpu時填補parallelism "bubble"，但不是所有gpu和驅動都能夠利用這種特性。

### Execution dependency chain 執行依賴鏈
srcStageMask的依賴會繼承到dstStageMask上，因此會形成依賴鏈，這一點比較隱晦。
```c++
vkCmdDispatch
vkCmdDispatch
vkCmdPipelineBarrier(srcStageMask = COMPUTE, dstStageMask = TRANSFER)
vkCmdPipelineBarrier(srcStageMask = TRANSFER, dstStageMask = COMPUTE)
vkCmdDispatch
vkCmdDispatch
```
這種情況56會等待12。

### Pipeline stages and render passes
計算和傳輸工作涉及到的pipeline stages很簡單，只包括：
```c++
TOP_OF_PIPE
DRAW_INDIRECT (for indirect compute only)
COMPUTE / TRANSFER
BOTTOM_OF_PIPE
```
render passes則更加複雜，並且很容易搞混哪些pipeline stages做什麼。通常包括兩族pipeline stages，一個是和幾何處理相關，一個是fragment相關(做光柵化和framebuffer操作)，因此除了TOP_OF_PIPE/BOTTOM_OF_PIPE之外，還包括：

```c++
Geometry
DRAW_INDIRECT – Parses indirect buffers
VERTEX_INPUT – Consumes fixed function VBOs and IBOs
VERTEX_SHADER – Actual vertex shader
TESSELLATION_CONTROL_SHADER
TESSELLATION_EVALUATION_SHADER
GEOMETRY_SHADER

Fragment
EARLY_FRAGMENT_TESTS
FRAGMENT_SHADER
LATE_FRAGMENT_TESTS
COLOR_ATTACHMENT_OUTPUT
```
後面兩段沒看懂。
EARLY_FRAGMENT_TESTS
This is the stage where early depth/stencil tests happen. This stage isn’t all that useful or meaningful except in some very obscure scenarios with frame buffer self-dependencies (aka, GL_ARB_texture_barrier). This is also where a render pass performs its loadOp of a depth/stencil attachment.

LATE_FRAGMENT_TESTS
This is where late depth-stencil tests take place, and also where depth-stencil attachments are stored with storeOp when a render pass is done.

HELPFUL TIP ON FRAGMENT TEST STAGES
It’s somewhat confusing to have two stages which basically do the same thing. When you’re waiting for a depth map to have been rendered in an earlier render pass, you should use srcStageMask = LATE_FRAGMENT_TESTS_BIT, as that will wait for the storeOp to finish its work.

When blocking a render pass with dstStageMask, just use a mask of EARLY_FRAGMENT_TESTS | LATE_FRAGMENT_TESTS.

NOTE: dstStageMask = EARLY_FRAGMENT_TESTS alone might work since that will block loadOp, but there might be shenanigans with memory barriers if you are 100% pedantic about any memory access happening in LATE_FRAGMENT_TESTS. If you’re blocking an early stage, it never hurts to block a later stage as well.

COLOR_ATTACHMENT_OUTPUT
This one is where loadOp, storeOp, MSAA resolves and frame buffer blend stage takes place, basically anything which touches a color attachment in a render pass in some way. If you’re waiting for a render pass which uses color to be complete, use srcStageMask = COLOR_ATTACHMENT_OUTPUT, and similar for dstStageMask when blocking render passes from execution.

### Memory barriers
執行順序和內存順序是不同的東西。GPU有多個不連貫的緩存，它們需要被仔細管理來避免渲染故障。這意味着只是對執行進行同步不足以確保GPU的不同單元能夠彼此傳輸數據。
所有cpu內存都被認爲是連貫的，但是x86以外的架構的內存順序都很弱。
Vulkan標準的兩個需要理解的概念是內存available和內存visible。這是對GPU不連貫緩存的一種抽象。
注：vulkan有一個正式的內存模型，但是開發者不需要詳細了解就可以正確使用。

#### The L2 cache/ main memory
和所有L1 cache和外部DDR memory相連， GPU DDR memory 和CPUmemory controller通過某種方式相連(PCI-e或UMA)。
當L2 cache是包含了最新的數據時，認爲memoryu是available的，因爲L1 caches能夠拉取最新數據。

#### Incoherent L1 caches


