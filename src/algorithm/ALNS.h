#ifndef _ALNS_H
#define _ALNS_H
#include "Car.h"
#include<vector>

class ALNS{  // 算法类
public:
	ALNS(vector<Customer*> allCustomer, Customer depot, float capacity, int maxIter,
            bool verbose=false, int pshaw=6, int pworst=3, float eta=0.025f);
	~ALNS(){};
	void run(vector<Car*> &finalCarSet, float &finalCost);  // 运行算法，相当于算法的main()函数
private:
	int maxIter;
    bool verbose;
};

#endif
