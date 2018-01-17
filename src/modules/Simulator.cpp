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
        vector<Car*> currentPlan) { 
    // 构造函数
    this->slotIndex = slotIndex;
    vector<Spot*>::iterator custIter;
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

vector<Spot*> Simulator::generateScenario(){
    // 产生情景
    // 根据动态顾客的随机信息产生其时间窗
    // 注意动态顾客只可能出现在slotIndex之后
    int leftBound = 0;
    int rightBound = 0;
    switch(STRATEGY) {
        case Negative: {
            leftBound = 1;
            rightBound = 4;
            break;
        }
        case Positive: {
            leftBound = 5;
            rightBound = 15;
            break;
        }
    }
    vector<Spot*> tempCustomer = copyCustomerSet(dynamicCustomerSet);
    vector<Spot*>::iterator iter = tempCustomer.begin();
    for(iter; iter<tempCustomer.end(); iter++){
        // 产生随机数选择顾客可能提出需求的时间
        float randFloat = random(0,1);
        float sumation = 0;
        // 时间段计数
        int count = roulette((*iter)->timeProb + slotIndex, TIME_SLOT_NUM - slotIndex);
        float t1 = (count+slotIndex) * TIME_SLOT_LEN;
        float t2 = (count+slotIndex+1) * TIME_SLOT_LEN;
        float tempt = random(t1, t2);
        // 时间轴长度
        float maxTime = TIME_SLOT_NUM * TIME_SLOT_LEN;
        (*iter)->startTime = min(tempt, maxTime - 5*(*iter)->serviceTime);
        float t3 = leftBound*(*iter)->serviceTime;
        float t4 = rightBound*(*iter)->serviceTime;  
        float timeWindowLen = random(t3, t4);       
        (*iter)->endTime = min((*iter)->startTime + timeWindowLen, maxTime);
    }
    return tempCustomer;
}

bool Simulator::checkFeasible(vector<Car*> carSet) {
    // 检查carSet中是否包括了所有的promiseCustomer
    vector<int> tempId = getID(promiseCustomerSet); // promise Customer的id
    sort(tempId.begin(), tempId.end());
    vector<Car*>::iterator carIter;
    for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
        vector<Spot*> tempCust = (*carIter)->getRoute().getAllCustomer();
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
        return false;
    } else {
        return true;
    }
}

void threadForInitial(Spot depot, float capacity, int coreId, vector<vector<Car*> > &planSet, 
        vector<Spot*> allCustomer, vector<int> validId, Matrix<int> &transformMatrix, 
        mutex &record_lck) {
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
    alg.run(solution, cost);
    vector<Car*>::iterator carIter;
    int totalRetainNum = 0;   // 看看就听过removeInvalidCustomer后还有多少剩余节点
    for(carIter = solution.begin(); carIter < solution.end(); carIter++) {
        (*carIter)->removeInvalidCustomer(validId, totalRetainNum);
        (*carIter)->updateTransformMatrix(transformMatrix);
    }
    record_lck.lock();
    try {
        if(validId.size() != totalRetainNum+1) {
            throw out_of_range("Miss some customers after remove invalid customers!");
        }
    } catch (exception &e) {
        cerr << "valid id size: " << validId.size()-1 << ";real size: " << totalRetainNum << endl;
        cerr << e.what() << endl;
        exit(1);
    }
    planSet.push_back(solution);
    cout << "The core with id #" << coreId << " finished its task" << endl;
    record_lck.unlock();
}

vector<Car*> Simulator::initialPlan(Spot depot, float capacity){     
    // 利用采样制定初始计划
    ostringstream ostr;
    vector<int>::iterator intIter;
    vector<Car*>::iterator carIter;
    mutex record_lck;    // 锁住planSet
    vector<thread> thread_pool;   // a pool to capitalize thread
    int i,j;
    int count = 0;
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
    for(i=0; i<validId.size(); i++) {
        for(j=0; j<validId.size(); j++) {
            transformMatrix.setValue(i,j,0);
        }       
    }
    // 所有采样得到的计划
    vector<vector<Car*> > planSet;
    planSet.reserve(SAMPLE_RATE);
    vector<vector<Car*> >::iterator planIter = planSet.begin();
    // 对所有的情景运行ALNS算法，并且把解放入planSet中
    // 在此过程中将根据validId对所有的解，仅保留id在validId中的顾客节点
    vector<Spot*>::iterator iter;
    ostr.str("");
    ostr << "----Sampling begins!" << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();

    int restSampleNum = SAMPLE_RATE;       // 尚未跑完的样本
    while(restSampleNum > 0) {
        // coreId: 线程id，从0开始
        int coreId = SAMPLE_RATE - restSampleNum + 1; 
        for(int i=0; i<min(CORE_NUM,restSampleNum); i++) {
            // 所有顾客信息
            vector<Spot*> allCustomer = copyCustomerSet(promiseCustomerSet);
            vector<Spot*> currentDynamicCust = generateScenario();  // 采样
            iter = currentDynamicCust.begin();
            for(iter; iter<currentDynamicCust.end(); iter++){
                allCustomer.push_back(*iter);
            }
            thread_pool.push_back(thread(threadForInitial, depot, capacity, coreId + i, 
                        ref(planSet), allCustomer, validId, ref(transformMatrix), ref(record_lck)));
        }
        for(auto& thread:thread_pool) {
            thread.join();
        }
        restSampleNum = restSampleNum - CORE_NUM;
        thread_pool.clear();
        thread_pool.resize(0);
    }

    // 然后对所有情景下的计划进行评分，取得分最高者作为初始路径计划
    ostr.str("");
    ostr << "----Now assessing the performance of each scenario" << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();
    vector<pair<int, int> > scoreForPlan;    // 每个计划的得分
    scoreForPlan.reserve(SAMPLE_RATE);
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
        cerr << e.what() << endl;
        exit(1);
    }
    cout << "Assessment finished!!" << endl;
    sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
    vector<Car*> outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
    cout << "Refreshed the outputPlan!" << endl;
    clearPlanSet(planSet);
    deleteCustomerSet(waitCustomerSet);
    deleteCustomerSet(promiseCustomerSet);
    withdrawPlan(currentPlan);
    return outputPlan;
}

void validPromise(vector<Car*>Plan, vector<Spot*> hurryCustomer, 
        vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId){
    // 对hurry customer确认promise
    // 给出"accept" 或者 "reject" 回应
    vector<Car*>::iterator carIter;
    vector<Spot*>::iterator custIter;
    // hurry customer的id
    vector<int> hurryCustomerId = getID(hurryCustomer);
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
        deleteCustomerSet(tempCust);
    }
    // 得到放弃的顾客id
    vector<int>::iterator intIter;
    for(intIter = hurryCustomerId.begin(); intIter < hurryCustomerId.end(); intIter++) {
        newAbandonedCustomerId.push_back(*intIter);
    }
}

void threadForReplan(float capacity, int coreId, vector<vector<Car*>> &planSet, 
        vector<Spot*> sampleCustomer, vector<Car*> currentPlan, vector<int> validId,
        Matrix<int> &transformMatrix, mutex &record_lck) {
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
    SSLR alg(sampleCustomer, currentPlan, capacity);
    alg.run(tempPlan, finalCost, record_lck);
    vector<Car*>::iterator carIter;
    int totalRetainNum = 0;
    for(carIter = tempPlan.begin(); carIter < tempPlan.end(); carIter++) {
        (*carIter)->removeInvalidCustomer(validId, totalRetainNum);
        (*carIter)->updateTransformMatrix(transformMatrix);
    }
    record_lck.lock();
    // unique_lock<mutex> lck1(record_lck);
    try {
        if(validId.size() != totalRetainNum+1) {
            throw out_of_range("Miss some customers after remove invalid customers!");
        }
    } catch (exception &e) {
        cerr << "valid id size: " << validId.size()-1 << ";real size: " << totalRetainNum << endl;
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "Core with id #" << coreId << " finished its task!" << endl;
    planSet.push_back(tempPlan);
    record_lck.unlock();
}

vector<Car*> Simulator::replan(vector<int> &newServedCustomerId, vector<int> &newAbandonedCustomerId, vector<int> &delayCustomerId, float capacity) {
    // 重新计划，用于vehicle出发之后
    // 首先需要筛选出着急回复以及不着急回复的顾客
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
            Spot *tempCust = new Spot(**custIter);
            hurryCustomer.push_back(tempCust);
        } else {
            // 否则，该顾客属于“有耐心的顾客”
            Spot *tempCust = new Customer(**custIter);
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
            try {
                if(checkFeasible(newPlan) == false) {
                    throw out_of_range("The plan after insert patient customer is invalid!");
                }
            } catch (exception &e) {
                cerr << e.what() << endl;
                exit(1);
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
        try {
            if(checkFeasible(newPlan) == false) {
                throw out_of_range("The plan after insert hurry customer is invalid!");
            }
        } catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
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
            try {
                if(checkFeasible(newPlan) == false) {
                    throw out_of_range("The plan after insert patient customer is invalid!");
                }
            } catch (exception &e) {
                cerr << e.what() << endl;
                exit(1);
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
    try {
        if(checkFeasible(newPlan) == false) {
            throw out_of_range("The original plan has been infeasible!");
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        exit(1);
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
    // 然后进行采样，调用SSLR算法计算各个采样情景下的计划
    // 并且计算评分矩阵
    // 初始化transformMatrix
    Matrix<int> transformMatrix(allServedCustomerId.size(), allServedCustomerId.size());
    for(int i=0; i<allServedCustomerId.size(); i++) {
        for(int j=0; j<allServedCustomerId.size(); j++) {
            transformMatrix.setValue(i,j,0);
        }
    }
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
    while(restSampleNum > 0) {
        thread_pool.clear();
        thread_pool.resize(0);
        int coreId = SAMPLE_RATE - restSampleNum + 1;
        for (int i = 0; i < min(CORE_NUM, restSampleNum); i++) {
            vector<Spot*> sampleCustomer = generateScenario(); // 产生动态顾客到达的情景
            thread_pool.push_back(thread(threadForReplan, capacity, coreId + i, 
                        ref(planSet), sampleCustomer, ref(newPlan), allServedCustomerId, 
                        ref(transformMatrix), ref(record_lck)));
        }
        for (auto& thread : thread_pool) {
            thread.join();
        }
        restSampleNum = restSampleNum - CORE_NUM;   
    }

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
        cerr << e.what() << endl;
        exit(1);
    }
    sort(scoreForPlan.begin(), scoreForPlan.end(), descendSort<int, int>);
    vector<Car*> outputPlan = copyPlan(planSet[scoreForPlan[0].second]);
    try {
        if(checkFeasible(outputPlan) == false) {
            throw out_of_range("The output plan is invalid!");
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
    clearPlanSet(planSet);
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
