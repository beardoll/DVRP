#ifndef _SSLR_H
#define _SSLR_H
#include<vector>
#include "../baseclass/Car.h"
#include "../baseclass/Spot.h"
#include "../public/PublicFunction.h"
#include<thread>
#include "LNSBase.h"


class SSLR: public LNSBase{  // SSALNSÀ„∑®
public:
    SSLR(vector<Spot*> allCustomer, vector<Spot*> depots, int maxIter=15000, 
            bool verbose=false, int pshaw=6, int pworst=3, float eta=0.0f); 
    ~SSLR();
    void run(vector<Car*> &finalCarSet, float &finalCost);
private:
    int maxIter;
    bool verbose;
};

#endif
