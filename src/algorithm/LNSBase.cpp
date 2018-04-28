#include "LNSBase.h"
#include "../baseclass/Matrix.h"
#include<stdexcept>
#include<algorithm>
#include<cassert>
#include<functional>
#include<cmath>
#include<sstream>
#include<limits>
#include<sstream>

void computeMax(vector<Spot*> allCustomer, float &maxd, float &mind, float &maxquantity){
    // 计算Pickup-Delivery对之间的最大/最小距离以及Delivery节点的最大货物需求量
    // e.g: (p1, d1)与(p2, d2)之间的距离为dist(p1, p2) + dist(d1, d2)
    // 我们利用customer节点的choice指针来找到它所选择的商店
    // allCustomer: 仅指顾客节点(D)
    int customerAmount = (int)allCustomer.size();
    // D1: 将对角线元素设置为0(便于取最大值)
    // D2: 将对角线元素设置为MAX_FLOAT(便于取最小值)
    Matrix<float> D1(customerAmount, customerAmount);
    Matrix<float> D2(customerAmount, customerAmount);
    float tempmax = -MAX_FLOAT;
    for(int i=0; i<customerAmount; i++){
        if(allCustomer[i]->quantity > tempmax){
            tempmax = allCustomer[i]->quantity;
        }
        D1.setValue(i,i, 0.0f);
        D2.setValue(i,i, MAX_FLOAT);
        for(int j=i+1; j<customerAmount; j++){
            Spot *p1 = allCustomer[i];
            Spot *d1 = allCustomer[i]->choice;
            Spot *p2 = allCustomer[j];
            Spot *d2 = allCustomer[j]->choice;
            float temp = sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2)) + 
                sqrt(pow(d1->x - d2->x, 2) + pow(d1->y - d2->y, 2));
            D1.setValue(i, j, temp);
            D2.setValue(i, j, temp);
            D1.setValue(j, i, temp);
            D2.setValue(j, i, temp);
        }
    }
    int t1, t2;
    maxd = D1.getMaxValue(t1, t2);
    mind = D2.getMinValue(t1, t2);
    maxquantity = tempmax;
}

LNSBase::LNSBase(int pshaw, int pworst, float eta, float capacity, float *randomRange, 
        vector<Spot*> allCustomer, Spot depot, bool hierarchicalCar, 
        bool allowNegativeCost){
    this->allCustomer = allCustomer;
    computeMax(allCustomer, maxd, mind, maxquantity);
    this->pshaw = pshaw;
    this->pworst = pworst;
    this->baseNoise = eta * maxquantity;
    this->depot = depot;
    this->capacity = capacity;
    this->randomRange = randomRange;
    this->hierarchicalCar = hierarchicalCar;
    this->allowNegativeCost = allowNegativeCost;
    this->DTpara = new float[4];
    this->DTpara[0] = 0.0f;
    this->DTpara[1] = 0.0f;
    this->DTpara[2] = 0.0f;
    this->DTpara[3] = 0.0f;
}

void LNSBase::resetDTpara(float *DTpara) {
    this->DTpara[0] = DTpara[0];
    this->DTpara[1] = DTpara[1];
    this->DTpara[2] = DTpara[2];
    this->DTpara[3] = DTpara[3];
}

void checkRepeatID(vector<int> sortedArray) {
    // 检查排序号的sortedArray中是否有重复元素
    if (sortedArray.size() == 0) return;
    vector<int>::iterator iter1, iter2;
    iter1 = sortedArray.begin();
    iter2 = iter1 + 1;
    for (; iter2 < sortedArray.end(); iter1++, iter2++) {
        if (*iter1 == *iter2) {
            throw invalid_argument("Catch repeat elements!");
        }
    }
}

void removeCustomerFromCar(vector<int> removedIndexset, vector<int> customerNum, 
        vector<Spot*> allCustomerInOrder, vector<Car*> &originCarSet, 
        vector<Spot*> &removedCustomer) {
    // 从originCarSet中移除由removedIndexset指示的顾客节点(P)及其选择的商店(D)
    // 并且将移除的顾客节点放入removedCustomer中
    // Args:
    //   * removedIndexset: 被移除的顾客index集合（在allCustomerInOrder中的位置）
    //   * customerNum: 每辆货车的顾客数量，按照car id顺序排列
    //   * allCustomerInOrder: 按customerNum顺序排列的顾客集合
    //   * removedCustomer: 被移除的顾客
    // Returns:
    //   * OriginCarSet: 初始携带所有顾客(P-D)，经本函数后移除掉removedCustomer
    //                   指定的(P-D)对
    int indexRemovedLen = removedIndexset.end() - removedIndexset.begin();
    for (int i=0; i<indexRemovedLen; i++) {
        int carIndex;
        // 被移除的顾客节点在allCustomerInOrder中的位置
        int currentIndex = removedIndexset[i]; 
        vector<int>::iterator iter;
        // 第一个比currentIndex的元素，锁定该顾客所在车辆后面一辆车
        iter = upper_bound(customerNum.begin(), customerNum.end(), currentIndex+1);
        // Note: 如果该顾客所在车辆后面有空车，则（iter-1）会锁定在该车辆后连续空车的
        //       最后一辆，因此我们需要将iter修正为非空车
        bool mark = false;
        while (!mark) {
            if (iter == customerNum.begin()) {
                // 锁定为第一辆车，无需回溯
                mark = true;
            } 
            else {
                if (*(iter-1) != currentIndex + 1) {
                    // 如果iter指向的车辆不是空车，停止回溯
                    mark = true;
                } 
                else {
                    iter = iter - 1;
                }
            }
        }
        carIndex = iter - customerNum.begin();
        try {
            // 由于delete操作会从路径中删除掉(P-D)对
            // 因此需要作备份
            Spot *customer = allCustomerInOrder[currentIndex];
            Spot *store = customer->choice;
            Spot *copyCustomer = new Spot(*customer);
            Spot *copyStore = new Spot(*(customer->choice));
            removedCustomer.push_back(copyCustomer);
            copyCustomer->choice = copyStore;
            copyStore->choice = copyCustomer;
            originCarSet[carIndex]->deleteCustomer(store, customer);
        } catch (exception &e) {
            cout << "All removed Indexes are: " << endl;
            for(int k=0; k<indexRemovedLen; k++) {
                cout << removedIndexset[k] << "\t";
            }
            cout << endl;
            ostringstream ostr;
            ostr.str("");
            ostr << "Deleted customer #" << allCustomerInOrder[currentIndex]->id << 
                " from car #" << originCarSet[carIndex]->getCarIndex() << endl;
            throw out_of_range(ostr.str());
        }
    }
}

void getAllCustomerInOrder(vector<Car*> originCarSet, vector<int> &customerNum, 
        vector<Spot*> &allCustomerInOrder) {
    // 获取Car集合中所有的顾客(仅有D，没有P)
    // Args:
    //   * originCarSet: 初始顾客集合（未被执行顾客移除操作）
    // Returns:  
    //   * customerNum: 各辆车服务的顾客数量
    //   * allCustomer: 所有的顾客节点(为路径中原有节点，非复制)
    int i=0;
    for(vector<Car*>::iterator it1 = originCarSet.begin(); it1 < originCarSet.end(); it1++){
        if(i==0){  // 如果customerNum中没有元素，则不需要累加
            customerNum.push_back((*it1)->getCustomerNum()); 
        }else{     // 后面的数需要累加
            customerNum.push_back((*it1)->getCustomerNum() + customerNum[i-1]);
        }
        i++;
        vector<Spot*> customerSet = (*it1)->getAllCustomer(); // 每辆货车所负责的顾客
        for(vector<Spot*>::iterator it2=customerSet.begin(); it2<customerSet.end(); it2++){
            allCustomerInOrder.push_back(*it2);   // 逐个顾客节点插入
        }
    }
}

void computeReducedCost(vector<Car*> originCarSet, vector<int> indexsetInRoute, 
        vector<int> removedIndexset, vector<pair<float, int> > &reducedCost, float DTpara[]){
    // 计算尚在OriginCarSet中的顾客节点的移除代价
    // Args:
    //   * originCarSet: 当前的车辆集
    //   * indexsetInRoute: 尚在路径中的顾客index（在allCustomerInOrder中的位置）
    //   * removedIndexset: 已经被移除的顾客index（在allCustomerInOrder中的位置）
    //   * DTpara[]: 针对不同优先级顾客的奖惩系数
    // Returns:
    //   * reducedCost: 所有顾客节点的移除代价，若该顾客已经被移除，则为正无穷
    //                  int数据为在allCustomerInOrder中的位置
    int i;
    int carNum = originCarSet.end() - originCarSet.begin();
    vector<float> reducedCostInRoute(0); // 尚在路径中的各个节点的移除代价
    for(i=0; i<carNum; i++){
        vector<float> tempReducedCost = originCarSet[i]->computeReducedCost(DTpara);
        reducedCostInRoute.insert(reducedCostInRoute.end(), tempReducedCost.begin(), tempReducedCost.end());
    }
    for(i=0; i<indexsetInRoute.end()-indexsetInRoute.begin(); i++){
        int index = indexsetInRoute[i];
        reducedCost[index].first = reducedCostInRoute[i];
        reducedCost[index].second = index;
    }
    for(i=0; i<removedIndexset.end() - removedIndexset.begin(); i++){
        int index = removedIndexset[i];
        reducedCost[index].first = MAX_FLOAT;  // 已经移除掉的节点，不作考虑
        reducedCost[index].second = index;	
    }
}

void generateMatrix(vector<int> &allIndex, vector<Car*> &removedCarSet, 
        vector<Spot*> removedCustomer, Matrix<float> &minInsertPerRoute, 
        Matrix<pair<Spot*, Spot*> > &minInsertPos, Matrix<float> &secondInsertPerRoute, 
        Matrix<pair<Spot*, Spot*> > &secondInsertPos, float baseNoise, 
        float DTpara[], float randomRange[], 
        bool allowNegativeCost){
    // 计算removedCustomer到removedCarSet的最小和次小插入代价
    // Args:
    //   * allIndex: 从0-len(removedCustomer)，作为返回值
    //   * removedCarSet: 被执行过remove操作的货车集合
    //   * removedCustomer: 被remove的顾客集
    //   * baseNoise: 由ALNS建议的noise，如果不需要则应为0
    //   * DTpara[]: 针对不同优先级顾客的奖惩因子，如果不需要应为0
    //   * randomRange: 随机数范围，左值和右值
    //   * allowNegativeCost: 指示计算Insertion cost时是否允许有负数。
    // Returns:
    //   * minInsertPerRoute: 各顾客节点在每辆货车上的最小插入代价, 横坐标
    //                        为车辆相对位置，纵坐标为removed customer相对位置
    //   * minInsertPos: 最小插入代价对应插入点的前一个节点对(refStore1, refCustomer1)
    //   * secondInsertPerRoute: 次小插入代价
    //   * secondInsertPos:次小插入代价对应插入点的前一个节点对(refStore2, refCustomer2)
    
    float DTH1, DTH2, DTL1, DTL2;
    // DTH1, DTH2(>0): 插入到working vehicle的奖励值
    // DTL1, DTL2(>0): 插入到virtual vehicle的惩罚值
    DTH1 = DTpara[0];
    DTH2 = DTpara[1];
    DTL1 = DTpara[2];
    DTL2 = DTpara[3];
    int removedCustomerNum = removedCustomer.size();
    int carNum = removedCarSet.size();
    for(int i=0; i<carNum; i++){
        // removedCarSet[i]->getRoute().refreshArrivedTime();  // 先更新一下各条路径的arrivedTime
        for(int j=0; j<removedCustomerNum; j++){
            if(i==0){
                allIndex.push_back(j);
            }
            float minValue, secondValue;
            Spot *refCustomer1, *refStore1, *refCustomer2, *refStore2;
            float noiseAmount = baseNoise;
            // 异构车辆，在算法中异构车辆为working vehicle和virtual vehicle
            if(removedCarSet[i]->judgeArtificial() == false){  // 如果不是虚拟车
                switch(removedCustomer[j]->priority) {         // then give bonus
                    case 1: {
                        noiseAmount -= DTH1;
                        break;
                    }
                    case 2: {
                        noiseAmount -= DTL1;
                        break;
                    }
                }
            } else {     // 如果是虚拟车，则需要进行惩罚
                switch(removedCustomer[j]->priority) {
                    case 1: {
                        noiseAmount += DTH2;
                        break;
                    }
                    case 2: {
                        noiseAmount += DTL2;
                        break;
                    }
                }
            }
            float randomNoise = noiseAmount * random(randomRange[0], randomRange[1]);
            Spot *currentCustomer = removedCustomer[j];
            removedCarSet[i]->computeInsertCost(currentCustomer->choice, currentCustomer,
                    minValue, refStore1, refCustomer1, secondValue, refStore2, refCustomer2,
                    randomNoise, allowNegativeCost);
            minInsertPerRoute.setValue(i, j, minValue);
            minInsertPos.setValue(i, j, make_pair(refStore1, refCustomer1));
            secondInsertPerRoute.setValue(i, j, secondValue);
            secondInsertPos.setValue(i, j, make_pair(refStore2, refCustomer2));
        }
    }
}

void updateMatrix(vector<int> restCustomerIndex, Matrix<float> &minInsertPerRoute, 
        Matrix<pair<Spot*, Spot*> > &minInsertPos, Matrix<float> &secondInsertPerRoute, 
        Matrix<pair<Spot*, Spot*> > &secondInsertPos, int selectedCarPos, vector<Car*> &removedCarSet,
        vector<Spot*>removedCustomer, float baseNoise, float DTpara[], 
        float randomRange[], bool allowNegativeCost){
    // 更新removedCustomer在selectedCarPos指定的货车中的最小以及次小插入代价
    // 也就是指更新返回值中selectedCarPos指定的那一行
    // Args:
    //   * restCustomerIndex: 剩余未被加入到货车路径的顾客节点pos（相对于
    //                        最初所有待插入节点集合的相对位置）
    //   * removedCarSet: 被执行过remove操作的货车集合
    //   * removedCustomer: 被remove的顾客(P)集(插入算法执行开始时)
    //   * selectedCarPos: 这次需要计算插入代价的货车在carSet中的相对位置
    //   * baseNoise: 由ALNS建议的noise，如果不需要则应为0
    //   * DTpara[]: 针对不同优先级顾客的奖惩因子，如果不需要应为0
    //   * randomRange: 随机数范围，左值和右值
    //   * allowNegativeCost: 指示计算Insertion cost时是否允许有负数。
    // Returns:
    //   * minInsertPerRoute: 各顾客节点在每辆货车上的最小插入代价, 横坐标
    //                        为车辆相对位置，纵坐标为removed customer相对位置
    //   * minInsertPos: 最小插入代价对应插入点的前一个顾客对(refStore1, refCustomer1)
    //   * secondInsertPerRoute: 次小插入代价
    //   * secondInsertPos:次小插入代价对应插入点的前一个顾客对(refStore2, refCustomer2)

    float DTH1, DTH2, DTL1, DTL2;
    // DTH1, DTH2(>0): 插入到working vehicle的奖励值
    // DTL1, DTL2(>0): 插入到virtual vehicle的惩罚值
    DTH1 = DTpara[0];
    DTH2 = DTpara[1];
    DTL1 = DTpara[2];
    DTL2 = DTpara[3];

    // 更新四个矩阵
    // removedCarSet[selectedCarPos]->getRoute().refreshArrivedTime();
    for(int i=0; i<(int)restCustomerIndex.size();i++) {
        int index = restCustomerIndex[i];   // 顾客下标
        float minValue, secondValue;
        Spot *refStore1, *refCustomer1, *refStore2, *refCustomer2;
        float noiseAmount = baseNoise;
        if(removedCarSet[selectedCarPos]->judgeArtificial() == false) { 
            // 如果不是虚构的车辆
            switch(removedCustomer[index]->priority) {  
                // 根据不同的顾客优先级，赋予不同的惩罚系数（当插入到artificial vehicle时）
                case 1:
                    noiseAmount = -DTH1;
                    break;
                case 2:
                    noiseAmount = -DTL1;
                    break;
            }		
        } else {   // 是虚构的车辆
            switch(removedCustomer[index]->priority) {  
                // 根据不同的顾客优先级，赋予不同的惩罚系数（当插入到artificial vehicle时）
                case 1:
                    noiseAmount = DTH2;
                    break;
                case 2:
                    noiseAmount = DTL2;
                    break;
            }		
        }
        float randomNoise = noiseAmount * random(randomRange[0], randomRange[1]);
        Spot *currentCustomer = removedCustomer[index];
        removedCarSet[selectedCarPos]->computeInsertCost(currentCustomer->choice, currentCustomer, 
                minValue, refStore1, refCustomer1, secondValue, refStore2, 
                refCustomer2, randomNoise, allowNegativeCost);
        minInsertPerRoute.setValue(selectedCarPos, index, minValue);
        minInsertPos.setValue(selectedCarPos, index, make_pair(refStore1, refCustomer1));
        secondInsertPerRoute.setValue(selectedCarPos, index, secondValue);
        secondInsertPos.setValue(selectedCarPos, index, make_pair(refStore2, refCustomer2));
    }
}

void LNSBase::shawRemoval(vector<Car*> &originCarSet, vector<Spot*> &removedCustomer,
        int q){
    // shaw removal: 移除特性相近的顾客节点
    // 每次循环移除 y^p*|L|个顾客，L为路径中剩余节点，y是0-1之间的随机数
    // Args:
    //    * removedCustomer: 被移除的顾客节点
    //    * q: 本次需要remove的顾客数量
    // Returns:	
    //    * originCarSet: 未执行remove操作前的货车集合
    //    * removedCarSet: 执行remove操作后的货车集合
    int phi = 9;
    int kai = 3;
    int psi = 2;
    // 货车数量
    int carAmount = originCarSet.end()-originCarSet.begin();  
    // 各辆车负责的顾客节点数目
    vector<int> customerNumInCar(0);         
    // 所有顾客节点，按货车的服务顺序排序，index小的货车的顾客节点排在前面
    vector<Spot*> allCustomerInOrder(0);
    int i,j;
    getAllCustomerInOrder(originCarSet, customerNumInCar, allCustomerInOrder);   
    // 顾客总数
    int customerTotalNum = allCustomerInOrder.end() - allCustomerInOrder.begin();    
    // 如果没有顾客,抛出warning
    if(customerTotalNum <= 0) {                                 
        cout << "WARNING: Currently no customers in plan (randomRemoval)" << endl;     
    }  
    // 相似矩阵：pair的第一个元素是相似值，第二个元素是相似对象在allCustomerInOrder中的索引
    vector<pair<float, int> > R(customerTotalNum*customerTotalNum);     // 相似矩阵
    float temp1;
    // 为所有的customer编号以确认其在allCustomerInOrder中的对应位置
    vector<int> allIndex(customerTotalNum);  // 0~customerTotalNum-1
    for(i=0; i<customerTotalNum; i++){
        allIndex[i] = i;
        for(j=0; j<customerTotalNum; j++){
            if(i==j) { 
                R[i*customerTotalNum+j].first = MAX_FLOAT;
                R[i*customerTotalNum+j].second = j;
            }
            else{
                Spot *customer1 = allCustomerInOrder[i];
                Spot *customer2 = allCustomerInOrder[j];
                float distSim = phi * (dist(customer1->choice, customer2->choice) + 
                        dist(customer1, customer2));
                float timeSim = kai * (abs(customer1->arrivedTime - customer2->arrivedTime) + 
                        abs(customer1->choice->arrivedTime - customer2->choice->arrivedTime));
                float quantitySim = psi * abs(customer1->quantity - customer2->quantity);
                float similarity = distSim/maxd + timeSim/maxt + quantitySim/maxquantity;
                R[i*customerTotalNum+j].first = similarity;   // i行j列
                R[i*customerTotalNum+j].second = j;
                R[j*customerTotalNum+i].first = similarity;
                R[j*customerTotalNum+i].second = i;      // j行i列
            }
        }
    }
    int selectedIndex;           // 被选中的节点在allCustomer中的下标
    vector<int> removedIndexset; // 所有被移除的节点的下标集合(allCustomer中)
    int maxTrail = customerTotalNum;
    int trailCount = 0;
    while((trailCount++) < maxTrail) {
        // 选取一个choice不为depot的顾客节点
        selectedIndex = int(random(0,customerTotalNum));          // 随机选取一个节点
        selectedIndex = min(selectedIndex, customerTotalNum-1);   // 防止越界
        if(allCustomerInOrder[selectedIndex]->choice->type == 'D') {
            selectedIndex = -1;
            continue;
        } 
        else break;
    }
    if(selectedIndex == -1) return;
    removedIndexset.push_back(selectedIndex);
    vector<int> indexsetInRoute(customerTotalNum-1);     // 尚在路径中的节点的下标集合
    set_difference(allIndex.begin(), allIndex.end(), removedIndexset.begin(), 
            removedIndexset.end(), indexsetInRoute.begin());
    maxTrail = 1;
    trailCount = 0;
    while((int)removedIndexset.size() < q && (trailCount++) < maxTrail){ 
        // 要移除掉一共q个节点
        // 当前要进行排序的相似矩阵（向量），仅包含尚在路径中的节点
        vector<pair<float, int> > currentR(0);      
        // 尚在路径中的节点个数
        int indexInRouteLen = indexsetInRoute.end() - indexsetInRoute.begin();
        vector<pair<float, int> >::iterator iter1;
        for(i=0; i<indexInRouteLen; i++){
            int index = indexsetInRoute[i];  // 相对于allCustomerInOrder
            currentR.push_back(R[selectedIndex*customerTotalNum + index]);
        }
        // 相似性从小到大进行排序
        sort(currentR.begin(), currentR.end(), ascendSort<float, int>);
        float y = rand()/(RAND_MAX+1.0f);  // 产生0-1之间的随机数
        int indexsetInRouteLen = indexsetInRoute.end() - indexsetInRoute.begin();  
        // 本次移除的节点数目 
        int removeNum = max((int)floor(pow(y,pshaw)*indexsetInRouteLen), 1);
        i = 0;
        int count = 0;
        while(i<(int)indexsetInRoute.size() && count<removeNum) {
            int index = currentR[i++].second;
            if(allCustomerInOrder[index]->choice->type == 'D') {
                // 如果customer指向的商店是仓库，说明这是一个虚拟的商店
                // 当计划开始后，在replan阶段，往往会出现store访问了但是customer尚未访问的
                // 情况，这时customer指向的store会临时安排为depot。为了避免将该customer转
                // 移到其他的车辆中（不允许store和customer在不同的车辆上），我们禁止对这一
                // 类顾客节点执行"remove"操作
                continue;
            } else {
                removedIndexset.push_back(index);
                count++;
            }
        }
        // 截至目前为止被移除的节点数目
        int indexRemovedLen = removedIndexset.end() - removedIndexset.begin();  
        int randint = (int)random(0,indexRemovedLen);  // 产生一个0-indexRemovedLen的随机数
        randint = min(randint, indexRemovedLen-1);     // 防止越界错误
        selectedIndex = removedIndexset[randint];
        sort(removedIndexset.begin(), removedIndexset.end());
        vector<int>::iterator iterINT;
        iterINT = set_difference(allIndex.begin(), allIndex.end(), removedIndexset.begin(), 
                removedIndexset.end(), indexsetInRoute.begin());
        indexsetInRoute.resize(iterINT - indexsetInRoute.begin());
    }

    // 检查removedIndexset中是否有相同的元素
    sort(removedIndexset.begin(), removedIndexset.end());          
    try {
        checkRepeatID(removedIndexset);
    }
    catch (exception &e) {
        throw out_of_range("In shaw removal: " + string(e.what()));
    }

    // 从原有路径中remove掉某些(P-D)对
    try {
        removeCustomerFromCar(removedIndexset, customerNumInCar, allCustomerInOrder, originCarSet, 
                                removedCustomer);
    } catch (exception &e) {
        throw out_of_range("In shaw removal: " + string(e.what()));
    }
}

void LNSBase::randomRemoval(vector<Car*> &originCarSet, vector<Spot*> &removedCustomer, int q){
    // 随机移除q个节点
    // Args:
    //   * q: 本次需要remove的顾客数量
    // Returns:
    //   * originCarSet: 未执行remove操作前的货车集合
    //   * removedCustomer: 被移除的顾客节点

    int i;
    vector<int> customerNumInCar(0);       // 各辆货车顾客节点数目 
    vector<Spot*> allCustomerInOrder(0);  // 所有顾客节点
    vector<int> allIndex;             // 0~customerTotalNum-1
    vector<int> indexsetInRoute(0);
    getAllCustomerInOrder(originCarSet, customerNumInCar, allCustomerInOrder);
    int customerTotalNum = allCustomerInOrder.end() - allCustomerInOrder.begin();
    vector<int> removedIndexset(0); 
    for(i=0; i<customerTotalNum; i++){
        allIndex.push_back(i);
    }

    //检查原车辆中是否有顾客
    if(customerTotalNum <= 0) {                                 
        cout << "Warning: Currently no customers in plan (randomRemoval)" << endl;     
    }                                                         

    indexsetInRoute = allIndex;
    int count = 0;
    int maxTrial = (int)indexsetInRoute.size(); // 最大尝试次数
    i = 0;
    while(i<maxTrial && count<q) {
        // 尚在路径中的节点个数
        i++;
        int indexInRouteLen = indexsetInRoute.end() - indexsetInRoute.begin();  
        int selectedIndex = int(random(0, indexInRouteLen));     // 在indexsetInRoute中的索引
        selectedIndex = min(selectedIndex, indexInRouteLen-1);   // avoid reaching the right side
        int index = indexsetInRoute[selectedIndex];
        if(allCustomerInOrder[index]->choice->type == 'D') {
            // 如果customer指向的商店是仓库，说明这是一个虚拟的商店
            // 当计划开始后，在replan阶段，往往会出现store访问了但是customer尚未访问的
            // 情况，这时customer指向的store会临时安排为depot。为了避免将该customer转
            // 移到其他的车辆中（不允许store和customer在不同的车辆上），我们禁止对这一
            // 类顾客节点执行"remove"操作
            continue;
        } else {
            removedIndexset.push_back(index);
            sort(removedIndexset.begin(), removedIndexset.end());
            vector<int>::iterator iterINT;
            iterINT = set_difference(allIndex.begin(), allIndex.end(), removedIndexset.begin(), 
                    removedIndexset.end(), indexsetInRoute.begin());
            indexsetInRoute.resize(iterINT - indexsetInRoute.begin());
            count++;
        }
    }

    // 检查在removedIndexset中是否有重复元素elements
    sort(removedIndexset.begin(), removedIndexset.end());
    try {
        checkRepeatID(removedIndexset);
    }
    catch (exception &e) {
        throw out_of_range("In random removal: " + string(e.what()));
    }

    try {
        removeCustomerFromCar(removedIndexset, customerNumInCar, allCustomerInOrder, 
                originCarSet, removedCustomer);
    } 
    catch (exception &e){
        throw out_of_range("In random removal: " + string(e.what()));
    }
}

void printIndex(vector<pair<float, int> > cost) {
    cout << "All index in route:" << endl;
    for(int i=0; i<cost.size();i++) {
        cout << cost[i].second << "\t";
    }
    cout << endl;
}


void LNSBase::worstRemoval(vector<Car*> &originCarSet, vector<Spot*> &removedCustomer,
        int q) {
    // worst removal算子	
    // Args:
    //   * q: 本次需要remove的顾客数量
    // Returns:    
    //   * originCarSet: 未执行remove操作前的货车集合
    //   * removedCustomer: 被移除的顾客节点
    int i;
    vector<int> customerNumInCar(0);       // 各辆货车顾客节点数目 
    vector<Spot*> allCustomerInOrder(0);  // 所有顾客节点
    vector<int> allIndex(0);          // 0~customerAmount-1，为所有的customer标记位置
    vector<int> indexsetInRoute(0);	  // 尚在路径中的节点下标
    vector<int> removedIndexset(0);   // 被移除的节点下标
    getAllCustomerInOrder(originCarSet, customerNumInCar, allCustomerInOrder);
    // originCarSet中的顾客数量
    int customerTotalNum = allCustomerInOrder.end() - allCustomerInOrder.begin();
    for(i=0; i<customerTotalNum; i++){
        allIndex.push_back(i);
    }
    // 如果当前plan中没有顾客节点，抛出WARNING
    if(customerTotalNum <= 0) {                                 
        cout << "Warning: Currently no customers in plan (worstRemoval)" << endl;     
    }                                                         
    indexsetInRoute = allIndex;
    int maxTrial = 20;
    int trialCount = 0;
    while((int)removedIndexset.size() < q && (trialCount++) < maxTrial){
        vector<pair<float, int> > reducedCost(customerTotalNum);  // 各节点的移除代价	
        computeReducedCost(originCarSet, indexsetInRoute, removedIndexset, reducedCost, DTpara);
        sort(reducedCost.begin(), reducedCost.end(), ascendSort<float, int>);   // 递增排序
        float y = rand()/(RAND_MAX+1.0f);  // 产生0-1之间的随机数
        int indexInRouteLen = indexsetInRoute.end() - indexsetInRoute.begin();
        int removedNum = static_cast<int>(max((float)floor(pow(y,pworst)*indexInRouteLen), 1.0f));
        assert(removedNum <= indexInRouteLen);
        int count = 0;
        i = 0;
        while(i<(int)indexsetInRoute.size() && count<removedNum) {
            int index = reducedCost[i++].second;
            if(allCustomerInOrder[index]->choice->type == 'D') {
                // 如果customer指向的商店是仓库，说明这是一个虚拟的商店
                // 当计划开始后，在replan阶段，往往会出现store访问了但是customer尚未访问的
                // 情况，这时customer指向的store会临时安排为depot。为了避免将该customer转
                // 移到其他的车辆中（不允许store和customer在不同的车辆上），我们禁止对这一
                // 类顾客节点执行"remove"操作
                continue;
            } else {
                removedIndexset.push_back(index);
                count++;
            }
        }
        sort(removedIndexset.begin(), removedIndexset.end());
        vector<int>::iterator iterINT;   // 整数向量迭代器
        iterINT = set_difference(allIndex.begin(), allIndex.end(), removedIndexset.begin(), 
                removedIndexset.end(), indexsetInRoute.begin());
        indexsetInRoute.resize(iterINT - indexsetInRoute.begin());
    }
    sort(removedIndexset.begin(), removedIndexset.end());                   
    try {
        checkRepeatID(removedIndexset);
    }
    catch (exception &e) {
        cout << "indexsetInRoute:" << endl;
        vector<int>::iterator iter;
        for(iter=indexsetInRoute.begin(); iter<indexsetInRoute.end(); iter++) {
            cout << *iter << "\t";
        }
        cout << endl;
        cout << "All ids are: " << endl;
        for(iter=allIndex.begin(); iter<allIndex.end(); iter++) {
            cout << *iter << "\t";
        }
        cout << endl;
        throw out_of_range("In worst removal: " + string(e.what()));
    }
    try {
        removeCustomerFromCar(removedIndexset, customerNumInCar, allCustomerInOrder, 
                            originCarSet, removedCustomer); 
    } 
    catch (exception &e) {
        throw out_of_range("In worst removal: " + string(e.what()));
    }
}

void LNSBase::greedyInsert(vector<Car*> &removedCarSet, vector<Spot*> removedCustomer,
        bool noiseAdd){
    // greedy insert算子, 把removedCustomer连同其选择的store插入到removedCarSet中
    // Args:
    //   * removedCustomer: 被remove的顾客(P)
    //   * noiseAdd: 是否需要添加噪声
    // Returns:
    //   * removedCarSet: 执行过remove算子的货车集合，最终会将removedCustomer(P-D)
    //                    重新插入其中

    // 需要插入到路径中的节点数目
    int removedCustomerNum = removedCustomer.end() - removedCustomer.begin();  
    int carNum = removedCarSet.end() - removedCarSet.begin();    // 车辆数目

    // 如果有removedCarSet为空，抛出异常
    try {
        if (carNum == 0) {
            throw invalid_argument("Empty car in removedCarSet!");
        } 
    }
    catch (exception &e){
        throw out_of_range("In greedy insertion: " + string(e.what()));
    }

    int newCarIndex = removedCarSet[carNum-1]->getCarIndex()+1;  // 新车的起始标号
    int i;
    vector<int> alreadyInsertIndex(0);		   // 已经插入到路径中的节点下标，相对于allIndex
    // 在每条路径中的最小插入代价矩阵（行坐标：车辆，列坐标：顾客）
    Matrix<float> minInsertPerRoute(carNum, removedCustomerNum);     	
    // 在每条路径中的最小插入代价所对应的节点
    Matrix<pair<Spot*, Spot*> > minInsertPos(carNum, removedCustomerNum);       	
    // 在每条路径中的次小插入代价矩阵
    Matrix<float> secondInsertPerRoute(carNum, removedCustomerNum);  	
    // 在每条路径中次小插入代价所对应的节点
    Matrix<pair<Spot*, Spot*> > secondInsertPos(carNum, removedCustomerNum);
    vector<int> allIndex(0);   // 对removedCustomer进行编号,1,2,3,...

    float tempBaseNoise = baseNoise;
    float *tempRandomRange = new float[2];
    if(noiseAdd == false) {
        // 不需要添加噪声量，则将噪声随机性降为0，并且将baseNoise置为0
        tempBaseNoise = 0;
        tempRandomRange[0] = 1;
        tempRandomRange[1] = 1;
    } else {
        tempRandomRange[0] = randomRange[0];
        tempRandomRange[1] = randomRange[1];
    }
    generateMatrix(allIndex, removedCarSet, removedCustomer, minInsertPerRoute,  
            minInsertPos, secondInsertPerRoute, secondInsertPos, 
            tempBaseNoise, DTpara, tempRandomRange, allowNegativeCost);
    // 剩下没有插入到路径中的节点下标，相对于removedCustomer
    vector<int> restCustomerIndex = allIndex;  
    // 各个removedcustomer的最小插入代价
    // 只包含没有插入到路径中的节点
    // 第一个整数是节点下标，第二个节点是车辆位置
    vector<pair<float, pair<int,int> > > minInsertPerRestCust(0);  
    while((int)alreadyInsertIndex.size() < removedCustomerNum){
        minInsertPerRestCust.clear();  // 每次使用之前先清空
        for(i=0; i<(int)restCustomerIndex.size(); i++){               // 只计算尚在路径中的节点
            int index = restCustomerIndex[i];
            int pos;
            float minValue;
            minValue = minInsertPerRoute.getMinAtCol(index, pos);
            minInsertPerRestCust.push_back(make_pair(minValue, make_pair(index, pos)));
        }	
        sort(minInsertPerRestCust.begin(), minInsertPerRestCust.end(), ascendSort<float, pair<int,int> >);
        // 被选中的顾客节点编号
        int selectedCustIndex = minInsertPerRestCust[0].second.first;  		
        if(minInsertPerRestCust[0].first != MAX_FLOAT){  
            // 如果找到了可行插入位置
            int selectedCarPos = minInsertPerRestCust[0].second.second;  // 被选中的车辆位置
            pair<Spot* , Spot*> ref = minInsertPos.getElement(selectedCarPos, selectedCustIndex);
            Spot* refStore = ref.first;
            Spot* refCustomer = ref.second;
            Spot* selectedCustomer = removedCustomer[selectedCustIndex];
            try {
                removedCarSet[selectedCarPos]->insertAfter(refStore, refCustomer, 
                        selectedCustomer->choice, selectedCustomer);
            } catch (exception &e) {
                cout << "Customer ID is #" << selectedCustomer->id << endl;
                throw out_of_range("In greedy insert, find valid position: " + string(e.what()));
            }
          
            alreadyInsertIndex.push_back(selectedCustIndex);
            vector<int>::iterator iterINT;
            // set_difference要求先排序
            sort(alreadyInsertIndex.begin(), alreadyInsertIndex.end());  
            // 更新restCustomerIndex
            iterINT = set_difference(allIndex.begin(), allIndex.end(), alreadyInsertIndex.begin(), 
                    alreadyInsertIndex.end(), restCustomerIndex.begin());
            restCustomerIndex.resize(iterINT-restCustomerIndex.begin());
            updateMatrix(restCustomerIndex, minInsertPerRoute, minInsertPos, 
                    secondInsertPerRoute, secondInsertPos, selectedCarPos, 
                    removedCarSet, removedCustomer, tempBaseNoise, 
                    DTpara, tempRandomRange, allowNegativeCost);
        } 
        else {  // 没有可行插入位置，则再新开一辆货车
            int selectedCarPos = carNum++;  // 被选中的车辆位置
            Car *newCar = new Car(depot, depot, capacity, newCarIndex++, hierarchicalCar);
            Spot *selectedCustomer;
            try {
                selectedCustomer = removedCustomer[selectedCustIndex];
                newCar->insertAtHead(selectedCustomer->choice, selectedCustomer);
            } catch (exception &e) {
                cout << "Selected index is: " << selectedCustIndex << endl;
                cout << "CustomerID is #" << selectedCustomer->id << " x-pos is " << selectedCustomer->x << " y-pos is: " << selectedCustomer->y << endl;
                showAllCustomerID(removedCustomer);
                throw out_of_range("In greedy insert, cannot find valid position: " + string(e.what()));
            }
            removedCarSet.push_back(newCar);  // 添加到货车集合中
            alreadyInsertIndex.push_back(selectedCustIndex); // 更新selectedCustIndex
            sort(alreadyInsertIndex.begin(), alreadyInsertIndex.end()); 
            vector<int>::iterator iterINT;
            // 更新restCustomerIndex
            iterINT = set_difference(allIndex.begin(), allIndex.end(), alreadyInsertIndex.begin(), 
                    alreadyInsertIndex.end(), restCustomerIndex.begin());
            restCustomerIndex.resize(iterINT-restCustomerIndex.begin());
            minInsertPerRoute.addOneRow();   // 增加一行
            minInsertPos.addOneRow();
            secondInsertPerRoute.addOneRow();
            secondInsertPos.addOneRow();
            updateMatrix(restCustomerIndex, minInsertPerRoute, minInsertPos, 
                    secondInsertPerRoute, secondInsertPos, selectedCarPos, 
                    removedCarSet, removedCustomer, tempBaseNoise, 
                    DTpara, tempRandomRange, allowNegativeCost);
        }
    }
    delete[] tempRandomRange;
}

void LNSBase::regretInsert(vector<Car*> &removedCarSet, vector<Spot*> removedCustomer,
							 bool noiseAdd){
    // regret insert算子, 把removedCustomer连同其指定的商店插入到removedCarSet中
    // Args:
    //   * removedCustomer: 被remove的顾客
    //   * noiseAdd: 是否需要添加噪声
    // Returns:
    //   * removedCarSet: 执行过remove算子的货车集合，最终会将removedCustomer(P-D)
    //                    重新插入其中

    int removedCustomerNum = removedCustomer.end() - removedCustomer.begin();  // 需要插入到路径中的节点数目
    int carNum = removedCarSet.end() - removedCarSet.begin();    // 车辆数目

    // 如果removedCarSet中货车数量为空，则抛出异常
    if (carNum == 0) {
        throw invalid_argument("In greedy insertion: Empty car in removedCarSet!");
    } 
    int newCarIndex = removedCarSet[carNum - 1]->getCarIndex();  // 新车编号
    int i;
    // 已经插入到路径中的节点下标，相对于allIndex
    vector<int> alreadyInsertIndex(0);		
    // 在每条路径中的最小插入代价矩阵（行坐标：车辆，列坐标：顾客）
    Matrix<float> minInsertPerRoute(carNum, removedCustomerNum);     
    // 在每条路径中的最小插入代价所对应的节点
    Matrix<pair<Spot*, Spot*> > minInsertPos(carNum, removedCustomerNum);       
    // 在每条路径中的次小插入代价矩阵
    Matrix<float> secondInsertPerRoute(carNum, removedCustomerNum);  
    // 在每条路径中次小插入代价所对应的节点
    Matrix<pair<Spot*, Spot*> > secondInsertPos(carNum, removedCustomerNum);
    vector<int> allIndex(0);   // 对removedCustomer进行编号

    float tempBaseNoise = baseNoise;
    float *tempRandomRange = new float[2];
    if(noiseAdd == false) {
        // 不需要添加噪声量，则将噪声随机性降为0，并且将baseNoise置为0
        tempBaseNoise = 0;
        tempRandomRange[0] = 1;
        tempRandomRange[1] = 1;
    } else {
        tempRandomRange[0] = randomRange[0];
        tempRandomRange[1] = randomRange[1];
    }
    generateMatrix(allIndex, removedCarSet, removedCustomer, minInsertPerRoute,  
            minInsertPos, secondInsertPerRoute, secondInsertPos,
            tempBaseNoise, DTpara, tempRandomRange, allowNegativeCost);
    // 尚未插入到路径中的节点位置，相对于最初的removedCustomer
    vector<int> restCustomerIndex = allIndex;  
    // 各个removedCustomer的最小插入代价与次小插入代价之差
    // 只包含没有插入到路径中的节点
    // 第一个整数是节点下标，第二个节点是车辆下标
    vector<pair<float, pair<int,int> > > regretdiffPerRestCust(0);
	
    while((int)alreadyInsertIndex.size() < removedCustomerNum){
        int selectedCustIndex;   // 选中的顾客节点编号
        int selectedCarPos;      // 选中的货车位置
        regretdiffPerRestCust.clear();
        for(i=0; i<(int)restCustomerIndex.size(); i++){
            int index = restCustomerIndex[i];        // 顾客节点下标
            float minValue, secondValue1, secondValue2;
            int pos1, pos2, pos3;
            minValue = minInsertPerRoute.getMinAtCol(index, pos1);          // 最小插入代价
            minInsertPerRoute.setValue(pos1, index, MAX_FLOAT);
            secondValue1 = minInsertPerRoute.getMinAtCol(index, pos2);      // 候选次小插入代价
            minInsertPerRoute.setValue(pos1, index, minValue);              // 恢复数据
            secondValue2 = secondInsertPerRoute.getMinAtCol(index, pos3);   // 候选次小插入代价
            if(minValue == MAX_FLOAT){  
                // 如果发现某个节点已经没有可行插入点，则优先安排之
                regretdiffPerRestCust.push_back(make_pair(MAX_FLOAT, make_pair(index, pos1)));
            } else if(minValue != MAX_FLOAT && secondValue1==MAX_FLOAT && secondValue2==MAX_FLOAT){
                // 如果只有一个可行插入点，则应该优先安排
                // 按照minValue的值，最小者应该率先被安排
                // 因此diff = LARGE_FLOAT - minValue
                regretdiffPerRestCust.push_back(make_pair(LARGE_FLOAT-minValue, 
                            make_pair(index, pos1)));
            } 
            else{
                if(secondValue1 <= secondValue2){
                    regretdiffPerRestCust.push_back(make_pair(abs(minValue-secondValue1), 
                                make_pair(index, pos1)));
                } else{
                    regretdiffPerRestCust.push_back(make_pair(abs(minValue-secondValue2), 
                                make_pair(index, pos1)));
                }
            }
        }
        // 应该由小到大进行排列
        sort(regretdiffPerRestCust.begin(), regretdiffPerRestCust.end(), 
                descendSort<float, pair<int, int> >);
        if(regretdiffPerRestCust[0].first == MAX_FLOAT) {
            // 如果所有的节点都没有可行插入点，则开辟新车
            selectedCarPos = carNum++;
            selectedCustIndex = regretdiffPerRestCust[0].second.first;
            Car *newCar = new Car(depot, depot, capacity, newCarIndex++, hierarchicalCar);
            Spot *selectedCustomer;
            try {
                selectedCustomer = removedCustomer[selectedCustIndex];
                newCar->insertAtHead(selectedCustomer->choice, selectedCustomer);
            } catch (exception &e) {
                cout << "Customer ID is #" << selectedCustomer->id << endl;
                throw out_of_range("In regret insert, has invalid position" + string(e.what()));
            }
            removedCarSet.push_back(newCar);  // 添加到货车集合中
            alreadyInsertIndex.push_back(selectedCustIndex); // 更新selectedCustIndex
            sort(alreadyInsertIndex.begin(), alreadyInsertIndex.end());
            vector<int>::iterator iterINT;
            // 更新restCustomerIndex
            iterINT = set_difference(allIndex.begin(), allIndex.end(), alreadyInsertIndex.begin(), 
                    alreadyInsertIndex.end(), restCustomerIndex.begin());
            restCustomerIndex.resize(iterINT-restCustomerIndex.begin());
            minInsertPerRoute.addOneRow();   // 增加一行
            minInsertPos.addOneRow();
            secondInsertPerRoute.addOneRow();
            secondInsertPos.addOneRow();
            updateMatrix(restCustomerIndex, minInsertPerRoute, minInsertPos, 
                    secondInsertPerRoute, secondInsertPos, selectedCarPos, 
                    removedCarSet, removedCustomer, tempBaseNoise, 
                    DTpara, tempRandomRange, allowNegativeCost);	
        } else {
            // 否则，不需要开辟新车
            selectedCarPos = regretdiffPerRestCust[0].second.second;
            selectedCustIndex = regretdiffPerRestCust[0].second.first;
            alreadyInsertIndex.push_back(selectedCustIndex);
            Spot *selectedCustomer;
            try {
                pair<Spot*, Spot*> ref = minInsertPos.getElement(selectedCarPos, selectedCustIndex);
                Spot *refStore = ref.first;
                Spot *refCustomer = ref.second;
                selectedCustomer = removedCustomer[selectedCustIndex];
                removedCarSet[selectedCarPos]->insertAfter(refStore, refCustomer, selectedCustomer->choice,
                        selectedCustomer);
            } catch (exception &e) {
                cout << "CustomerID is #" << selectedCustomer->id << endl;
                throw out_of_range("In regret insert, has valid position" + string(e.what()));
            }
            sort(alreadyInsertIndex.begin(), alreadyInsertIndex.end());
            vector<int>::iterator iterINT;
            iterINT = set_difference(allIndex.begin(), allIndex.end(), alreadyInsertIndex.begin(), 
                    alreadyInsertIndex.end(), restCustomerIndex.begin());
            restCustomerIndex.resize(iterINT-restCustomerIndex.begin());
            updateMatrix(restCustomerIndex, minInsertPerRoute, minInsertPos, 
                    secondInsertPerRoute, secondInsertPos, selectedCarPos, 
                    removedCarSet, removedCustomer, tempBaseNoise, 
                    DTpara, tempRandomRange, allowNegativeCost);
        }
    }
    delete[] tempRandomRange;
}

void LNSBase::reallocateCarIndex(vector<Car*> &originCarSet){  
    // 重新为货车编号
    // returns:
    //   * originCarSet: 初始货车集合，经本函数后其中的货车编号可能该Bain
    int count = 0;
    for(int i=0; i<(int)originCarSet.size(); i++){
        if(originCarSet[i]->judgeArtificial() == false) {  
            // 若为真实的车辆，则不需要重新编号
            count++;
        } else{  
            // 若为虚假的车辆，则重新编号
            originCarSet[i]->changeCarIndex(count);
            count++;
        }
    }
}

void LNSBase::removeNullRoute(vector<Car*> &originCarSet, bool mark){    
    // 清除OriginCarSet中的空车辆
    // 若mark=true, 则只允许清除虚拟的空车
    vector<Car*>::iterator iter;
    vector<Car*>::iterator temp;
    int count = 0;
    for(iter=originCarSet.begin(); iter<originCarSet.end();){
        if ((*iter)->getRoute()->getSize() == 0){
            if(mark == true) {
                if ((*iter)->judgeArtificial() == true) { 
                    // 如果是空车而且是虚拟的车
                    delete(*iter);
                    iter = originCarSet.erase(iter);
                } else{
                    (*iter)->changeCarIndex(count++);
                    ++iter;				
                }
            }
            else {
                delete(*iter);
                iter = originCarSet.erase(iter);
            }
        } 
        else {
            (*iter)->changeCarIndex(count++);
            ++iter;
        }
    }
}

size_t LNSBase::codeForSolution(vector<Car*> originCarSet){  
    // 对每个解（多条路径）进行hash编码
    stringstream ss;
    string allStr = "";
    for(int i=0; i<(int)originCarSet.size(); i++) {
        vector<int> IDInCar = originCarSet[i]->getAllID();
        for(vector<int>::iterator iter=IDInCar.begin(); iter<IDInCar.end(); iter++) {
            ss.str("");
            ss << *iter;
            allStr += ss.str();
        }
    }
    ss.str("");
    hash<string> str_hash;
    return str_hash(allStr);
}

float LNSBase::getCost(vector<Car*> originCarSet){
    // 返回originCarSet的路长
    float totalCost = 0;
    for(int i=0; i<(int)originCarSet.size(); i++){
        float temp;
        if(originCarSet[i]->judgeArtificial() == true) {
            temp = originCarSet[i]->getRoute()->getLen(DTpara, true);
        } else {
            temp = originCarSet[i]->getRoute()->getLen(DTpara, false);
        }
        totalCost += temp;
    }
    return totalCost;
}

float LNSBase::getTrueCost(vector<Car*> carSet) {
    // 得到carSet的真实路径长度
    // 不管是artificial还是working car，都不惩罚或者奖励
    float cost = 0;
    float temp[4] = {0, 0, 0, 0};
    for(int i=0; i<(int)carSet.size(); i++) {
        cost += carSet[i]->getRoute()->getLen(temp, true);
    }
    return cost;
}

void LNSBase::updateWeight(int *freq, float *weight, int *score, float r, int num) {  
    // 更新权重
    for(int i=0; i<num; i++){
        if(*freq != 0){
            *weight = *weight *(1-r) + *score / *freq*r;
        } else {    // 如果在上一个segment中没有使用过该算子，权重应当下降
            *weight = *weight*(1-r);
        }
        freq++;
        weight++;
        score++;
    }
}

void LNSBase::updateProb(float *removeProb, float *removeWeight, int removeNum){
    // 更新概率
    float accRemoveWeight = 0;  // remove权重之和
    for(int k=0; k<removeNum; k++){
        accRemoveWeight += removeWeight[k];
    }
    for(int i=0; i<removeNum; i++){
        *removeProb = *removeWeight/accRemoveWeight;
        removeProb++;
        removeWeight++;
    }
}

