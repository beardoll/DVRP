#ifndef _ALNS_H
#define _ALNS_H
#include "../baseclass/Car.h"
#include "LNSBase.h"
#include<vector>

float RANDOM_RANGE[2] = {0, 1};

class ALNS: public LNSBase {  // 算法类
public:
    ALNS(vector<Customer*> allCustomer, Customer depot, float capacity, int maxIter,
            bool verbose=false, int pshaw=6, int pworst=3, float eta=0.025f): LNSBase(
                pshaw, pworst, eta, capacity, RANDOM_RANGE, allCustomer, depot) 
    {
        this->maxIter = maxIter;
        this->verbose = verbose;
    }
    ~ALNS(){};
    void run(vector<Car*> &finalCarSet, float &finalCost);  // 运行算法，相当于算法的main()函数
private:
    int maxIter;
    bool verbose;
};

#endif
