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

Simulator::Simulator(int slotIndex, vector<Customer*> promiseCustomerSet, 
        vector<Customer*> waitCustomerSet, vector<Customer*> dynamicCustomerSet, 
        vector<Car*> currentPlan) { 
    // 构造函数
    this->slotIndex = slotIndex;
    vector<Customer*>::iterator custIter;
    this->promiseCustomerSet.reserve(promiseCustomerSet.end() - promiseCustomerSet.begin());
    this->promiseCustomerSet = copyCustomerSet(promiseCustomerSet);
	
    this->waitCustomerSet.reserve(waitCustomerSet.end() - waitCustomerSet.begin());
    this->waitCustomerSet = copyCustomerSet(waitCustomerSet);

    this->dynamicCustomerSet.reserve(dynamicCustomerSet.end() - dynamicCustomerSet.begin());
    this->dynamicCustomerSet = copyCustomerSet(dynamicCustomerSet);
	
    this->currentPlan.reserve(currentPlan.size());
    this->currentPlan = copyPlan(currentPlan);
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

vector<Customer*> Simulator::generateScenario(Customer depot){
    // 产生情景
    // 根据动态顾客的随机信息产生其时间窗
    // 注意动态顾客只可能出现在slotIndex之后
    int leftAlpha, rightAlpha;
    switch(STRATEGY) {
        case Negative: {
            leftAlpha = max(1.0, 0.5*ALPHA);
            rightAlpha = ALPHA;
            break;
        }
        case Positive: {
            leftAlpha = ALPHA;
            rightAlpha = 2*ALPHA;
            break;
        }
    }
    vector<Customer*> tempCustomer = copyCustomerSet(dynamicCustomerSet);
    vector<Customer*>::iterator iter = tempCustomer.begin();
    for(iter; iter<tempCustomer.end(); iter++){
        // 产生随机数选择顾客可能提出需求的时间
        float randomAlpha = random(leftAlpha, rightAlpha);
        float randFloat = random(0,1);
        float sumation = 0;
        // 时间段计数
        int count = roulette((*iter)->timeProb + slotIndex, TIME_SLOT_NUM - slotIndex);
        float t1 = (count+slotIndex) * TIME_SLOT_LEN;
        float t2 = (count+slotIndex+1) * TIME_SLOT_LEN;
        float tempt = random(t1, t2);
        // 时间轴长度
        float maxTime = TIME_SLOT_NUM * TIME_SLOT_LEN;
        float minTimeWindowLen = dist(&depot, *iter);
        (*iter)->startTime = min(tempt, maxTime - randomAlpha * minTimeWindowLen);
        (*iter)->endTime = random((*iter)->startTime + randomAlpha*minTimeWindowLen,
                maxTime);
    }
    return tempCustomer;
}

bool Simulator::checkFeasible(vector<Car*> carSet) {
    // 检查carSet中是否包括了所有的promiseCustomer
    vector<int> tempId = getID(promiseCustomerSet); // promise Customer的id
    sort(tempId.begin(), tempId.end());
    vector<Car*>::iterator carIter;
    for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
        vector<Customer*> tempCust = (*carIter)->getRoute().getAllCustomer();
        vector<Customer*>::iterator custIter;
        for(custIter=tempCust.begin(); custIter<tempCust.end(); custIter++) {
            vector<int>::iterator intIter = find(tempId.begin(), tempId.end(), (*custIter)->id);
            if(intIter < tempId.end()) {
                // 如果找到，就从tempId中删除
                tempId.erase(intIter);
            }
        }
    }
    if(tempId.size() != 0) {
        return false;
    } else {
        return true;
    }
}

void threadForInitial(Customer depot, float capacity, int coreId, vector<vector<Car*> > &planSet, 
        vector<Customer*> allCustomer, vector<int> validId, Matrix<int> &transformMatrix, 
        mutex &record_lck, bool &flag) {
    // 路径初始化的线程操作
    // Args:
    //   * depot: 仓库节点
    //   * capacity: 车容量
    //   * coreId: 本线程的id
    //   * allCustomer: 初始的所有顾客节点
    //   * validId: 得到了service promise的顾客id
    //   * record_lck: 锁住planSet变量，以向其写入数据
    // Returns:
    //   * planSet: 新增当前情景解到其中
    //   * transformMatrix: 获得service promise的顾客间的转移频率
    ALNS alg(allCustomer, depot, capacity, 10000*ITER_PERCENTAGE);
    vector<Car*> solution(0);
    float cost = 0;
    try{
        alg.run(solution, cost);
    } catch (exception &e) {
        record_lck.lock();
        cout << e.what() << endl;
        flag = false;
        record_lck.unlock();
        return;
    }
    vector<Car*>::iterator carIter;
    int totalRetainNum = 0;   // 看看就听过removeInvalidCustomer后还有多少剩余节点
    for(carIter = solution.begin(); carIter < solution.end(); carIter++) {
        (*carIter)->removeInvalidCustomer(validId, totalRetainNum);
        (*carIter)->updateTransformMatrix(transformMatrix);
    }
    if(validId.size() != totalRetainNum+1) {
        record_lck.lock();
        cout << "Miss some customers after remove invalid customers!" << endl;
        flag = false;
        record_lck.unlock();
        return;
    }
    record_lck.lock();
    planSet.push_back(solution);
    if(SHOW_DETAIL) cout << "The core with id #" << coreId << " finished its task" << endl;
    record_lck.unlock();
    return;
}

void retainVehicles(vector<Car*> &carSet, Customer depot, float capacity) {
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
            if((*carIter)->getRoute().getSize() == 0) {
                delete(*carIter);
                carIter = carSet.erase(carIter);
                cutNum--;
            } else {
                carIter++;
            }
        }
        if(cutNum > 0) {
            throw out_of_range("Cannot find so many vehicles to dismiss");
        } else {
            for(int i=0; i<carSet.size(); i++) {
                carSet[i]->changeCarIndex(i);
            }
        }
    }
}

vector<Car*> Simulator::initialPlan(Customer depot, float capacity){     
    // 利用采样制定初始计划
    ostringstream ostr;
    vector<int>::iterator intIter;
    vector<Car*>::iterator carIter;
    vector<Car*> outputPlan;
    if(SAMPLING) {
        // 顾客集按照id大小进行排序
        sort(promiseCustomerSet.begin(), promiseCustomerSet.end());
        vector<int> validId;
        validId.push_back(0);   // 第一个节点时仓库节点
        // 所有在计划开始前已知的顾客id（属于必须服务的顾客）
        vector<int> tempId = getID(promiseCustomerSet);
        for(intIter = tempId.begin(); intIter < tempId.end(); intIter++) {
            validId.push_back(*intIter);
        }
        // 初始化transformMatrix
        Matrix<int> transformMatrix(validId.size(), validId.size());
        transformMatrix.setAll(0);
        // 所有采样得到的计划
        // 对所有的情景运行ALNS算法，并且把解放入planSet中
        // 在此过程中将根据validId对所有的解，仅保留id在validId中的顾客节点
        vector<Customer*>::iterator iter;
        ostr.str("");
        ostr << "----Sampling begins!" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        
        vector<vector<Car*> > planSet;
        mutex record_lck;    // 锁住planSet
        vector<thread> thread_pool;   // a pool to capitalize thread

        int restSampleNum = SAMPLE_RATE;       // 尚未跑完的样本
        bool flag = true;
        while(restSampleNum > 0) {
            // coreId: 线程id，从0开始
            int coreId = SAMPLE_RATE - restSampleNum + 1; 
            for(int i=0; i<min(CORE_NUM,restSampleNum); i++) {
                // 所有顾客信息
                vector<Customer*> allCustomer = copyCustomerSet(promiseCustomerSet);
                vector<Customer*> currentDynamicCust = generateScenario(depot);  // 采样
                iter = currentDynamicCust.begin();
                for(iter; iter<currentDynamicCust.end(); iter++){
                    allCustomer.push_back(*iter);
                }
                thread_pool.push_back(thread(threadForInitial, depot, capacity, coreId + i, 
                            ref(planSet), allCustomer, validId, ref(transformMatrix), 
                            ref(record_lck), ref(flag)));
            }
            for(auto& thread:thread_pool) {
                thread.join();
            }
            if(flag == false) {
                throw out_of_range("Problem in subthread");
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
            cout << "Assessment finished!!" << endl;
            sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
            outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
        } else {
            // 如果不采用assessment，则随机选取一个plan
            int randomSelection = int(random(0, SAMPLE_RATE));
            randomSelection = min(randomSelection, SAMPLE_RATE-1);
            outputPlan = copyPlan(planSet[randomSelection]);
        }
        cout << "Refreshed the outputPlan!" << endl;
        clearPlanSet(planSet);
    } else {
        // 不采用sampling方案
        vector<Customer*> allCustomer = copyCustomerSet(promiseCustomerSet);
        ALNS alg(allCustomer, depot, capacity, 10000*ITER_PERCENTAGE);
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
    deleteCustomerSet(waitCustomerSet);
    deleteCustomerSet(promiseCustomerSet);
    withdrawPlan(currentPlan);
    return outputPlan;
}

void validPromise(vector<Car*>Plan, vector<Customer*> hurryCustomer, 
        vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId){
    // 对hurry customer确认promise
    // 给出"accept" 或者 "reject" 回应
    vector<Car*>::iterator carIter;
    vector<Customer*>::iterator custIter;
    // hurry customer的id
    vector<int> hurryCustomerId = getID(hurryCustomer);
    sort(hurryCustomerId.begin(), hurryCustomerId.end());
    int i;
    for(carIter = Plan.begin(); carIter < Plan.end(); carIter++){
        vector<Customer*> tempCust = (*carIter)->getAllCustomer();
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
        deleteCustomerSet(tempCust);
    }
    // 得到放弃的顾客id
    vector<int>::iterator intIter;
    for(intIter = hurryCustomerId.begin(); intIter < hurryCustomerId.end(); intIter++) {
        newAbandonedCustomerId.push_back(*intIter);
    }
}

void threadForReplan(float capacity, int coreId, vector<vector<Car*>> &planSet, 
        vector<Customer*> sampleCustomer, vector<Car*> currentPlan, vector<int> validId,
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
    SSLR alg(sampleCustomer, currentPlan, capacity, 10000*ITER_PERCENTAGE);
    try{
        alg.run(tempPlan, finalCost, record_lck);
    } catch (exception &e) {
        record_lck.lock();
        cout << e.what() << endl;
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
        cout << "Miss some customers after remove invalid customers!" << endl;
        flag = false;
        record_lck.unlock();
        return;
    }

    record_lck.lock();
    if(SHOW_DETAIL) cout << "Core with id #" << coreId << " finished its task!" << endl;
    planSet.push_back(tempPlan);
    record_lck.unlock();
    return;
}

vector<Car*> Simulator::replan(vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId, vector<int> &delayCustomerId, float capacity) {
    // 重新计划，用于vehicle出发之后
    // 首先需要筛选出着急回复以及不着急回复的顾客
    // Returns:
    //   * newServedCustomerId:  (wait customer中)通过replan接受到服务的顾客
    //   * newAbandonedCustomerId: (wait customer中)通过replan确定不能接收到服务的顾客
    //   * delayCustomer: 对于patient customer, 如果当前不能确认服务，则可在未来再为其服务
    ostringstream ostr;
    vector<Customer*> hurryCustomer;
    vector<Customer*> patientCustomer;
    vector<Customer*>::iterator custIter;
    mutex record_lck;              // 为planSet上锁
    vector<thread> thread_pool;    // pool for storing all threads
    int count = 0;
    // 下一个时间段的终止时间（下下个时间段的开始时间）
    float nextMoment = (slotIndex+1) * TIME_SLOT_LEN; 
    for(custIter = waitCustomerSet.begin(); custIter < waitCustomerSet.end(); custIter++) {
        if((*custIter)->tolerantTime <= nextMoment) {  
            // 该顾客着急于下时间段前得到回复
            Customer *tempCust = new Customer(**custIter);
            hurryCustomer.push_back(tempCust);
        } else {
            // 否则，该顾客属于“有耐心的顾客”
            Customer *tempCust = new Customer(**custIter);
            patientCustomer.push_back(tempCust);
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
            SSLR alg(patientCustomer, currentPlan, capacity, iterForWaitCustomer);
            alg.run(newPlan, finalCost, record_lck);
            if(checkFeasible(newPlan) == false) {
                throw out_of_range("The plan after insert patient customer is invalid!");
            }
            validPromise(newPlan, patientCustomer, newServedCustomerId, delayCustomerId);
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
        SSLR alg(hurryCustomer, currentPlan, capacity, iterForWaitCustomer);
        alg.run(newPlan, finalCost, record_lck);
        if(checkFeasible(newPlan) == false) {
            throw out_of_range("The plan after insert hurry customer is invalid!");
        }
        validPromise(newPlan, hurryCustomer, newServedCustomerId, newAbandonedCustomerId);
        if(patientCustomer.size() != 0) {
            ostr.str("");
            ostr << "Replan for patient customer, the number is " << patientCustomer.size() << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            // 如果patientCustomer不为空，则应该对其进行replan
            SSLR alg2(patientCustomer, newPlan, capacity, iterForWaitCustomer);
            alg2.run(newPlan, finalCost, record_lck);
            if(checkFeasible(newPlan) == false) {
                throw out_of_range("The plan after insert patient customer is invalid!");
            }
            validPromise(newPlan, patientCustomer, newServedCustomerId, delayCustomerId);		
        } else {
            ostr.str("");
            ostr << "There are no patient customer" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
        }
    }

    deleteCustomerSet(hurryCustomer);
    deleteCustomerSet(patientCustomer);
	
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

    // 首先得到"OK promise"的顾客的id，用于求解评分矩阵
    vector<int> allServedCustomerId;    // 所有得到了service promise的顾客id
    allServedCustomerId.push_back(0);   // 仓库节点是评分矩阵中的第一个节点
    vector<int>::iterator intIter;
    vector<int> promiseCustomerId = getID(promiseCustomerSet);
    for(intIter = promiseCustomerId.begin(); intIter < promiseCustomerId.end(); intIter++) {
        allServedCustomerId.push_back(*intIter);
    }
    for(intIter = newServedCustomerId.begin(); intIter < newServedCustomerId.end(); intIter++) {
        allServedCustomerId.push_back(*intIter);
    }
    sort(allServedCustomerId.begin(), allServedCustomerId.end());
    vector<Car*> outputPlan;
    if(SAMPLING) {
        // 然后进行采样，调用SSLR算法计算各个采样情景下的计划
        // 并且计算评分矩阵
        // 初始化transformMatrix
        Matrix<int> transformMatrix(allServedCustomerId.size(), allServedCustomerId.size());
        transformMatrix.setAll(0);
        vector<vector<Car*> > planSet;     // store all threads of all scenarios
        planSet.reserve(SAMPLE_RATE);
        vector<vector<Car*> >::iterator planIter;
        ostr.str("");
        ostr << "----Sampling begins!" << endl;
        ostr << "----In replan, there will be " << dynamicCustomerSet.size() 
             << " dynamic customers" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();

        // 执行sampling
        int restSampleNum = SAMPLE_RATE;
        bool flag = true;
        while(restSampleNum > 0) {
            thread_pool.clear();
            thread_pool.resize(0);
            int coreId = SAMPLE_RATE - restSampleNum + 1;
            for (int i = 0; i < min(CORE_NUM, restSampleNum); i++) {
                Customer depot = newPlan[0]->getRearNode();
                vector<Customer*> sampleCustomer = generateScenario(depot); // 产生动态顾客到达的情景
                thread_pool.push_back(thread(threadForReplan, capacity, coreId + i, 
                            ref(planSet), sampleCustomer, ref(newPlan), allServedCustomerId, 
                            ref(transformMatrix), ref(record_lck), ref(flag)));
            }
            for (auto& thread : thread_pool) {
                thread.join();
            }
            if(!flag) {
                throw out_of_range("Problem in subthread!");
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
                    int pos = planIter - planSet.begin();             // 在采样得到的计划中的位置
                    int score = 0;
                    for(carIter = planIter->begin(); carIter < planIter->end(); carIter++) {
                        score += (*carIter)->computeScore(transformMatrix);
                    }
                    scoreForPlan.push_back(make_pair(score, pos));
                }
            } catch (exception &e) {
                throw out_of_range(e.what());
            }
            sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
            cout << "Assessment finished, its score is: " << scoreForPlan[0].first << endl;
            outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
        } else {
            // 如果不采用assessment，则随机选取一个plan
            int randomSelection = int(random(0, SAMPLE_RATE));
            randomSelection = min(randomSelection, SAMPLE_RATE-1);
            outputPlan = copyPlan(planSet[randomSelection]);
        }
        clearPlanSet(planSet);
    } else {
        // 如果不进行采样
        cout << "Not use sampling in replan..." << endl;
        vector<Customer*> tempCustomer;
        vector<Car*> tempPlan = copyPlan(newPlan);
        float finalCost = 0;
        SSLR alg(tempCustomer, tempPlan, capacity, 10000*ITER_PERCENTAGE);
        alg.run(outputPlan, finalCost, record_lck);
    }
    if(checkFeasible(outputPlan) == false) {
        throw out_of_range("The output plan is invalid!");
    }

    deleteCustomerSet(waitCustomerSet);
    deleteCustomerSet(promiseCustomerSet);
    deleteCustomerSet(dynamicCustomerSet);
    withdrawPlan(currentPlan);
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
