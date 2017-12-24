#include "Car.h"
#include "PublicFunction.h"
#include<cmath>

Car::Car(Customer &headNode, Customer &rearNode, float capacity, int index, bool artificial):
    carIndex(index), route(headNode, rearNode, capacity), artificial(artificial)
{
    state = wait;
    nearestDepatureTime = 0;
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
    this->nearestDepatureTime = item.nearestDepatureTime;
    this->nextArriveTime = item.nextArriveTime;
    this->travelDistance = item.travelDistance;
}


Car& Car::operator= (Car &item){ 
    // 重载赋值操作
    this->route = item.route;
    this->carIndex = item.carIndex;
    this->state = item.state;
    this->artificial = item.artificial;
    this->nearestDepatureTime = item.nearestDepatureTime;
    this->nextArriveTime = item.nextArriveTime;
    this->travelDistance = item.travelDistance;
    return *this;
}


//================ 得到货车属性 =================//
Car Car::getNullCar(){
    // 复制货车的首节点和尾节点以及剩余容量
    float leftCapacity = route.getLeftQuantity();
    Car newCar(getHeadNode(), getRearNode(), leftCapacity, carIndex);
    return newCar;
}


//================ insert cost和remove cost =================//
void Car::computeInsertCost(Customer item, float &minValue, Customer &customer1, 
        float &secondValue, Customer &customer2, float randomNoise, bool allowNegativeCost){
    // 计算item节点到本车路径的最小插入代价和次小插入代价
    // randomNoise: 随机化的噪声量
    // allowNegativeCost: 是否允许出现负的insertion cost
    route.computeInsertCost(item, minValue, customer1, secondValue, customer2, 
            randomNoise, allowNegativeCost);
}

vector<float> Car::computeReducedCost(float DTpara[]){  
    // 计算车辆要服务的所有节点的移除代价
    // DTpara: 不同优先级顾客计算reduce cost时的bonus
    return route.computeReducedCost(DTpara, artificial);
}


//================ insert 和 delete Customer方法 =================//
void Car::insertAtRear(Customer &item){
    route.insertAtRear(item);
} 

bool Car::insertAtHead(Customer &item){
    route.insertAtHead(item);
}

void Car::insertAfter(const Customer &item1, const Customer &item2) {
    // 简单地在item1后面插入item2
    Customer itema, itemb;
    itema = item1;
    itemb = item2;
    if(item1.id == 0) {
        // 说明插入到虚拟始发点后方
        // 虚拟始发点的定义见capturePartRoute()
        // 这里需要更新一下nextArriveTime
        // 如果车子在路途中，而且更换路径后目的地改变，则应该修改nextArriveTime属性
        Customer itemx = route.currentPos();  // 货车最近驻点（其实在路途中，驻点指的是顾客点）
        float currentTime = nearestDepatureTime + sqrt(pow(item1.x - itemx.x, 2) 
                + pow(item1.y - itemx.y, 2));
        nextArriveTime = currentTime + sqrt(pow(item1.x - item2.x, 2) + pow(item1.y - item2.y, 2));
        itema = itemx;
	}

    route.insertAfter(itema, itemb);
}

void Car::deleteCustomer(Customer &item) {
    route.deleteNode(item);
}


//================ part Route操作 =================//
void Car::replaceRoute(Car &newCar, float currentTime){      
    // 将newCar的路径插入到当前货车路径的current节点之后
    updateState(currentTime);  // 先更新状态
    Customer currentNode = route.currentPos();  // 最近出发点
    Customer originNextNode = route.nextPos();  // 原目的地
    route.replaceRoute(newCar.getRoute());      // replaceRoute不更改货车的状态
    Customer changedNextNode = route.nextPos(); // 更改后的目的地
    if(state == depature && originNextNode.id != changedNextNode.id) {
        // 如果车子在路途中，而且更换路径后目的地改变，则应该修改nextArriveTime属性
        float dist = nextArriveTime - nearestDepatureTime;
        // x,y是货车当前位置
        float x = (currentTime - nearestDepatureTime) / dist * (originNextNode.x - 
                currentNode.x) + currentNode.x;
        float y = (currentTime - nearestDepatureTime) / dist * (originNextNode.y - 
                currentNode.y) + currentNode.y;
        nextArriveTime = currentTime + sqrt(pow(x - changedNextNode.x, 2) + 
                pow(y - changedNextNode.y, 2));
    }
}

Car Car::capturePartRoute(float time){   
    // 抓取route的current指针之后的路径，并且返回一辆车
    // time为抓取的时间
    updateState(time);    // 先将状态更新
    Customer currentNode = route.currentPos();  // 从该点出发
    Customer nextNode = route.nextPos();        // 下一站目的地
    Customer *startNode = new Customer;         // 车子的出发点
    startNode->id = 0;   
    startNode->type = 'D';
    startNode->priority = 0;
    // 确定货车的位置信息以及时间信息
    switch(state) {
        case depature: {
            // 车子在路途中，构造虚拟的初始点
            // 该点地理位置位于出发点和目的地连线上的某一点
            // 该点的arrivedTime设定为当前时间，而服务时间为0，和仓库一样
            float dist = nextArriveTime - nearestDepatureTime;
            startNode->x = (time - nearestDepatureTime) / dist * (nextNode.x - 
                    currentNode.x) + currentNode.x;
            startNode->y = (time - nearestDepatureTime) / dist * (nextNode.y - 
                    currentNode.y) + currentNode.y;
            startNode->arrivedTime = time;
            startNode->serviceTime = 0;
            startNode->startTime = time;
            break;
        }
        case wait: {
            // 车子处于等待状态，直接取当前节点作为起始点
            // 车子随时可以出发，所以serviceTime为0
            startNode->x = currentNode.x;
            startNode->y = currentNode.y;
            startNode->arrivedTime = time;
            startNode->serviceTime = 0;
            startNode->startTime = time;
            break;
        }
        case serving: {
            // 车子当前在服务顾客，起始点为当前服务点
            // 而服务时间设定到服务结束时间减去当前时间
            // 注意当货车在等待着为顾客服务时，我们也将状态设定为serving
            // 货车到达顾客点后立即更新了nearestDepatureTime，因此我们可以利用之
            Customer currentPos = route.currentPos();
            startNode->x = currentNode.x;
            startNode->y = currentNode.y;
            startNode->arrivedTime = time;
            startNode->serviceTime = nearestDepatureTime - time;  
            // 服务时间已经过去了一部分，注意顾客到达后应该确定arrivedTime
            // time - baseTime表示已经服务过的时间
            startNode->startTime = time;
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
    Customer depot = route.getRearNode();          // 任何一辆车，终点都是depot
    Car newCar(*startNode, depot, leftQuantity, carIndex, false);
    Route tempRoute = route.capture();                         // 抓取current指针后的路径
    vector<Customer*> tempCust = tempRoute.getAllCustomer();   // 获得current指针后的所有顾客
    vector<Customer*>::iterator custIter;
    for(custIter = tempCust.begin(); custIter < tempCust.end(); custIter++) {
        try {
            newCar.insertAtRear(**custIter);
        } catch (exception &e) {
            cout << e.what() << endl;
        }
    }
    deleteCustomerSet(tempCust);
    return newCar;
}


//================ state相关 =================//
void Car::updateState(float time){
    // 更新货车状态
    switch(state){
        case depature: {
            // 原状态是出发，则下一状态是货车执行服务
            // 在这里更新货车的nearestDepartureTime
            if(time == nextArriveTime) {
                // 若当前时间正好是状态改变的时间，则状态改变
                travelDistance += nextArriveTime - nearestDepatureTime;   // 更新travelDistance
                route.moveForward();   // 执行服务，更改当前驻点
                Customer currentPos = route.currentPos();  // 当前驻点
                route.currentPos().arrivedTime = time;     // 更新当前顾客的到达时间
                if(currentPos.id == 0) {
                    state = offwork;   // 到达仓库，收车
                } else {
                    Customer nextPos = route.nextPos();
                    if(time < currentPos.startTime) {
                        time = currentPos.startTime;
                    }
                    nearestDepatureTime = time + currentPos.serviceTime;
                    if(nearestDepatureTime == time) {  
                        // 如果货车既不需要等待，也不需要服务，则继续出发
                        state = depature;
                        nextArriveTime = nearestDepatureTime + sqrt(pow(currentPos.x - nextPos.x, 2) 
                                + pow(currentPos.y - nextPos.y, 2));
                    } else {
                        route.decreaseLeftQuantity(currentPos.quantity);
                        state = serving;
                    }
                }
            }
            break;
        }
        case serving: {
            // 原状态是执行服务，则下一状态是货车出发
            Customer currentPos = route.currentPos();
            Customer nextPos = route.nextPos();
            if(time == nearestDepatureTime) {
                // 可以进行状态转换
                state = depature;
                nextArriveTime = nearestDepatureTime + sqrt(pow(currentPos.x - nextPos.x, 2) 
                        + pow(currentPos.y - nextPos.y, 2));
            }
            break;
        }
        case wait: {
            // do nothing now
            // 必须是启动了货车之后才能进行状态转换
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
    Customer currentPos = route.currentPos();
    switch(state){
        case depature: {
            Customer nextPos = route.nextPos();
            event.time = nextArriveTime;
            event.eventType = carArrived;
            event.customerId = nextPos.id;
            break;
        }
        case wait: {
            // do nothing	
            // 返回无效事件，说明货车还没有启动
            break;
        }
        case serving: {
            Customer currentPos = route.currentPos();
            event.time = nearestDepatureTime;
            event.customerId = currentPos.id; 
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
        state = depature;
        Customer currentPos = route.currentPos();  // 当前驻点
        Customer nextPos = route.nextPos();        // 下一目的地
        nearestDepatureTime = currentTime;
        float time = currentTime + sqrt(pow(currentPos.x - nextPos.x, 2) 
                + pow(currentPos.y - nextPos.y, 2));
        nextArriveTime = time;
        event.time = time;
        event.eventType = carArrived;
        event.carIndex = carIndex;
        event.customerId = nextPos.id;
    }
    return event;
}


//================ assessment相关 =================//
void Car::removeInvalidCustomer(vector<int> validCustomerId){
    // 移除路径中的无效顾客
    // 记录保留下来的顾客在removeCustomerId中的位置，posVec
    posVec = route.removeInvalidCustomer(validCustomerId);
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
