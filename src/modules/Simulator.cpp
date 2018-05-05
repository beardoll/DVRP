#include "Simulator.h"
#include "../algorithm/ALNS.h"
#include "../baseclass/Matrix.h"
#include "../public/PublicFunction.h"
#include "../algorithm/SSLR.h"
#include<algorithm>
#include "../run/TxtRecorder.h"
#include "../run/Config.h"
#include<thread>
#include<condition_variable>

using namespace std;

Simulator::Simulator(int slotIndex, vector<Spot*> promiseCustomerSet, 
        vector<Spot*> waitCustomerSet, vector<Spot*> dynamicCustomerSet, 
        vector<Car*> currentPlan, vector<Spot*> storeSet) { 
    // 构造函数
    this->slotIndex = slotIndex;
    this->promiseCustomerSet = promiseCustomerSet;
    this->waitCustomerSet = waitCustomerSet;
    this->dynamicCustomerSet = dynamicCustomerSet;
    this->currentPlan = currentPlan;
    this->storeSet = storeSet;
}

Simulator::~Simulator(){  
    // 析构函数
    //clearCarSet();
    //clearCustomerSet();
}

void clearPlanSet(vector<vector<Car*> > planSet) {
    // 清除planSet
    vector<vector<Car*> >::iterator iter;
    for(iter = planSet.begin(); iter < planSet.end(); iter++) {
        withdrawPlan((*iter));
    }
    planSet.resize(0);
}

vector<Spot*> Simulator::generateScenario(Spot depot){
    // Intro:
    //   * 产生动态顾客出现的情景（时间窗、位置等）
    // Args:
    //   * depot: 仓库节点，用来估算时间窗长度
    float leftBound, rightBound;  // 时间窗长度浮动因子(>=1)
    // 动态顾客的起始id
    int beginIdx = CUSTOMER_NUM;
    switch(STRATEGY) {
        case Negative: {
            leftBound = max(0.5*ALPHA, 1.0);
            rightBound = ALPHA;
            break;
        }
        case Positive: {
            leftBound = ALPHA;
            rightBound = 2*ALPHA;
            break;
        }
    }
    // 随机产生顾客节点
    float innerR = R3;   // 内圈
    float outerR = R4;   // 外圈

    //float timeHorizon = TIME_SLOT_LEN * TIME_SLOT_NUM; // 仿真的时间轴长度
    float timeHorizon = LATEST_SERVICE_TIME;
    // 当前timeSlot的起始时间点，动态顾客必须在此之后提出需求
    float currentTime = TIME_SLOT_LEN * slotIndex;
    int subcircleNum = SUBCIRCLE_NUM;  // 扇形数量
    float deltaAngle = 2 * PI / subcircleNum;  // 各个区域夹角
    int storeNum = (int)storeSet.size();
    vector<Spot*> dynamicCustomer;
    for(int j=0; j<SUBCIRCLE_NUM; j++) {
        int customerNum;
        for(int i=slotIndex; i<TIME_SLOT_NUM-1; i++) {
            customerNum = poissonSampling(LAMBDA[j]*FACTOR, TIME_SLOT_LEN);
            for(int x=0; x<customerNum; x++) {
                // 按概率生成顾客
                float currentAlpha = random(leftBound, rightBound);
                float theta = random(deltaAngle*j, deltaAngle*(j+1));
                float r = random(innerR, outerR);
                Spot *c = new Spot();
                c->id = beginIdx++;
                c->x = r*sin(theta);
                c->y = r*cos(theta);
                c->serviceTime = random(0, 5);
                c->prop = 1;
                c->type = 'C';
                // 随机选出商店
                int index = int(random(0, storeNum));
                index = min(storeNum-1, index);
                Spot *store = new Spot(*storeSet[index]);
                store->type = 'S';
                c->choice = store;
                store->choice = c;
                float distFromCustomerToStore = dist(c, c->choice);
                float distFromDepotToStore = dist(&depot, c->choice);
                // 最短时间窗
                float minTimeLen = distFromCustomerToStore + distFromDepotToStore;
                if(currentTime + currentAlpha * minTimeLen > timeHorizon) {
                    // 产生了不合法样本，删除之
                    delete c, store;
                    continue;
                } 
                else {
                    // 保证足够长的时间窗
                    c->startTime = random(currentTime, (slotIndex+1)*TIME_SLOT_LEN);
                    c->startTime = min(c->startTime, timeHorizon-ALPHA*minTimeLen);
                    float minEndTime = max(BEGIN_SLOT_INDEX * TIME_SLOT_LEN + 
                        1.2 * minTimeLen, double(c->startTime+ALPHA*minTimeLen));
                    c->endTime = random(minEndTime, timeHorizon);
                    c->quantity = random(0, MAX_DEMAND);
                    dynamicCustomer.push_back(c);
                }
            }
        }
    }
    return dynamicCustomer;
}

bool Simulator::checkFeasible(vector<Car*> carSet) {
    // 检查carSet中是否包括了所有的promiseCustomer
    vector<int> tempId = getCustomerID(promiseCustomerSet); // promise Customer的id
    sort(tempId.begin(), tempId.end());
    vector<Car*>::iterator carIter;
    for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
        vector<Spot*> tempCust = (*carIter)->getRoute()->getAllCustomer();
        vector<Spot*>::iterator custIter;
        for(custIter=tempCust.begin(); custIter<tempCust.end(); custIter++) {
            vector<int>::iterator intIter = find(tempId.begin(), tempId.end(), (*custIter)->id);
            if(intIter < tempId.end()) {
                // 如果找到，就从tempId中删除
                tempId.erase(intIter);
            }
        }
    }
    if(tempId.size() != 0) {
        cout << "Miss some promise customers: " << endl;
        for(int i=0; i<tempId.size(); i++) {
            cout << tempId[i] << "\t";
        }
        cout << endl;
        return false;
    } else {
        return true;
    }
}

void threadForInitial(Spot depot, float capacity, int coreId, vector<vector<Car*> > &planSet, 
        vector<Spot*> allCustomer, vector<int> validId, Matrix<int> &transformMatrix, mutex &record_lck, bool &flag) {
    // 路径初始化的线程操作
    // Args:
    //   * depot: 仓库节点
    //   * capacity: 车容量
    //   * coreId: 本线程的id
    //   * allCustomer: 初始的所有顾客节点(D)
    //   * validId: 得到了service promise的顾客id
    //   * record_lck: 锁住planSet变量，以向其写入数据
    // Returns:
    //   * planSet: 新增当前情景解到其中
    //   * transformMatrix: 获得service promise的顾客间的转移频率
    if(SHOW_DETAIL){
        record_lck.lock();
        cout << "There are totally " << allCustomer.size() << " customers" << endl;
        record_lck.unlock();
    }
    ALNS alg(allCustomer, depot, capacity, 10000*ITER_PERCENTAGE, DEBUG);
    vector<Car*> solution(0);
    float cost = 0;
    try{
        alg.run(solution, cost);
    } catch(exception &e) {
        record_lck.lock();
        flag = false;
        record_lck.unlock();
        return;
    }
    vector<Car*>::iterator carIter;
    int totalRetainNum = 0;   // 看看经过removeInvalidCustomer后还有多少剩余节点
    for(carIter = solution.begin(); carIter < solution.end(); carIter++) {
        (*carIter)->removeInvalidCustomer(validId, totalRetainNum);
        (*carIter)->updateTransformMatrix(transformMatrix);
    }
    record_lck.lock();
    planSet.push_back(solution);
    if(SHOW_DETAIL) {
        cout << "The core with id #" << coreId << " finished its task" << endl;
    }
    record_lck.unlock();
    return;
}

void retainVehicles(vector<Car*> &carSet, Spot depot, float capacity) {
    // 保留VEHICLE_NUM数量的车
    // 只清除当前没有顾客节点的车辆
    // 如果carSet中的车辆数不足VEHICLE_NUM，则填补欠缺的空车
    // 只在initialPlan中使用
    if(carSet.size() <= VEHICLE_NUM) {
        int addNum = VEHICLE_NUM - carSet.size();
        for(int i=0; i<addNum; i++) {
            Car *newCar = new Car(depot, depot, capacity, carSet.size());
            carSet.push_back(newCar);
        }
    } else {
        int cutNum = carSet.size() - VEHICLE_NUM;
        int i=0;
        vector<Car*>::iterator carIter;
        for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
            if(cutNum <= 0) break;
            if((*carIter)->getRoute()->getSize() == 0) {
                delete(*carIter);
                carIter = carSet.erase(carIter);
                cutNum--;
            } else {
                carIter++;
            }
        }
        if(cutNum > 0) {
            for(int i=0; i<carSet.size(); i++) {
                carSet[i]->changeCarIndex(i);
            }
            // throw out_of_range("Cannot find so many vehicles to dismiss");
        } else {
            for(int i=0; i<carSet.size(); i++) {
                carSet[i]->changeCarIndex(i);
            }
        }
    }
}

vector<Car*> Simulator::initialPlan(Spot depot, float capacity){     
    // 利用采样制定初始计划
    ostringstream ostr;
    vector<int>::iterator intIter;
    vector<Car*>::iterator carIter;
    vector<Car*> outputPlan;
    if(SAMPLING) {
        // 所有在计划开始前已知的顾客id（属于必须服务的顾客）
        vector<int> validId;
        validId.push_back(0);   // 第一个节点时仓库节点
        // 顾客集按照id大小进行排序
        sort(promiseCustomerSet.begin(), promiseCustomerSet.end());
        vector<int> tempId = getCustomerID(promiseCustomerSet);
        validId.insert(validId.end(), tempId.begin(), tempId.end());
        
        // 初始化transformMatrix
        Matrix<int> transformMatrix(validId.size(), validId.size());
        transformMatrix.setAll(0);
        
        // 所有采样得到的计划
        // 对所有的情景运行ALNS算法，并且把解放入planSet中
        // 在此过程中将根据validId对所有的解，仅保留id在validId中的顾客节点
        ostr.str("");
        ostr << "----Sampling begins!" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        int restSampleNum;
        if(DEBUG) {
            // DEBUG模式下只使用单线程
            // 在sampling=false时也只使用单线程
            restSampleNum = 1;
        } else {
            restSampleNum = SAMPLE_RATE;   // 尚未跑完的样本
        }

        // 存放情景解
        vector<vector<Car*> > planSet;
        mutex record_lck;    // 锁住planSet
        vector<thread> thread_pool;   // a pool to capitalize thread

        bool flag = true;
        while(restSampleNum > 0) {
            // coreId: 线程id，从0开始
            int coreId = SAMPLE_RATE - restSampleNum + 1; 
            for(int i=0; i<min(CORE_NUM,restSampleNum); i++) {
                // 所有顾客信息
                vector<Spot*> allCustomer = copyCustomerSet(promiseCustomerSet);
                vector<Spot*> currentDynamicCust = generateScenario(depot);  // 采样
                allCustomer.insert(allCustomer.end(), currentDynamicCust.begin(), currentDynamicCust.end());
                thread_pool.push_back(thread(threadForInitial, depot, capacity,
                    coreId + i, ref(planSet), allCustomer, validId, ref(transformMatrix),
                    ref(record_lck), ref(flag)));
            }
            for(auto& thread:thread_pool) {
                thread.join();
            }
            if(flag == false) {
                throw out_of_range("Problem in subthread!");
            }
            restSampleNum = restSampleNum - CORE_NUM;
            thread_pool.clear();
            thread_pool.resize(0);
        }

        if(ASSESSMENT) {
            // 然后对所有情景下的计划进行评分，取得分最高者作为初始路径计划
            ostr.str("");
            ostr << "----Now assessing the performance of each scenario" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            vector<pair<int, int> > scoreForPlan;    // 每个计划的得分
            vector<vector<Car*> >::iterator planIter;
            try {
                for(planIter = planSet.begin(); planIter < planSet.end(); planIter++) {
                    int pos = planIter - planSet.begin();   // 在采样得到的计划中的位置
                    int score = 0;
                    for(carIter = planIter->begin(); carIter < planIter->end(); carIter++) {
                        score += (*carIter)->computeScore(transformMatrix);
                    }
                    scoreForPlan.push_back(make_pair(score, pos));
                }
            } catch (exception &e) {
                throw out_of_range(e.what());
            }
            cout << "Assessment finished, its score is: " << scoreForPlan[0].first << endl;
            sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
            outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
        } else {
            // 如果不采用assessment，则随机选取一个plan
            if(DEBUG) outputPlan = copyPlan(planSet[0]);
            else {
                int randomSelection = int(random(0, SAMPLE_RATE));
                randomSelection = min(randomSelection, SAMPLE_RATE-1);
                outputPlan = copyPlan(planSet[randomSelection]);
            }
        }
        clearPlanSet(planSet);
    } else {
        // 如果不采用sampling方案
        vector<Spot*> allCustomer = copyCustomerSet(promiseCustomerSet);
        ALNS alg(allCustomer, depot, capacity, 10000*ITER_PERCENTAGE, DEBUG);
        vector<Car*> solution(0);
        float cost = 0;
        alg.run(solution, cost);
        cout << "Using ALNS without sampling to initialize..." << endl;
        outputPlan = solution;
    } 
    if(CONSTRAIN_CAR_NUM) {
        retainVehicles(outputPlan, depot, capacity);
        cout << "Successfully cut the outputPlan!" << endl;
    }
    return outputPlan;
}

void validPromise(vector<Car*>Plan, vector<Spot*> hurryCustomer, 
        vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId){
    // 对hurry customer确认promise
    // 给出"accept" 或者 "reject" 回应
    vector<Car*>::iterator carIter;
    vector<Spot*>::iterator custIter;
    // hurry customer的id
    vector<int> hurryCustomerId = getCustomerID(hurryCustomer);
    sort(hurryCustomerId.begin(), hurryCustomerId.end());
    int i;
    for(carIter = Plan.begin(); carIter < Plan.end(); carIter++){
        vector<Spot*> tempCust = (*carIter)->getAllCustomer();
        for(custIter = tempCust.begin(); custIter < tempCust.end(); custIter++) {
            int tempId = (*custIter)->id;
            vector<int>::iterator tempIter = find(hurryCustomerId.begin(), 
                    hurryCustomerId.end(), tempId);
            if(tempIter < hurryCustomerId.end()) {
                // 如果tempId在hurryCustomerId中
                newServedCustomerId.push_back(tempId);
                hurryCustomerId.erase(tempIter);
            }
        }
    }
    // 得到放弃的顾客id
    vector<int>::iterator intIter;
    for(intIter = hurryCustomerId.begin(); intIter < hurryCustomerId.end(); intIter++) {
        newAbandonedCustomerId.push_back(*intIter);
    }
}

void threadForReplan(float capacity, int coreId, vector<vector<Car*>> &planSet, 
        vector<Spot*> sampleCustomer, vector<Car*> currentPlan, vector<int> validId,
        Matrix<int> &transformMatrix, mutex &record_lck, bool &flag) {
    // replan (SSLR) 的多线程
    // Args:
    //   * capacity: 货车容量
    //   * coreId: 线程ID
    //   * sampleCustomer: 未得到service promise的顾客（经采样）
    //   * currentPlan: 当前计划
    // Returns:
    //   * planSet: 在线程中为其添加一个新的解
    //   * transformMatrix: 得到service promise的顾客之间的转移频数
    vector<Car*> tempPlan;
    float finalCost = 0;
    SSLR alg(sampleCustomer, currentPlan, capacity, 10000*ITER_PERCENTAGE, DEBUG);
    try{
        alg.run(tempPlan, finalCost, record_lck);
    } catch (exception &e) {
        record_lck.lock();
        flag = false;
        record_lck.unlock();
        return;
    }
    vector<Car*>::iterator carIter;
    int totalRetainNum = 0;
    for(carIter = tempPlan.begin(); carIter < tempPlan.end(); carIter++) {
        (*carIter)->removeInvalidCustomer(validId, totalRetainNum);
        (*carIter)->updateTransformMatrix(transformMatrix);
    }
  
    // unique_lock<mutex> lck1(record_lck);
    if(validId.size() != totalRetainNum+1) {
        record_lck.lock();
        cout << "valid id size: " << validId.size()-1 << ";real size: " << totalRetainNum << endl;
        cout << "Miss some customers after remove invalid customers!" << endl;
        flag = false;
        record_lck.unlock();
        return;
    }
    record_lck.lock();
    planSet.push_back(tempPlan);
    if(SHOW_DETAIL) {
        cout << "Core with id #" << coreId << " finished its task!" << endl;
    }
    record_lck.unlock();
    return;
}

vector<Car*> Simulator::replan(vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId, 
        vector<int> &delayCustomerId, float capacity) {
    // 重新计划，用于vehicle出发之后
    // 首先需要筛选出着急回复以及不着急回复的顾客
    // Args:
    //   * sampling: 是否使用sampling算法调整当前计划
    // Returns:
    //   * newServedCustomerId:  (wait customer中)通过replan接受到服务的顾客
    //   * newAbandonedCustomerId: (wait customer中)通过replan确定不能接收到服务的顾客
    //   * delayCustomer: 对于patient customer, 如果当前不能确认服务，则可在未来再为其服务
    ostringstream ostr;
    vector<Spot*> hurryCustomer;
    vector<Spot*> patientCustomer;
    vector<Spot*>::iterator custIter;
    mutex record_lck;              // 为planSet上锁
    vector<thread> thread_pool;    // pool for storing all threads
    int count = 0;
    // 下一个时间段的终止时间（下下个时间段的开始时间）
    float nextMoment = (slotIndex+1) * TIME_SLOT_LEN; 
    for(custIter = waitCustomerSet.begin(); custIter < waitCustomerSet.end(); custIter++) {
        if((*custIter)->tolerantTime <= nextMoment) {  
            // 该顾客着急于下时间段前得到回复
            hurryCustomer.push_back(*custIter);
        } else {
            // 否则，该顾客属于“有耐心的顾客”
            patientCustomer.push_back(*custIter);
        }
    }
    vector<Car*> newPlan;
    vector<Car*>::iterator carIter;
    float finalCost = 0;
    const int iterForWaitCustomer = 30000*ITER_PERCENTAGE;
    if(hurryCustomer.size() == 0) {  
        // 如果没有hurryCustomer，则不需要为其replan
        ostr.str("");
        ostr << "There are no hurry Customer" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        if(patientCustomer.size() != 0) {
            ostr.str("");
            ostr << "Replan for patient customer, the number is " << patientCustomer.size() << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            // 如果patientCustomer为空，则不需要对其进行replan
            vector<Spot*> copyPatientCustomer = copyCustomerSet(patientCustomer);
            SSLR alg(copyPatientCustomer, currentPlan, capacity, iterForWaitCustomer);
            alg.run(newPlan, finalCost, record_lck);
            if(checkFeasible(newPlan) == false) {
                throw out_of_range("The plan after insert patient customer is invalid!");
            }
            validPromise(newPlan, copyPatientCustomer, newServedCustomerId, 
                    delayCustomerId);
        } else {
            // 如果两个customerSet都为空，则直接复制currentPlan至newPlan
            ostr.str("");
            ostr << "There are no patient customer" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            newPlan = copyPlan(currentPlan);
        }
    } else {
        // 对hurryCustomer进行replan
        ostr.str("");
        ostr << "Replan for hurry customer, the number is " <<  hurryCustomer.size() << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        vector<Spot*> copyHurryCustomer = copyCustomerSet(hurryCustomer);
        SSLR alg(copyHurryCustomer, currentPlan, capacity, iterForWaitCustomer);
        alg.run(newPlan, finalCost, record_lck);
        if(checkFeasible(newPlan) == false) {
            throw out_of_range("The plan after insert hurry customer is invalid!");
        }
        validPromise(newPlan, copyHurryCustomer, newServedCustomerId, newAbandonedCustomerId);
        if(patientCustomer.size() != 0) {
            ostr.str("");
            ostr << "Replan for patient customer, the number is " << 
                patientCustomer.size() << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            // 如果patientCustomer不为空，则应该对其进行replan
            vector<Spot*> copyPatientCustomer = copyCustomerSet(patientCustomer);
            vector<Car*> tempPlan;
            SSLR alg2(copyPatientCustomer, newPlan, capacity, iterForWaitCustomer);
            alg2.run(tempPlan, finalCost, record_lck);
            newPlan = tempPlan;
            if(checkFeasible(newPlan) == false) {
                throw out_of_range("The plan after insert patient customer is invalid!");
            }
            validPromise(newPlan, copyPatientCustomer, newServedCustomerId, 
                    delayCustomerId);		
        } else {
            ostr.str("");
            ostr << "There are no patient customer" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
        }
    }
	
    ostr.str("");
    ostr << "There are newly " << newServedCustomerId.size() << " customers get service!" << endl;
    ostr << "There are still " << delayCustomerId.size() << " customers waiting for promise!" << endl;
    ostr << "There are newly " << newAbandonedCustomerId.size() << " customers rejected!" << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();
    
    // 检查newPlan，也就是即将执行SSLR算法的originPlan是否包含了所有promise customer
    if(checkFeasible(newPlan) == false) {
        throw out_of_range("The original plan has been infeasible!");
    }

    vector<Car*> outputPlan;
    // 首先得到"OK promise"的顾客的id，用于求解评分矩阵
    vector<int> allServedCustomerId;    // 所有得到了service promise的顾客id
    allServedCustomerId.push_back(0);   // 仓库节点是评分矩阵中的第一个节点
    vector<int>::iterator intIter;
    vector<int> promiseCustomerId = getCustomerID(promiseCustomerSet);
    allServedCustomerId.insert(allServedCustomerId.end(), promiseCustomerId.begin(),
            promiseCustomerId.end());
    allServedCustomerId.insert(allServedCustomerId.end(), newServedCustomerId.begin(), 
            newServedCustomerId.end());
    sort(allServedCustomerId.begin(), allServedCustomerId.end());
    if(SAMPLING) {
        // 然后进行采样，调用SSLR算法计算各个采样情景下的计划
        // 并且计算评分矩阵
        // 初始化transformMatrix
        Matrix<int> transformMatrix(allServedCustomerId.size(), allServedCustomerId.size());
        transformMatrix.setAll(0);
        vector<vector<Car*> > planSet;     // store all threads of all scenarios
        vector<vector<Car*> >::iterator planIter;
        ostr.str("");
        ostr << "----Sampling begins!" << endl;
        ostr << "----In replan, there will be " << dynamicCustomerSet.size() 
             << " dynamic customers" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();

        // 执行sampling
        int restSampleNum;
        if(DEBUG) {
            // DEBUG模式下只使用单线程
            restSampleNum = 1;
        } else {
            restSampleNum = SAMPLE_RATE;
        }
        while(restSampleNum > 0) {
            thread_pool.clear();
            thread_pool.resize(0);
            int coreId = SAMPLE_RATE - restSampleNum + 1;
            bool flag = true;
            for (int i = 0; i < min(CORE_NUM, restSampleNum); i++) {
                Spot *depot = newPlan[0]->getRoute()->getRearNode();
                vector<Spot*> sampleCustomer;
                if(SAMPLING) sampleCustomer = generateScenario(*depot); // 产生动态顾客到达的情景
                vector<Car*> temp = copyPlan(newPlan);
                thread_pool.push_back(thread(threadForReplan, capacity, coreId + i, 
                            ref(planSet), sampleCustomer, temp, allServedCustomerId, 
                            ref(transformMatrix), ref(record_lck), ref(flag)));
            }
            for (auto& thread : thread_pool) {
                thread.join();
            }
            if(flag == false) {
                throw out_of_range("Error in subthread!");
            }
            restSampleNum = restSampleNum - CORE_NUM;   
        }
       
        if(ASSESSMENT) {
            // 评价每个scenario的性能
            ostr.str("");
            ostr << "----Now assessing the performance of each scenario" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            vector<pair<int, int> > scoreForPlan;    // 每个计划的得分
            scoreForPlan.reserve(SAMPLE_RATE);
            try {
                for(planIter = planSet.begin(); planIter < planSet.end(); planIter++) {
                    int pos = planIter - planSet.begin();    // 在采样得到的计划中的位置
                    int score = 0;
                    for(carIter = planIter->begin(); carIter < planIter->end(); carIter++) {
                        score += (*carIter)->computeScore(transformMatrix);
                    }
                    scoreForPlan.push_back(make_pair(score, pos));
                }
            } catch (exception &e) {
                throw out_of_range(e.what());
            }
            cout << "Assessment finished, its score is: " << scoreForPlan[0].first << endl;
            sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
            outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
        } else {
            // 如果不采用assessment，则随机选取一个plan
            if(DEBUG) outputPlan = copyPlan(planSet[0]);
            else {
                int randomSelection = int(random(0, SAMPLE_RATE));
                randomSelection = min(randomSelection, SAMPLE_RATE-1);
                outputPlan = copyPlan(planSet[randomSelection]);
            }
        }
        clearPlanSet(planSet);
    } else {
        // 如果不进行采样
        vector<Spot*> tempCustomer;
        vector<Car*> tempPlan = copyPlan(newPlan);
        float finalCost = 0;
        SSLR alg(tempCustomer, tempPlan, capacity, 10000*ITER_PERCENTAGE, DEBUG);
        alg.run(outputPlan, finalCost, record_lck);
    }
    
    if(checkFeasible(outputPlan) == false) {
        throw out_of_range("The output plan is invalid!");
    }

    withdrawPlan(newPlan);
    return outputPlan;
}

vector<Car*> Simulator::no_replan() {
    // 不采取任何replan措施
    ostringstream ostr;
    ostr.str("");
    ostr << "We don't apply replan this time!!" << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();
    return currentPlan;
}
