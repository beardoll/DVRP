#include "Car.h"
#include "../public/PublicFunction.h"
#include "../run/Config.h"
#include<cmath>
#include <stdexcept>

Car::Car(Spot &headNode, Spot &rearNode, int carIndex, int depotIndex, 
        bool artificial): route(headNode, rearNode, headNode.quantity, 
        headNode.timeDuration)
{
    this->carIndex = carIndex;
    this->artificial = artificial;
    this->depotIndex = depotIndex;
}

Car::~Car(){  
    // 内嵌对象的析构函数会被调用，不用在此处delete route
}

Car::Car(const Car& item):route(item.route){  
    // 复制构造函数
    this->carIndex = item.carIndex;
    this->depotIndex = item.depotIndex;
    this->artificial = artificial;
}


Car& Car::operator= (Car &item){ 
    // 重载赋值操作
    this->route = item.route;
    this->carIndex = item.carIndex;
    this->depotIndex = item.depotIndex;
    this->artificial = item.artificial;
    return *this;
}

vector<int> Car::getAllID(){
    // 获得所有节点的ID，按顺序
    return route.getAllID();
}


//================ insert cost和remove cost =================//
void Car::computeInsertCost(Spot *cur, float &minValue, Spot *&ref1, float &secondValue, 
        Spot *&ref2, float randomNoise, bool allowNegativeCost){
        
    // 计算cur节点在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点ref1/ref2
    // Args:
    //   * randomNoise: 随机化的噪声量
    //   * allowNegativeCost: 是否允许出现负的insertion cost
    // Returns:
    //   * ref1: 代价最小的插入点，若minValue=MAX_FLOAT，则为NULL
    //   * ref2: 代价次小的插入点，若secondValue=MAX_FLOAT，则为NULL
    route.computeInsertCost(cur, minValue, ref1, secondValue, ref2, randomNoise, allowNegativeCost);
}

vector<float> Car::computeReducedCost(float DTpara[]){  
    // 得到路径中所有服务对(P-D)的移除代价，值越小表示它可以节省更多的代价
    // Args:
    //   * DTpara: 不同优先级顾客计算reduce cost时的bonus
    return route.computeReducedCost(DTpara, artificial);
}


//================ insert 和 delete Customer方法 =================//
void Car::insertAtRear(Spot *item){
    route.insertAtRear(item);
} 

void Car::insertAfter(Spot *ref, Spot *cur) {
    // 在ref后面插入cur
    route.insertAfter(refStore, refCustomer, store, customer);
}

void Car::deleteCustomer(Spot *node) {
    // 删除节点node
    route.deleteNode(node);
}
