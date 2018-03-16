#include "SSLR.h"
#include "../baseclass/Matrix.h"
#include "../run/TxtRecorder.h"
#include<cmath>
#include<stdexcept>
#include<algorithm>
#include<cassert>
#include<functional>

using namespace std;

float RANDOM_RANGE_SSLR[2] = {0, 1};

vector<Spot*> feedDataForLNSBase(vector<Spot*> waitCustomer, vector<Car*> originPlan) {
    // 返回allCustomer，其中对waitCustomer优先级赋值为2，对originPlan的顾客优先级赋值为1
    vector<Spot*>::iterator custPtr;
    vector<Spot*> lowPriorityCust;
    for(custPtr = waitCustomer.begin(); custPtr < waitCustomer.end(); custPtr++){
        (*custPtr)->priority = 2;
        lowPriorityCust.push_back(*custPtr);
    }
    vector<Car*>::iterator carPtr;
    vector<Spot*> highPriorityCust;
    for(carPtr = originPlan.begin(); carPtr < originPlan.end(); carPtr++) {
        vector<Spot*> custVec = (*carPtr)->getAllCustomer();
        for(custPtr = custVec.begin(); custPtr < custVec.end(); custPtr++) {
            (*custPtr)->priority = 1;
            highPriorityCust.push_back(*custPtr);
        }
    }
    vector<Spot*> output = mergeCustomer(lowPriorityCust, highPriorityCust);
    return output;
}

SSLR::SSLR(vector<Spot*> waitCustomer, vector<Car*> originPlan, float capacity, int maxIter,
        bool verbose, int pshaw, int pworst, float eta): LNSBase(pshaw, 
        pworst, eta, capacity, RANDOM_RANGE_SSLR, feedDataForLNSBase(waitCustomer, 
        originPlan), *originPlan[0]->getRearNode(), true, true) 
{
    this->maxIter = maxIter;
    this->verbose = verbose;
    vector<Spot*>::iterator custIter;
    // 对waitCustomer，其优先级已经在基类的初始化中赋值为2
    this->waitCustomer = waitCustomer;
    vector<Car*>::iterator carIter;
    // 对于原本就在路径中的节点，其优先级已经在基类的初始化中赋值为1
    this->originPlan = originPlan;
}

SSLR::~SSLR() {}

bool judgeFeasible(vector<Car*> carSet, vector<Car*> refCarSet, int &infeasibleNum) {
	// 判断carSet是否可行
    // 主要判断refCarSet中的顾客是否都在carSet的working vehicle中
    infeasibleNum = 0;
    bool mark = true;
    vector<Car*>::iterator carIter;
    vector<int> refIDs = getCustomerID(refCarSet);
    vector<int>::iterator iter, iter1;
    sort(refIDs.begin(), refIDs.end());
    vector<Car*> workingCarSet;
    for(carIter = carSet.begin(); carIter < carSet.end(); carIter++) {
        if((*carIter)->judgeArtificial() == false) {
            workingCarSet.push_back(*carIter);
        }
    }
    vector<int> currentIDs = getCustomerID(workingCarSet);
    for(iter = currentIDs.begin(); iter < currentIDs.end(); iter++) {
        iter1 = find(refIDs.begin(), refIDs.end(), *iter);
        if(iter1 < refIDs.end()) {
            refIDs.erase(iter1);
        }
        sort(refIDs.begin(), refIDs.end());
    }
    infeasibleNum = refIDs.size();
    mark = (infeasibleNum==0);
    return mark;
}

float* computeDTpara(vector<Spot*> allCustomer, vector<Spot*> waitCustomer, Spot depot,
        float maxd, float mind){
    // 计算对不同优先级顾客的奖惩系数
    // Args:
    //   * maxd: 所有顾客之间的最大距离
    //   * allCustomer: 所有顾客节点（包括所有不同优先级顾客）
    //   * waitCustomer: 低优先级顾客
    int PR2Num = (int)waitCustomer.size();
    int PR1Num = (int)allCustomer.size() - PR2Num;
    vector<Spot*>::iterator custPtr;
    float DTH1, DTH2, DTL1, DTL2;
    float distToDepot = 0;    // 各个顾客节点到仓库的距离
    for(custPtr = allCustomer.begin(); custPtr < allCustomer.end(); custPtr++) {
        distToDepot += dist(*custPtr, &depot);
    }
    DTL2 = 50;
    DTL1 = 4*maxd + 1;
    DTH2 = 80;
    float tempsigma1 = 4*maxd + DTH2;
    //float tempsigma2 = 2*(PR1NUM + PR2NUM + PR3NUM) * maxd + PR2NUM * DT22 + PR3NUM * DT32 - 
    //	(PR1NUM + PR2NUM + PR3NUM) * mind + PR2NUM * DT21 + PR3NUM * DT31 - DT12;
    float tempsigma2 = 4*distToDepot - DTH2 + PR2Num * (DTL1 + DTL2) - 
        (PR1Num + PR2Num + 1) * mind * 2;
    DTH1 = max(tempsigma1, tempsigma2) + 1;
    float *DTpara = new float[4];
    DTpara[0] = DTH1;
    DTpara[1] = DTH2;
    DTpara[2] = DTL1;
    DTpara[3] = DTL2;
    //cout << "DTH1: " << DTpara[0] << " DTH2: " << DTpara[1] << " DTL1: " <<
    //    DTpara[2] << " DTL2:" << DTpara[3] << endl;
    return DTpara;
}

void SSLR::run(vector<Car*> &finalCarSet, float &finalCost, mutex &print_lck){  
    // 运行算法，相当于算法的main()函数
    int i;
    int customerTotalNum = (int)allCustomer.size();  // 总的顾客数
    int originCarNum = (int)originPlan.size();   // 初始拥有的货车数量
    vector<Spot*>::iterator custPtr;
    vector<Car*>::iterator carIter;

    // 如果计划中没有顾客，则抛出warning
    if (customerTotalNum == 0) {                                 
        cout << "WARNNING: In replan, but no customers!" << endl;            
    }                                                              

    // 计算对不同优先级顾客的奖惩系数
    float *DTpara = computeDTpara(allCustomer, waitCustomer, depot, maxd, mind);
    resetDTpara(DTpara);
	
    // 构造base solution
    vector<Car*> baseCarSet = copyPlan(originPlan);
    if (waitCustomer.size() != 0) {       
        // 只有当waitCustomer不为空时才有"replan"的价值
        // 使用virtual car去装载waitCustomer
        vector<Car*> tempCarSet1;
        Car *tcar = new Car(depot, depot, capacity, 100, true);
        tempCarSet1.push_back(tcar);
        vector<Spot*> copyWaitCustomer = copyCustomerSet(waitCustomer);
        greedyInsert(tempCarSet1, copyWaitCustomer, false);
        for (carIter = tempCarSet1.begin(); carIter < tempCarSet1.end(); carIter++) {
            baseCarSet.push_back(*carIter);
        }
    }
    // 基准代价，如果得到的解优于这个解，则一定可行
    // 一般来说比这个解更差的解时不可行的
    float baseCost = getCost(baseCarSet);   
	
    // 构造初始全局最优解
    vector<Car*> currentCarSet(0);
    vector<Spot*> currentCustomer(0);
    for(carIter = originPlan.begin(); carIter < originPlan.end(); carIter++) {
        // 保留原有的车辆，记录其起点以及终点以及剩余容量、基准时间
        vector<Spot*> temp;
        Car *newCar = new Car(*((*carIter)->getNullCar(temp)));
        currentCustomer.insert(currentCustomer.end(), temp.begin(), temp.end());
        currentCarSet.push_back(newCar);
    }
    vector<Spot*> copyWaitCustomer = copyCustomerSet(waitCustomer);
    currentCustomer.insert(currentCustomer.end(), copyWaitCustomer.begin(), copyWaitCustomer.end());
    // 以当前所拥有的working car为基础，构造初始解（完全重新构造）
    regretInsert(currentCarSet, currentCustomer, false);  
    // 全局最优解，初始化与当前解相同
    vector<Car*> globalCarSet = copyPlan(currentCarSet);        
    float currentCost = getCost(currentCarSet);
    float globalCost = currentCost;

    vector<size_t> hashTable(0);  // 哈希表
    hashTable.push_back(codeForSolution(currentCarSet));

    // 评分机制相关参数的设定
    const int removeNum = 3;    // remove heuristic的个数
    const int insertNum = 2;    // insert heuristic的个数
    float removeProb[removeNum];  // 各个remove heuristic的概率
    float insertProb[insertNum];  // 各个insert heuristic的概率
    float noiseProb[2] = {0.5, 0.5};        // 噪声使用的概率
    for(i=0; i<removeNum; i++){
        removeProb[i] = 1.0f/removeNum;
    }
    for(i=0; i<insertNum; i++){
        insertProb[i] = 1.0f/insertNum;
    }
    float removeWeight[removeNum];  // 各个remove heuristic的权重
    float insertWeight[insertNum];  // 各个insert heuristic的权重
    float noiseWeight[2];   // 加噪声/不加噪声 分别的权重
    setOne<float>(removeWeight, removeNum);
    setOne<float>(insertWeight, insertNum);
    setOne<float>(noiseWeight, 2);
    int removeFreq[removeNum];      // 各个remove heuristic使用的频率
    int insertFreq[insertNum];      // 各个insert heuristic使用的频率
    int noiseFreq[2];               // 噪声使用的频率，第一个是with noise，第二个是without noise
    setZero<int>(removeFreq, removeNum);
    setZero<int>(insertFreq, insertNum);
    setZero<int>(noiseFreq, 2);
    int removeScore[removeNum];     // 各个remove heuristic的得分
    int insertScore[insertNum];     // 各个insert heuristic的得分
    int noiseScore[2];              // 噪声得分
    setZero<int>(removeScore, removeNum);
    setZero<int>(insertScore, insertNum);
    setZero<int>(noiseScore, 2);
    // 三项得分设定
    int sigma1 = 33;
    int sigma2 = 9;
    int sigma3 = 13;
    float r = 0.1f;       // weight更新速率

    // 其余核心参数
    int segment = 100;   // 每隔一个segment更新removeProb, removeWeight等参数
    float w = 0.05f;      // 初始温度设定有关参数
    float T = w * abs(currentCost) / (float)log(2);   // 初始温度
    float ksi = 0.8f;    // 每次移除的最大节点数目占总节点数的比例
    float c = 0.9998f;    // 降温速率
    vector<Spot*> removedCustomer(0);    // 被移除的节点
    vector<Car*> tempCarSet = copyPlan(currentCarSet);      // 暂时存放当前解

    pair<bool, int> removalSelectTrend = make_pair(false, 0);
    for(int iter=0; iter<maxIter; iter++){
        if(iter%segment == 0){  // 新的segment开始
            if(verbose == true) {
                cout << "...............Segement:" << (int)floor(iter/segment)+1 << 
                    "................" << endl;
                cout << "base cost is: " << baseCost << endl;
                cout << "current best cost is:" << globalCost << endl;
                cout << "hash table length is:" << hashTable.size() << endl;
                cout << "shaw   removal:" <<  "(score)-" << removeScore[0] 
                    << '\t' << "(freq)-" << removeFreq[0] << endl;
                cout << "random removal:" <<  "(score)-" << removeScore[1] 
                    << '\t' << "(freq)-" << removeFreq[1] << endl;
                cout << "worst  removal:" <<  "(score)-" << removeScore[2] 
                    << '\t' << "(freq)-" << removeFreq[2] << endl;
                cout << "greedy  insert:" <<  "(score)-" << insertScore[0] 
                    << '\t' << "(freq)-" << insertFreq[0] << endl;
                cout << "regret  insert:" <<  "(score)-" << insertScore[1] 
                    << '\t' << "(freq)-" << insertFreq[1] << endl;
                cout << "noise    addIn:" <<  "(score)-" << noiseScore[0]  
                    << '\t' << "(freq)-" << noiseFreq[0]  << endl;
                cout << endl;
            }
            if(iter != 0){      // 如果不是第一个segment
                // 更新权重
                updateWeight(removeFreq, removeWeight, removeScore, r, removeNum);
                updateWeight(insertFreq, insertWeight, insertScore, r, insertNum);
                updateWeight(noiseFreq, noiseWeight, noiseScore, r, 2);
                // 更新概率
                updateProb(removeProb, removeWeight, removeNum);
                updateProb(insertProb, insertWeight, insertNum);
                updateProb(noiseProb, noiseWeight, 2);
                // 将各变量置零
                setZero<int>(removeFreq, removeNum);
                setZero<int>(removeScore, removeNum);
                setZero<int>(insertFreq, insertNum);
                setZero<int>(insertScore, insertNum);
                setZero<int>(noiseFreq, 2);
                setZero<int>(noiseScore, 2);
            }
        }

        // 产生随机数选取remove heuristic和insert heuristic
        // 以概率选择remove heuristic
        int removeIndex;
        float sumation;
        if(removalSelectTrend.first == false) {
            float removeSelection = random(0,1);  // 产生0-1之间的随机数
            sumation = removeProb[0];
            removeIndex = 0;    // remove heuristic编号
            while(sumation < removeSelection){
                sumation += removeProb[++removeIndex];
            }
            removeIndex = min(removeIndex, removeNum-1);  // 防止溢出
        }
        else{
            removeIndex = removalSelectTrend.second;
        }
        removalSelectTrend.first = false;
        // 以概率选择insert heurisitc
        float insertSelection = random(0,1);
        sumation = insertProb[0];
        int insertIndex = 0;
        while(sumation < insertSelection){
            sumation += insertProb[++insertIndex];
        }
        insertIndex = min(insertIndex, insertNum-1);   // 防止溢出
        // 以概率选择是否增加噪声影响
        float noiseSelection = random(0,1);
        bool noiseAdd = false;
        if(noiseProb[0] > noiseSelection) {
            noiseAdd = true;
        }

        // 相应算子使用次数加一
        removeFreq[removeIndex]++;
        insertFreq[insertIndex]++;
        noiseFreq[1-(int)noiseAdd]++;

        // decide the number to remove
        int currentRemoveNum;  
        // 最多移除的节点数
        int maxRemoveNum = min(100, static_cast<int>(floor(ksi*customerTotalNum)));  
        // 最少移除的节点数
        int minRemoveNum = 4;
        // 当前需要移除的节点数目
        currentRemoveNum = (int)floor(random(minRemoveNum, maxRemoveNum)); 

        removedCustomer.clear();
        removedCustomer.resize(0);

        // 执行remove heuristic
        // dangerous!!
        removeIndex = 2;
        //////////////
        switch(removeIndex) {
            case 0: {
                // 首先得到maxArrivedTime
                float maxArrivedTime = -MAX_FLOAT;
                for(i=0; i<(int)tempCarSet.size(); i++){
                    // tempCarSet[i]->getRoute().refreshArrivedTime;
                    vector<float> temp = tempCarSet[i]->getRoute()->getArrivedTime();
                    sort(temp.begin(), temp.end(), greater<float>());
                    if(temp[0] > maxArrivedTime) {
                        maxArrivedTime = temp[0];
                    }
                }
                // 重置类成员maxt
                this->maxt = maxArrivedTime;
                // 运行removal算子
                shawRemoval(tempCarSet, removedCustomer, currentRemoveNum);
                break;
            }
            case 1: {
                randomRemoval(tempCarSet, removedCustomer, currentRemoveNum);
                break;
            }
            case 2: {
                worstRemoval(tempCarSet, removedCustomer, currentRemoveNum);
                break;
            }
        }
        // 执行insert heuristic
        switch(insertIndex) {
            case 0: {
                greedyInsert(tempCarSet, removedCustomer, noiseAdd);
                break;
            }
            case 1: {
                regretInsert(tempCarSet, removedCustomer, noiseAdd);
                break;
            }
        }

        try {
            if (getCustomerNum(tempCarSet) != customerTotalNum) {
                throw out_of_range("Lose some customers in SSLR!");
            }
        } 
        catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
        }

        // 移除空路径
        removeNullRoute(tempCarSet, true);
        
        // 使用模拟退火算法决定是否接收该解
        bool accept = false;
        float newCost = getCost(tempCarSet);
        float acceptProb = exp(-(newCost - currentCost)/T);
        if(acceptProb > rand()/(RAND_MAX+1.0f)) {
            accept = true;
        }
        T = T * c;   // 降温
        size_t newRouteCode = codeForSolution(tempCarSet);
        
        // 接下来判断是否需要加分
        // 加分情况如下：
        // 1. 当得到一个全局最优解时
        // 2. 当得到一个尚未被接受过的，而且更好的解时
        // 3. 当得到一个尚未被接受过的解，虽然这个解比当前解差，但是这个解被接受了
        if(newCost < globalCost){  // 情况1
            // 得到了全局最优解，我们减少remove的顾客数
            ksi = 0.4f;
            removeScore[removeIndex] += sigma1;
            insertScore[insertIndex] += sigma1;
            noiseScore[1-(int)noiseAdd] += sigma1;
            withdrawPlan(globalCarSet);
            globalCarSet = copyPlan(tempCarSet);
            globalCost = newCost;
        } else {
            vector<size_t>::iterator tempIter = find(hashTable.begin(), hashTable.end(), newRouteCode);
            if(tempIter == hashTable.end()){  
                // 该解从来没有被接受过
                if(newCost < currentCost){    
                    // 如果比当前解要好，情况2
                    // 如果得到了更好的解，则减少remove的顾客数
                    ksi = 0.4f;  
                    removeScore[removeIndex] += sigma2;
                    insertScore[insertIndex] += sigma2;
                    noiseScore[1-(int)noiseAdd] += sigma2;
                } 
                else {      
                    if(accept == true) {       
                        // 情况3
                        if(newCost > baseCost) {
                            // 如果接受了更差的解，则增加对当前解的扰动
                            ksi = 0.8f;   				
                            // 这时强制使用random removal来破坏当前的解
                            removalSelectTrend.first = true;   	
                            removalSelectTrend.second = 1;     // random removal
                        }
                        else {
                            // 没有接受更差的解，则使用中等的remove顾客数
                            ksi = 0.6f;
                        }
                        removeScore[removeIndex] += sigma3;
                        insertScore[insertIndex] += sigma3;
                        noiseScore[1-(int)noiseAdd] += sigma3;						
                    }
                }
            }
        }
        if(accept == true) {    
            // 如果被接受了，则更新currentCarSet， 并且tempCarSet不变
            vector<size_t>::iterator tempIter = find(hashTable.begin(), hashTable.end(), newRouteCode);
            if(tempIter == hashTable.end()){
                hashTable.push_back(newRouteCode); 
            }
            currentCost = newCost;     // 如果被接收，则更新当前解
            withdrawPlan(currentCarSet);
            currentCarSet = copyPlan(tempCarSet);
        } 
        else {    
            // 否则，tempCarSet恢复为currentCarSet
            withdrawPlan(tempCarSet);
            tempCarSet = copyPlan(currentCarSet);
        }
    }    

    withdrawPlan(finalCarSet);
    finalCarSet.reserve(originPlan.size());
    ostringstream ostr;
    ostr.str("");
    print_lck.lock();
    // unique_lock<mutex> lck(print_lck);
    int infeasibleNum;
    
    if(judgeFeasible(globalCarSet, originPlan, infeasibleNum) == false) {
        // 如果搜索不到更好的解，则维持原来的解
        ostr << "SSLR: we should use the origin plan, there are " << infeasibleNum << 
            " high priority customers left in virtual vehicles." << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        print_lck.unlock();
        finalCarSet = copyPlan(originPlan);
    } else {
        ostr << "SSLR: we will use the new plan" << endl;
        TxtRecorder::addLine(ostr.str());
        for (carIter = globalCarSet.begin(); carIter < globalCarSet.end(); carIter++) {
            if ((*carIter)->judgeArtificial() == false) {
                Car *tempCar = new Car(**carIter);
                finalCarSet.push_back(tempCar);
            }
        }
        cout << ostr.str();
        print_lck.unlock();
    }
    delete [] DTpara;
    finalCost = globalCost;
    withdrawPlan(baseCarSet);
    withdrawPlan(tempCarSet);
    withdrawPlan(globalCarSet);
    hashTable.clear();
}

