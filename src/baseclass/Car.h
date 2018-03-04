#ifndef _CAR_H
#define _CAR_H
#include "Route.h"
#include "../modules/EventElement.h"
#include "Matrix.h"
#include<vector>

enum State{wait, depature, serving, offwork};

class Car{
public:
    Car(Spot &headNode, Spot &rearNode, float capacity, int index, 
            bool artificial = false);  // 构造函数
    Car(Route &route, int index, bool artificial=false);  // 构造函数
    ~Car();          // 析构函数
    Car(const Car& item);  //复制构造函数
    Car& operator= (Car &item);       // 重载赋值操作

    // 获取货车属性
    int getCarIndex() {return carIndex;}  // 得到车辆编号
    bool judgeArtificial() {return artificial;} // 返回车辆性质
    Route* getRoute(){ return &route;}      // 得到本车路径
    float getCapacity() {return route.getCapacity();}    // 返回车容量
    vector<Spot*> getAllCustomer() { return route.getAllCustomer();}
    int getCustomerNum(){ return route.getSize();}       // 获取(P-D)对的数目
    Car* getNullCar(vector<Spot*> &removedCustomer);  // 将所有的顾客删除掉，返回一辆空车
    float getTravelDistance() { return travelDistance; } // 获取货车走过的总路长
    float getAddDistance() {
        // 获取货车为服务动态顾客的附加路长
        return travelDistance - route.getOriginLen(); 
    }
    vector<int> getAllID();   // 获取货车内所有节点的ID，按顺序

    // 更改货车属性
    void changeCarIndex(int newIndex) {carIndex = newIndex;}    // 更改车辆编号
    void setProperty(bool newProperty) { artificial = newProperty; } // 设置货车的新属性

    // 计算insert cost和remove cost
    void computeInsertCost(Spot *store, Spot *customer, float &minValue,
            Spot *refStore1, Spot *refCustomer1, float &secondValue, Spot *refStore2,
            Spot *refCustomer2, float randomNoise=0, bool allowNegativeCost=false)
    vector<float> computeReducedCost(float DTpara[]);  // 计算所有节点的移除代价

    // getCustomer方法
    Spot* getHeadNode(){return route.getHeadNode();}    // 得到车辆的头结点
    Spot* getRearNode(){return route.getRearNode();}    // 得到车辆的尾节点
    Spot* getCurrentNode() {return route.currentPos();} // 得到current指针指向的节点

    // insert 和 delete Customer方法
    void insertAtRear(Spot *item);   // 在路径的尾部插入节点
    void insertAtHead(Spot *store, Spot *customer);     // 在路径头部插入节点
    void insertAfter(Spot *ref, Spot *current);   //  在ref后插入current
    void insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer);
    void deleteCustomer(Spot *item); // 删除节点
    void deleteCustomer(Spot *store, Spot *customer);

    // part Route操作
    // 将newCar的路径插入到当前货车路径的current节点之后
    void replaceRoute(Car *newCar, float currentTime);      	    
    Car* capturePartRoute(float time);    // 抓取route的current指针之后的路径，并且返回一辆车

    // state相关
    void updateState(float time);        // 更新状态
    EventElement getCurrentAction(float time);        // 获得货车当前时刻的动作
    State getState(){ return state;}     // 返回货车当前的状态
    EventElement launchCar(float currentTime);         // 启动货车，当货车处于wait状态时有效

    // assessment相关
    void removeInvalidCustomer(vector<int> validCustomerId, int &retainNum);    // 移除路径中的无效顾客
    void updateTransformMatrix(Matrix<int> &transformMatrix);
    int computeScore(Matrix<int> transformMatrix);
private:
    State state;    // 货车状态（服务？行进？等待？）
    Route route;    // 计划要走的路径
    float nearestDepatureTime;   // 最新的出发时间
    float nextArriveTime;        // 下一站的到达时间
    bool artificial;  // 为true表示是虚构的车辆，false表示真实的车辆
    int carIndex;     // 货车编号
    float travelDistance;   // 车辆行驶的总里程
    vector<int> posVec;   // 当移除了无效节点后，记录路径中节点在validCustomerId的位置 
};

#endif
