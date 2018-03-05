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
    void construct(vector<Spot*> &staticCustomerSet, vector<Spot*> &dynamicCustomerSet,
            vector<Spot*> &storeSet, Spot &depot);   // 创造顾客样本
private:
    float r1, r2, r3;
    int storeNum, subcircleNum, customerNum;
    float *lambda;
    vector<Spot*> storeSet;
    vector<Spot*> customerSet;
    Spot *depot;
};

#endif
