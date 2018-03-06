#ifndef BENCH_WRAPPER_H
#define BENCH_WRAPPER_H

#include "tinystr.h"
#include "tinyxml.h"
#include <iostream>
#include <string>
#include <vector>
#include "../baseclass/Spot.h"
#include "../baseclass/Car.h"

using namespace std;

class BenchWrapper{
    public:
        BenchWrapper(){};
        ~BenchWrapper(){};
        void insertFloatToNode(TiXmlElement *element, float data);
        TiXmlElement* createNode(string name, float data);
        void saveSpotInfo(vector<Spot*> customers, TiXmlElement *root);
        void saveBench(string path, vector<Spot*> staticCustomer, vector<Spot*>dynamicCustomer,
                vector<Spot*> store, Spot depot, float capacity);
        void saveResult(string filename, vector<Car*> carSet, vector<Spot*> rejectCustomers, 
                vector<Spot*> dynamicCustomers, Spot depot, float travelLen, float extra);
        void getFloatFromChildNode(TiXmlHandle parent, string childName, float &value);
        void getFloatArrayFromChildNode(TiXmlHandle parent, string childName, float *array);
        void loadStoreInfo(vector<Spot*> &stores, TiXmlElement *nodeElem);
        void loadCustomerInfo(vector<Spot*> &customers, vector<Spot*> stores, TiXmlElement *nodeElem);
        void loadBench(string fileName, vector<Spot*> &staticCustomers, 
                vector<Spot*> &dynamicCustomers, vector<Spot*> stores, Spot &depot, float &capacity);

};
#endif
