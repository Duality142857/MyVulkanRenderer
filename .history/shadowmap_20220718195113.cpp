#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


void test()
{
    glm::perspective(45,1,-1,-50);
}

// #include<GLFW/glfw3.h>
// #include<iostream>
#include"my_pch.h"
#include"my_window.h"
#include"my_device.h"
#include"my_swapchain.h"
#include"my_buffer.h"
#include"my_pipeline.h"
#include"my_renderer.h"
#include"my_descriptors.h"
#include"my_texture.h"
#include"my_gui.h"
#include"my_sphere.h"
#include"my_event.h"
#include"my_inputsystem.h"
#include"my_scene.h"
#include"descriptor.h"

// #define WITHLOG
static inline void log(const std::string& msg)
{
#ifdef WITHLOG
    std::cout<<msg<<std::endl;
#endif
}
static constexpr int descriptorSetFrames=5;
std::vector<VkDescriptorSet> descriptorSets_scene(descriptorSetFrames);
std::vector<VkDescriptorSet> descriptorSets_offscreen(descriptorSetFrames);

struct UBO_scene
{
    alignas(16) MyGeo::Mat4f model;
    alignas(16) MyGeo::Mat4f view;
    alignas(16) MyGeo::Mat4f proj;
    alignas(16) MyGeo::Mat4f lightMVP;
    alignas(16) MyGeo::Vec3f lightPos;
    alignas(16) MyGeo::Vec3f lightColor;
    alignas(16) MyGeo::Vec3f eyePos;
    alignas(16) MyGeo::Vec3f ks;
    alignas(16) MyGeo::Vec3f kd;
};

struct UBO_offscreen
{
    MyGeo::Mat4f depthMVP;
    MyGeo::Mat4f sceneModel;
};






class Shadow
{
public:
    float fps=60.f;
    float frameInterval=1000.f/fps;
    int width=800, height=600;
    float t_init=0;
    float lightIntensity=10;
    float lightColor[3]={1,1,1};

    float depthBiasConstant=1.25f;
    float depthBiasSlope=1.75f;

    Dispatcher dispatcher;
    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    MyDescriptors mydescriptors{myswapChain};
    Descriptors descriptors{mydevice};

    VkDescriptorSetLayout descriptorSetLayout;


    MyPipeline scenePipeline{mydescriptors};
    MyPipeline shadowPipeline{mydescriptors};

    MyRenderer renderer{dispatcher,scenePipeline,shadowPipeline};

    MyTexture mytexture{mydevice,"../resources/MC003_Kozakura_Mari.png"};
    MyGui gui{myswapChain};

    std::vector<MyBuffer> uniformBuffers_scene;
    std::vector<MyBuffer> uniformBuffers_offscreen;

    std::vector<std::shared_ptr<MyModel>> mymodels;

    void addModel_Instance(int modelId, const MyModel& model)
    {
        mymodels.emplace_back(std::make_shared<InstanceModel>(model,MyGeo::translateMatrix({0,3,0})));
    }

    void addModel_Shape(int modelId, GeoShape shape, const std::initializer_list<float>& scales)
    {
        std::cout<<"modelid: "<<modelId<<std::endl;
        mymodels.emplace_back(std::make_shared<ShapeModel>(modelId,mydevice,myswapChain,shape,scales));
    }
    void addModel_Ext(int modelId, const std::string& filename)
    {
        mymodels.emplace_back(std::make_shared<ExtModel>(modelId,mydevice,myswapChain,filename));
    }
    void drawModels(VkCommandBuffer cmdBuffer)
    {
        for(auto& model:mymodels)
        {
            model->bind(cmdBuffer);
            model->draw(cmdBuffer,model->indices.size(),1);
        }
    }

    virtual void run()
    {
        createDescriptors();
        init();
        mainLoop();
        cleanup();
    }
    void createDescriptors()
    {
        //!create pool
        std::vector<VkDescriptorPoolSize> poolsizes;
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,100));
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,100));
        descriptors.createDescriptorPool(poolsizes,1000);

        //!create descriptorSetLayout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.push_back(descriptors.setLayoutBinding(0,1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(1,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(2,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        descriptors.createDescriptorSetLayout(descriptorSetLayout,layoutBindings);

        //!allocateDescriptorsets
        std::vector<VkDescriptorSetLayout> setLayouts(descriptorSetFrames,descriptorSetLayout);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_scene);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_offscreen);
    }

    virtual void init()
    {
        scenePipeline.createGraphicsPipeline("../shaders/shadowmapping/a.vert.spv","../shaders/shadowmapping/a.frag.spv");
        shadowPipeline.createdepthPipeline("../shaders/shadowmapping/offscreen.vert.spv",0);
        
        gui.init();
        createUniformBuffers();
        addModel_Ext(1,"../resources/Marry.obj");

        ShapeModel sphere{1,mydevice,myswapChain,GeoShape::Sphere,{1}};
        addModel_Instance(1,sphere);
        
        addModel_Shape(0,GeoShape::Sphere,{0.03f});
        // addModel_Shape(2, GeoShape::Sphere,{2});
        addModel_Shape(2,GeoShape::Rect,{15,15});
    }

    virtual void cleanup()
    {
        gui.cleanup();
        renderer.cleanup();
        for(auto& ub:uniformBuffers_scene) ub.clear();
        for(auto& ub:uniformBuffers_offscreen) ub.clear();
        mydescriptors.cleanup();
    }

    virtual void renderFrame() 
    {
        log("startframe");
        uint32_t imageIndex=renderer.startFrame();
        log("update framedata scene");
        updateFrameData_scene(imageIndex);
        log("update framedata offscreen");
        updateFrameData_offscreen(imageIndex);
        log("update descriptorsets");
        updateDescriptorSets(renderer.commandBuffers[renderer.currentFrame],imageIndex);
        log("updateUi");
        updateUi();
        log("record commands");
        recordCommand(renderer.commandBuffers[renderer.currentFrame],imageIndex);
        log("end frame");
        renderer.endFrame(imageIndex);
    }

    virtual void mainLoop()
    {
        int frameCount=0;
        float tcount=0;
        while(!glfwWindowShouldClose(mydevice.mywindow.window))
        {
            log("loop");
            auto t0=mytime::now();
            glfwPollEvents();
            renderFrame();
            auto t1=mytime::now();
            auto dt=mytime::getDuration<std::chrono::microseconds>(t0,t1);
            

            if(dt<frameInterval*1000)
            {
                gui.fps=fps;
                mytime::sleep(frameInterval-dt/1000);
                t_init+=frameInterval*0.001;
            }
            else 
            {
                gui.fps=1e6f/dt;
                t_init+=dt*1e-6;
            }
        }
        vkDeviceWaitIdle(mydevice.device);
    }

    
    virtual void recordCommand(VkCommandBuffer cmdBuffer, int imageIndex)
    {
        uint32_t setIndex=imageIndex % descriptorSetFrames;

        vkResetCommandBuffer(cmdBuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        // renderer.startRecord(cmdBuffer,myswapChain.renderPass,myswapChain.framebuffers[imageIndex],scenePipeline.pipeline);
        renderer.beginCommandBuffer(cmdBuffer);

        //offscreen cmds
        {
            // renderer.beginRenderPass(
            //     cmdBuffer,
            //     myswapChain.offscreenRenderpass,
            //     myswapChain.offscreenFrameBuffer,
            //     offscreenPipeline.pipeline);//pipeline bound

            VkRenderPassBeginInfo renderpassBeginInfo{};
            renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassBeginInfo.renderPass = myswapChain.offscreenRenderpass;
            renderpassBeginInfo.framebuffer = myswapChain.offscreenFrameBuffer;
            renderpassBeginInfo.renderArea.offset = {0, 0};
            renderpassBeginInfo.renderArea.extent = myswapChain.offscreenBufferExtent;

            std::array<VkClearValue, 1> clearValues{};
            // clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[0].depthStencil = {1.0f, 0};

            renderpassBeginInfo.clearValueCount = 1;
            renderpassBeginInfo.pClearValues = clearValues.data();  

            vkCmdBeginRenderPass(cmdBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.width=myswapChain.offscreenBufferExtent.width;
            viewport.height=myswapChain.offscreenBufferExtent.height;
            viewport.minDepth=0.f;
            viewport.maxDepth=1.f;
            vkCmdSetViewport(cmdBuffer,0,1,&viewport);

            VkRect2D rect{};
            rect.extent.width=myswapChain.offscreenBufferExtent.width;
            rect.extent.height=myswapChain.offscreenBufferExtent.height;
            rect.offset.x=0.f;
            rect.offset.y=0.f;
            vkCmdSetScissor(cmdBuffer,0,1,&rect);

            vkCmdSetDepthBias(cmdBuffer,depthBiasConstant,0.f,depthBiasSlope);  

            vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,shadowPipeline.pipelineLayout,0,1,&descriptorSets_offscreen[setIndex],0,nullptr);

            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipeline);   

            drawModels(cmdBuffer);
            vkCmdEndRenderPass(cmdBuffer);
        }

        {
            VkRenderPassBeginInfo renderpassBeginInfo{};
            renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassBeginInfo.renderPass = myswapChain.renderPass;
            renderpassBeginInfo.framebuffer = myswapChain.framebuffers[imageIndex];
            renderpassBeginInfo.renderArea.offset = {0, 0};
            renderpassBeginInfo.renderArea.extent = myswapChain.swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color={ { 0.025f, 0.025f, 0.025f, 1.0f } };
            clearValues[1].depthStencil={1.f,0};
            renderpassBeginInfo.clearValueCount=2;
            renderpassBeginInfo.pClearValues=clearValues.data();
            
            vkCmdBeginRenderPass(cmdBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.width=myswapChain.swapChainExtent.width;
            viewport.height=myswapChain.swapChainExtent.height;
            viewport.minDepth=0.f;
            viewport.maxDepth=1.f;
            vkCmdSetViewport(cmdBuffer,0,1,&viewport);

            VkRect2D rect{};
            rect.extent.width=myswapChain.swapChainExtent.width;
            rect.extent.height=myswapChain.swapChainExtent.height;
            rect.offset.x=0.f;
            rect.offset.y=0.f;
            vkCmdSetScissor(cmdBuffer,0,1,&rect);

            vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,scenePipeline.pipelineLayout,0,1,&descriptorSets_scene[setIndex],0,nullptr);

            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipeline.pipeline);   
            
            drawModels(cmdBuffer);
            ImGui_ImplVulkan_RenderDrawData(gui.drawData,cmdBuffer);
            vkCmdEndRenderPass(cmdBuffer);
        }
        if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
        // renderer.endRecord(cmdBuffer);
    }



    MyGeo::Vec3f pointRotate(const MyGeo::Vec3f& position, const MyGeo::Vec3f& center, const MyGeo::Vec3f& leftDragVec)
    {
        auto angle=10.f*leftDragVec.norm2()/(float)(myswapChain.swapChainExtent.width*myswapChain.swapChainExtent.height);
        auto leftDragVec_normalized=leftDragVec.normalVec();
        auto axis=leftDragVec_normalized.cross(MyGeo::Vec3f{0,0,1});
        auto res=rotationMat(axis,angle)(MyGeo::Vec4f{position-center,1.f}).head+center;
        return res;
    }

    MyGeo::Vec3f camPos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f eyePos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f lookat=MyGeo::Vec3f{0,2.5,0};
    MyGeo::Camera cam{camPos,lookat,{0,1,0}};
    MyGeo::Vec3f lightPos={0.4,3,5};//{0,7,3};

    UBO_scene ubo_scene{};
    UBO_offscreen ubo_offscreen{};
    MyGeo::Mat4f modelMat=MyGeo::Eye<float,4>();

    MyGeo::Camera lightCam{lightPos,{0,0,0},{0,1,0}};

    void updateFrameData_offscreen(uint32_t currentImage)
    {
        lightCam.position=lightPos;
        lightCam.lookdirection=(lightCam.lookat-lightCam.position).normalVec();
        lightCam.setNearFar(-0.1,-50);
        lightCam.setFov(120,1);
        lightCam.updateMat();
        
        //model= identity
        ubo_offscreen.depthMVP=lightCam.projMat*lightCam.viewMat;
        ubo_offscreen.sceneModel=ubo_scene.model;
        ubo_scene.lightMVP=ubo_offscreen.depthMVP;

        void* data;
        vkMapMemory(mydevice.device, uniformBuffers_offscreen[currentImage].memory, 0, sizeof(ubo_offscreen), 0, &data);
        memcpy(data, &ubo_offscreen, sizeof(ubo_offscreen));
        vkUnmapMemory(mydevice.device, uniformBuffers_offscreen[currentImage].memory);

        vkMapMemory(mydevice.device, uniformBuffers_scene[currentImage].memory, 0, sizeof(ubo_scene), 0, &data);
        memcpy(data, &ubo_scene, sizeof(ubo_scene));
        vkUnmapMemory(mydevice.device, uniformBuffers_scene[currentImage].memory);
    }

    virtual void updateFrameData_scene(uint32_t currentImage) 
    {
        mywindow.tick();
        // ubo_scene.ks=mymodels[0]->ks;
        // ubo_scene.kd=mymodels[0]->kd;
        ubo_scene.ks={0.2,0.2,0.2};
        ubo_scene.kd={0.8,0.8,0.8};
        
        static auto startTime=mytime::now();
        auto currentTime = mytime::now();
        float time_elapsed=mytime::getDuration<std::chrono::milliseconds>(startTime,currentTime)*0.0001f;

        if(!ImGuizmo::IsUsing() &&!gui.anyWindowFocused() && mydevice.mywindow.leftmousePressed && mywindow.dragAngle>0.1f) 
        {//
            // std::cout<<"dragangle: "<<mywindow.dragAngle<<std::endl;
            cam.position=rotationMat(mywindow.dragAxis,mywindow.dragAngle)(MyGeo::Vec4f({cam.position-cam.lookat,1.f})).head+cam.lookat;
            cam.lookdirection=(cam.lookat-cam.position).normalVec();
            mywindow.dragAngle=0;
        }

static MyGeo::Vec3f xaxis,yaxis,zaxis;
        zaxis=(cam.position-cam.lookat).normalVec();
        yaxis={0,1,0};
        xaxis=yaxis.cross(zaxis).normalVec();
        yaxis=zaxis.cross(xaxis);

        if(false && !ImGuizmo::IsUsing() && !gui.anyWindowFocused() && mydevice.mywindow.rightmousePressed && mydevice.mywindow.rightDragVec!=MyGeo::Vec2f{0,0}) 
        {
            MyGeo::Vec3f rightDragVec={mydevice.mywindow.rightDragVec,0};
            camPos+=0.2*(xaxis*(rightDragVec.x/(float)myswapChain.swapChainExtent.width)-yaxis*(rightDragVec.y/(float)myswapChain.swapChainExtent.height));
            lookat+=0.2*(xaxis*(rightDragVec.x/(float)myswapChain.swapChainExtent.width)-yaxis*(rightDragVec.y/(float)myswapChain.swapChainExtent.height));
            cam.lookat=lookat;
            cam.position=camPos;
        }

        cam.setNearFar(-0.1,-50);

        float fov=40-2*mydevice.mywindow.mousescrollVal;
        cam.setFov(fov, myswapChain.swapChainExtent.width / (float)  myswapChain.swapChainExtent.height);
        cam.updateMat();
        ubo_scene.model=modelMat;
        ubo_scene.view=cam.viewMat;
        ubo_scene.proj=cam.projMat;
        ubo_scene.lightColor={lightIntensity*lightColor[0],lightIntensity*lightColor[1],lightIntensity*lightColor[2]};
        lightPos={std::sin(time_elapsed*12)*2,std::cos(time_elapsed*8)*3+5.f,std::cos(time_elapsed*12)*2};
        ubo_scene.lightPos=lightPos;
        ubo_scene.eyePos=cam.position;
        void* data;
        // vkMapMemory(mydevice.device, uniformBuffers_scene[currentImage].memory, 0, sizeof(ubo_scene), 0, &data);
        // memcpy(data, &ubo_scene, sizeof(ubo_scene));
        // vkUnmapMemory(mydevice.device, uniformBuffers_scene[currentImage].memory);
    }
private:
    virtual void updateDescriptorSets(VkCommandBuffer cmdBuffer, uint32_t imageIndex)
    {   
        uint32_t setIndex=imageIndex % descriptorSetFrames;

        //这些info中包含了具体的buffer的信息，比如buffer句柄，descriptor大小和在buffer中的offset，如果buffer只对应一个descriptor则offset为0
        //将这些info传给VkWriteDescriptorSet结构体作为参数，和descriptorset联系起来
        VkDescriptorBufferInfo uniformBufferInfo_scene=descriptors.bufferInfo(uniformBuffers_scene[imageIndex],sizeof(UBO_scene));

        VkDescriptorBufferInfo uniformBufferInfo_offscreen=descriptors.bufferInfo(uniformBuffers_offscreen[imageIndex],sizeof(UBO_offscreen));

        VkDescriptorImageInfo imageInfo=descriptors.imageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,mytexture.textureImageView,mytexture.textureSampler);

        VkDescriptorImageInfo offscreenDepthImageInfo=descriptors.imageInfo(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,myswapChain.offscreenImageView,myswapChain.offscreenImageSampler);

        
        std::array<VkWriteDescriptorSet,3> descriptorWrites_scene{};
        descriptorWrites_scene[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[0].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[0].dstBinding = 0;
        descriptorWrites_scene[0].dstArrayElement = 0;
        descriptorWrites_scene[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites_scene[0].descriptorCount = 1;
        descriptorWrites_scene[0].pBufferInfo = &uniformBufferInfo_scene;

        descriptorWrites_scene[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[1].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[1].dstBinding = 1;
        descriptorWrites_scene[1].dstArrayElement = 0;
        descriptorWrites_scene[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites_scene[1].descriptorCount = 1;
        descriptorWrites_scene[1].pImageInfo = &imageInfo;

        descriptorWrites_scene[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[2].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[2].dstBinding = 2;
        descriptorWrites_scene[2].dstArrayElement = 0;
        descriptorWrites_scene[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites_scene[2].descriptorCount = 1;
        descriptorWrites_scene[2].pImageInfo = &offscreenDepthImageInfo;

        vkUpdateDescriptorSets(mydevice.device,descriptorWrites_scene.size(),descriptorWrites_scene.data(),0,nullptr);

        std::array<VkWriteDescriptorSet,1> descriptorWrites_offscreen{};
        descriptorWrites_offscreen[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_offscreen[0].dstSet = descriptorSets_offscreen[setIndex];
        descriptorWrites_offscreen[0].dstBinding = 0;
        descriptorWrites_offscreen[0].dstArrayElement = 0;
        descriptorWrites_offscreen[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites_offscreen[0].descriptorCount = 1;
        descriptorWrites_offscreen[0].pBufferInfo = &uniformBufferInfo_offscreen;
        vkUpdateDescriptorSets(mydevice.device,descriptorWrites_offscreen.size(),descriptorWrites_offscreen.data(),0,nullptr);
    }

    virtual void createUniformBuffers() 
    {
        VkDeviceSize bufferSize_scene = sizeof(UBO_scene);
        VkDeviceSize bufferSize_offscreen=sizeof(UBO_offscreen);

        for(int i=0;i!=myswapChain.images.size();++i) 
        {
            uniformBuffers_scene.push_back({mydevice});
            uniformBuffers_offscreen.push_back({mydevice});

        }
        for (size_t i = 0; i < myswapChain.images.size(); i++) 
        {
            mydevice.createBuffer(bufferSize_scene, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_scene[i].buffer, uniformBuffers_scene[i].memory);

            mydevice.createBuffer(bufferSize_offscreen, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_offscreen[i].buffer, uniformBuffers_offscreen[i].memory);
        }
    }


    virtual void updateUi()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        ImGuiIO& io=ImGui::GetIO();
        bool t_open=false;
        ImGui::StyleColorsDark();
        ImGui::Begin("defaultTitle",nullptr,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("%.1f fps", gui.fps);

	    ImGui::PushItemWidth(110.0f);
        int itemIndex;

        if(ImGui::CollapsingHeader("Load",ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* items[]={"extModel","geoModel"};
            // ImGui::Combo("type",&itemIndex,items,2,2);
            static const char* currentItem=items[0];
            if(ImGui::BeginCombo("Load Type",currentItem,ImGuiComboFlags_PopupAlignLeft))
            {
                bool selected=false;
                for(int i=0;i!=IM_ARRAYSIZE(items);++i)
                {
                    if(ImGui::Selectable(items[i],&selected))
                    {
                        currentItem=items[i];
                    }
                }
                ImGui::EndCombo();

            }

        }
        
        
        ImGui::PopItemWidth();
        ImGui::ColorEdit3("Light Color", lightColor, ImGuiColorEditFlags_NoAlpha);
        ImGui::SliderFloat("LightIntensity", &lightIntensity, 2.0f, 20.0f);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
        
        int width,height;
        glfwGetFramebufferSize(mydevice.mywindow.window,&width,&height);
        
        ImGuizmo::SetRect(0.f,0.f,(float)width,(float)height);
        auto view=cam.viewMat.col[0].data.data();
        auto proj=(cam.projMat*MyGeo::scaleMatrix({1,-1,1})).col[0].data.data();

        ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::TRANSLATE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());
        ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::ROTATE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());

        ImGui::End();
        ImGui::Render();
        gui.drawData=ImGui::GetDrawData();

    }


};





int main()
{
    Shadow shadowapp;
    shadowapp.run();
    std::cout<<std::endl;
}