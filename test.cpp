#include<vector>
#include<map>
#include<functional>
#include<iostream>
enum EventType{Event_A=1, Event_B=2};


//事件基类
class Event
{
public:
    virtual EventType type() const =0;
};

class Event1: public Event
{
public:
    static constexpr EventType etype=Event_A;
    EventType type() const override
    {
        return Event_A;
    }
};

class Event2: public Event
{
public:
    static constexpr EventType etype=Event_B;
    EventType type() const override
    {
        return Event_B;
    }
};

//分发器
class Dispatcher
{
public:
    std::map<EventType,std::vector<std::function<void(const Event& e)>>> observers;

    void subscribe(EventType etype, std::function<void(const Event&)> obs)
    {
        observers[etype].push_back(obs);
    }
    void dispatch(const Event& e)
    {
        if(observers.find(e.type())==observers.end()) return;
        for(const auto& obs: observers[e.type()])
        {
            obs(e);
        }
    }
};


void onEvent(const Event& e)
{
    std::cout<<"event: "<<e.type()<<std::endl;
}

int main()
{
    Dispatcher dispatcher;
    Event1 e1;
    Event2 e2;
    dispatcher.subscribe(Event1::etype, onEvent);
    dispatcher.dispatch(e1);
    dispatcher.dispatch(e2);//未注册Event2，因此没有触发观察者
}
//result:
//event: 1