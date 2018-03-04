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
    this->storeNum = STORE_NUM;
    this->subcircleNum = SUBCIRCLE_NUM;
    this->customerNum = CUSTOMER_NUM;
    this->lambda = LAMBDA;
} // 构造函数

void SetBench::constructStoreSet() {
    // 构造商家集合
    float innerR = r1;
    float outerR = r2;
    vector<Spot*> storeSet;
    for(int i=0; i<storeNum; i++) {
        float r = random(innerR, outerR);
        float theta = random(0, 2*PI);
        Spot *store = new Spot();
        // 商店id从customerNum+1 ~ customerNum+storeNum
        store->id = customerNum + storeSet.size() + 1;
        store->x = r * sin(theta);
        store->y = r * cos(theta);
        store->type = "P";
        store->startTime = 0;
        store->serviceTime = random(0, 10);
        store->prop = 0;
        storeSet.push_back(store);
    }
    this->storeSet = storeSet;
}

void SetBench::constructCustomerSet() {
    vector<Spot*> customerSet;
    float innerR = r2;
    float outerR = r3;
    float timeHorizon = TIME_SLOT_LEN * TIME_SLOT_NUM; // 仿真的时间轴长度
    float deltaT = 10; // 采样间隔时间
    float deltaAngle = 2 * PI / subcircleNum;  // 各个区域夹角
    float alpha = ALPHA;  // 时间窗长度与dist(顾客，商家)的比例系数
    int sliceNum = int(timeHorizon/deltaT);
    while(customerSet.size() < customerNum){
        for(int t=0; t<sliceNum; t++) {
            for(int j=0; j<subcircleNum; j++) {
                float p = lambda[j] * deltaT * exp(-lambda[j] * deltaT);
                if(p < random(0,1)) {
                    // 按概率生成顾客
                    float theta = random(deltaAngle*j, deltaAngle*(j+1));
                    float r = random(innerR, outerR);
                    Spot c = new Spot();
                    // 顾客的id从1~customerNum
                    c->id = (int)customerSet.size() + 1;
                    c->x = r * sin(theta);
                    c->y = r * cos(theta);
                    c->serviceTime = random(0, 10);
                    c->prop = 0;
                    // 随机选出商店
                    int index = int(random(0, storeNum));
                    index = min(storeNum-1, index);
                    Spot *store = new Spot(*storeSet[index]);
                    c->choice = store;
                    float distFromCustomerToStore = dist(c, c->choice);
                    // 保证足够长的时间窗
                    c->startTime = random(0, timeHorizon-alpha*distFromCustomerToStore);
                    c->endTime = random(c->startTime, timeHorizon);
                    c->demand = random(0, MAX_DEMAND);
                    customerSet.push_back(c);
                    if(customerSet.size() == customerNum) break;
                }
            }
        }
    }
    this->customerSet = customerSet;
}

void SetBench::constructDepot() {
    // 仓库节点
    Spot depot = new Spot();
    depot->x = 0;
    depot->y = 0;
    depot->id = 0;
    depot->type = 'D';
    this->depot = depot;
}

void SetBench::construct(vector<Spot*> &staticCustomerSet, vector<Spot*> &dynamicCustomerSet,
        vector<Spot*> &storeSet, Spot &depot){
    // 根据概率情况构造样本
    constructStoreSet();
    constructCustomerSet();
    constructSpot();
    int i;
    int dynamicNum = (int)floor(customerNum*DYNAMICISM);  // 动态到达的顾客数量
    vector<int> staticPos;           // 静态到达的顾客节点在customerSet中的定位
    // 动态到达的BHs在BHs集合下的坐标
    vector<int> dynamicPos = getRandom(0, customerNum, dynamicNum, staticPos);   	
    vector<Spot*>::iterator iter = customerSet.begin();
    staticCustomerSet.resize(0);
    dynamicCustomerSet.resize(0);
    for(iter; iter<customerSet.end(); iter++) {
        // 当前顾客节点于customerSet中的定位
        // 这里默认customerSet是按id升序排列
        int count = iter - customerSet.begin();  
        // 判断count是否是dynamicPos中的元素
        vector<int>::iterator iter2 = find(dynamicPos.begin(), dynamicPos.end(), count);
        if(iter2 != dynamicPos.end()) {   
            // 在dynamicPos集合中
            dynamicCustomerSet.push_back(*iter);
        } else {  
            staticCustomerSet.push_back(*iter);
        }
        timeWindowLen = (*iter)->endTime - (*iter)->startTime;
        // 可容忍的最晚得到答复的时间，为0.6-0.8倍的时间窗长度 + startTime
        (*iter)->tolerantTime = (*iter)->startTime + random(0.6, 0.8) * timeWindowLen;
    }
    storeSet = this->storeSet;
    depot = this->depot;
}
