#ifndef _LOADSOLOMON_H
#define _LOADSOLOMON_H

#include "../baseclass/Customer.h"
#include<string>
#include<vector>

using namespace std;

void getData(string filename, vector<Customer*> &allCustomer, Customer &depot, float &capacity);

#endif
