#include "PublicFunction.h"
#include<cstdlib>
#include<iostream>
#include<algorithm>
#include<vector>
#include<numeric>
#include<sstream>
#include<iostream>
#include<limits>
#include<map>
#include<cmath>
#include<ctime>
#include<string>
#include<functional>
#include<cstddef>
#include "../baseclass/Matrix.h"
#include "../algorithm/ALNS.h"

bool ascendSortForCustId(Spot* item1, Spot* item2) {
    return item1->id < item2->id;
}

float random(float start, float end){
    // 产生start到end之间的随机数
    // some times using (int)output will get the boundary value "end" !!
    return (float)(start+(end-start)*rand()/(RAND_MAX+1.0));
}

float dist(Spot *current, Spot *next) {
    // 计算current与next节点之间的距离
    float cost = sqrt(pow(current->x - next->x, 2) + 
            pow(current->y - next->y, 2));
    return cost;

}

int getCustomerNum(vector<Car*> originCarSet) {
    // 获得路径集中顾客节点的数目
    int customerNum = 0;
    for (int i = 0; i<(int)originCarSet.size(); i++) {
        customerNum += originCarSet[i]->getRoute()->getSize();
    }
    return customerNum;
}

bool carSetEqual(vector<Car*> carSet1, vector<Car*> carSet2){
    // 判断carSet1和carSet2是否一样(根据其所有的顾客id顺序判断)
    if(carSet1.size() != carSet2.size()) {return false;}
    bool mark = true;
    for(int i=0; i<(int)carSet1.size(); i++){
        vector<Spot*> cust1 = carSet1[i]->getRoute()->getAllCustomer();
        vector<Spot*> cust2 = carSet2[i]->getRoute()->getAllCustomer();
        if(cust1.size() != cust2.size()) { 
            mark = false; 
            break;
        }
        for(int j=0; j<(int)cust1.size(); j++) {
            if(cust1[j]->id != cust2[j]->id) {
                mark = false; 
                break;
            }
        }
    }
    return mark;
}

bool customerSetEqual(vector<Spot*>c1, vector<Spot*>c2){
    // 判断customer集合c1与c2是否一样（根据id判断）
    if(c1.size() != c2.size()) {return false;}
    bool mark = true;
    for(int i=0; i<(int)c1.size(); i++) {
        if(c1[i]->id != c2[i]->id) {
            mark = false; 
            break;
        }
    }
    return mark;

}

vector<Spot*> extractCustomer(vector<Car*> plan) {
    // 将plan中的顾客节点全部抽取出来
    vector<Spot*> allCustomer;
    vector<Spot*>::iterator custIter;
    vector<Car*>::iterator carIter;
    for (carIter=plan.begin(); carIter<plan.end(); carIter++) {
        vector<Spot*> temp = (*carIter)->getAllCustomer();
        for (custIter=temp.begin(); custIter<temp.end(); custIter++) {
            if((*custIter)->type == 'C') {
                allCustomer.push_back(*custIter);
            }
        }
    }
    return allCustomer;
}

vector<int> getCustomerID(vector<Spot*> customerSet){
    // 得到customerSet的所有顾客ID
    vector<int> ids(0);
    ids.reserve(customerSet.end() - customerSet.begin());
    vector<Spot*>::iterator iter = customerSet.begin();
    for(iter; iter<customerSet.end(); iter++){
        ids.push_back((*iter)->id);
    }
    sort(ids.begin(), ids.end());
    return ids;
}

vector<int> getCustomerID(vector<Car*> carSet) {
    vector<int> ids(0);
    vector<Car*>::iterator carIter;
    for(carIter = carSet.begin(); carIter < carSet.end(); carIter++) {
        vector<Spot*> tempCust = (*carIter)->getAllCustomer();
        vector<int> currentIDs = getCustomerID(tempCust);
        ids.insert(ids.end(), currentIDs.begin(), currentIDs.end());
    }
    sort(ids.begin(), ids.end());
    return ids;
}

void showAllCustomerID(vector<Car*> carSet) {
    // 显示carSet中所有顾客的id信息
    vector<int> allCustomerId = getCustomerID(carSet);
    sort(allCustomerId.begin(), allCustomerId.end());
    vector<int>::iterator intIter;
    int count = 0;
    for(intIter = allCustomerId.begin(); intIter < allCustomerId.end(); intIter++) {
        if (count % 8 == 0) {
            cout << endl;
        }
        cout << (*intIter) << '\t';
        count++;
    }
    cout << endl;
}

void showAllCustomerID(vector<Spot*> customerSet) {
    vector<int> allCustomerId = getCustomerID(customerSet);
    sort(allCustomerId.begin(), allCustomerId.end());
    vector<int>::iterator intIter;
    int count = 0;
    for(intIter = allCustomerId.begin(); intIter < allCustomerId.end(); intIter++) {
        if (count % 8 == 0) {
            cout << endl;
        }
        cout << (*intIter) << '\t';
        count++;
    }
    cout << endl;
}

void showDetailForPlan(vector<Car*> carSet) {
    // 展示carSet中每一条路径的具体信息
    // 包括顾客顺序，以及每个顾客的位置，时间窗，到达时间等
    vector<Car*>::iterator carIter;
    vector<Spot*>::iterator custIter;
    for(carIter = carSet.begin(); carIter < carSet.end(); carIter++) {
        int index = carIter - carSet.begin();
        Spot *depot = (*carIter)->getHeadNode();
        cout << "----------------------" << endl;
        cout << "Route " << index << ":" << endl;
        cout << "Depot index: #" << depot->id << " x-" << depot->x << "\t" << " y-" << depot->y << endl;
        vector<Spot*> tempCust = (*carIter)->getAllCustomer();
        for(custIter = tempCust.begin(); custIter < tempCust.end(); custIter++) {
            cout << "Spot index: #" << (int)(custIter-tempCust.begin()) << "\t"
                << ": x-" << (*custIter)->x << "\t" << "y-" << (*custIter)->y << "\t" 
                << "AT-" << (*custIter)->arrivedTime << "\t" << "ST-" << (*custIter)->startTime 
                << "\t" << "ET-" << (*custIter)->endTime << endl;
        }
        cout << "Depot index: #" << depot->id <<" x-" << depot->x << "\t" <<  "y-" << depot->y << endl;
    }
}

