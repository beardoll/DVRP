#ifndef _CAR_H
#define _CAR_H
#include "Route.h"
#include "Matrix.h"
#include<vector>

enum State{wait, departure, serving, offwork};

class Car{
public:
    Car(Spot &headNode, Spot &rearNode, int carIndex, int depotIndex, 
        bool artificial = false);  // 构造函数
    ~Car();                // 析构函数
    Car(const Car& item);  // 复制构造函数
    Car& operator= (Car &item);       // 重载赋值操作

    bool timeWindowJudge(Spot *ref, Spot *cur) {
        return route.timeWindowJudge(ref, cur);
    }

    // 获取货车属性
    int getCarIndex() { return carIndex; }  // 得到车辆编号
    int getDepotIndex() { return depotIndex; } // 得到来源仓库编号
    bool judgeArtificial() {return artificial;} // 返回车辆性质
    Route* getRoute(){ return &route;}      // 得到本车路径
    float getCapacity() {return route.getCapacity();}    // 返回车容量
    vector<Spot*> getAllCustomer() { return route.getAllCustomer();}
    int getCustomerNum(){ return route.getSize();}       // 获取顾客数目
    vector<int> getAllID();   // 获取货车内所有节点的ID，按顺序
    bool checkTimeConstraint() {return route.checkTimeConstraint(); }
    Spot* findCustomer(int id) { return route.findCustomer(id); }
	float getTrueLen() { return route.getTrueLen(); }
    float getDemand() { return route.getQuantity(); }
    float getTimeDuration() { return route.getTimeDuration(); }

    // 更改货车属性
    void changeCarIndex(int newIndex) {carIndex = newIndex;}    // 更改车辆编号
    void setProperty(bool newProperty) { artificial = newProperty; } // 设置货车的新属性

    // 计算insert cost和remove cost
    void computeInsertCost(Spot *cur, float &minValue, Spot *&ref1, float &secondValue, 
            Spot *&ref2, float randomNoise=0, bool allowNegativeCost=false);
    vector<float> computeReducedCost(float DTpara[]);  // 计算所有节点的移除代价

    // getCustomer方法
    Spot* getHeadNode(){return route.getHeadNode();}    // 得到车辆的头结点
    Spot* getRearNode(){return route.getRearNode();}    // 得到车辆的尾节点

    // insert 和 delete Customer方法
    void insertAtRear(Spot *item);   // 在路径的尾部插入节点
    void insertAfter(Spot *ref, Spot *cur);
    void deleteCustomer(Spot *node);
private:
    Route route;    // 计划要走的路径
    bool artificial;  // 为true表示是虚构的车辆，false表示真实的车辆
    int carIndex;     // 货车编号
    int depotIndex;   // 来源仓库的id
};

#endif
