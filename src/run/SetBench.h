#ifndef _SETBENCH_H
#define _SETBENCH_H

#include "../baseclass/Customer.h"
#include<vector>

using namespace std;

class SetBench{  // 该类对benchmark作修改，生成动态顾客数据
public:
    SetBench(vector<Customer*> originCustomerSet);  // 构造函数
    ~SetBench(){};  // 析构函数
    void constructProbInfo(); // 为probInfo赋值
    void construct(vector<Customer*> &staticCustomerSet, 
            vector<Customer*> &dynamicCustomerSet, Customer depot);   // 创造顾客样本
private:
    vector<Customer*> originCustomerSet; // 原始顾客数据
};


#endif
