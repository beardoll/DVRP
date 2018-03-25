#include "Car.h"
#include "../public/PublicFunction.h"
#include "../run/Config.h"
#include<cmath>
#include <stdexcept>

Car::Car(Spot &headNode, Spot &rearNode, float capacity, int index, bool artificial):
    carIndex(index), route(headNode, rearNode, capacity), artificial(artificial)
{
    state = wait;
    nearestDepartureTime = 0;
    nextArriveTime = 0;
    travelDistance = 0;
}

Car::Car(Route &route, int index, bool artificial): carIndex(index), 
    artificial(artificial), route(route) {
}

Car::~Car(){  
    // 内嵌对象的析构函数会被调用，不用在此处delete route
}

Car::Car(const Car& item):route(item.route), artificial(item.artificial){  
    // 复制构造函数
    this->carIndex = item.carIndex;
}


Car& Car::operator= (Car &item){ 
    // 重载赋值操作
    this->route = item.route;
    this->carIndex = item.carIndex;
    this->artificial = item.artificial;
    return *this;
}

vector<int> Car::getAllID(){
    // 获得所有节点的ID，按顺序
    return route.getAllID();
}


//================ insert cost和remove cost =================//
void Car::computeInsertCost(Spot *store, Spot *customer, float &minValue, 
        Spot *&refStore1, Spot *&refCustomer1, float &secondValue, Spot *&refStore2,
        Spot *&refCustomer2, float randomNoise, bool allowNegativeCost){
        
    // 计算服务对(store, customer)在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点(refStore, refCustomer)
    // Args:
    //   * randomNoise: 随机化的噪声量
    //   * allowNegativeCost: 是否允许出现负的insertion cost
    // Returns:
    //   * refStore1, refCustomer1: 若minValue=MAX_FLOAT，则二者均为NULL
    //   * refStorer2, refCustomer2: 若secondValue=MAX_FLOAT，则二者均为NULL
    route.computeInsertCost(store, customer, minValue, refStore1, refCustomer1,
            secondValue, refStore2, refCustomer2, randomNoise, allowNegativeCost);
}

vector<float> Car::computeReducedCost(float DTpara[]){  
    // 得到路径中所有服务对(P-D)的移除代价，值越小表示它可以节省更多的代价
    // Args:
    //   * DTpara: 不同优先级顾客计算reduce cost时的bonus
    return route.computeReducedCost(DTpara, artificial);
}


//================ insert 和 delete Customer方法 =================//
void Car::insertAtRear(Spot *item){
    // 一般是用在往空车里面塞节点，因此不重新计算nextArriveTime
    try {
        route.insertAtRear(item);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
} 

void Car::insertAtHead(Spot *store, Spot *customer){
    // 只能当车子为空时才可使用，也不重新计算nextArriveTime
    try {
        route.insertAtHead(store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
}

void Car::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, 
        Spot *customer) {
    // 在refStore后面插入store，在refCustomer后面插入customer
    // update: 是否需要更新stand节点
    try {
        route.insertAfter(refStore, refCustomer, store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
    // 更新nextArriveTime，不管下一站目的地是否改变
    Spot *lastStop = route.getStand();  // 货车最近驻点（上一个访问的节点）
    // 当前时间 = 从lastStop出发的时间 + dist(lastStop, refStore)
    float currentTime = lastStop->arrivedTime + lastStop->serviceTime;
    Spot *nextStop = route.nextPos();
    nextArriveTime = currentTime + dist(lastStop, nextStop);
    // 更新nearestDepartureTime
    if(state == departure) {
        nearestDepartureTime = currentTime;
    }
}

void Car::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store,
        Spot *customer, float time) {
    // 需要更新stand节点的版本
    updateState(time);  
    try {
        route.insertAfter(refStore, refCustomer, store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
    // 更新nextArriveTime，不管下一站目的地是否改变
    Spot *lastStop = route.getStand();  // 货车最近驻点（上一个访问的节点）
    // 当前时间 = 从lastStop出发的时间 + dist(lastStop, refStore)
    float currentTime = lastStop->arrivedTime + lastStop->serviceTime;
    Spot *nextStop = route.nextPos();
    nextArriveTime = currentTime + dist(lastStop, nextStop);
    // 更新nearestDepartureTime
    if(state == departure) {
        nearestDepartureTime = currentTime;
    }
}

void Car::deleteCustomer(Spot *store, Spot *customer) {
    // 删除服务对(store, customer)
    try {
        route.deleteNode(store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
}
