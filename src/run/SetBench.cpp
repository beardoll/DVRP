#include "SetBench.h"
#include "../public/PublicFunction.h"
#include "Config.h"
#include<algorithm>
#include<cmath>

SetBench::SetBench() {
    // 将整个地图分成三圈，depot为中心，商家处于最内环，而顾客处于最外环
    // Args(来自于Config.h):
    //   * R1, R2: 商家所在区域
    //   * R3, R4: 顾客所在区域
    //   * NUM_STORE: 商家数目
    //   * NUM_SUBCIRCLE: 顾客区域划分数目
    //   * LAMBDA: Poisson到达过程参数，vector类型，长度等于NUM_SUBCIRCLE
} // 构造函数

void SetBench::constructStoreSet() {
    // 构造商家集合
    float innerR = R1;
    float outerR = R2;
    vector<Spot*> storeSet;
    for(int i=0; i<STORE_NUM; i++) {
        float r = random(innerR, outerR);
        float theta = random(0, 2*PI);
        Spot *store = new Spot();
        // 商店id从1000开始
        store->id = 1000 + storeSet.size() + 1;
        store->x = r * sin(theta);
        store->y = r * cos(theta);
        store->type = 'S';
        store->startTime = 0;
        store->serviceTime = random(0, 5);
        store->prop = 0;
        storeSet.push_back(store);
    }
    this->storeSet = storeSet;
}

void SetBench::constructCustomerSet() {
    vector<Spot*> customerSet(0);
    float innerR = R3;
    float outerR = R4;
    //float timeHorizon = (float)TIME_SLOT_LEN * TIME_SLOT_NUM; // 仿真的时间轴长度
    float timeHorizon = LATEST_SERVICE_TIME;
    float deltaT = 1; // 采样间隔时间
    float deltaAngle = 2 * PI / SUBCIRCLE_NUM;  // 各个区域夹角
    bool mark = true;
    int count = 0; 
    for(int j=0; j<SUBCIRCLE_NUM; j++) {
        for(int i=0; i<TIME_SLOT_NUM-1; i++) {
            int customerNum = poissonSampling(LAMBDA[j], TIME_SLOT_LEN);
            for(int x=0; x<customerNum; x++) {
                // 按概率生成顾客
                float theta = random(deltaAngle*j, deltaAngle*(j+1));
                float r = random(innerR, outerR);
                Spot *c = new Spot();
                // 顾客的id从1开始
                c->id =  ++count;
                c->x = r * sin(theta);
                c->y = r * cos(theta);
                c->serviceTime = random(0, 5);
                c->prop = 0;
                c->type = 'C';
                // 随机选出商店
                int index = int(random(0, STORE_NUM));
                index = min(STORE_NUM-1, index);
                Spot *store = new Spot(*storeSet[index]);
                store->type = 'S';
                c->choice = store;
                store->choice = c;
                float distFromCustomerToStore = dist(c, c->choice);
                float distFromDepotToStore = dist(depot, c->choice);
                float minTimeLen = distFromCustomerToStore + distFromDepotToStore;
                if(i*TIME_SLOT_LEN+ALPHA*minTimeLen > timeHorizon) {
                    count--;
                    continue;
                } else {
                    // 保证足够长的时间窗
                    c->startTime = random(i*TIME_SLOT_LEN, (i+1)*TIME_SLOT_LEN);
                    c->startTime = min(c->startTime, timeHorizon-ALPHA*minTimeLen);
                    c->endTime = random(c->startTime+ALPHA*minTimeLen, timeHorizon);
                    float windowLen = c->endTime - c->startTime;
                    c->tolerantTime = c->startTime + random(0.6*windowLen, 0.8*windowLen);
                    c->quantity = random(0, MAX_DEMAND);
                    customerSet.push_back(c);
                }
            }
        }
    }
    this->customerSet = customerSet;
    cout << "The size of customerSet: " << this->customerSet.size() << endl;
}

void SetBench::constructDepot() {
    // 仓库节点
    Spot *depot = new Spot();
    depot->x = 0;
    depot->y = 0;
    depot->id = 0;
    depot->type = 'D';
    this->depot = depot;
}

void SetBench::construct(vector<Spot*> &staticCustomerSet, vector<Spot*> &dynamicCustomerSet,
        vector<Spot*> &storeSet, Spot &depot, float currentTime){
    // 构造样本
    // currentTime: startTime < currentTime的为static, 否则为dynamic
    // 需要重写这里的逻辑
    constructDepot();
    constructStoreSet();
    constructCustomerSet();
    vector<Spot*>::iterator iter = customerSet.begin();
    for(iter = customerSet.begin(); iter < customerSet.end(); iter++) {
        if((*iter)->startTime <= currentTime) {
            staticCustomerSet.push_back(*iter);
        } else {
            dynamicCustomerSet.push_back(*iter);
        }
    }
    depot = *this->depot;
    storeSet = this->storeSet;
    cout << "static customer number: " << staticCustomerSet.size() << endl;
    cout << "dynamic customer number: " << dynamicCustomerSet.size() << endl;
}

