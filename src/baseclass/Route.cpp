#include "Route.h"
#include<iostream>
#include<cassert>
#include<vector>
#include<cmath>
#include<limits>
#include<cstdlib>
#include<algorithm>
#include<stdexcept>
#include "../public/PublicFunction.h"
#include "../run/Config.h"

using namespace std;
Route::Route(Spot &headNode, Spot &rearNode, float capacity):capacity(capacity)
{ 
    // 构造函数
    head = new Spot(headNode);
    rear = new Spot(rearNode);
    stand = new Spot(headNode);
    stand->front = head;
    stand->next = rear;
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
            copyPtr = new Spot(*L.head);
            copyPtr->front = NULL;
            head = copyPtr;
        } else{
            temp = new Spot(*originPtr);
            temp->front = copyPtr;
            copyPtr->next = temp;
            copyPtr = temp;
            if(originPtr->type == 'C') {
                // 追溯其指向的商户(D)
                temp = L.head;
                int i = 0;
                while(temp != L.rear) {
                    if(temp == originPtr->choice) break;
                    temp = temp->next;
                    i++;
                }
                temp = head;
                for(int j=0; j<i; j++) {
                    temp = temp->next;
                }
                if(temp->type == 'D') {
                    // 商店是depot，则只需要单向索引
                    copyPtr->choice = temp;
                } else {
                    // 双向索引
                    copyPtr->choice = temp;
                    temp->choice = copyPtr;
                }
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
    // stand指针的设定
    stand = new Spot(*L.stand);
    stand->front = current;
    stand->next = current->next;
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
    delete stand;
    stand = NULL;
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
    // 只能插入到stand节点后面
    Spot *ptr = current;
    if(DEBUG) {
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
    }
    if(ref == stand->front) {
        // 说明current节点插入到stand节点后面
        stand->next = current;
    }
    ref->next->front = current;
    current->next = ref->next;
    current->front = ref;
    ref->next = current;
    if(current->type == 'C') {
        quantity += current->quantity;
    }
    size++;
    try{
        checkArrivedTime();
    } catch (exception &e) {
        cout << "In insertAfter: " << e.what() << endl;
        exit(1);
    }
}

void Route::insertAfter(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer){
    // 在链表中refStore指针指向的节点后面插入store指针指向节点
    // 在链表中refCustomer指针指向的节点后面插入customer指向节点
    assert(store->type == 'S' && customer->type == 'C');
    if(DEBUG) {
        Spot *ptr = current;
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
            cout << "refStore: " << refStore->type << " refCustomer: " << refCustomer->type
                << endl;
            throw out_of_range("Cannot find the position to insert!");
            return;
        }
    }
    if(refStore == stand->front) {
        stand->next = store;
    } 
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
    try {
        checkArrivedTime();  // 插入节点后，检查arrivedTime
    } catch (exception &e) {
        cout << "refStore: " << refStore->id << " refCustomer: " << refCustomer->id << endl;
        cout << "While inserting: store: " << store->id << " customer: " 
            << customer->id << endl;
        throw out_of_range("In insertAfter: " + string(e.what()));
    }
}

void Route::insertAtHead(Spot *store, Spot *customer){ 
    // 在表头插入store和customer
    // 注意store须在customer前面（对于pickup-delivery问题，本函数慎用）
    // 只有当current指针为head时返回true
    assert(store->type == 'S' && customer->type == 'C');
    if(current == head && size == 0) {
        // 要求路径必须为空才可以这种方式插入
        if(timeWindowJudge(head, head, store, customer) == false) {
            throw out_of_range("Invalid inserting!");
        }
        head->next = store;
        store->next = customer;
        store->front = head;
        rear->front = customer;
        customer->next = rear;
        customer->front = store;
        stand->next = store;
        quantity = quantity + customer->quantity;
        size++;
        try {
            checkArrivedTime();  // 插入节点后，更新arrivedTime
        } catch (exception &e) {
            cout << "store id: " << store->id << " customer id: "
                << customer->id << endl;
            cout << "In insertAtHead: " << e.what() << endl;
            exit(1);
        }
    }
    else{
        throw out_of_range("Cannot insert node after head!");
    }
}

void Route::insertAtRear(Spot *node) {
    // 在表尾插入node，注意这里不检查插入合法性
    // 具体而言，默认current指针位于head
    // 需要由用户自己保证节点是可以凑成(P-D)对
    rear->front->next = node;
    node->next = rear;
    node->front = rear->front;
    rear->front = node;
    if(node->type == 'C') {
        quantity = quantity + node->quantity;
        size++;
        if(node->choice->type == 'D') {
            node->choice = head;
        }
    }
    stand->next = current->next;
    try{
        checkArrivedTime();  // 插入节点后，检查arrivedTime
    } catch (exception &e) {
        throw out_of_range("In insert at rear: " + string(e.what()));
    }
}

void Route::deleteNode(Spot *node) {
    // 删除node节点
    if(DEBUG) {
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
    }
    if(stand->next == node) stand->next = node->next;
    node->front->next = node->next;
    node->next->front = node->front;
    if(node->type == 'C') {
        quantity -= node->quantity;
        size--;
    }
    try{
        checkArrivedTime();
    } catch (exception &e) {
        cout << "In deleteNode: " << e.what() << endl;
        exit(1);
    }
    delete node;
}

void Route::deleteNode(Spot *store, Spot *customer){
    // 删除链表中指针值与store和customer相同的节点
    // 只能删除current指针后面的节点
    assert(store->type == 'S' || store->type == 'D');
    assert(customer->type == 'C');
    if(current == rear) {
        // 已经走完了路径中的所有节点，禁止删除
        throw out_of_range("Forbid deleting for we have finished the route!");
    }

    if (current == NULL) {
        throw out_of_range("The current node is NULL!");
    }

    if(DEBUG) {
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
            return;
        }
    }
    if(stand->next == store) {
        if(store->next == customer) stand->next = customer->next;
        else stand->next = store->next;
    }
    store->front->next = store->next;
    store->next->front = store->front;
    customer->front->next = customer->next;
    customer->next->front = customer->front;
    delete store;
    delete customer;
    size--;
    quantity = quantity - customer->quantity;
    try{
        checkArrivedTime();  // 删除节点后，更新arrivedTime
    } catch (exception &e) {
        cout << "In deleteNode: " << e.what() << endl;
        exit(1);
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

Spot* Route::findCustomer(int id) {
    for(Spot *node=head->next; node!=rear; node=node->next) {
        if(node->id == id) {
            return node;
        }
    }
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
    vector<Spot*> customerSet;
    for(Spot *ptr=head; ptr!=rear; ptr=ptr->next){
        if(ptr->type == 'C') {
            customerSet.push_back(ptr);
        }
    }
    return customerSet;
}

vector<Spot*> Route::getAllSpot() {
    // 得到head到rear(不包括二者)的所有节点
    vector<Spot*> spotSet;
    for(Spot *ptr=head->next; ptr!=rear; ptr=ptr->next) {
        spotSet.push_back(ptr);
    }
    return spotSet;

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

    float len = 0;
    if(size == 0) {
        // 无顾客节点
        return len;
    }
    if(artificial == false) { // real vehicle routing scheme
        for(Spot *ptr = head->next; ptr->next != NULL; ptr = ptr->next) {
            Spot *pre = ptr->front;
            Spot *next = ptr->next;
            float cost = 0.0f;
            try{
                cost = dist(pre, ptr) + dist(ptr, next);
            } catch (exception &e) {
                cerr << "In getLen, Route.cpp: " << e.what() << endl;
                exit(1);
            }
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
        for(Spot *ptr = head->next; ptr->next != NULL; ptr = ptr->next) {
            Spot *pre = ptr->front;
            Spot *next = ptr->next;
            float cost = 0.0f;
            try {
                cost = dist(pre, ptr) + dist(ptr, next);
            } catch (exception &e) {
                cerr << "In getLen, Route.cpp: " << e.what() << endl;
                exit(1);
            }
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
            originLen = originLen + dist(front, back);
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
    current = current->next;
    stand->x = current->x;
    stand->y = current->y;
    stand->front = current;
    stand->next = current->next;
    current->visit = true;
    if(current == NULL) {  // 已经完成任务
        return false;
    } else {
        return true;
    }
}

void Route::setStand(float x, float y, float arrivedTime, float serviceTime) {
    stand->x = x;
    stand->y = y;
    stand->arrivedTime = arrivedTime;
    stand->serviceTime = serviceTime;
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
            if(ptr->choice->type == 'D') {
                // 对于choice为Depot的顾客节点，其removeCost为无穷大
                // 表示这类节点不可以被remove
                costArr.push_back(LARGE_FLOAT);
            } else {
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
                    switch(ptr->priority){
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
    }
    return costArr;
} 

bool Route::timeWindowJudge(Spot *refStore, Spot *refCustomer, Spot *store, Spot *customer){
    // 判断将store插入到refStore后面并且将customer插入到refCustomer后面是否会违反时间窗约束
    // 注意refStore和refCustomer都可能是"store"或者"customer"
    // 但是refStore必定在refCustomer前面
    assert(store->type == 'S' && customer->type == 'C');
    Spot *temp;    // temp节点为store插入点，可能是驻点
    float time = stand->arrivedTime; // 驻点当前时间为基准时间
    time += stand->serviceTime;
    temp = stand;
    // 从refStore出发的time，不判断current到refStore的时间窗约束
    // temp成为名义上的refStore，正好位于store前面
    if(refStore != current) {
        temp = current->next;
        time += dist(stand, temp);
        if(temp->type == 'C' && time < temp->startTime) {
            time = temp->startTime;
        }
        time += temp->serviceTime;
        temp = temp->next;
        while(temp != refStore && temp != NULL) {
            // temp==NULL说明refStore不存在，一般不会出现
            time += dist(temp->front, temp);
            if(temp->type == 'C' && time < temp->startTime) {
                time = temp->startTime;
            }
            time += temp->serviceTime;
            temp = temp->next;
        }
        if(temp == NULL) return false;
        time += dist(temp->front, temp);
        if(refStore->type == 'C') {
            if(time > refStore->endTime) return false;
            if(time < refStore->startTime) time = refStore->startTime;
        }
        time += temp->serviceTime;
    }

    // 现在time为从refStore节点的出发时间
    // 注意refStore有可能会变成stand
    time += dist(temp, store);
    time += store->serviceTime;
    // pre指向store或者customer(视refStore是否正好为refCustomer而定)
    // cur指向refStore的下一个节点
    Spot *pre, *cur; // 用来判断customer之后的时间窗约束是否被遵守
    
    // 判断从store到customer为止的时间窗约束是否被遵守
    if(refStore == refCustomer) {
        // 对于refStore==refCustomer的情况，作特殊处理
        time += dist(store, customer);
        if(time > customer->endTime) return false;
        if(time < customer->startTime) time = customer->startTime;
        time += customer->serviceTime;
        pre = customer;
        cur = temp->next;
    } else {
        pre = store;
        cur = temp->next;
    }
    
    if(time > OFF_WORK_TIME) return false;
    // 接下来判断customer之后的时间窗约束是否被遵守
    // 这里refCustomer已经不可能是stand，只能是路径中的节点
    while(true) {
        if(pre == refCustomer) {
            // 前一节点是refCustomer，那么下一节点应该是customer
            cur = customer;
        }
        if(cur == rear) break;
        time += dist(pre, cur);
        if(cur->type == 'C') {
            if(time > cur->endTime) return false;
            if(time < cur->startTime) time = cur->startTime;
        }
        time += cur->serviceTime;
        if(time > OFF_WORK_TIME) return false;
        if(cur == customer) {
            // pre由customer暂时代替，但是不真正地将customer插入
            // 此时current指针指向的是路径中实际存在的节点（customer之后）
            cur = pre->next;
            pre = customer;
        } else {
            pre = cur;
            cur = cur->next;
        }
    }
    return true;
}

void Route::computeInsertCost(Spot *store, Spot* customer, float &minValue, 
        Spot *&refStore1, Spot *&refCustomer1, float &secondValue, Spot *&refStore2,
        Spot *&refCustomer2, float randomNoise, bool allowNegativeCost){
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
    refStore1 = NULL;
    refCustomer1 = NULL;
    refStore2 = NULL;
    refCustomer2 = NULL;
    minValue = MAX_FLOAT;
    secondValue = MAX_FLOAT;
    if(quantity + customer->quantity > capacity) {
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

void Route::checkArrivedTime(){   
    // 更新一下各个节点的到达时刻
    // 头结点的arrivedTime + serviceTime将作为基准时间
    float time = stand->arrivedTime;
    time += stand->serviceTime;
    Spot *pre = stand;
    Spot *cur = stand->next;
    while(cur != rear) {
        time += dist(pre, cur);
        cur->arrivedTime = time;
        if(cur->type == 'C') {
            if(time > cur->endTime) {
                cout << "problem in: " << cur->id << endl;
                cout << "Now time is: " << time << " end time for him: " <<
                    cur->endTime << endl;
                Spot *temp = head->next;
                cout << "Now ids are: " << endl;
                for(temp; temp != rear; temp = temp->next) {
                    cout << temp->id << "\t";
                }
                cout << endl;
                throw out_of_range("Violating time constraints");
            }
            if(time < cur->startTime) time = cur->startTime;
        }
        time += cur->serviceTime;
        if(time > OFF_WORK_TIME) {
            cout << "Time is: " << time << endl;
            throw out_of_range("Exceeding the off work time!");
        }
        pre = pre->next;
        cur = cur->next;
    }
}


//=============== 路径的替换和提取 ================//
Route* Route::getEmptyRoute(vector<Spot*> &removedCustomer) {
    // 获取空的路径，但是需要保留路径中choice为depot的customer
    // removedCustomer: current指针之后的顾客节点 - 保留的顾客节点
    Route *newRoute = new Route(*head, *rear, leftQuantity);
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
                Spot *customer = new Spot(*temp);
                customer->choice = newRoute->head;
                newRoute->insertAtRear(customer);
            }
        }
        temp = temp->next;
    }
    return newRoute;
}

vector<Spot*> Route::capture(){ 
    // Intro:
    //   * 抓取current指针后的所有节点
    //   * 对于已经访问过对应pickup节点的delivery节点，其选择的store暂时为
    //   * 抓取路径的head节点
    vector<Spot*> output;
    if(current->next == rear) { // current指针后已经没有路径
        return output;
    }
    for(Spot* ptr=current->next; ptr != rear; ptr = ptr->next) {
        Spot *temp = new Spot(*ptr);
        if(temp->type == 'C') {
            if(temp->choice->visit == true) {
                // customer对应的store已经被访问过
                temp->choice = rear;
            } else {
                // 由于节点是复制的，因此choice信息丢失，需要
                // 根据原有的位置关系确定choice指向
                int count = 0;
                Spot* temp2 = current->next;
                while(temp2->choice->id != ptr->id) {
                    temp2 = temp2->next;
                    count++;
                }
                output[count]->choice = temp;
                temp->choice = output[count];
            }
        }
        output.push_back(temp);
    }
    return output;
}

void Route::replaceRoute(Route *route) {  
    // 以route替换掉current指针后的路径
    // 对于route中choice为depot的customer，需要找回其原本指向的商店
    vector<Spot*> customerPool(CUSTOMER_NUM);
    Spot *ptr1, *ptr2;
    // 清空本路径中current指针后面的节点 
    if(current->next != rear) { // current后面还有节点
        // 清除原路径中current指针后面的元素
        // 不包括对rear节点的清除
        ptr1 = current->next;
        while(ptr1 != rear) {
            if(ptr1->type == 'C') {
                customerPool[ptr1->id] = ptr1->choice;
            }    
            ptr2 = ptr1->next;
            deleteNode(ptr1);
            ptr1 = ptr2;
        }
    }
    // 修改route中choice为depot的customer其选择的商店
    ptr1 = route->head->next;
    while(ptr1 != NULL) {
        if(ptr1->type == 'C' && ptr1->choice->type == 'D') {
            Spot *store = customerPool[ptr1->id];
            store->choice = ptr1;
            ptr1->choice = store;
        }
        ptr1 = ptr1->next;
    }

    // 将route中除head和rear外的节点都复制到current指针后
    ptr1 = route->head->next;
    while(ptr1 != route->rear) {
        ptr2 = ptr1->next;
        try {
            insertAtRear(ptr1);
        } catch (exception &e) {
            throw out_of_range("While replace route: " + string(e.what()));
        }
        ptr1 = ptr2; 
    }
    // 清空变量
    customerPool.clear();
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

bool Route::checkTimeConstraint() {
    // 检查当前路径是否满足时间窗约束
    if(size == 0) return true;
    Spot *pre = head;
    Spot *cur = head->next;
    float time = 0;
    while(cur != rear) {
        time += dist(pre, cur);
        if(cur->type == 'C') {
            if(time < cur->startTime) {
                time = cur->startTime;
            }
            if(time > cur->endTime) {
                return false;
            }
        }
        time += cur->serviceTime;
        pre = pre->next;
        cur = cur->next;
    }
    return true;
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
                delete ptr2;
                ptr2 = ptr1->next;
                delete ptr1;
                ptr1 = ptr2;
            } else {
                retainNum++;
                int pos = intIter - validCustomerId.begin();
                posVec.push_back(pos);
                ptr1 = ptr1->next;
            } 
        } else {
            ptr1 = ptr1->next;
        }
    }
    posVec.push_back(0);  // 仓库节点位置
    return posVec;
}
