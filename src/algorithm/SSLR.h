#ifndef _SSLR_H
#define _SSLR_H
#include<vector>
#include "Car.h"
#include<thread>
#include<mutex>
#include "LNSBase.h"

class SSLR: public LNSBase{  // SSALNS算法
public:
    SSLR(vector<Customer*> waitCustomer, vector<Car*> originPlan, float capacity, int maxIter=15000,
            bool verbose=false, int pshaw=6, int pworst=3, float eta=0.025f);
    ~SSALNS();
    void run(vector<Car*> &finalCarSet, float &finalCost, mutex &print_lck);  // 运行算法，相当于算法的main()函数
private:
    vector<Customer*> waitCustomer;   // 待插入的顾客
    vector<Customer*> allCustomer;    // 所有顾客
    vector<Car*> originPlan;          // 初始计划
    Customer depot;
    float capacity;
    int maxIter;
    bool verbose;
};

#endif
