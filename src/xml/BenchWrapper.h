#ifndef BENCH_WRAPPER_H
#define BENCH_WRAPPER_H

#include "tinystr.h"
#include "tinyxml.h"
#include <iostream>
#include <string>
#include <vector>
#include "../baseclass/Customer.h"
#include "../baseclass/Car.h"

using namespace std;

class BenchWrapper{
    public:
        BenchWrapper(){};
        ~BenchWrapper(){};
        void insertFloatToNode(TiXmlElement *element, float data);
        TiXmlElement* createNode(string name, float data);
        void saveCustomerInfo(vector<Customer*> customers, TiXmlElement *root);
        void saveBench(string path, vector<Customer*> staticCustomer, vector<Customer*>dynamicCustomer,
                Customer depot, float capacity);
        void saveResult(string filename, vector<Car*> carSet, vector<Customer*> rejectCustomers, 
                vector<Customer*> dynamicCustomers, Customer depot, float travelLen, float extra);
        void getFloatFromChildNode(TiXmlHandle parent, string childName, float &value);
        void getFloatArrayFromChildNode(TiXmlHandle parent, string childName, float *array);
        void loadCustomerInfo(vector<Customer*> &customers, TiXmlElement *nodeElem);
        bool loadBench(string fileName, vector<Customer*> &staticCustomers, 
                vector<Customer*> &dynamicCustomers, Customer &depot, float &capacity);

}
#endif
