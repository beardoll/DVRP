#ifndef _DISPATCHER_H
#define _DISPATHCER_H

#include "EventElement.h"
#include "../baseclass/Car.h"
#include<vector>

class Dispatcher{   // 调度中心类
public:
    Dispatcher(vector<Spot*> staticCustomerSet, vector<Spot*> dynamicCustomerSet, 
            vector<Spot*> storeSet, Spot depot, float capacity);
    ~Dispatcher(){}; // 析构函数
    void destroy();  // 销毁Dispatcher
    EventElement handleNewCustomer(int slotIndex, Spot* newCustomer);    // 处理新顾客(dynamic)到达
    EventElement handleCarArrived(float time, int carIndex);            // 处理货车到达事件
    EventElement handleFinishedService(float time, int carIndex);       // 处理货车完成服务事件
    // EventElement handleDepature(float time, int carIndex);              // 处理货车出发事件
    vector<EventElement> handleNewTimeSlot(int slotIndex); // 新时间段开始 
    void carFinishTask(int carIndex);       // 收车 
    vector<int> getRejectCustomerId() {return rejectCustomerId; }
    vector<Car*> getFinishedPlan() {return finishedPlan; }
private:
    vector<Spot*> allCustomer;    // 所有的顾客
    vector<Spot*> storeSet;
    vector<int> dynamicCustomerId;    // 动态到达的顾客的id
    vector<int> servedCustomerId;     // 已经服务过的顾客id
    vector<int> promisedCustomerId;   // (未服务过的)已经得到'OK' promise的顾客id
    vector<int> waitCustomerId;       // (未服务过的且已知的)还在等待'OK' promise的顾客id
    vector<int> rejectCustomerId;     // (未服务过的且已知的)得到了'NO' promise的顾客id
    vector<Car*> currentPlan;         // 当前计划
    vector<Car*> finishedPlan;        // 已完成计划    
    Spot depot;
    float capacity;
    int timeSlotLen;   // 时间段的长度
    int timeSlotNum;   // 时间段的个数
    int globalCarIndex;
    int samplingRate;  // 采样率
    float iter_percentage;
    int predictMethod;
};

#endif
