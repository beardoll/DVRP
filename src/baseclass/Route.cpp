#include "Route.h"
#include "Customer.h"
#include "../public/PublicFunction.h"
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
Route::Route(Customer &headNode, Customer &rearNode, float capacity):capacity(capacity)
{ 
    // 构造函数
    head = new Customer;
    *head = headNode;  // 复制节点
    head->type = 'D';
    rear = new Customer;
    *rear = rearNode;
    rear->type = 'D';
    stand = new Customer;
    *stand = headNode;
    stand->type = 'D';
    stand->front = head;
    stand->next = rear;
    head->front = NULL;
    head->next = rear;
    rear->front = head;
    rear->next = NULL;
    current = head;  // 初始化current指针指向head节点
    size = 0;
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
    Customer* originPtr = L.head;
    Customer* copyPtr = head;
    Customer* temp = NULL;
    while(originPtr!=NULL){
        // 从头节点一直复制到尾节点
        if(originPtr == L.head){  // 正在复制第一个节点
            copyPtr = new Customer;
            copyPtr->front = NULL;
            head = copyPtr;
            *copyPtr = *L.head;
        } else{
            temp = new Customer;
            *temp = *originPtr;
            temp->front = copyPtr;
            copyPtr->next = temp;
            copyPtr = temp;
		}
        if(L.current == originPtr){
            // current指针的复制
            current = copyPtr;
        }
		originPtr = originPtr->next;
    }
    temp->next = NULL;
    rear = temp;
    // stand指针的设定
    stand = new Customer;
    *stand = *L.stand;
    stand->front = current;
    stand->next = current->next;
}

Customer& Route::operator[] (int k){
    assert(k>=0 && k<size);
    Customer* temp = head->next;
    for(int i=0; i<k; i++){
        temp = temp->next;
    }
    return *temp;
}

const Customer& Route::operator[] (int k) const{
    assert(k>=0 && k<size);
    Customer* temp = head->next;
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

void Route::clear(){  // 清空链表，不清空head节点和rear节点?
    Customer* ptr1 = head;
    Customer* ptr2;
    while(ptr1!=NULL){
        ptr2 = ptr1->next;
        delete ptr1;
        ptr1 = ptr2;
    }
    head = NULL;
    rear = NULL;
    current = NULL;
    delete stand;
    stand = NULL;
    size = 0;
}

void Route::printRoute(){ // 打印链表
    Customer* ptr = head;
    for(; ptr!=NULL; ptr=ptr->next) {
        cout << "id:" << ptr->id << ' ' << "type:" << ' ' << ptr->type << endl;
    }
}


//=============== 插入以及删除节点操作 ================//
void Route::insertAfter(Customer item1, Customer item2){
    // 在链表中与item1相同的节点后面插入节点item2
    Customer* temp = new Customer;
    *temp = item2;
    Customer* ptr = head;
    while(ptr!=rear){
        if (ptr->id == item1.id){  // 根据id来判断两个节点是否相同
            break;
        }
        ptr = ptr->next;
    }
    if(ptr == rear) {
        // 没有找到，返回false
        delete temp;
        throw out_of_range("Cannot find the position to insert!");
    } else{
        if(ptr->id == stand->front->id) {
            // 说明item2节点插入到stand节点后面
            stand->next = temp;
        }
        quantity = quantity + item2.quantity;
        temp->next = ptr->next;
        ptr->next->front = temp;
        temp->front = ptr;
        ptr->next = temp;
        size++;
        try{
            checkArrivedTime();
        } catch (exception &e) {
            throw out_of_range("In insertAfter: " + string(e.what()));
        }
    }
}

void Route::insertAtHead(Customer item){ 
    // 在表头插入item
    // 只有当current指针为head时返回true
    if(current == head) {
        Customer *temp = new Customer;
        *temp = item;
        temp->next = head->next;
        head->next->front = temp;
        head->next = temp;
        temp->front = head;
        stand->next = temp;
        quantity = quantity + item.quantity;
        size++;
        try {
            checkArrivedTime();
        } catch(exception &e) {
            throw out_of_range("In insertAtHead: " + string(e.what()));
        }
    }
    else{
        throw out_of_range("The car has departured, cannot insert node after head!");
    }
}

void Route::insertAtRear(Customer item){   
    // 在表尾插入item
    // 只有当表尾不是current节点时返回true
    if(current != rear) {
        Customer *temp = new Customer;
        *temp = item;
        temp->next = rear;
	    temp->front = rear->front;
	    rear->front->next = temp;
	    rear->front = temp;
        stand->next = current->next;
	    quantity = quantity + item.quantity;
	    size++;
        try {
            checkArrivedTime();
        } catch (exception &e) {
            throw out_of_range("In insert at rear: " + string(e.what()));
        }
	} else {
        throw out_of_range("Has reached the end node, cannot insert any nodes!");
    }
}

void Route::deleteNode(Customer item){
    // 删除链表中与item相同的节点
    // 只能删除current指针后面的节点
    if(current == rear) {
        // 已经走完了路径中的所有节点，禁止删除
        throw out_of_range("Forbid deleting for we have finished the route!");
    }
    Customer* temp1 = current->next;

    if (current == NULL) {
        throw out_of_range("The current node is NULL!");
    }
    if (temp1 == NULL) {
        throw out_of_range("We have reached the rear!");
    }

    while(temp1!=rear) {
        if(temp1->id == item.id) {
            break;
        }
        temp1 = temp1->next;
	}
	if(temp1 == rear) {  // 没有找到
        throw out_of_range("We want to delete inexistent customer!");
	} else {
        if(stand->next->id == temp1->id) stand->next = temp1->next;
        Customer* nextNode = temp1->next;
        Customer* frontNode = temp1->front;
        frontNode->next = nextNode;
        nextNode->front = frontNode;
        delete temp1;
        size--;
        quantity = quantity - item.quantity;
        try{
            checkArrivedTime();
        } catch (exception &e) {
            throw out_of_range("In deleteNode: " + string(e.what()));
        }
    }
}


//=============== 获得单节点操作 ================//
Customer& Route::currentPos(){ // 返回当前位置
    return *current;
}

Customer& Route::getHeadNode() {
    Customer* newCust = new Customer(*head);
    return *newCust; 
}

Customer& Route::getRearNode() {
    Customer* newCust = new Customer(*rear);
    return *newCust; 
}


//=============== 获取链表属性 ================//
int Route::getSize() {
    return this->size;
}

vector<Customer*> Route::getAllCustomer(){  // 得到路径中所有的顾客节点
    // 返回的customer是用new产生的堆对象，如果内存溢出务必注意此处
    vector<Customer*> customerSet(size);
    Customer* ptr = head->next;
    Customer* ptr2;
    for(int i=0; i<size; i++){
        ptr2 = new Customer;
        *ptr2 = *ptr;
        customerSet[i] = ptr2;
        ptr = ptr->next;
    }
    return customerSet;
}

float Route::getLen(float DTpara[], bool artificial){   // 得到路径长度
    // 返回值为实际的路径长度加上惩罚因子
    // 提取DTpara
    float DTH1, DTH2, DTL1, DTL2;
    float *DTIter = DTpara;
    DTH1 = *(DTIter++);
    DTH2 = *(DTIter++);
    DTL1 = *(DTIter++);
    DTL2 = *(DTIter++);

    Customer *ptr1 = head;
    Customer *ptr2 = head->next;
    if(artificial == false) { // real vehicle routing scheme
        float len = 0;
        while(ptr2 != NULL){
            float temp1 = 0;
            switch(ptr1->priority){
                case 0: {
                    temp1 = 0.0f;
                    break;
                }
                case 1: {
                    temp1 = -DTH1;
                    break;
                }
                case 2: {
                    temp1 = -DTL1;
                    break;
                }
            }
            len = len + dist(ptr1, ptr2);
            len += temp1;
            ptr2 = ptr2->next;
            ptr1 = ptr1->next;
        }
        return len;
    } else {
        float len = 0;
        while(ptr2 != NULL){
            float temp1 = 1.0f;
            switch(ptr1->priority){
                case 0: {
                    temp1 = 0.0f;
                    break;
            }
                case 1: {
                    temp1 = DTH2;
                    break;
                }
                case 2: {
                    temp1 = DTL2;
                break;
                }
            }
            len = len + dist(ptr1, ptr2);
            len += temp1;
            ptr2 = ptr2->next;
            ptr1 = ptr1->next;
        }
        return len;		
    }
}

float Route::getOriginLen() {  
    // 得到服务静态节点的路径代价
    // 注意，以property标识顾客属性，当property为0时表示静态，为1表示动态
    Customer* front = head;         // 搜索的起始节点
    Customer* back = front->next;   // 下一个节点
    float originLen = 0;
    while(back != NULL) {
        // 首尾节点，即仓库，在此计算范围之内
        if(back->prop != 0) {
            back = back->next;
        } 
        else {
            originLen += dist(front, back);
            front = back;
            back = back->next;
        }
    }
    return originLen;
}


vector<float> Route::getArrivedTime(){     // 得到本车所有节点的arrivedTime
    Customer *temp = head->next;
    vector<float> arrivedTimes;
    while(temp != rear) {
        arrivedTimes.push_back(temp->arrivedTime);
        temp = temp->next;
    }
    return arrivedTimes;
}


//=============== 修改链表属性 ================//
bool Route::moveForward(){
    current = current->next;
    stand->x = current->x;
    stand->y = current->y;
    stand->front = current;
    stand->next = current->next;
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
    // 得到所有顾客节点的移除代价
    // 值越小表示移除它可以节省更多的代价
    // artificial: 为true表示是一辆虚假的车
    // 如果需要真正的移除代价，则下面的DT都取为0即可
    float DTH1, DTH2, DTL1, DTL2;
    float *DTIter = DTpara;
    DTH1 = *(DTIter++);
    DTH2 = *(DTIter++);
    DTL1 = *(DTIter++);
    DTL2 = *(DTIter++);
    vector<float> costArr(0);
    Customer *ptr1 = head;   // 前节点
    Customer *ptr2, *ptr3;
    for(int i=0; i<size; i++){
        ptr2 = ptr1->next;  // 当前节点
        ptr3 = ptr2->next;  // 后节点
        float temp = -dist(ptr1, ptr2) - dist(ptr2, ptr3) + dist(ptr1, ptr3); 
        float temp1 = 0;
        if(artificial == true) {
            switch(ptr1->priority){
                case 0: {
                    temp1 = 0;
                    break;
                }
                case 1: {
                    temp1 = -DTH2;
                    break;
                }
                case 2: {
                    temp1 = -DTL2;
                    break;
                }
            }
        } else {
            switch(ptr1->priority){
                case 0: {
                    temp1 = 0;
                    break;
                }
                case 1: {
                    temp1 = DTH1;
                    break;
                }
                case 2: {
                    temp1 = DTL1;
                    break;
                }
            }		
        }
        temp += temp1;
        costArr.push_back(temp);
        ptr1 = ptr1->next;
    }
    return costArr;
} 

bool Route::timeWindowJudge(Customer *pre, Customer item){  
    // 计算把item插入到pre后面是否会违反时间窗约束
    // 暂时不考虑仓库的关仓时间
    // pos是pre的位置, 0表示仓库
    float time = stand->arrivedTime;
    time += stand->serviceTime;
    Customer *temp, *temp2;
    // 从current到pre
    if(pre != current) {
        temp = current->next;
        time += dist(stand, temp);
        if(time < temp->startTime) time = temp->startTime;
        time += temp->serviceTime;
        temp = temp->next;
        while(temp != pre && temp != NULL) {
            // temp == NULL证明pre不存在，一般不可能
            time += dist(temp->front, temp);
            if(time > temp->endTime) return false;
            if(time < temp->startTime) time = temp->startTime;
            time += temp->serviceTime;
            time += temp->serviceTime;
            temp = temp->next;
        }
        if(temp == NULL) return false;
        time += dist(temp->front, temp);
        if(time > pre->endTime) return false;
        if(time < pre->startTime) time = pre->startTime;
        time += temp->serviceTime;
    }

    // 现在time是从pre出发的时间
    // 接下来是判断插入item后会不会违反item以及其后继节点的时间窗约束
    time = time + dist(pre, &item);
    if(time > item.endTime) return false;
    if(time < item.startTime) time = item.startTime;
    time = time + item.serviceTime;
    // 是否影响pre的下一个节点
    temp = pre->next;
    if(temp == rear) return true;
    time = time + dist(temp, &item);
    if(time > temp->endTime) return false;
    if(time < temp->startTime) time = temp->startTime;
    time = time + temp->serviceTime;

    // 然后判断会不会违反更靠后的节点的时间窗约束
    temp = pre->next;
    temp2 = temp->next;
    while(temp2 !=rear){ 
        time = time + dist(temp, temp2);
        if(time > temp2->endTime) return false;
        if(time < temp2->startTime) time = temp2->startTime;
        time = time + temp2->serviceTime;
        temp = temp->next;
        temp2 = temp2->next;
    }
    return true;
}

void Route::computeInsertCost(Customer item, float &minValue, Customer &customer1, 
        float &secondValue, Customer &customer2,
        float randomNoise, bool allowNegativeCost){
    // 计算item节点在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点前面的顾客节点
    // pertubation: 扰动的噪声量
    // allowNegativeCost: 为true表示插入代价取非负数，为false表示可取负数
    // randomNoise: 随机噪声量
    Customer *pre;
    minValue = MAX_FLOAT;
    secondValue = MAX_FLOAT;
    customer1.id = -1;
    customer2.id = -1;
    for(pre=current; pre!=rear; pre=pre->next) {  // 一共有size+1个位置可以考虑插入
        if(quantity + item.quantity <= capacity){   // 容量约束
            if(timeWindowJudge(pre, item) == true) { // 满足时间窗约束
                float cost = dist(pre, &item) + dist(&item, pre->next) -
                    dist(pre, pre->next);
                cost += randomNoise;
                if(allowNegativeCost == false) {
                    cost = max(0.0f, cost);
                }
                if(cost <= minValue){  // 找到了更小的，更新minValue和secondValue
                    secondValue = minValue;
                    customer2 = customer1;
                    minValue = cost;
                    customer1 = *pre;
                }
            }
        }
    }
}

void Route::checkArrivedTime() {
    float time = stand->arrivedTime;
    time += stand->serviceTime;
    Customer *pre = stand;
    Customer *cur = stand->next;
    while(cur != rear) {
        time += dist(pre, cur);
        cur->arrivedTime = time;
        if(time > cur->endTime) {
            cout << "Problem in: " << cur->id << " type is: " << cur->type << endl;
            cout << "Now time is: " << time << " end time for him: " <<
                cur->endTime << endl;
            Customer *temp = head->next;
            cout << "Now ids are: " << endl;
            for(temp; temp != rear; temp = temp->next) {
                cout << temp->id << "\t";
            }
            cout << endl;
            throw out_of_range("Violating time constraints");
        }
        if(time < cur->startTime) time = cur->startTime;
        time += cur->serviceTime;
        pre = pre->next;
        cur = cur->next;
    }
}


//=============== 路径的替换和提取 ================//
Route& Route::capture(){ 
    // 抓取current指针后的路径
    // current指针当前节点将作为head节点
    // 将当前路径的capacity和leftQuantity原样复制
    Route* ptr1 = new Route(*current, *rear, capacity);
    if(current->next == rear) { // current指针后已经没有路径
        return *ptr1;
    }
    Customer *ptr2 = current->next;
    Customer *ptr3 = NULL;
    Customer *ptr4 = NULL;
    ptr4 = ptr1->head;
    while(ptr2 != rear) {
        ptr3 = new Customer;
        *ptr3 = *ptr2;
        ptr4->next = ptr3;
        ptr3->front = ptr4;
        ptr4 = ptr3;
        ptr1->quantity = ptr1->quantity + ptr2->quantity;
        ptr2 = ptr2->next;
        ptr1->size++;
    }
    ptr3->next = ptr1->rear;
    ptr1->rear->front = ptr3;
    ptr1->setLeftQuantity(leftQuantity);
    return *ptr1;
}

void Route::replaceRoute(const Route &route) {  // 以route替换掉current指针后的路径
    Customer* ptr1;
    Customer *ptr2, *ptr3;
    if(current->next != rear) { // current后面还有节点，需要先清除原有路径
        ptr2 = current->next;
        // 清除原路径中current指针后面的元素
        // 不包括对rear节点的清除
        while(ptr2 != rear) {
            quantity -= ptr2->quantity;
            ptr3 = ptr2->next;
            delete ptr2;
            ptr2 = ptr3;
            size--;
        }
    }
    current->next = rear;
    rear->front = current;
    stand->next = rear;
    // 将route中除head和rear外的节点都复制到current指针后
    ptr1 = route.head->next;
    while(ptr1 != route.rear) {
        ptr2 = ptr1->next;
        try{
            insertAtRear(*ptr1);
        } catch (exception &e) {
            throw out_of_range("While replace route: " + string(e.what()));
        }
        ptr1 = ptr2;
    }
    return;
}

//=============== 其余辅助性质的函数 ================//
bool Route::checkPassRoute(){
    // 检查已走过的路径是否违反时间窗约束
    // 这里只检查到达下一个节点的时刻是否小于前一个节点的时间窗起始时刻
    Customer* ptr1 = head;
    Customer* ptr2 = head->next;
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
    // 仅保留id在validCustomerId中的节点
    // 返回保留的节点在validCustomerId中的位置
    vector<int> posVec;
    posVec.push_back(0);   // 仓库节点位置
    Customer* ptr1 = head->next;
    while(ptr1 != rear) {
        int currentId = ptr1->id;
        vector<int>::iterator intIter = find(validCustomerId.begin(), 
                validCustomerId.end(), currentId);
        if(intIter == validCustomerId.end()) {
            // 如果找不到，说明该节点是invalid，删除之
            quantity -= ptr1->quantity;
            size--;
            ptr1->front->next = ptr1->next;
            ptr1->next->front = ptr1->front; 
        } else {
            retainNum++;
            int pos = intIter - validCustomerId.begin();
            posVec.push_back(pos);
        } 
        ptr1 = ptr1->next;
    }
    posVec.push_back(0);  // 仓库节点位置
    return posVec;
}
