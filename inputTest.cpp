#include"my_inputsystem.h"



int main()
{
    Window window(800,600);
    while(!glfwWindowShouldClose(window.window))
    {
        window.tick();
        auto t0=mytime::now();
        glfwPollEvents();

        auto t1=mytime::now();
        auto dt=mytime::getDuration<std::chrono::milliseconds>(t0,t1);
        mytime::sleep(50);
    }
}