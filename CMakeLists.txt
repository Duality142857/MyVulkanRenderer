cmake_minimum_required(VERSION 3.20)
project(MyVulkan VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(glfw-3.3.6\\include)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(Vulkan_BIN "C:\\VulkanSDK\\1.3.211.0\\Bin")
    set(Vulkan_SDK "C:\\VulkanSDK\\1.3.211.0")
    set(Vulkan_LIBRARY "C:\\VulkanSDK\\1.3.211.0\\Lib\\vulkan-1.lib")
    set(Vulkan_INCLUDE_PATH "C:\\VulkanSDK\\1.3.211.0\\Include")
    find_package(Vulkan REQUIRED)
    
    include_directories(C:\\VulkanSDK\\1.3.211.0\\Include)

    add_subdirectory(glfw-3.3.6)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    set(IMGUI_DIR D:/approot/imgui)
    include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
    include_directories(D:/approot/imgui)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STB_INCLUDE_PATH /home/number/codes/learnvulkan/include)
    set(TINYOBJ_INCLUDE_PATH /home/number/libraries/tinyobjloader)
    set(GLFW_DIR /home/number/tools/glfw)
    set(IMGUI_DIR /home/number/approot/ImGuiApps/imgui/)
    message("set linux paths")
endif()
include_directories(/home/number/approot/ImGuiApps/imgui/)

message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
include_directories(ext)
include_directories(/home/number/approot/opensource/ImGuizmo)

add_library(imguithings STATIC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp /home/number/approot/opensource/ImGuizmo/ImGuizmo.cpp)


function(buildApp appname appsource)
    add_executable(${appname} ${appsource} src/my_buffer.cpp src/my_descriptors.cpp src/my_device.cpp src/my_pipeline.cpp src/my_renderer.cpp src/my_swapchain.cpp src/my_texture.cpp)

    target_precompile_headers(${appname} PRIVATE my_pch.h)

    target_link_libraries(${appname} imguithings)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${appname}
    -lglfw3
    -lvulkan
    -ldl
    -lGL
    -lrt
    -lm
    -lXrandr
    -lXi
    -lX11
    -lpthread
    -lXinerama
    -lXcursor
    )
    else()
    message("windows")
    target_link_libraries(appname glfw Vulkan::Vulkan)
endif()
endfunction(buildApp)

buildApp(instancing instancing.cpp)
buildApp(shadowmap shadowmap.cpp)
buildApp(csTest csTest.cpp)