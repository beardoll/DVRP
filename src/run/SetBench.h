#ifndef _SETBENCH_H
#define _SETBENCH_H

#include "../baseclass/Customer.h"
#include<vector>

using namespace std;

class SetBench{  // 该类对benchmark作修改，生成动态顾客数据
public:
    SetBench(){};
    ~SetBench(){};  // 析构函数
    void constructProbInfo(vector<Customer*> originCustomerSet); // 为probInfo赋值
    void changeTWL(vector<Customer*> customerSet, Customer depot, float alpha);

    void changeDYN(vector<Customer*> originCustomerSet, Customer depot, float dynamicism,
    vector<Customer*> &staticCustomer, vector<Customer*> &dynamicCustomer);
    void construct(vector<Customer*> originCustomerSet, vector<Customer*> &staticCustomerSet, 
            vector<Customer*> &dynamicCustomerSet, Customer depot);   // 创造顾客样本
};


#endif
