#ifndef _SETBENCH_H
#define _SETBENCH_H

#include "../baseclass/Spot.h"
#include<vector>

using namespace std;

class SetBench{  // 该类对benchmark作修改，生成动态顾客数据
public:
    SetBench();  // 构造函数
    ~SetBench(){};  // 析构函数
    void constructStoreSet();
    void constructCustomerSet();
    void constructDepot();
    void changeTWL(vector<Spot*> customerSet, Spot *depot, float newAlpha);
    void changeDYN(vector<Spot*> customerSet, Spot *depot, int beginIndex,
            vector<Spot*> &staticCustomer, vector<Spot*> &dynamicCustomer);
    void construct(vector<Spot*> &staticCustomerSet, vector<Spot*> &dynamicCustomerSet,
            vector<Spot*> &storeSet, Spot &depot, float currentTime);   // 创造顾客样本
private:
    vector<Spot*> storeSet;
    vector<Spot*> customerSet;
    Spot *depot;
};

#endif
