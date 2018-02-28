#ifndef _ROUTE_H
#define _ROUTE_H

#include<iostream>
#include<vector>
#include "Spot.h"

using namespace std;


class Route{
public:
    Route(Spot &headNode, Spot &rearNode, float capacity);      // 构造函数
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
    void insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer); 
    void insertAtHead(Spot *store, Spot *customer);
    void insertAtRear(Spot *node);
    void deleteNode(Spot *node);
    void deleteNode(Spot *store, Spot *customer);

    // 获得单节点操作
    Spot* currentPos();   // 返回当前位置
    Spot* nextPos() {return *(current->next);}      // 下一个节点的位置
    Spot* pastPos() {return *(current->front);}     // 前一个节点的位置
    Spot* getHeadNode();    // 得到头结点
    Spot* getRearNode();    // 得到尾节点

    // 获取链表属性
    float getQuantity() {return this->quantity;}            // 得到本车已使用的容量(装载顾客)
    float getLeftQuantity() {return this->leftQuantity;}    // 得到本车剩余容量（与车辆行驶情况相关）
    float getCapacity() {return this->capacity;}            // 返回本车的车载量
    float getLen(float DTpara[], bool artificial = false);  // 得到路径长度
    float getOriginLen();    // 计算路径中单纯服务静态顾客节点所花费的路径代价
    int getSize();           // 得到当前链表的大小
    vector<float> getArrivedTime();      // 得到本车所有节点的arrivedTime
    vector<Spot*> getAllCustomer();  // 得到路径中所有的顾客节点

    // 修改链表属性
    void decreaseLeftQuantity(float amount) {leftQuantity -= amount;}   
    // 减少剩余的车容量（服务了新的顾客）
    void setLeftQuantity(float newValue) {leftQuantity = newValue;}
    void setCurrentArrivedTime(float time) {current->arrivedTime = time;}      
    // 设置当前节点的到达时间
    bool moveForward();      // 向前进

    // 计算插入/删除节点代价
    // 计算所有节点的移除代价
    vector<float> computeReducedCost(float DTpara[], bool artificial = false);
    // 计算item节点在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点前面的顾客节点
	// penaltyPara为惩罚系数，若不需要惩罚则penaltyPara = 0
    void computeInsertCost(Spot item, float &minValue, Spot &customer1, 
            float &secondValue, Spot &customer2, float pertubation = 0.0f, 
            bool regularization = true);  

    // 计算把item插入到pre后面是否会违反时间窗约束
    bool timeWindowJudge(Spot *pre, int pos, Spot item);
    
    // 更新一下各个节点的到达时刻
    void refreshArrivedTime();   

    // 路径的替换和提取
    // 以route替换掉current指针后的路径
    void replaceRoute(const Route &route);  
    // 抓取current指针后的路径
    Route& capture();  

    // 其余辅助性质的函数
    vector<int> removeInvalidCustomer(vector<int> validCustomerId, int &retainNum);
    bool checkPassRoute();
private:
    Spot *head, *current, *rear;  // 表头，表尾和当前指针，当前指针指向货车当前的驻地
    int size;         // 链表的长度
    float quantity;   // 当前已使用的容量(与装载的顾客数量有关)
    float leftQuantity;   // 剩余容量（与车辆行驶情况有关）
    float capacity;       // 车载量，在这里保存
    void copy(const Route& L);  // 复制链表，供复制构造函数和重载“=”函数使用
    vector<float> arrivedTime;  // 在各个节点的到达时刻
};


#endif
