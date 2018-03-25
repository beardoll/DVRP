#include "Dispatcher.h"
#include<algorithm>
#include "Simulator.h"
#include "../public/PublicFunction.h"
#include "../run/TxtRecorder.h"
#include "../run/Config.h"
#include <stdexcept>

Dispatcher::Dispatcher(vector<Spot*> staticCustomerSet, vector<Spot*> dynamicCustomerSet, 
        vector<Spot*> storeSet, Spot depot, float capacity) {
    this->depot = depot;
    this->capacity = capacity;
    this->storeSet = storeSet;
    this->globalCarIndex = 0;
    int custNum = staticCustomerSet.end() - staticCustomerSet.begin();
    custNum += dynamicCustomerSet.end() - dynamicCustomerSet.begin(); // 总顾客数
    servedCustomerId.reserve(custNum);     // 已经服务过的顾客id
    promisedCustomerId.reserve(custNum);
    waitCustomerId.reserve(custNum);
    rejectCustomerId.reserve(custNum);
    vector<Spot*>::iterator custIter = staticCustomerSet.begin();
    for(custIter; custIter < staticCustomerSet.end(); custIter++) {
        // 在计划开始前已经提出需求的顾客都属于promiseCustomer
        allCustomer.push_back(*custIter);
        promisedCustomerId.push_back((*custIter)->id);
    }
    for(custIter = dynamicCustomerSet.begin(); custIter < dynamicCustomerSet.end(); custIter++) {
        allCustomer.push_back(*custIter);	
        dynamicCustomerId.push_back((*custIter)->id);
    }
    sort(allCustomer.begin(), allCustomer.end(), ascendSortForCustId);  // 按id进行递增排序
    sort(promisedCustomerId.begin(), promisedCustomerId.end());
    sort(dynamicCustomerId.begin(), dynamicCustomerId.end());
}


void Dispatcher::destroy() {
    deleteCustomerSet(allCustomer);
}

void Dispatcher::carFinishTask(int carIndex){       
    // 收车
    // 我们会从currentPlan中删除id为carIndex的顾客
    // 并且将其加入到finishedPlan
    vector<Car*>::iterator carIter;
    ostringstream ostr;
    for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
        if((*carIter)->getCarIndex() == carIndex) {
            Car *newCar = new Car(**carIter);
            delete(*carIter);
            currentPlan.erase(carIter);
            finishedPlan.push_back(newCar);
            break;
        }
    }
    if(currentPlan.size() == 0) {
        ostr.str("");
        ostr << "=============== END ================" << endl;
        ostr << "----OH!! Finished!" << endl;
        ostr << "----" << servedCustomerId.size() << " customers get served finally" << endl;
        ostr << "----" << rejectCustomerId.size() << " customers get rejected finally" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
    } else {
        // 如果所有剩余的车辆都是空车
        bool mark = true;
        int num = 0;
        for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
            if((*carIter)->getRoute()->getSize() != 0) {
                mark = false;
                break;
            } else {
                num++;
            }
        }
        if(mark == true) {
            ostr.str("");
            ostr << "=============== END ================" << endl;
            ostr << "----There are " << num << " Cars remains unused!" << endl;
            ostr << "----" << servedCustomerId.size() << " customers get served finally" << endl;
            ostr << "----" << rejectCustomerId.size() << " customers get rejected finally" << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();	
        }
    }
}

void checkFeasible(vector<Car*> carSet, vector<int> promisedCustomerId){
    // 判断promiseCustomerSet中的顾客是否都在carSet中
    vector<int> tempId = promisedCustomerId;  // 得到了promise的顾客id
    sort(tempId.begin(), tempId.end());
    vector<Car*>::iterator carIter;
    for(carIter = carSet.begin(); carIter < carSet.end(); carIter++) {  
        vector<Spot*> tempCust = (*carIter)->getAllCustomer();
        vector<Spot*>::iterator custIter;
        for(custIter = tempCust.begin(); custIter < tempCust.end(); custIter++) {
            vector<int>::iterator intIter = find(tempId.begin(), tempId.end(), (*custIter)->id);
            if(intIter < tempId.end()) {
                // 如果找到了，就删掉
                tempId.erase(intIter);
            }
        }
    }
    if(tempId.size() != 0) {   // if there are promiseCustomers excluded
        throw out_of_range("Not all promise customers are in the car set!");
    }
}

void checkConnection(vector<Car*> carSet) {
    // 检查每一辆车中store和customer的数量是否对等
    vector<Car*>::iterator carIter;
    for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
        vector<Spot*> allSpot = (*carIter)->getRoute()->getAllSpot();
        int count = 0;
        vector<Spot*>::iterator custIter;
        for(custIter=allSpot.begin(); custIter<allSpot.end(); custIter++) {
            if((*custIter)->id >= 1000) {
                // id大于等于1000目前是store
                count++;
            } else {
                count--;
            }
        }
        if(count != 0) {
            ostringstream ostr;
            ostr << "Car #" << (*carIter)->getCarIndex() << 
                "has unbalanced ids"; 
            throw out_of_range(ostr.str());
            break;
        }
    }
}


vector<EventElement> Dispatcher::handleNewTimeSlot(int slotIndex){ 
    // 新时间段开始
    cout << "Handle new time slot!" << endl;
    vector<Spot*> promiseCustomerSet;
    vector<Spot*> waitCustomerSet;
    vector<Spot*> dynamicCustomerSet;
    vector<int>::iterator custIdIter;
    vector<Car*>::iterator carIter;
    vector<Car*> updatedPlan;
    for(custIdIter = promisedCustomerId.begin(); custIdIter< promisedCustomerId.end(); custIdIter++) {
        promiseCustomerSet.push_back(allCustomer[*custIdIter - 1]);  // id从1开始编号
    }
    for(custIdIter = dynamicCustomerId.begin(); custIdIter < dynamicCustomerId.end(); custIdIter++) {
        dynamicCustomerSet.push_back(allCustomer[*custIdIter - 1]);
    }
    vector<EventElement> newEventList;
    ostringstream ostr;
    if(slotIndex == 1) {  // 路径计划需要初始化
        ostr.str("");
        ostr << "============ Now Initialize the routing plan ===========" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        Simulator smu(slotIndex, promiseCustomerSet, waitCustomerSet, dynamicCustomerSet, 
                currentPlan, storeSet);
        depot.arrivedTime = slotIndex * TIME_SLOT_LEN;
        currentPlan = smu.initialPlan(depot, capacity);
        globalCarIndex = currentPlan.size();
        for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
            EventElement newEvent = (*carIter)->launchCar(slotIndex*TIME_SLOT_LEN);  // 将车辆发动
            if(newEvent.customerId != 0 && newEvent.customerId != -1) {
                cout << "Car #" << newEvent.carIndex << " has been launched!" << endl;
            }
            newEventList.push_back(newEvent);
        }
        try {
            checkConnection(currentPlan);
        } catch(exception &e) {
            cout << "In initial: " << e.what() << endl;
        }
        ostr.str("");
        ostr << "----Initialization Finished! Now there are " << currentPlan.size() 
            << " cars dispartched!" << endl << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
    } else {
        // 调取每辆车未走过的路径进行仿真，得到新的路径计划
        ostr.str("");
        ostr << "============ Now replan, the time slot is: " << slotIndex << "============" << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        float currentTime = slotIndex * TIME_SLOT_LEN;
        for(custIdIter = waitCustomerId.begin(); custIdIter < waitCustomerId.end(); custIdIter++) {
            Spot *temp = allCustomer[*custIdIter-1];
            waitCustomerSet.push_back(temp);
        }
        vector<Car*> futurePlan;
        for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
            // 抓取车子还没有走过的计划
            Car *tempCar = (*carIter)->capturePartRoute(currentTime);
            futurePlan.push_back(tempCar);
        }
        if (currentPlan.size() != 0) {  // 有货车可派时，才进行replan
            Simulator smu(slotIndex, promiseCustomerSet, waitCustomerSet, dynamicCustomerSet, 
                    futurePlan, storeSet);
            vector<int> newservedCustomerId;
            vector<int> newAbandonedCustomerId;
            vector<int> delayCustomerId;
            updatedPlan = smu.replan(newservedCustomerId, newAbandonedCustomerId, 
                    delayCustomerId, capacity);
            withdrawPlan(futurePlan);
            //updatedPlan = smu.no_replan();
            vector<Spot*>::iterator custIter;

            // 更新promiseCustomerId, rejectCustomerId以及waitCustomerId
            vector<int>::iterator intIter1, intIter2;
            promisedCustomerId.insert(promisedCustomerId.end(), newservedCustomerId.begin(), 
                    newservedCustomerId.end());
            sort(promisedCustomerId.begin(), promisedCustomerId.end());
            
            // tempVec: 存放new served以及new abandoned顾客
            //          将原来的wait Customer和tempVec作差即可得到最新的wait Customer
            vector<int> tempVec;
            tempVec.insert(tempVec.end(), newservedCustomerId.begin(), newservedCustomerId.end());

            rejectCustomerId.insert(rejectCustomerId.end(), newAbandonedCustomerId.begin(),
                    newAbandonedCustomerId.end());
            sort(rejectCustomerId.begin(), rejectCustomerId.end());
            tempVec.insert(tempVec.end(), newAbandonedCustomerId.begin(),
                    newAbandonedCustomerId.end());
            
            // 检查是否tempVec的元素都在原来的wait Customer中
            for(intIter1 = tempVec.begin(); intIter1 < tempVec.end(); intIter1++) {
                intIter2 = find(waitCustomerId.begin(), waitCustomerId.end(), *intIter1);
                if(intIter2 == waitCustomerId.end()) { 
                    // 没有找到，报错
                    cerr << "tempVec not totally in waitCustomerId!!" << endl;
                    exit(1);
                }   
            }

            if(tempVec.size() != 0) {
                // 提取最新的waitCustomerId
                sort(waitCustomerId.begin(), waitCustomerId.end());
                sort(tempVec.begin(), tempVec.end());
                vector<int> tempVec2(20);
                intIter1 = set_difference(waitCustomerId.begin(), waitCustomerId.end(), 
                        tempVec.begin(), tempVec.end(), tempVec2.begin());
                tempVec2.resize(intIter1 - tempVec2.begin());
                waitCustomerId = tempVec2;
            }
            
            // 将变更后的future plan安插到currentPlan对应位置之后
            int count = 0;
            for (carIter = updatedPlan.begin(); carIter < updatedPlan.end(); carIter++) {
                currentPlan[count]->replaceRoute(*carIter, currentTime);
                EventElement newEvent;
                if (currentPlan[count]->getState() == wait) {
                    // 如果货车原来处于wait状态，则需要将其发动
                    newEvent = currentPlan[count]->launchCar(currentTime);
                    if(newEvent.customerId != -1) {
                        cout << "Car #" << newEvent.carIndex << 
                            " has been launched!" << endl;
                    }
                }
                else {
                    newEvent = currentPlan[count]->getCurrentAction(currentTime);
                }
                newEventList.push_back(newEvent);
                count++;
            }
            
            // 之前出现过replace part route后allCustomer数据被替换
            // 因此在这里跟踪这个情况
            for(int i=0; i<allCustomer.size(); i++) {
                if(allCustomer[i]->type != 'C') {
                    cout << i << "th elements in allCustomer is not customer" << endl;
                    exit(1);
                }
            }

            try {
                checkConnection(currentPlan);
            } catch(exception &e) {
                cout << "In replan: " << e.what() << endl;
            }
            ostr.str("");
            ostr << "----Replan Finished! Now there are " << currentPlan.size() 
                << " cars working!" << endl << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
               
        }
        else {
            ostr.str("");
            ostr << "----no car is applicable!!!" << endl << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
        }
    }
    return newEventList;
}

int mapID(Spot *node) {
    int id;
    if(node->type == 'D') {
        id = 0;
    } else if (node->type == 'S') {
        id = node->choice->id;
    } else {
        id = node->id;
    }
    return id;
}

Spot* getMappedNode(int id, char type, Car *currentCar) {
    if(type == 'D') {
        return currentCar->getCurrentNode();
    } 
    else {
        Spot *customer = currentCar->findCustomer(id);
        if(type == 'S') {
            return customer->choice;
        } else {
            return customer;
        }
    }
}

EventElement Dispatcher::handleNewCustomer(int slotIndex, Spot *newCustomer){
    // 处理新顾客(dynamic)到达
    // Args:
    //    * slotIndex: 用以判断顾客是否有耐心等到下一个slotIndex
    //    * newCustomer: 新顾客节点
    // Returns: (存放于成员变量中)
    //    * newCar: 为了服务newCustomer，可能需要派遣新的骑手
    //    * globalCarIndex: 新车的id，如果派遣了新车，则自加1         
    ostringstream ostr;
    ostr.str("");
    ostr<< "----Time: " << newCustomer->startTime << ", Customer with id #" 
        << newCustomer->id << " is arriving..." << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();
    Spot *copyNewCustomer = new Spot(*newCustomer);
    Spot *copyNewStore = new Spot(*(newCustomer->choice));
    copyNewCustomer->choice = copyNewStore;
    copyNewStore->choice = copyNewCustomer;
    vector<int>::iterator intIter = find(dynamicCustomerId.begin(), dynamicCustomerId.end(), 
            copyNewCustomer->id);
    dynamicCustomerId.erase(intIter);
    float minInsertCost = MAX_FLOAT;
    // insertPos:
    //    * 第一个int是货车编号（于currentPlan中的位置）
    //    * 第二个pair组合都是customer id(即使插入点是store，也记录其customer)
    pair<int, pair<int, int> > insertPos;   
    pair<int, pair<char, char> > insertType;
    vector<Car*>::iterator carIter;
    float currentTime = copyNewCustomer->startTime;       // 顾客提出需求的时间正好是时间窗开始的时间
    for (carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
        // 求copyNewCustomer在每条route的最小插入代价
        Car *tempCar;
        try {
            tempCar = (*carIter)->capturePartRoute(currentTime);
        } catch (exception &e) {
            cout << "While handling new customer: " << endl;
            cout << e.what() << endl;
        }
        Spot *refStore1, *refCustomer1, *refStore2, *refCustomer2;
        float minValue, secondValue;
        tempCar->computeInsertCost(copyNewCustomer->choice, copyNewCustomer, minValue, 
                refStore1, refCustomer1, secondValue, refStore2, refCustomer2, 
                currentTime);
        if(minValue < minInsertCost) {
            int pos = carIter - currentPlan.begin();  
            minInsertCost = minValue;           
            insertPos = make_pair(pos, make_pair(mapID(refStore1), 
                        mapID(refCustomer1)));
            insertType = make_pair(pos, make_pair(refStore1->type, 
                        refCustomer1->type));
        }
    }
    EventElement newEvent;
    if(minInsertCost == MAX_FLOAT) {
        // 没有可行插入点
        if(copyNewCustomer->tolerantTime < slotIndex * TIME_SLOT_LEN) { 
            ostr.str("");
            ostr << "He is rejected!" << endl;
            ostr << "His tolerance time is " << copyNewCustomer->tolerantTime << endl;
            ostr << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            rejectCustomerId.push_back(copyNewCustomer->id);
            sort(rejectCustomerId.begin(), rejectCustomerId.end());

            //// 等不到replan，则优先安排新的骑手为其服务
            //Spot *newDepot = new Spot(depot);
            //newDepot->arrivedTime = copyNewCustomer->startTime;
            //Car *newCar = new Car(*newDepot, *newDepot, capacity, globalCarIndex);
            //Spot *refStore1, *refCustomer1, *refStore2, *refCustomer2;
            //float minValue, secondValue;
            //newCar->computeInsertCost(copyNewCustomer->choice, copyNewCustomer, minValue, 
            //        refStore1, refCustomer1, secondValue, refStore2, refCustomer2, 
            //        currentTime);
            //if(minValue == MAX_FLOAT) {
            //    // 如果已经来不及派送，则拒绝为其服务
            //    // 最好尽量避免tolerantTime-endTime为不可达时间
            //    ostr.str("");
            //    ostr << "He is rejected!" << endl;
            //    ostr << "His tolerance time is " << copyNewCustomer->tolerantTime << endl;
            //    ostr << endl;
            //    TxtRecorder::addLine(ostr.str());
            //    cout << ostr.str();
            //    rejectCustomerId.push_back(copyNewCustomer->id);
            //    sort(rejectCustomerId.begin(), rejectCustomerId.end());
            //    delete newCar;
            //} 
            //else {
            //    // 否则，将其安排给新的骑手
            //    promisedCustomerId.push_back(copyNewCustomer->id);
            //    sort(promisedCustomerId.begin(), promisedCustomerId.end());
            //    globalCarIndex++;
            //    newCar->insertAtHead(copyNewCustomer->choice, copyNewCustomer);
            //    newEvent = newCar->launchCar(copyNewCustomer->startTime);
            //    currentPlan.push_back(newCar);
            //    cout << "Open new Car #" << newCar->getCarIndex() <<
            //        " to serve him" << endl << endl;
            //}
        } else {  
            // 否则，进入等待的顾客序列
            ostr.str("");
            ostr << "He will wait for replan!" << endl << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
            waitCustomerId.push_back(copyNewCustomer->id);  
            sort(waitCustomerId.begin(), waitCustomerId.end());
        }
    } else {
        promisedCustomerId.push_back(copyNewCustomer->id);  // 这些顾客一定会得到服务
        sort(promisedCustomerId.begin(), promisedCustomerId.end());
        int selectedCarPos = insertPos.first;
        int refStoreID = insertPos.second.first;
        int refCustomerID = insertPos.second.second;
        char refStoreType = insertType.second.first;
        char refCustomerType = insertType.second.second;
        Car *selectedCar = currentPlan[selectedCarPos];
        Spot *refStore = getMappedNode(refStoreID, refStoreType, selectedCar);
        Spot *refCustomer = getMappedNode(refCustomerID, refCustomerType, selectedCar);
        try {
            selectedCar->insertAfter(refStore, refCustomer, 
                    copyNewCustomer->choice, copyNewCustomer, currentTime);
        } catch (exception &e) {
            cout << "current id: " << selectedCar->getRoute()->currentPos()->id << endl;
            cout << "refStoreType: " << refStore->type << " refCustomerType: "
                << refCustomer->type << endl;
            cout << e.what() << endl;
            exit(1);
        }
        if(selectedCar->getState() == wait) {  // if the car stays asleep
            newEvent = selectedCar->launchCar(currentTime);
            cout << "launch Car #" << selectedCar->getCarIndex() << endl; 
        } else {
            newEvent = selectedCar->getCurrentAction(currentTime);
        }
        int carIndex = selectedCar->getCarIndex();
        try {
            selectedCar->getRoute()->checkArrivedTime();
        } catch (exception &e) {
            cout << "Car #" << carIndex << " invalid!!!" << endl;
            exit(1);
        }
        ostr.str("");
        ostr << "He is arranged to Car #" << carIndex << endl;
        ostr << "refStore: #" << refStore->id << " refCustomer: #" <<
            refCustomer->id << endl << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
    }
    return newEvent;
}

EventElement Dispatcher::handleCarArrived(float time, int carIndex){                 
    // 处理货车到达事件
    // 在此处更新顾客的到达时间
    ostringstream ostr;
    vector<Car*>::iterator carIter;
    int pos;  // carIndex对应的车辆在currentPlan中的位置
    for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
        if((*carIter)->getCarIndex() == carIndex) {
            pos = carIter - currentPlan.begin();
            break;
        }
    }
    EventElement tempEvent = currentPlan[pos]->getCurrentAction(time);
    if(currentPlan[pos]->getState() == offwork) {
        // 收车
        ostr.str("");
        ostr << "----Time " << time << ", Car #" << currentPlan[pos]->getCarIndex() 
            << " finished its task!" << endl << endl;
        TxtRecorder::addLine(ostr.str());
        cout << ostr.str();
        carFinishTask(tempEvent.carIndex);
    } else {
        // 更新newservedCustomerId以及promisedCustomerId
        Spot *currentPos = currentPlan[pos]->getCurrentNode();
        int currentId = currentPos->id;
        if(currentPos->type == 'C'){
            vector<int>::iterator intIter = find(promisedCustomerId.begin(), 
                promisedCustomerId.end(), currentId);
            promisedCustomerId.erase(intIter);
            servedCustomerId.push_back(currentId);
            sort(servedCustomerId.begin(), servedCustomerId.end());
            ostr.str("");
            ostr << "----Time " << time << ", Car #" << currentPlan[pos]->getCarIndex() 
                << " arrives at customer #" << currentId << endl << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
        } else {
            ostr.str("");
            ostr << "----Time " << time << ", Car #" << currentPlan[pos]->getCarIndex()
                << " arrives at store #" << currentId << endl << endl;
            TxtRecorder::addLine(ostr.str());
            cout << ostr.str();
        }
    }
    return tempEvent;
}

EventElement Dispatcher::handleFinishedService(float time, int carIndex){       
    // 处理货车完成服务事件
    ostringstream ostr;
    EventElement newEvent(-1, carArrived, -1, -1);     // 无效事件
    vector<Car*>::iterator carIter;
    int pos;  // carIndex对应的车辆在currentPlan中的位置
    for(carIter = currentPlan.begin(); carIter < currentPlan.end(); carIter++) {
        if((*carIter)->getCarIndex() == carIndex) {
            pos = carIter - currentPlan.begin();
            break;
        }
    }
    newEvent = currentPlan[pos]->getCurrentAction(time);
    try {
        currentPlan[pos]->getRoute()->checkArrivedTime();
    } catch (exception &e) {
        cout << "Car #" << carIndex << " seems wrong after providing service." << endl;
        exit(1);
    }
    Spot *currentNode = currentPlan[pos]->getCurrentNode();
    int currentId = currentNode->id;
    ostr.str("");
    if(currentNode->type == 'C') {
        ostr << "----Time " << time << ", Car #" << carIndex << 
            " finished service in customer #" << currentId << endl;
    } else {
        ostr << "----Time " << time << ", Car #" << carIndex << 
            " finished service in store #" << currentId << endl;
    }
    ostr << "Its end time for servce is " << currentPlan[pos]->getCurrentNode()->endTime 
        << endl << endl;
    TxtRecorder::addLine(ostr.str());
    cout << ostr.str();
    return newEvent;
}


