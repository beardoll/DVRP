#ifndef _ROUTE_H
#define _ROUTE_H

#include<iostream>
#include<vector>
#include "Spot.h"

using namespace std;


class Route{
public:
    Route(Spot &headNode, Spot &rearNode, float capacity, float timeDuration); // 构造函数
    ~Route();   // 析构函数
    Route(const Route &L);  // 复制构造函数
    Route& operator= (const Route &L);  // 重载赋值运算符 
    Spot& operator[] (int k);     // 得到链表中的第k个元素
    const Spot& operator[] (int k) const; // 静态函数
    bool isEmpty();   // 判断链表是否为空
    void printRoute();   // 打印路径
    void clear();     // 清空链表

    // 插入以及删除节点操作
    void insertAfter(Spot *pre, Spot *current);    
    void insertAtRear(Spot *node);
    void deleteNode(Spot *node);

    // 获得单节点操作
    Spot* getHeadNode();    // 得到头结点
    Spot* getRearNode();    // 得到尾节点
    Spot* findCustomer(int id);

    // 获取链表属性
    float getQuantity() {return this->quantity;}            // 得到本车已使用的容量(装载顾客)
    float getCapacity() {return this->capacity;}            // 返回本车的车载量
    float getLen(float DTpara[], bool artificial = false);  // 得到路径长度(加惩罚)
	float getTrueLen();      // 得到真实路径长度(不加惩罚)
    int getSize();           // 得到当前链表的大小
    vector<Spot*> getAllCustomer();  // 得到路径中所有的顾客节点
    vector<int> getAllID();
    vector<float> getArrivedTime();
    float getTimeDuration();

    // 计算插入/删除节点代价
    // 计算所有节点的移除代价
    vector<float> computeReducedCost(float DTpara[], bool artificial = false);
    // 计算item节点在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点前面的顾客节点
    // penaltyPara为惩罚系数，若不需要惩罚则penaltyPara = 0
    void computeInsertCost(Spot *cur, float &minValue, Spot *&ref1, float &secondValue, Spot *&ref2, 
        float randomNoise = 0.0f, bool allowNegativeCost = true);  

    // 计算把item插入到pre后面是否会违反时间窗约束
    bool timeWindowJudge(Spot *ref, Spot *cur);
    // 其余辅助性质的函数
    bool checkTimeConstraint();
private:
    // 表头，表尾，当前指针和驻地
    // 当前指针指向前一次出发地（若未出发则为仓库）
    // 驻地指货车当前所在地
    Spot *head, *rear;
    int size;         // 链表的长度
    float timeDuration; // 车辆行驶时间最大值
    float quantity;   // 当前已使用的容量(与装载的顾客数量有关)
    float capacity;       // 车载量，在这里保存
    void copy(const Route& L);  // 复制链表，供复制构造函数和重载“=”函数使用
};

#endif
