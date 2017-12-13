#ifndef _LOADSOLOMON_H
#define _LOADSOLOMON_H

#include "Customer.h"
#include<string>
#include<vector>

using namespace std;

bool getData(string filename, vector<Customer*> &allCustomer, Customer &depot, float &capacity);

#endif
