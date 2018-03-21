#include "Car.h"
#include "../public/PublicFunction.h"
#include "../run/Config.h"
#include<cmath>
#include <stdexcept>

Car::Car(Spot &headNode, Spot &rearNode, float capacity, int index, bool artificial):
    carIndex(index), route(headNode, rearNode, capacity), artificial(artificial)
{
    state = wait;
    nearestDepartureTime = 0;
    nextArriveTime = 0;
    travelDistance = 0;
}

Car::Car(Route &route, int index, bool artificial): carIndex(index), 
    artificial(artificial), route(route)
{
    state = wait;
    nearestDepartureTime = 0;
    nextArriveTime = 0;
    travelDistance = 0;
}

Car::~Car(){  
    // 内嵌对象的析构函数会被调用，不用在此处delete route
}

Car::Car(const Car& item):route(item.route), artificial(item.artificial){  
    // 复制构造函数
    this->state = item.state;
    this->carIndex = item.carIndex;
    this->state = item.state;
    this->artificial = item.artificial;
    this->nearestDepartureTime = item.nearestDepartureTime;
    this->nextArriveTime = item.nextArriveTime;
    this->travelDistance = item.travelDistance;
}


Car& Car::operator= (Car &item){ 
    // 重载赋值操作
    this->route = item.route;
    this->carIndex = item.carIndex;
    this->state = item.state;
    this->artificial = item.artificial;
    this->nearestDepartureTime = item.nearestDepartureTime;
    this->nextArriveTime = item.nextArriveTime;
    this->travelDistance = item.travelDistance;
    return *this;
}


//================ 得到货车属性 =================//
Car* Car::getNullCar(vector<Spot*> &removedCustomer){
    // 复制货车的首节点和尾节点以及剩余容量
    // 需要保留choice指向depot的customer，他们不可以被转移到别的车上
    // 返回的Car需要从外部进行delete
    Route *emptyRoute = route.getEmptyRoute(removedCustomer);
    Car* newCar = new Car(*emptyRoute, carIndex);
    return newCar;
}

vector<int> Car::getAllID(){
    // 获得所有节点的ID，按顺序
    return route.getAllID();
}


//================ insert cost和remove cost =================//
void Car::computeInsertCost(Spot *store, Spot *customer, float &minValue, 
        Spot *&refStore1, Spot *&refCustomer1, float &secondValue, Spot *&refStore2,
        Spot *&refCustomer2, float randomNoise, bool allowNegativeCost){
        
    // 计算服务对(store, customer)在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点(refStore, refCustomer)
    // Args:
    //   * randomNoise: 随机化的噪声量
    //   * allowNegativeCost: 是否允许出现负的insertion cost
    // Returns:
    //   * refStore1, refCustomer1: 若minValue=MAX_FLOAT，则二者均为NULL
    //   * refStorer2, refCustomer2: 若secondValue=MAX_FLOAT，则二者均为NULL
    route.computeInsertCost(store, customer, minValue, refStore1, refCustomer1,
            secondValue, refStore2, refCustomer2, randomNoise, allowNegativeCost);
}

vector<float> Car::computeReducedCost(float DTpara[]){  
    // 得到路径中所有服务对(P-D)的移除代价，值越小表示它可以节省更多的代价
    // Args:
    //   * DTpara: 不同优先级顾客计算reduce cost时的bonus
    return route.computeReducedCost(DTpara, artificial);
}


//================ insert 和 delete Customer方法 =================//
void Car::insertAtRear(Spot *item){
    // 一般是用在往空车里面塞节点，因此不重新计算nextArriveTime
    try {
        route.insertAtRear(item);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
} 

void Car::insertAtHead(Spot *store, Spot *customer){
    // 只能当车子为空时才可使用，也不重新计算nextArriveTime
    try {
        route.insertAtHead(store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
}

void Car::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, 
        Spot *customer) {
    // 在refStore后面插入store，在refCustomer后面插入customer
    // update: 是否需要更新stand节点
    try {
        route.insertAfter(refStore, refCustomer, store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
    // 更新nextArriveTime，不管下一站目的地是否改变
    Spot *lastStop = route.getStand();  // 货车最近驻点（上一个访问的节点）
    // 当前时间 = 从lastStop出发的时间 + dist(lastStop, refStore)
    float currentTime = lastStop->arrivedTime + lastStop->serviceTime;
    Spot *nextStop = route.nextPos();
    nextArriveTime = currentTime + dist(lastStop, nextStop);
    // 更新nearestDepartureTime
    if(state == departure) {
        nearestDepartureTime = currentTime;
    }
}

void Car::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store,
        Spot *customer, float time) {
    // 需要更新stand节点的版本
    updateState(time);  
    try {
        route.insertAfter(refStore, refCustomer, store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
    // 更新nextArriveTime，不管下一站目的地是否改变
    Spot *lastStop = route.getStand();  // 货车最近驻点（上一个访问的节点）
    // 当前时间 = 从lastStop出发的时间 + dist(lastStop, refStore)
    float currentTime = lastStop->arrivedTime + lastStop->serviceTime;
    Spot *nextStop = route.nextPos();
    nextArriveTime = currentTime + dist(lastStop, nextStop);
    // 更新nearestDepartureTime
    if(state == departure) {
        nearestDepartureTime = currentTime;
    }
}

void Car::deleteCustomer(Spot *store, Spot *customer) {
    // 删除服务对(store, customer)
    try {
        route.deleteNode(store, customer);
    } catch (exception &e) {
        throw out_of_range(e.what());
        exit(1);
    }
}


//================ part Route操作 =================//
void Car::replaceRoute(Car *newCar, float currentTime){      
    // 将newCar的路径插入到当前货车路径的current节点之后
    // 此函数与capturePartRoute配套使用，默认时间节点不变
    // 由于在capture中已经更新了stand，在这里无需更新
    Spot* originNextNode = route.nextPos();  // 原目的地
    Spot* changedNextNode = newCar->getRoute()->nextPos(); // 更改后的目的地
    if(state == departure && originNextNode->id != changedNextNode->id) {
        // 如果车子在路途中，而且更换路径后目的地改变，则应该修改nextArriveTime属性
        Spot *standPos = route.getStand();
        nextArriveTime = currentTime + dist(standPos, changedNextNode);
    }
    try{
        route.replaceRoute(newCar->getRoute());      // replaceRoute不更改货车的状态
    } catch (exception &e) {
        cout << "Car #" << carIndex << ":" << endl;
        cout << e.what() << endl;
        exit(1);
    }
}

Car* Car::capturePartRoute(float time){   
    // 抓取route的current指针之后的路径，并且返回一辆车
    // time为抓取的时间
    updateState(time);    // 先将状态更新
    Spot* currentNode = route.currentPos();  // 从该点出发
    Spot* nextNode = route.nextPos();        // 下一站目的地
    Spot* startNode = new Spot();         // 车子的出发点
    startNode->id = 0;   
    startNode->type = 'D';
    startNode->priority = 0;
    // 确定货车的位置信息以及时间信息
    switch(state) {
        case departure: {
            // 车子在路途中，构造虚拟的初始点
            // 该点地理位置位于出发点和目的地连线上的某一点
            // 该点的arrivedTime设定为当前时间，而服务时间为0，和仓库一样
            currentNode = route.getStand();   // departure状态下驻点作为当前点
            float dist = nextArriveTime - nearestDepartureTime;
            startNode->x = (time - nearestDepartureTime) / dist * (nextNode->x - 
                    currentNode->x) + currentNode->x;
            startNode->y = (time - nearestDepartureTime) / dist * (nextNode->y - 
                    currentNode->y) + currentNode->y;
            startNode->arrivedTime = time;
            startNode->serviceTime = 0;
            startNode->startTime = time;
            route.setStand(startNode->x, startNode->y, time);
            nearestDepartureTime = time;
            break;
        }
        case wait: {
            // 车子处于等待状态，直接取当前节点作为起始点
            // 车子随时可以出发，所以serviceTime为0
            startNode->x = currentNode->x;
            startNode->y = currentNode->y;
            startNode->arrivedTime = time;
            startNode->serviceTime = 0;
            startNode->startTime = time;
            route.setStand(startNode->x, startNode->y, time);
            break;
        }
        case serving: {
            // 车子当前在服务顾客，起始点为当前服务点
            // 而服务时间设定到服务结束时间减去当前时间
            // 注意当货车在等待着为顾客服务时，我们也将状态设定为serving
            // 注意货车到达顾客点后立即更新了nearestDepartureTime，因此我们可以利用之
            Spot *currentPos = route.currentPos();
            startNode->x = currentNode->x;
            startNode->y = currentNode->y;
            startNode->arrivedTime = time;
            startNode->serviceTime = nearestDepartureTime - time;  
            // 服务时间已经过去了一部分，注意顾客到达后应该确定arrivedTime
            // time - baseTime表示已经服务过的时间
            startNode->startTime = time;
            route.setStand(startNode->x, startNode->y, time, startNode->serviceTime);
            break;
        }
        case offwork: {  
            // 收车了的车子是不可用的
            // 此时返回一辆空车，其中startNode没有任何意义
            break;		
        }
    }
    // 将current指针后的顾客置入newCar中，注意货车剩余容量leftQuanity
    float leftQuantity = route.getLeftQuantity();  // 货车剩余容量
    Spot *depot = route.getRearNode();          // 任何一辆车，终点都是depot
    Car *newCar = new Car(*startNode, *depot, leftQuantity, carIndex, false);
    try {
        route.checkArrivedTime();
    } catch(exception &e) {
        cout << "Problem happens before!!" << endl;
    }
    vector<Spot*> nodes = route.capture();
    vector<Spot*>::iterator custIter;
    for(custIter = nodes.begin(); custIter < nodes.end(); custIter++) {
        try {
            newCar->insertAtRear(*custIter);
        } catch (exception &e) {
            cout << "In car #" << carIndex << ":" << endl;
            cerr << "In capture: " << e.what() << endl;
            exit(1);
        }
    }
    return newCar;
}


//================ state相关 =================//
void Car::updateState(float time){
    // 更新货车状态
    switch(state){
        case departure: {
            // 原状态是出发，则下一状态是货车执行服务
            // 在这里更新货车的nearestDepartureTime
            if(time == nextArriveTime) {
                // 若当前时间正好是状态改变的时间，则状态改变
                travelDistance += nextArriveTime - nearestDepartureTime;   // 更新travelDistance
                route.moveForward();   // 执行服务，更改当前驻点
                Spot *currentPos = route.currentPos();  // 当前驻点
                // *Note*: 在这里正式更新当前顾客的到达时间
                route.currentPos()->arrivedTime = time;
                if(currentPos->id == 0) {
                    state = offwork;   // 到达仓库，收车
                } else {
                    Spot *nextPos = route.nextPos();
                    if(currentPos->type == 'C' && time < currentPos->startTime) {
                        // 只考虑customer类型节点的时间窗问题
                        time = currentPos->startTime;
                    }
                    nearestDepartureTime = time + currentPos->serviceTime;
                    if(nearestDepartureTime == time) {  
                        // 如果货车既不需要等待，也不需要服务，则继续出发
                        route.setStand(currentPos->x, currentPos->y, time);
                        state = departure;
                        nextArriveTime = nearestDepartureTime + dist(currentPos, nextPos);
                    } else {
                        route.setStand(currentPos->x, currentPos->y, time, 
                                currentPos->serviceTime);
                        if(currentPos->type == 'C') {
                            // 货车的剩余载货量减少
                            route.decreaseLeftQuantity(currentPos->quantity);
                        }
                        state = serving;
                    }
                }
            }
            break;
        }
        case serving: {
            // 原状态是执行服务，则下一状态是货车出发
            Spot* currentPos = route.currentPos();
            Spot* nextPos = route.nextPos();
            if(time == nearestDepartureTime) {
                if(nextPos->type == 'D' && time < LATEST_SERVICE_TIME)  {
                    // 如果下一站是回到仓库而且未到下班时间，则等待
                    state = wait;
                    nearestDepartureTime = LATEST_SERVICE_TIME;
                    route.setStand(currentPos->x, currentPos->y, time);
                    cout << "Car #" << carIndex << ": nothing to do, wait" << endl;
                } else {
                    // 继续出发
                    route.setStand(currentPos->x, currentPos->y, time);
                    state = departure;
                    nextArriveTime = nearestDepartureTime + dist(currentPos, nextPos);
                }
            break;
            }
        }
        case wait: {
            // do nothing now
            if(time == nearestDepartureTime) {
                Spot *currentPos = route.currentPos();
                Spot *nextPos = route.nextPos();
                nextArriveTime = time + dist(currentPos, nextPos);
                route.setStand(currentPos->x, currentPos->y, time);
                state = departure;
            }
            break;
        }
        case offwork: {
            // do nothing now
            // 收车之后理应没有后续动作
            break;
        }
    }
}

EventElement Car::getCurrentAction(float time){        
    // 获得货车当前时刻的动作
    EventElement event;
    event.carIndex = carIndex;
    updateState(time);   // 先更新状态
    Spot *currentPos = route.currentPos();
    switch(state){
        case departure: {
            Spot *nextPos = route.nextPos();
            event.time = nextArriveTime;
            event.eventType = carArrived;
            event.customerId = nextPos->id;
            break;
        }
        case wait: {
            // do nothing
            event.time = nearestDepartureTime;
            event.eventType = finishedService;
            event.customerId = 0;
            break;
        }
        case serving: {
            Spot *currentPos = route.currentPos();
            event.time = nearestDepartureTime;
            event.customerId = currentPos->id; 
            event.eventType = finishedService;
            break;
        }
        case offwork: {
            // 收车
            event.time = time;
            event.customerId = 0;
            event.eventType = carOffWork;
            break;
        }
    }
    return event;
}

EventElement Car::launchCar(float currentTime){         
    // 启动货车，当货车处于wait状态时有效
    EventElement event;
    if(state == wait && route.getSize() != 0) {
        // 当货车有顾客点时才会启动
        Spot *currentPos = route.currentPos();  // 当前驻点
        Spot *nextPos = route.nextPos();        // 下一目的地
        if(nextPos->type == 'D' && currentTime < LATEST_SERVICE_TIME) {
            // 停车等待策略：如果下一站是仓库并且还没有到停止服务时间
            // 则继续呆在原地等待，如果往后再没有服务，则在LATEST_SERVICE_TIME
            // 时启程返回仓库，故返回一个有效事件
            state = wait;
            event.time = LATEST_SERVICE_TIME;
            event.eventType = finishedService;
            event.carIndex = carIndex;
            event.customerId = 0;
        } else {
            // 车辆继续出发
            state = departure;
            currentPos->arrivedTime = currentTime;
            nearestDepartureTime = currentTime;
            float time = currentTime + dist(currentPos, nextPos);
            nextArriveTime = time;
            event.time = time;
            event.eventType = carArrived;
            event.carIndex = carIndex;
            event.customerId = nextPos->id;
        }
    }
    return event;
}

//================ assessment相关 =================//
void Car::removeInvalidCustomer(vector<int> validCustomerId, int& retainNum){
    // 移除路径中的无效顾客
    // 记录保留下来的顾客在removeCustomerId中的位置，posVec
    posVec = route.removeInvalidCustomer(validCustomerId, retainNum);
}


void Car::updateTransformMatrix(Matrix<int> &transformMatrix){
    // 对transformMatrix进行更新
    // transformMatrix各个位置对应顾客由validCustomerId来指定
    assert(posVec.size() != 0);  // 为0表示没有进行removeInvalidCustomer的操作
    vector<int>::iterator intIter;
    for(intIter = posVec.begin(); intIter < posVec.end() - 1; intIter++) {
        int frontPos, backPos;
        frontPos = *(intIter);
        backPos = *(intIter+1);
        int temp = transformMatrix.getElement(frontPos, backPos);
        transformMatrix.setValue(frontPos, backPos, temp+1);
    }
}

int Car::computeScore(Matrix<int> transformMatrix){
    // 计算当前货车的路径在transformMatrix指标下的得分
    // transformMatrix各个位置对应顾客由validCustomerId来指定
    assert(posVec.size() != 0);  // 为0表示没有进行removeInvalidCustomer的操作
    vector<int>::iterator intIter;
    int score = 0;
    for(intIter = posVec.begin(); intIter < posVec.end() - 1; intIter++) {
        int frontPos, backPos;
        frontPos = *(intIter);
        backPos = *(intIter+1);
        if(frontPos !=0 && backPos !=0) {
            score += transformMatrix.getElement(frontPos, backPos);
        }
    }
    return score;
}
