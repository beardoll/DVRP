#include "Timer.h"
#include "Dispatcher.h"
#include<cassert>
#include<algorithm>
#include "../run/Config.h"

bool ascendSortEvent(const EventElement &a, const EventElement &b){  // 递增排序
    return a.time < b.time;
}

EventElement Timer::pop(){  
    // 将事件推出
    assert(eventList.size() > 0);
    vector<EventElement>::iterator iter = eventList.begin(); 
    vector<EventElement>::iterator iter2;
    EventElement newElement;
    if(iter->carIndex == -1 && eventList.size() > 1) {
        // carIndex为-1表示"newTimeSlot"事件
        // 下面检查紧随其的事件是否也在同一事件发生，若是,
        // 则其他事件具有更高优先级（其他事件优先级相同）
        iter2 = iter + 1;
        if(iter2->time == iter->time) {
            newElement = *iter2;
            eventList.erase(iter2);
        } 
        else {
            newElement = *iter;
            eventList.erase(iter);
        }
    } 
    else {
        newElement = *iter;
        eventList.erase(iter);
    }
    return newElement;
}

Timer::Timer(vector<Customer*> staticCustomerSet, vector<Customer*> dynamicCustomerSet, 
        float capacity, Customer depot) {   
    // 构造函数，输入参数为所有顾客，以及各时间段开始值
    this->staticCustomerSet = staticCustomerSet;
    this->dynamicCustomerSet = dynamicCustomerSet;
    this->capacity = capacity;
    this->depot = depot;
    EventElement newEvent;
    int i;
    float timeSlotLen = REPLAN_END_TIME / TIME_SLOT_NUM / SPLIT;
    for(i=0; i<=TIME_SLOT_NUM*SPLIT; i++) {  
        // 增加“时间段到达”事件
        newEvent = EventElement(i*timeSlotLen, newTimeSlot, -1, -1);
        eventList.push_back(newEvent);
    }
    vector<Customer*>::iterator iter = dynamicCustomerSet.begin();
    for(iter; iter< dynamicCustomerSet.end(); iter++) {  
        // 增加“新顾客到达事件”
        newEvent = EventElement((*iter)->startTime, newCustomer, -1, (*iter)->id);
        eventList.push_back(newEvent);
    }
    sort(eventList.begin(), eventList.end(), ascendSortEvent);
}

void Timer::addEventElement(EventElement &newEvent){  
    // 往事件表中增加事件
    eventList.push_back(newEvent);
    sort(eventList.begin(), eventList.end(), ascendSortEvent);
}

void Timer::deleteEventElement(int carIndex){     
    // 删除与carIndex相关的事件
    // "carIndex=-1"表示"newTimeSlot"事件，不允许被删除
    if(carIndex == -1) return;
    vector<EventElement>::iterator iter = eventList.begin();
    for(iter; iter<eventList.end(); ){
        if((*iter).carIndex == carIndex) {
            EventElement *p = &(*iter);
            delete(p);
            iter = eventList.erase(iter);
            break;
        } else {
            iter++;
        }
    }
}

void searchCustomer(int customerId, vector<Customer*> customerSet, Customer &customer) {
    // 根据customerId在customerSet中寻找顾客
    bool mark = false;
    vector<Customer*>::iterator custIter;
    for(custIter = customerSet.begin(); custIter < customerSet.end(); custIter++) {
        if((*custIter)->id == customerId) {
            mark = true;
            customer = **custIter;
            break;
        }
    }
    if (mark == false) {    
        throw out_of_range("Cannot search the customer, it may be missed!");    
    }
}

void Timer::updateEventElement(EventElement &newEvent){  
    // 更新事件
    vector<EventElement>::iterator iter = eventList.begin();
    if(newEvent.time == -1) {
        // 传入无效事件，不更新
        return;	
    } else {
        bool found = false;
        for(iter; iter<eventList.end(); iter++){
            if((*iter).carIndex == newEvent.carIndex) {
                // 判断更新事件的位置，并更新之
                // 注意对于每一辆车，在事件表中仅存放一个事件
                found = true;
                (*iter).time = newEvent.time;
                (*iter).eventType = newEvent.eventType;
                (*iter).customerId = newEvent.customerId;
            }
        }
        if(found == false) {
            // 找不到是因为货车在此之前没有将时间表录入
            // 这种情况发生在货车在仓库等待了一段时间后才收到服务任务
            addEventElement(newEvent);
        }
    }
}

// enum EventType{newCustomer, carArrived, finishedService, carDepature, newTimeSlot, carOffWork};

void Timer::run(vector<Car*> &finishedPlan, vector<Customer*> &rejectCustomer, 
        float &travelDistance, float &addAveDistance) {
    // 调度中心初始化
    Dispatcher disp(staticCustomerSet, dynamicCustomerSet, depot, capacity);
    int slotIndex = 0;  // 第0个时间段
    while(eventList.size() != 0) {
        EventElement currentEvent = pop();  // 弹出最近事件
        switch(currentEvent.eventType) {
            case newCustomer: {   // 新顾客到达
                Customer cust;
                try {
                    searchCustomer(currentEvent.customerId, dynamicCustomerSet, cust);
                }
                catch (exception &e) {
                    throw out_of_range("In event \"newCustomer\": ");
                }
                EventElement newEvent = disp.handleNewCustomer(slotIndex, cust);
                updateEventElement(newEvent);
                break;
            }
            case carArrived: {
                EventElement newEvent = disp.handleCarArrived(currentEvent.time, 
                        currentEvent.carIndex);
                updateEventElement(newEvent);
                break;
            }
            case finishedService: {
                EventElement newEvent = disp.handleFinishedService(currentEvent.time, 
                        currentEvent.carIndex);
                updateEventElement(newEvent);
                break;
            }
            case newTimeSlot: {
                vector<EventElement> newEventList = disp.handleNewTimeSlot(slotIndex);
                vector<EventElement>::iterator eventIter;
                for(eventIter = newEventList.begin(); eventIter < newEventList.end(); eventIter++) {
                    // 逐个加入事件列表中
                    updateEventElement(*eventIter);
                }
                slotIndex++;
                break;
            }
            case carOffWork: {
                // do nothing now
                break;
            }
        }
    }
    vector<Customer*> allCustomer;
    vector<Customer*>::iterator custIter;
    for(custIter = staticCustomerSet.begin(); custIter < staticCustomerSet.end(); custIter++) {
        allCustomer.push_back(*custIter);
    }
    for(custIter = dynamicCustomerSet.begin(); custIter < dynamicCustomerSet.end(); custIter++) {
        allCustomer.push_back(*custIter);
    }
    sort(allCustomer.begin(), allCustomer.end());
    vector<int> rejectCustomerId = disp.getRejectCustomerId();
    vector<int>::iterator intIter;
    for(intIter = rejectCustomerId.begin(); intIter < rejectCustomerId.end(); intIter++) {
        rejectCustomer.push_back(allCustomer[*intIter-1]);
    }
    finishedPlan = disp.getFinishedPlan();
    vector<Car*>::iterator carIter;
    travelDistance = 0;
    addAveDistance = 0;
    for(carIter = finishedPlan.begin(); carIter < finishedPlan.end(); carIter++) {
        travelDistance += (*carIter)->getTravelDistance();
        addAveDistance += (*carIter)->getAddDistance();
    }
    // 为服务动态顾客所额外增加的平均路长
    addAveDistance /= (dynamicCustomerSet.size() - rejectCustomer.size());
    disp.destroy();
}
