#ifndef _SETBENCH_H
#define _SETBENCH_H

#include "../baseclass/Customer.h"
#include<vector>

using namespace std;

class SetBench{  // 该类对benchmark作修改，生成动态顾客数据
public:
    SetBench();  // 构造函数
    ~SetBench(){};  // 析构函数
    void constructProbInfo(); // 为probInfo赋值
    void construct(vector<Customer*> &staticCustomerSet, 
            vector<Customer*> &dynamicCustomerSet);   // 创造顾客样本
private:
    float r1, r2, r3;
    int numStore, numSubcircle;
    vector<float> lambda;
    int currentID;
};


#endif
