#include "Route.h"
#include<iostream>
#include<cassert>
#include<vector>
#include<cmath>
#include<limits>
#include<cstdlib>
#include<algorithm>
#include<stdexcept>

const float MAX_FLOAT = numeric_limits<float>::max();

using namespace std;
Route::Route(Spot &headNode, Spot &rearNode, float capacity):capacity(capacity)
{ 
    // 构造函数
    head = new Spot(headNode);
    rear = new Spot(rearNode);
    head->front = NULL;
    head->next = rear;
    rear->front = head;
    rear->next = NULL;
    current = head;  // 初始化current指针指向head节点
    size = 0;
    arrivedTime.push_back(head->arrivedTime);
    quantity = 0;
    leftQuantity = capacity;
}

Route::~Route(){ // 析构函数
    this->clear();
}

//=============== 链表基本操作 ================//
void Route::copy(const Route &L){
    // 应当对L的head节点和rear节点都复制
    // 除此之外，注意private中所有的数据成员都要复制过来
    this->size = L.size;
    this->capacity = L.capacity;
    this->quantity = L.quantity;
    this->leftQuantity = L.leftQuantity;
    this->arrivedTime = L.arrivedTime;
    Spot* originPtr = L.head;
    Spot *copyPtr, *temp;
    while(originPtr!=NULL){
        // 从头节点一直复制到尾节点
        if(originPtr == L.head){  
            // 正在复制head节点
            copyPtr = new Spot(L.head);
            copyPtr->front = NULL;
            head = copyPtr;
        } else{
            temp = new Spot(*originPtr);
            temp->front = copyPtr;
            copyPtr->next = temp;
            copyPtr = temp;
            if(originPtr->type == 'C') {
                // 追溯其指向的商户(P)
                temp = L.head->next;
                int i = 0;
                while(temp != L.rear) {
                    if(temp == originPtr->choice) break;
                    temp = temp->next;
                    i++;
                }
                temp = head->next;
                for(int j=0; j<i; j++) {
                    temp = temp->next;
                }
                // 双向索引
                copyPtr->choice = temp;
                temp->choice = copyPtr;
            }
		}
        if(L.current == originPtr){
            // current指针的复制
            current = copyPtr;
        }
		originPtr = originPtr->next;
    }
    copyPtr->next = NULL;
    rear = copyPtr;
}

Spot& Route::operator[] (int k){
    assert(k>=0 && k<size);
    Spot* temp = head->next;
    for(int i=0; i<k; i++){
        temp = temp->next;
    }
    return *temp;
}

const Spot& Route::operator[] (int k) const{
    assert(k>=0 && k<size);
    Spot* temp = head->next;
    for(int i=0; i<k; i++){
        temp = temp->next;
    }
    return *temp;
}

Route::Route(const Route &L){ // 复制构造函数	
    this->copy(L);
}


Route& Route::operator= (const Route &L){ 
    // 重载"="运算符，用以配合深复制
    this->clear();  // 清空当前链表	
    this->copy(L);
    return *this;
}

bool Route::isEmpty(){ //判断链表是否为空
    return (size==0);
}

void Route::clear(){  
    // 清空链表，包括head节点和rear节点
    Spot* ptr1 = head;
    Spot* ptr2;
    while(ptr1!=NULL){
        ptr2 = ptr1->next;
        delete ptr1;
        ptr1 = ptr2;
    }
    head = NULL;
    rear = NULL;
    current = NULL;
    size = 0;
}

void Route::printRoute(){ // 打印链表
    Spot* ptr = head;
    for(; ptr!=NULL; ptr=ptr->next) {
        cout << "id:" << ptr->id << ' ' << "type:" << ' ' << ptr->type << endl;
    }
}


//=============== 插入以及删除节点操作 ================//
void Route::insertAfter(Spot *ref, Spot *current) {
    // 在ref节点后面插入current节点
    Spot *ptr = head;
    bool mark = false;
    while(ptr!=rear) {
        if(ptr == ref) {
            mark = true;
            break;
        }
    }
    if(mark == false) {
        throw out_of_range("Cannot find the position to insert!");
        return;
    }
    ref->next->front = current;
    current->next = ref->next;
    current->front = ref;
    ref->next = current;
    if(current->type == 'C') {
        quantity += current->quantity;
    }
    size++;
    refreshArrivedTime()
}

void Route::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer){
    // 在链表中refStore指针指向的节点后面插入store指针指向节点
    // 在链表中refCustomer指针指向的节点后面插入customer指向节点
    assert(store->type == 'S' && customer->type == 'C');
    Spot *ptr = head;
    int count = 2;   // 必须两个ref节点都找到
    while(ptr != rear){
        if (ptr == refStore){  
            count--;
        }
        if (ptr == refCustomer) {
            count--;
        }
        if(count == 0) {
            break;
        }
        ptr = ptr->next;
    }
    if(count > 0) {
        // 没有完全找到，返回false
        throw out_of_range("Cannot find the position to insert!");
    } else{
        // 更新quantity的值，并且插入store以及customer
        quantity += customer->quantity;
        refStore->next->front = store;
        store->next = refStore->next;
        refStore->next = store;
        store->front = refStore;
        // 这里需要考虑如果refStore与refCustomer是同一个节点的问题
        if(refStore == refCustomer) {
            store->next->front = customer;
            customer->next = store->next;
            customer->front = store;
            store->next = customer;
        } else {
            refCustomer->next->front = customer;
            customer->next = refCustomer->next;
            refCustomer->next = customer;
            customer->front = refCustomer;
        }
        size++;
        refreshArrivedTime();  // 插入节点后，更新arrivedTime
    }
}

void Route::insertAtHead(Spot *store, Spot *customer){ 
    // 在表头插入store和customer
    // 注意store须在customer前面（对于pickup-delivery问题，本函数慎用）
    // 只有当current指针为head时返回true
    assert(store->type == 'S' && customer->type == 'C');
    if(current == head && size == 0) {
        // 要求路径必须为空才可以这种方式插入
        head->next = store;
        store->next = customer;
        store->front = head;
        rear->front = customer;
        customer->next = rear;
        quantity = quantity + customer->quantity;
        size++;
        refreshArrivedTime();  // 插入节点后，更新arrivedTime
    }
    else{
        throw out_of_range("Cannot insert node after head!");
    }
}

void Route::insertAtRear(Spot *node) {
    // 在表尾插入node，注意这里不检查插入合法性
    // 需要由用户自己保证节点是可以凑成(P-D)对
    rear->front->next = node;
    node->next = rear;
    if(node->type == 'C') {
        quantity = quantity + node->quantity;
        size++;
        refreshArrivedTime();  // 插入节点后，更新arrivedTime
    }
}


void Route::deleteNode(Spot *node) {
    // 删除node节点
    bool mark = false;
    for(Spot* ptr = current; ptr != rear; ptr = ptr->next) {
        if(ptr == node) {
            mark = true;
            break;
        }
    }
    if(mark == false) {
        throw out_of_range("Cannot find the node to delete!");
    }
    node->front->next = node->next;
    node->next->front = node->front;
    if(node->type == 'C') {
        quantity -= node->quantity;
        size--;
    }
    refreshArrivedTime();
    delete node;
}

void Route::deleteNode(Spot *store, Spot *customer){
    // 删除链表中指针值与store和customer相同的节点
    // 只能删除current指针后面的节点
    assert(store->type == 'S' && customer->type == 'C');
    if(current == rear) {
        // 已经走完了路径中的所有节点，禁止删除
        throw out_of_range("Forbid deleting for we have finished the route!");
    }

    if (current == NULL) {
        throw out_of_range("The current node is NULL!");
    }

    Spot* temp1 = current->next;
    int count = 2;  // 需要同时找到store和customer才可删除
    while(temp1!=rear) {
        if(temp1 == store) {
            count--;
        }
        if(temp1 == customer) {
            count--;
        }
        if(count == 0) break;
        temp1 = temp1->next;
    }
    if(count > 0) {  
        // 没有完全找到
        throw out_of_range("We want to delete inexistent customer!");
    } else {
        store->front->next = store->next;
        store->next->front = store->front;
        customer->front->next = customer->next;
        customer->next->front = customer->front;
        delete store;
        delete customer;
        size--;
        quantity = quantity - customer.quantity;
        refreshArrivedTime();  // 删除节点后，更新arrivedTime
    }
}


//=============== 获得单节点操作 ================//
Spot* Route::currentPos(){ 
    // 返回当前位置
    return current;
}

Spot* Route::getHeadNode() {
    // 返回头结点
    return head; 
}

Spot* Route::getRearNode() {
    // 返回尾节点
    return rear; 
}


//=============== 获取链表属性 ================//
int Route::getSize() {
    return this->size;
}

vector<Spot*> Route::getAllCustomer(){  
    // 得到路径中所有的顾客节点(D)
    // 返回的customer是用路径的节点，在外部不能随便操作
    // Returns:
    //   * customerSet: 路径中所有的顾客节点（按顺序）
    vector<Spot*> customerSet(size);
    Spot* ptr = head->next;
    Spot* ptr2;
    for(Spot *ptr=head; ptr!=rear; ptr=ptr->next){
        if(ptr->type == 'C') {
            customerSet.push_back(ptr);
        }
    }
    return customerSet;
}

float Route::getLen(float DTpara[], bool artificial){   
    // 得到路径长度（根据需要添加惩罚）
    // Args:
    //   * DTpara: 对不同类型的车/顾客组合的惩罚因子
    //   * artificial: 车辆属性，为true表示virtual car
    // Returns:
    //   * len: 路径长度
    float DTH1, DTH2, DTL1, DTL2;
    float *DTIter = DTpara;
    DTH1 = *(DTIter++);
    DTH2 = *(DTIter++);
    DTL1 = *(DTIter++);
    DTL2 = *(DTIter++);

    Spot *ptr1 = head;
    Spot *ptr2 = head->next;
    float len = 0;
    if(artificial == false) { // real vehicle routing scheme
        for(Spot *ptr = head->next; ptr != rear; ptr = ptr->next) {
            Spot *pre = ptr->front;
            Spot *next = ptr->next;
            float cost = dist(pre, ptr) + dist(ptr, next);
            if(ptr->type == 'C') {
                switch(ptr->priority){
                    case 0: {
                        cost += 0.0f;
                        break;
                    }
                    case 1: {
                        cost -= DTH1;
                        break;
                    }
                    case 2: {
                        cost -= DTL1;
                        break;
                    }
                }
            }
            len += cost;
        }
    } else {
        for(Spot *ptr = head->next; ptr != rear; ptr = ptr->next) {
            Spot *pre = ptr->front;
            Spot *next = ptr->next;
            float cost = dist(pre, ptr) + dist(ptr, next);
            if(ptr->type == 'C') {
                switch(ptr->priority){
                    case 0: {
                        cost += 0.0f;
                        break;
                    }
                    case 1: {
                        cost += DTH2;
                        break;
                    }
                    case 2: {
                        cost += DTL2;
                        break;
                    }
                }
            }
            len += cost;
        }
    }
    return len;
}

float Route::getOriginLen() {  
    // 得到服务静态节点的路径代价
    // 以property标识顾客属性，当property为0时表示静态，为1表示动态
    Spot* front = head;         // 搜索的起始节点
    Spot* back = front->next;   // 下一个节点
    float originLen = 0;
    while(back != NULL) {
        // 只计算静态节点的运输代价
        if(back->prop != 0) {
            back = back->next;
        }
        else {
            origenLen = originLen + dist(front, back);
            front = back;
            back = back -> next;
        }
    }
    return originLen;
}


vector<float> Route::getArrivedTime(){     // 得到本车所有节点的arrivedTime
    return arrivedTime;
}

vector<int> Route::getAllID() {
    // 获取路径中所有的ID，包括P和D
    vector<int> IDs;
    for(Spot *temp = head->next; temp != rear; temp = temp->next) {
        IDs.push_back(temp->id);
    }
    return IDs;
}

//=============== 修改链表属性 ================//
bool Route::moveForward(){
    current->visit = true;
    current = current->next;
    if(current == NULL) {  // 已经完成任务
        return false;
    } else {
        return true;
    }
}


//=============== 计算插入/删除节点代价 ================//
vector<float> Route::computeReducedCost(float DTpara[], bool artificial){ 
    // 得到所有服务对(P-D)的移除代价，值越小表示移除它可以节省更多的代价
    // Args:
    //   * artificial: 为true表示是一辆虚假的车
    //   * DTpara[]: 对不同种类的顾客/车辆的惩罚系数。如果需要得到
    //               真正的reduce cost，则全部设置为0即可
    // Returns:
    //   * costArr: 所有服务对的移除代价，按照路径中customer的顺序
    float DTH1, DTH2, DTL1, DTL2;
    float *DTIter = DTpara;
    DTH1 = *(DTIter++);
    DTH2 = *(DTIter++);
    DTL1 = *(DTIter++);
    DTL2 = *(DTIter++);
    vector<float> costArr(0);
    for(Spot* ptr = head; ptr != rear; ptr = ptr->next) {
        if(ptr->type == 'C') {
            // 从customer寻迹找到store
            Spot *customer = ptr;
            Spot *customerPre = customer->front;
            Spot *customerNext = customer->next;
            Spot *store = ptr->choice;
            Spot *storePre = store->front;
            Spot *storeNext = store->next;
            float diff1 = -dist(customerPre, customer) - dist(customer, 
                    customerNext) + dist(customerPre, customerNext);
            float diff2 = -dist(storePre, store) - dist(store, storeNext) + 
                    dist(storePre, storeNext);
            float cost = diff1 + diff2;
            if(artificial == true) {
                switch(ptr->priority){
                    case 0: {
                        cost += 0;
                        break;
                    }
                    case 1: {
                        cost -= DTH2;
                        break;
                    }
                    case 2: {
                        cost -= DTL2;
                        break;
                    }
                }
            } else {
                switch(ptr1->priority){
                    case 0: {
                        cost += 0;
                        break;
                    }
                    case 1: {
                        cost += DTH1;
                        break;
                    }
                    case 2: {
                        cost += DTL1;
                        break;
                    }
                }		
            }
            costArr.push_back(cost);
        }
    }
    return costArr;
} 

bool Route::timeWindowJudge(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer){
    // 判断将store插入到refStore后面并且将customer插入到refCustomer后面是否会违反时间窗约束
    // 注意refStore和refCustomer都可能是"store"或者"customer"
    // 但是refStore必定在refCustomer前面
    assert(store->type == 'S' && customer->type == 'C');
    int pos = 0;
    for(Spot *temp=head; temp!=refStore; temp=temp->next) {
        // 找到refStore在路径中的位置，以提取arrivedTime。
        pos++;
    }
    float time = arrivedTime[pos];
    Spot *ptr1, *ptr2;

    // 接下来是判断插入store以及customer会否违反时间窗约束
    if(refStore->type == 'C' && time < refStore->startTime){   
        // arrivedTime[pos]只加到了refStore的arrived time，没有判断是否提前到达
        // 只考虑customer节点的时间窗
        time = refStore->startTime;
    }
    // 判断是否违反store后面的时间窗约束
    // 注意store本身没有时间窗约束
    time += store->serviceTime;
    Spot *pre, *current;
    // 有可能refStore和refCustomer是同一个节点，对此作特殊处理
    if(refStore == refCustomer) {
        // store -> customer
        float travelLen = dist(store, customer);
        time += travelLen;
        if(time < customer->startTime) {
            time = customer->startTime;
        }
        if(time > customer->endTime) {
            return false;
        }
        time += customer->serviceTime;
        // customer -> store->next
        if(refStore->next == rear) {
            // 已经将store, customer放置于路径尾端
            return true;
        } else {
            float travelLen = dist(customer, refStore->next);
            time += travelLen;
            if(refStore->next->type == 'C') {
                if(time < refStore->next->startTime) {
                    time = refStore->next->startTime;
                }
                if(time > refStore->next->endTime) {
                    return false;
                }
            }
            time += refStore->next->serviceTime;
        }
        pre = refStore->next;
        current = pre->next;
    } else {
        pre = store;
        current = refStore->next;
    }
    bool mark = true; 
    while(true) {
        if(current == rear) break;
        if(pre == refCustomer) {
            // 前一节点是refCustomer，那么下一节点应该是customer
            current = customer;
        }
        float travelLen = dist(pre, current);
        time += travelLen;
        if(current->type == 'C') {
            if(time > current->endTime) {
                mark = false;
                break;
            }
            if(time < current->startTime) {
                time = current->startTime;
            }
        }
        time += current->serviceTime;
        if(pre == refCustomer) {
            // pre由customer暂时代替，但是不真正地将customer插入
            // 此时current指针指向的是路径中实际存在的节点（customer之后）
            current = pre->next;
            pre = customer;
        } else {
            pre = current;
            current = current->next;
        }
    }
    return mark;
}

void Route::computeInsertCost(Spot *store, Spot* customer, float &minValue, 
        Spot *refStore1, Spot refCustomer1, float &secondValue, Spot *refStore2,
        Spot *refCustomer2, float randomNoise, bool allowNegativeCost){
    // 计算服务对(store, customer)在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点(refStore, refCustomer)
    // 如果store == head，则refStore没有意义（这种情况出现在当货车已经经过顾客
    // 点C指定的store后，路径重新规划，则此时货车所在地（head节点）将充当store的角色）
    // Args:
    //   * pertubation: 扰动的噪声量
    //   * allowNegativeCost: 为true表示插入代价取非负数，为false表示可取负数
    //   * randomNoise: 随机噪声量
    // Returns:
    //   * refStore1, refCustomer1: 若minValue=MAX_FLOAT，则二者均为NULL
    //   * refStore2, refCustomer2: 若secondVlue=MAX_FLOAT，则二者均为NULL
    assert(store->type == 'S' && customer->type == 'C');
    refStore1 = NULL;
    refCustomer1 = NULL;
    refStore2 = NULL;
    refCustomer2 = NULL;
    minValue = MAX_FLOAT;
    secondValue = MAX_FLOAT;
    if(quantity + refCustomer->quantity > capacity) {
        // 超出车容量约束，则无需计算
        return;
    }
    if(store == head) {
        for(Spot* refCustomer=current; refCustomer != rear; refCustomer = 
                refCustomer->next) {
            if(timeWindowJudge(head, refCustomer, store, customer) == true) {
                float diff = dist(refCustomer, customer) + dist(customer, 
                        refCustomer->next) - dist(refCustomer, refCustomer->next);
                float cost = diff + randomNoise;
                if(allowNegativeCost == false) {
                    cost = max(0.0f, cost);
                }
                if(cost <= minValue) {
                    // 找到了新的最小者，更新返回值
                    secondValue = minValue;
                    refCustomer2 = refCustomer1;
                    refStore2 = NULL;
                    refCustomer1 = refCustomer;
                    refStore1 = NULL;
                    minValue = cost;
                }
            }
        }
    } else {
        for(Spot* refStore=current; refStore != rear; refStore=refStore->next) {
            for(Spot* refCustomer=refStore; refCustomer != rear; 
                    refCustomer = refCustomer->next) {
                if(timeWindowJudge(refStore, refCustomer, store, customer) 
                        == true) {
                    // 满足时间窗约束
                    float diff1 = dist(refStore, store) + dist(store, 
                            refStore->next) - dist(refStore, refStore->next);
                    float diff2 = dist(refCustomer, customer) + dist(customer, 
                        refCustomer->next) - dist(refCustomer, refCustomer->next);
                    float cost = diff1 + diff2 + randomNoise;
                    if(allowNegativeCost == false) {
                        cost = max(0.0f, cost);
                    }
                    if(cost <= minValue) {
                        // 找到了新的最小者，更新返回值
                        secondValue = minValue;
                        refCustomer2 = refCustomer1;
                        refStore2 = refStore1;
                        refCustomer1 = refCustomer;
                        refStore1 = refStore;
                        minValue = cost;
                    }
                }
            }
        }
    }
}

void Route::refreshArrivedTime(){   
    // 更新一下各个节点的到达时刻
    // 头结点的arrivedTime + serviceTime将作为基准时间
    arrivedTime.clear();
    Spot* tfront = head;
    while(tfront != current->next){
        // 从头结点到current节点之前的arrivedTime都不需要重新计算
        arrivedTime.push_back(tfront->arrivedTime);
        tfront = tfront->next;
    }
    tfront = current;
    Spot* tcurrent = current->next;
    float time = current->arrivedTime + current->serviceTime;
    while(tcurrent != rear){
        // current节点后面的arrivedTime需要重新计算
        time = time + dist(tfront, tcurrent);
        arrivedTime.push_back(time);
        tcurrent->arrivedTime = time;
        // tcurrent->arrivedTime = time;
        if(tcurrent->type == 'C' && time < tcurrent->startTime){
            // 只有顾客节点有“时间窗”
            time = tcurrent->startTime;
        }
        time = time + tcurrent->serviceTime;
        tfront = tfront->next;
        tcurrent = tcurrent->next;
    }
}


//=============== 路径的替换和提取 ================//
Route* Route::getEmptyRoute(vector<Spot*> &removedCustomer) {
    // 获取空的路径，但是需要保留路径中choice为depot的customer
    // removedCustomer: current指针之后的顾客节点 - 保留的顾客节点
    Route *newRoute = new Route(head, rear, leftQuantity);
    Spot *temp = current->next;
    while(temp != NULL) {
        if(temp->type == 'C') {
            if(temp->choice->type != 'D') {
                Spot *customer = new Spot(*temp);
                Spot *store = new Spot(*(customer->choice));
                customer->choice = store;
                store->choice = customer;
                removedCustomer.push_back(customer);
            }
            else {
                Spot *customer = newSpot(*temp);
                customer->choice = newRoute->head;
                newRoute->insertAtRear(customer);
            }
        }
        temp = temp->next;
    }
    return newRoute;
}

Route* Route::capture(){ 
    // 抓取current指针后的路径
    // current指针当前节点将作为head节点
    // 将当前路径的capacity和leftQuantity原样复制
    // 对于已经访问过对应pickup节点的delivery节点，其选择的store暂时为
    // 抓取路径的head节点
    Route* newRoute = new Route(*current, *rear, capacity);
    if(current->next == rear) { // current指针后已经没有路径
        return *newRoute;
    }
    for(Spot* ptr=current->next; ptr != rear; ptr = ptr->next) {
        Spot *temp = new Spot(*ptr);
        if(temp->type == 'C' && temp->choice->visit == true) {
            // customer对应的store已经被访问过
            temp->choice = newRoute->getHeadNode();
            newRoute->insertAtRear(temp);
        }
    }
    return newRoute;
}

void Route::replaceRoute(Route &route) {  
    // 以route替换掉current指针后的路径
    // 对于route中choice为depot的customer，需要找回其原本指向的商店
    vector<Spot*> customerPool(NUM_OF_CUSTOMER);
    Spot *ptr1, *ptr2, *ptr3;
    // 清空本路径中current指针后面的节点 
    if(current->next != rear) { // current后面还有节点
        // 清除原路径中current指针后面的元素
        // 不包括对rear节点的清除
        ptr1 = current->next;
        while(ptr1 != rear) {
            if(ptr1->type == 'C') {
                customerPool[ptr1->id] = ptr1->choice;
                ptr2 = ptr1->next;
                deleteNode(ptr1);
                ptr1 = ptr2;
            }
        }
    }
    // 修改route中choice为depot的customer其选择的商店
    ptr1 = route.current->next;
    while(ptr1 != NULL) {
        if(ptr1->type == 'C' && ptr1->choice->type == 'D') {
            Spot *store = customerPool[ptr1->id];
            store->choice = ptr1;
            ptr1->choice = store;
        }
    }

    // 将route中除head和rear外的节点都复制到current指针后
    ptr1 = route.head->next;
    while(ptr1 != route.rear) {
        try {
            insertAtRear(ptr1);
        } catch (exception &e) {
            cout << "While replace route: " << e.what() << endl;
        }
        ptr1 = ptr1->next; 
    }
    // 清空变量
    customerPool.clear();
    route.clear();
    return;
}

//=============== 其余辅助性质的函数 ================//
bool Route::checkPassRoute(){
    // 检查已走过的路径是否违反时间窗约束
    // 这里只检查到达下一个节点的时刻是否小于前一个节点的时间窗起始时刻
    Spot* ptr1 = head;
    Spot* ptr2 = head->next;
    bool mark = true;
    if(current == head) {  // 车子还没从仓库出发，无需检查
        return true;
    } else {
        while(ptr2 != current->next) {
            // 一直检查到current节点
            float leastArriveTime = ptr1->startTime;  // 到达下一个节点的最快时间
            if(ptr2->arrivedTime < leastArriveTime) {
                mark = false;
            }
            ptr1 = ptr1 -> next;
            ptr2 = ptr2 -> next;
        }
        return mark;
    }
}

vector<int> Route::removeInvalidCustomer(vector<int> validCustomerId, int &retainNum){
    // 仅保留id在validCustomerId中的customer节点对应的服务对
    // 注意通过customer->choice可以得到顾客选取的商店
    // Returns: 
    //   * retainNum: route所拥有的valid customer数量
    //   * posVec: 保留下来的顾客节点在validCustomerId中的位置组成的向量
    vector<int> posVec;
    posVec.push_back(0);   // 仓库节点位置
    Spot* ptr1 = head->next;
    while(ptr1 != rear) {
        if(ptr1->type == 'C') {
            int currentId = ptr1->id;
            vector<int>::iterator intIter = find(validCustomerId.begin(), 
                    validCustomerId.end(), currentId);
            if(intIter == validCustomerId.end()) {
                // 如果找不到，说明该节点是invalid，删除之
                quantity -= ptr1->quantity;
                size--;
                ptr1->front->next = ptr1->next;
                ptr1->next->front = ptr1->front;
                Spot *ptr2 = ptr1->choice;  // 该顾客选择的商店，也要一并删除
                ptr2->front->next = ptr2->next;
                ptr2->next->front = ptr2->front;
            } else {
                retainNum++;
                int pos = intIter - validCustomerId.begin();
                posVec.push_back(pos);
            } 
            ptr1 = ptr1->next;
        }
    }
    posVec.push_back(0);  // 仓库节点位置
    return posVec;
}
