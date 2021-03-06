
// #define GLFW_INCLUDE_VULKAN
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION


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

// static void testtest(){std::cout<<"asdf"<<std::endl;}


struct UniformBufferObject
{
    alignas(16) MyGeo::Mat4f model;
    alignas(16) MyGeo::Mat4f view;
    alignas(16) MyGeo::Mat4f proj;
    alignas(16) MyGeo::Vec3f lightPos;
    alignas(16) MyGeo::Vec3f lightColor;
    alignas(16) MyGeo::Vec3f eyePos;
    alignas(16) MyGeo::Vec3f ks;
    alignas(16) MyGeo::Vec3f kd;
    alignas(16) float specFactor;
};



class App
{
public:
    float fps=60.f;
    float frameInterval=1000.f/fps;
    int width=800, height=600;
    float t_init=0;
    float lightIntensity=10;
    float lightColor[3]={1,1,1};
    int sphereNum=1;
    float scaleFactor=1.0;
    
    Dispatcher eventDispatcher;

    MyWindow mywindow{width,height};
    MyDevice mydevice{mywindow};
    MySwapChain myswapChain{mydevice};
    // DescriptorStats stats{1,1,4,myswapChain.images.size()};
    MyDescriptors mydescriptors{myswapChain};
    
    MyPipeline mypipeline{mydescriptors};
    MyRenderer renderer{eventDispatcher,mypipeline};
    MyTexture mytexture{mydevice,"../resources/MC003_Kozakura_Mari.png"};

    MyGui gui{myswapChain};
    std::vector<MyBuffer> uniformBuffers;

    // ExtModel extmodel{mydevice,myswapChain,"../resources/teapot.obj"};
    // ShapeModel rect{1,mydevice,myswapChain,GeoShape::Rect,{10.f,10.f}};
    // ShapeModel sphere{2,mydevice,myswapChain,GeoShape::Sphere,{3.f}};
    // ShapeModel sphereLight{0,mydevice,myswapChain,GeoShape::Sphere,{0.2f}};


    std::vector<std::shared_ptr<MyModel>> mymodels;

    App()
    {
        // eventDispatcher.subscribe(MouseDrag_Event,[this](const Event& e){});
    }

    void onMouseDrag()
    {
        
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


    // void bindModels(VkCommandBuffer cmdBuffer)
    // {
    //     for(auto& model:mymodels)
    //     {
    //         model->bind(cmdBuffer);
    //     }
    // }

    // template<int InstanceCount>
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
        init();
        mainLoop();
        cleanup();
    }

    virtual void init()
    {
        mypipeline.createGraphicsPipeline("../shaders/vert_a.spv","../shaders/frag_a.spv");
        
        gui.init();
        createUniformBuffers();
        
            // ShapeModel sphere{2,mydevice,myswapChain,GeoShape::Sphere,{3.f}};
    // ShapeModel sphereLight{0,mydevice,myswapChain,GeoShape::Sphere,{0.2f}};
        addModel_Shape(0,GeoShape::Sphere,{0.03f});
        addModel_Ext(1,"../resources/Marry.obj");

        addModel_Shape(2,GeoShape::Rect,{15,15});
        // addModel_Shape(1,GeoShape::Sphere,{3.f});
        // addModel_Ext(2,"../resources/teapot.obj");

    }


    virtual void cleanup()
    {
        gui.cleanup();
        renderer.cleanup();
        for(auto& ub:uniformBuffers) ub.clear();
        mydescriptors.cleanup();
    }

//At a high level, rendering a frame in Vulkan consists of a common set of steps:
// Wait for the previous frame to finish
// Acquire an image from the swap chain
// Record a command buffer which draws the scene onto that image
// Submit the recorded command buffer
// Present the swap chain image
    virtual void renderFrame() 
    {
        uint32_t imageIndex=renderer.startFrame();
        updateFrameData(imageIndex);
        updateUi();
        recordCommand(renderer.commandBuffers[renderer.currentFrame],imageIndex);
        renderer.endFrame(imageIndex);
    }
    virtual void mainLoop()
    {
        int frameCount=0;
        float tcount=0;
        while(!glfwWindowShouldClose(mydevice.mywindow.window))
        {
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
        vkResetCommandBuffer(cmdBuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        renderer.startRecord(cmdBuffer,myswapChain.renderPass,myswapChain.framebuffers[imageIndex],mypipeline.pipeline);
        // sphereLight.bind(cmdBuffer);
        // sphereLight.draw(cmdBuffer,sphereLight.indices.size(),1);
        // extmodel.bind(cmdBuffer);
        updateDescriptorSets(cmdBuffer,imageIndex);

        drawModels(cmdBuffer);
        
        // mymodels[0]->bind(cmdBuffer);
        // mymodels[0]->draw(cmdBuffer,mymodels[0]->indices.size(),1);
        // mymodels[1]->bind(cmdBuffer);
        // mymodels[1]->draw(cmdBuffer,mymodels[0]->indices.size(),sphereNum);
        // mymodels[2]->bind(cmdBuffer);
        // mymodels[2]->draw(cmdBuffer,mymodels[2]->indices.size(),(int)t_init);
        
        // drawModels<4>(cmdBuffer);
        
        // extmodel.draw(cmdBuffer,extmodel.indices.size());
        // rect.bind(cmdBuffer);
        // rect.draw(cmdBuffer,rect.indices.size());
        // sphere.bind(cmdBuffer);
        // sphere.draw(cmdBuffer,sphere.indices.size(),5);



        ImGui_ImplVulkan_RenderDrawData(gui.drawData,cmdBuffer);

        renderer.endRecord(cmdBuffer);
    }

    MyGeo::Vec3f pointRotate(const MyGeo::Vec3f& position, const MyGeo::Vec3f& center, const MyGeo::Vec3f& leftDragVec)
    {
        auto angle=10.f*leftDragVec.norm2()/(float)(myswapChain.swapChainExtent.width*myswapChain.swapChainExtent.height);
        auto leftDragVec_normalized=leftDragVec.normalVec();
        auto axis=leftDragVec_normalized.cross(MyGeo::Vec3f{0,0,1});
        auto res=rotationMat(axis,angle)(MyGeo::Vec4f{position-center,1.f}).head+center;
        return res;
    }
// var cameraPosition = [-20, 180, 250]; {0.4,3.5,5}

    MyGeo::Vec3f camPos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f eyePos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f lookat=MyGeo::Vec3f{0,2.5,0};
    MyGeo::Camera cam{camPos,lookat,{0,1,0}};

    UniformBufferObject ubo{};
    MyGeo::Mat4f modelMat=MyGeo::Eye<float,4>()*MyGeo::scaleMatrix({scaleFactor,scaleFactor,scaleFactor});//*MyGeo::scaleMatrix({std::sin(time_elapsed),std::cos(time_elapsed),0.5*(std::sin(time_elapsed)+std::cos(time_elapsed))});;

    virtual void updateFrameData(uint32_t currentImage) 
    {
        mywindow.tick();
        ubo.ks=mymodels[1]->ks;
        ubo.kd=mymodels[1]->kd;
        
        static auto startTime=mytime::now();
        auto currentTime = mytime::now();
        float time_elapsed=mytime::getDuration<std::chrono::milliseconds>(startTime,currentTime)*0.0001f;


        // if(!ImGuizmo::IsUsing() &&!gui.anyWindowFocused() && mydevice.mywindow.leftmousePressed && mydevice.mywindow.leftDragVec!=MyGeo::Vec2f{0,0}) 
        // {
        //     MyGeo::Vec3f leftDragVec={mydevice.mywindow.leftDragVec,0};
        //     camPos=pointRotate(camPos,lookat,leftDragVec);
        //     cam.position=camPos;
        // }
        if(!ImGuizmo::IsUsing() &&!gui.anyWindowFocused() && mydevice.mywindow.leftmousePressed) 
        {//
        //auto res=rotationMat(axis,angle)(MyGeo::Vec4f{position-center,1.f}).head+center;
            cam.position=rotationMat(mywindow.dragAxis,mywindow.dragAngle)(MyGeo::Vec4f({cam.position-cam.lookat,1.f})).head+cam.lookat;
            cam.lookdirection=(cam.lookat-cam.position).normalVec();
            mywindow.dragAngle=0;
            // cam.position=camPos;
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
            lookat=camPos;
            cam.lookat=lookat;
            cam.position=camPos;
        }

        cam.setNearFar(-0.1,-50);

        float fov=40-2*mydevice.mywindow.mousescrollVal;
        cam.setFov(fov, myswapChain.swapChainExtent.width / (float)  myswapChain.swapChainExtent.height);
        cam.updateMat();
        ubo.model=modelMat;
        ubo.view=cam.viewMat;
        ubo.proj=cam.projMat;
        ubo.lightColor={lightIntensity*lightColor[0],lightIntensity*lightColor[1],lightIntensity*lightColor[2]};
        ubo.lightPos={std::sin(time_elapsed*12)*2,std::cos(time_elapsed*8)*3,std::cos(time_elapsed*12)*2};
        ubo.eyePos=cam.position;
        ubo.specFactor=1;
        void* data;
        vkMapMemory(mydevice.device, uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(mydevice.device, uniformBuffers[currentImage].memory);
    }
private:


    virtual void updateDescriptorSets(VkCommandBuffer cmdBuffer, uint32_t imageIndex)
    {   
        uint32_t setIndex=imageIndex%mydescriptors.frameNum;

        VkDescriptorBufferInfo bufferInfo=mydescriptors.getBufferInfo(uniformBuffers[imageIndex],sizeof(UniformBufferObject));
        VkDescriptorImageInfo imageInfo=mydescriptors.getImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,mytexture.textureImageView,mytexture.textureSampler);
        
        std::array<VkWriteDescriptorSet,2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mydescriptors.descriptorSets.scene[setIndex];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mydescriptors.descriptorSets.scene[setIndex];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(mydevice.device,descriptorWrites.size(),descriptorWrites.data(),0,nullptr);

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mypipeline.pipelineLayout, 0, 1, &mydescriptors.descriptorSets.scene[setIndex], 0, nullptr);


    }

    virtual void createUniformBuffers() 
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        // uniformBuffers.resize(myswapChain.images.size());
        for(int i=0;i!=myswapChain.images.size();++i) uniformBuffers.push_back({mydevice});
        // uniformBuffersMemory.resize(swapChainImages.size());

        for (size_t i = 0; i < myswapChain.images.size(); i++) 
        {
            mydevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer, uniformBuffers[i].memory);
        }
    }

    virtual void updateUi()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        // ImGuizmo::IsOver();
        // ImGuizmo::IsUsing();

        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::ROTATE,ImGuizmo::MODE::WORLD,);
        // ImGuizmo::DrawCubes(view,proj,MyGeo::Eye<float,4>().col[0].data.data(),1);


        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        // ImGui::ShowDemoWindow();
        
        // auto mouseDraggingFlag=ImGui::IsMouseDragging(0, 200);
        // std::cout<<"mouse draged "<<mouseDraggingFlag<<std::endl;

        // auto mp=ImGui::GetMousePos();
        // std::cout<<"mouse pos: "<<mp.x<<','<<mp.y<<std::endl;

        // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        ImGuiIO& io=ImGui::GetIO();
        bool t_open=false;
        // ImGui::StyleColorsClassic();
        // ImGui::StyleColorsLight();
        ImGui::StyleColorsDark();
        // ImGui::Begin(title.c_str(),nullptr,ImGuiWindowFlags_MenuBar);
        ImGui::Begin("defaultTitle",nullptr,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("%.1f fps", fps);

	    ImGui::PushItemWidth(110.0f);
        int itemIndex;
        // bool ImGui::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)

        // addModel_Ext(0,"a");

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
        ImGui::SliderInt("SphereNum", &sphereNum,0,10);
        ImGui::SliderFloat("SphereScaleFactor", &scaleFactor, 0.1f, 5.0f);


        // const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
        // ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));
        // ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
        // ImGui::BeginChild("Scrolling");
        // for (int n = 0; n < 50; n++)
        //     ImGui::Text("%04d: Some text", n);
        // ImGui::EndChild();
        // ImGui::ProgressBar(0.66,{0,100},nullptr);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
        
        int width,height;
        glfwGetFramebufferSize(mydevice.mywindow.window,&width,&height);


        // ImGuizmo::SetRect(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y,windowWidth,windowHeight);
        
        ImGuizmo::SetRect(0.f,0.f,(float)width,(float)height);
        auto view=cam.viewMat.col[0].data.data();
        auto proj=(cam.projMat*MyGeo::scaleMatrix({1,-1,1})).col[0].data.data();

        ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::TRANSLATE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());// MyGeo::rotationMatrix({0,1,0},45).col[0].data.data());

        // if(ImGuizmo::IsUsing())
        // {
        //     // ubo.model=
        // }

        ImGui::End();
        ImGui::Render();
        gui.drawData=ImGui::GetDrawData();

    }
};


int main(int, char**) 
{
    App app;
    app.run();
    // std::cout<<"hello world!"<<std::endl;
}