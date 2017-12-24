#include "SetBench.h"
#include "PublicFunction.h"
#include<algorithm>
#include<cmath>

SetBench::SetBench(vector<Customer*> originCustomerSet, int timeSlotLen, int timeSlotNum, 
        float dynamicism): timeSlotLen(timeSlotLen), timeSlotNum(timeSlotNum), dynamicism(dynamicism)
{
    this->originCustomerSet = copyCustomerSet(originCustomerSet);
} // 构造函数

void SetBench::constructProbInfo(){ 
    // 设置各个节点的概率信息
    vector<int> BHsPos(0); // BHs的位置
    int i;
    // float temp[6] = {0.4, 0.2, 0.2, 0.1, 0.1, 0};
    vector<Customer*>::iterator iter = originCustomerSet.begin();
    for(iter; iter < originCustomerSet.end(); iter++) {
        // vector<float> dist = randomVec(timeSlotNum);   // 在各个slot提出需求的概率
        // vector<float> dist(temp, temp+6);
        int index = random(0, timeSlotNum-1);
        for(i=0; i<timeSlotNum; i++) {
            if(i == index) {
                (*iter)->timeProb[i] = 0.5;
            } else if(i == timeSlotNum - 1) {
                (*iter)->timeProb[i] = 0;
            } else {
                (*iter)->timeProb[i] = 0.5/(timeSlotNum - 1);
            }
            //(*iter)->timeProb[i] = dist[i];
        }
    }
}

void SetBench::construct(vector<Customer*> &staticCustomerSet, vector<Customer*> &dynamicCustomerSet){
    // 根据概率情况构造样本
    constructProbInfo();
    int customerAmount = originCustomerSet.end() - originCustomerSet.begin();
    int i;
    int dynamicNum = (int)floor(customerAmount*dynamicism);  // 动态到达的顾客数量
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
        int selectSlot = roulette((*iter)->timeProb, timeSlotNum);   
        float t1 = selectSlot * timeSlotLen;         // 时间段的开始
        float t2 = (selectSlot+1) * timeSlotLen;     // 时间段的结束
        float tempt = random(t1, t2);
        float maxActiveTime = timeSlotNum * timeSlotLen;  // 货车可工作的最晚时间
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
