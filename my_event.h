#pragma once
// #include<string>
// #include<functional>
// #include<map>
// #include<vector>
// #include<iostream>
// #include"my_pch.h"
// #include"test.h"


// #include<mygeo/geo.h>
// #include<mygeo/vec.h>



using MouseCode=uint16_t;

enum : MouseCode
{
    // From glfw3.h
    Button0                = 0,
    Button1                = 1,
    Button2                = 2,
    Button3                = 3,
    Button4                = 4,
    Button5                = 5,
    Button6                = 6,
    Button7                = 7,

    ButtonLast             = Button7,
    ButtonLeft             = Button0,
    ButtonRight            = Button1,
    ButtonMiddle           = Button2
};


enum EventType
{
    WindowResized_Event=0,MouseButton_Event,MouseDrag_Event
};



class Event
{
public:
    // using EventType=const char*;
    virtual EventType type() const =0;
};

// class TestEvent : public Event
// {
// public:
//     static constexpr EventType Type=TestEvent;
//     EventType type() const override { return Type;}
// };

class WindowResizedEvent: public Event
{
public:
    // WindowResizedEvent(int width, int height):width{width},height{height}
    // {}
    WindowResizedEvent(){}
    EventType type() const override { return Type;}
    static constexpr EventType Type=WindowResized_Event;

    int getWidth(){return width;}
    int getHeight(){return height;}
    int width, height;
};

class MouseButtonEvent: public Event 
{
public:
    int button=GLFW_MOUSE_BUTTON_LEFT;
    int action=GLFW_PRESS;
    MyGeo::Vec2f cursorPosition;

    static constexpr EventType Type=MouseButton_Event;
    EventType type() const override { return Type;}

    MouseButtonEvent(int _button,int _action, const MyGeo::Vec2f& cursorPosition):button{_button},action{_action}, cursorPosition{cursorPosition}{}
};

class MouseDragEvent: public Event 
{
public:
    int button=GLFW_MOUSE_BUTTON_LEFT;
    MyGeo::Vec2f dragVec;
    static constexpr EventType Type=MouseDrag_Event;
    EventType type() const override { return Type;}
    MouseDragEvent(int _button, const MyGeo::Vec2f& _dragVec):button{_button},dragVec{_dragVec}{}
};


class Dispatcher
{
public:
    using SlotType=std::function<void(const Event&)>;
    void subscribe(const EventType& etype , SlotType&& slot)
    {
        observers[etype].emplace_back(slot);
    }
    void dispatch(const Event& event)
    {
        if(observers.find(event.type())==observers.end()) return;
        auto&& valid_observers=observers[event.type()];
        for(auto&& observer:valid_observers)
        {
            observer(event);
        }
    }
private:
    std::map<EventType,std::vector<SlotType>> observers;

};



// class TestObserver
// {
// public:
//     TestObserver(int _data):data{_data}{}
//     int data;
//     void operator()(const Event& event)
//     {
//         if(event.type()==TestEvent::Type)
//         {
//             const TestEvent& testevent=static_cast<const TestEvent&>(event);
//             std::cout<<__PRETTY_FUNCTION__<<": "<<testevent.type()<<data<<std::endl;
//         }
//     }
// };


// int main()
// {
//     TestEvent te;
//     Dispatcher dispatcher;
//     TestObserver obs1{1},obs2{2};
//     dispatcher.subscribe(TestEvent::Type,obs1);
//     dispatcher.subscribe(TestEvent::Type,obs2);
//     dispatcher.dispatch(te);
// }




// enum EventType
// {
//     None=0,
//     MousePressed, MouseReleased, MouseScrolled, MouseMoved,
//     WindowClosed, WindowResized
// };
// class Event
// {
// public:
//     virtual EventType getEventType() const = 0;
//     virtual const char* getEventName() const =0;
// };
// #define EVENT_TYPE_FUNCTIONS(event)     \
// const char* getEventName() const override\
// {\
//     return #event;\
// }\
// EventType getEventType() const override \
// {\
//     return EventType::event;\
// }\




// class MousePressedEvent :Event 
// {
// public:
//     MousePressedEvent(const MouseCode button):button{button}{}
//     MouseCode button;
//     EVENT_TYPE_FUNCTIONS(MousePressed)

// };

// class MouseReleasedEvent :Event 
// {
// public:
//     MouseReleasedEvent(const MouseCode button):button{button}{}
//     MouseCode button;
//     EVENT_TYPE_FUNCTIONS(MouseReleased)
// };

// class MouseScrolledEvent :Event 
// {
// public:
//     MouseScrolledEvent(){}
//     EVENT_TYPE_FUNCTIONS(MouseScrolledEvent)
// };

// bool onEvent(const Event& e)
// {
//     EventType event;
//     if(e.getEventType())
// }





// //pch.h
// #include<vector>
// #include<array>
// #include<iostream>

// //main.cpp
// #include"pch.h"
// #include<vector>