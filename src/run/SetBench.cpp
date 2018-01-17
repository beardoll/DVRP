#include "SetBench.h"
#include "../public/PublicFunction.h"
#include "Config.h"
#include<algorithm>
#include<cmath>

SetBench::SetBench() {
    // 将整个地图分成三圈，depot为中心，商家处于最内环，而顾客处于最外环
    // Args(来自于Config.h):
    //   * R1, R2, R3: 各环与depot的距离
    //   * NUM_STORE: 商家数目
    //   * NUM_CUSTOMER: 顾客数目
    //   * NUM_SUBCIRCLE: 顾客区域划分数目
    //   * LAMBDA: Poisson到达过程参数，vector类型，长度等于NUM_SUBCIRCLE
    this->r1 = R1;
    this->r2 = R2;
    this->r3 = R3;
    this->numStore = NUM_STORE;
    this->numSubcircle = NUM_SUBCIRCLE;
    this->lambda = LAMBDA;
    this->currentID;
} // 构造函数

vector<Customer*> SetBench::constructStoreSet() {
    // 构造商家集合
    float innerR = r1;
    float outerR = r2;
    vector<Customer*> storeSet;
    for(int i=0; i<numStore; i++) {
        float r = random(innerR, outerR);
        float theta = random(0, 2*PI);
        Customer store = new Customer;
        store->id = currentID++;
        store->x = r * sin(theta);
        store->y = r * cos(theta);
        store->type = "P";
        store->startTime = 0;
        store->serviceTime = random(0, 10);
        store->prop = 0;
        storeSet.push_back(store);
    }
    return storeSet;
}

vector<Customer*> SetBench::constructCustomerSet() {
    vector<Customer*> customerSet;
    float innerR = r2;
    float outerR = r3;
    int count = 0;
    float timeHorizon = TIME_SLOT_LEN * TIME_SLOT_NUM;
    float deltaT = 10; // 采样间隔时间
    float deltaAngle = 2 * PI / numSubcircle;  // 各个区域夹角
    int numSlice = int(timeHorizon/deltaT);
    while(Customer.size() < numCustomer){
        for(int t=0; t<numSlice; t++) {
            for(int j=0; j<numSubcircle; j++) {
                float p = lambda[j] * deltaT * exp(-lambda[j] * deltaT);
                if(p < random(0,1)) {
                    // 按概率生成顾客
                    float theta = random(deltaAngle*j, deltaAngle*(j+1));
                    float r = random(innerR, outerR);
                    Customer c = new Customer;
                    c->id = currentID++;
                    c->x = r * sin(theta);
                    c->y = r * cos(theta);
                    c->serviceTime = random(0, 10);
                    c->prop = 0;
                    c->choice = int(random(0, numStore));
                    customerSet.push_back(c);
                    if(customerSet.size() == numCustomer) break;
                }
            }
        }
    }
    return customerSet;
}

void SetBench::construct(vector<Customer*> &staticCustomerSet, vector<Customer*> &dynamicCustomerSet){
    // 根据概率情况构造样本
    constructProbInfo();
    int customerAmount = originCustomerSet.end() - originCustomerSet.begin();
    int i;
    int dynamicNum = (int)floor(customerAmount*DYNAMICISM);  // 动态到达的顾客数量
    vector<int> staticPos;           // 静态到达的顾客节点在originCustomerSet中的定位
    // 动态到达的BHs在BHs集合下的坐标
    vector<int> dynamicPos = getRandom(0, customerAmount, dynamicNum, staticPos);   	
    vector<Customer*>::iterator iter = originCustomerSet.begin();
    staticCustomerSet.resize(0);
    dynamicCustomerSet.resize(0);
    for(iter; iter<originCustomerSet.end(); iter++) {
        // 当前顾客节点于originCustomerSet中的定位
        // 这里默认originCustomerSet是按id升序排列
        int count = iter - originCustomerSet.begin();  
        // 寻找count是否是dynamicPos中的元素
        vector<int>::iterator iter2 = find(dynamicPos.begin(), dynamicPos.end(), count);
        if(iter2 != dynamicPos.end()) {   
            // 在dynamicPos集合中
            (*iter)->prop = 1;
            dynamicCustomerSet.push_back(*iter);
        } else {  
            (*iter)->prop = 0;
            staticCustomerSet.push_back(*iter);
        }
        // 利用轮盘算法采样得出顾客可能提出需求的时间段
        int selectSlot = roulette((*iter)->timeProb, TIME_SLOT_NUM);   
        float t1 = selectSlot * TIME_SLOT_LEN;         // 时间段的开始
        float t2 = (selectSlot+1) * TIME_SLOT_LEN;     // 时间段的结束
        float tempt = random(t1, t2);
        float maxActiveTime = TIME_SLOT_NUM * TIME_SLOT_LEN;  // 货车可工作的最晚时间
        // 至少宽限5倍的serviceTime
        (*iter)->startTime =  min(tempt, maxActiveTime - 5 * (*iter)->serviceTime); 
        float t3 = 3*(*iter)->serviceTime;
        float t4 = 12*(*iter)->serviceTime;
        float timeWindowLen = random(t3, t4);  // 时间窗长度
        (*iter)->endTime = min((*iter)->startTime + timeWindowLen, maxActiveTime);
        timeWindowLen = (*iter)->endTime - (*iter)->startTime;
        // 可容忍的最晚得到答复的时间，为0.3-0.6倍的时间窗长度 + startTime
        (*iter)->tolerantTime = (*iter)->startTime + random(0.6, 0.8) * timeWindowLen;
    }
}
