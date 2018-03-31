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
Route::Route(Spot &headNode, Spot &rearNode, float capacity, float timeDuration):
    capacity(capacity), timeDuration(timeDuration) 
{ 
    // 构造函数
    head = new Spot(headNode);
    rear = new Spot(rearNode);
    head->front = NULL;
    head->next = rear;
    rear->front = head;
    rear->next = NULL;
    size = 0;
    quantity = 0;
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
    this->timeDuration = L.timeDuration;
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
		ptr = ptr->next;
    }
    if(mark == false) {
        throw out_of_range("Cannot find the position to insert!");
        return;
    }
    ref->next->front = current;
    current->next = ref->next;
    current->front = ref;
    ref->next = current;
    quantity += current->quantity;
    size++;
    if(checkTimeConstraint() == false) {
        throw out_of_range("In insertAfter, time constraints violated");
    }
}

void Route::insertAtRear(Spot *node) {
    // 在表尾插入node
    rear->front->next = node;
    node->next = rear;
    node->front = rear->front;
    rear->front = node;
    quantity = quantity + node->quantity;
    size++;
    if(checkTimeConstraint() == false) {
        throw out_of_range("In insertAtRear, time constraints violated");
    }
}

void Route::deleteNode(Spot *node) {
    // 删除node节点
    bool mark = false;
    for(Spot* ptr = head; ptr != rear; ptr = ptr->next) {
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
    quantity -= node->quantity;
    size--;
    if(checkTimeConstraint() == false) {
        throw out_of_range("In deleteNode, time constraints violated");
    }
    delete node;
}

//=============== 获得单节点操作 ================//
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
    for(Spot *ptr=head->next; ptr!=rear; ptr=ptr->next){
            customerSet.push_back(ptr);
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

    float len = 0;
    if(size == 0) {
        // 无顾客节点
        return len;
    }
    Spot *pre = head;
    Spot *cur = head->next;
    if(artificial == false) { // real vehicle routing scheme
        while(cur != NULL) {
            float temp1 = 0;
            switch(pre->priority) {
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
            len += dist(pre, cur);
            len += temp1;
            pre = pre->next;
            cur = cur->next;
        }
        return len;
    } else {
        while(cur != NULL) {
            float temp1 = 0;
            switch(pre->priority) {
                case 0: {
                    temp1 = 0;
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
            len += dist(pre, cur);
            len += temp1;
            pre = pre->next;
            cur = cur->next;
        }
        return len;
    }
}

float Route::getTrueLen(){   
    // 得到真实的路径长度（不添加惩罚）
    // Args:
    //   * DTpara: 对不同类型的车/顾客组合的惩罚因子
    //   * artificial: 车辆属性，为true表示virtual car
    // Returns:
    //   * len: 路径长度

    float len = 0;
    if(size == 0) {
        // 无顾客节点
        return len;
    }
    Spot *pre = head;
    Spot *cur = head->next;
    while(cur != NULL) {
        len += dist(pre, cur);
        pre = pre->next;
        cur = cur->next;
    }
    return len;

}

float Route::getTimeDuration() {
    // time duration: travel len + serviceTime
    float timeDuration = 0;
    if(size == 0) {
        return 0;
    }
    Spot *pre = head;
    Spot *cur = head->next;
    while(cur!=NULL) {
        timeDuration += dist(pre, cur);
        timeDuration += cur->serviceTime;
        pre = pre->next;
        cur = cur->next;
    }
    return timeDuration;
}

vector<int> Route::getAllID() {
    // 获得所有id，包括head和rear
    Spot *ptr;
    vector<int> IDs;
    for(ptr=head; ptr!=NULL; ptr=ptr->next) {
        IDs.push_back(ptr->id);
    }
    return IDs;
}

vector<float> Route::getArrivedTime() {
    Spot *ptr;
    vector<float> arrivedTimes;
    for(ptr=head->next; ptr!=rear; ptr=ptr->next) {
        arrivedTimes.push_back(ptr->arrivedTime);
    }
    return arrivedTimes;
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
    for(Spot* ptr = head->next; ptr != rear; ptr = ptr->next) {
        float cost = -dist(ptr->front, ptr) - dist(ptr, ptr->next)
                     + dist(ptr->front, ptr->next);
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
    return costArr;
} 

bool Route::timeWindowJudge(Spot *ref, Spot *cur){
    // 判断将cur插入到ref后面是否会违反时间窗约束
    // 同时计算timeDuration
    Spot *temp = head;  
    float time = 0;
    float td = 0;
    // 计算到达ref的时间
    while(temp != ref) {
        if(temp->type == 'D') {
            time = temp->arrivedTime;
            time += temp->serviceTime;
            td += temp->serviceTime;
        } else {
			if(time > temp->endTime) return false;
            if(time < temp->startTime) time = temp->startTime;
            time += temp->serviceTime;
            td += temp->serviceTime;
        }
        time += dist(temp, temp->next);
        td += dist(temp, temp->next);
        if(td > timeDuration) return false;
        temp = temp->next;
    }
    // 计算是否违反cur的时间窗约束
    if(time < ref->startTime) time = ref->startTime;
    time += ref->serviceTime;
    td += ref->serviceTime;
    time += dist(ref, cur);
    td += dist(ref, cur);
    if(time > cur->endTime) return false;
    if(time < cur->startTime) time = cur->startTime;
    time += cur->serviceTime;
    td += cur->serviceTime;
    if(td > timeDuration) return false;
    // 当前时间为从cur出发的时间，判断是否违反原路径中ref以后的时间窗约束
    temp = ref->next;
    if(temp != rear) {
        time += dist(cur, temp);
        td += dist(cur, temp);
        if(time > temp->endTime) return false;
        if(time < temp->startTime) time = temp->startTime;
        time += temp->serviceTime;
        td += temp->serviceTime;
        if(td > timeDuration) return false;
        temp = temp->next;
        while(temp != rear) {
            time += dist(temp->front, temp);
            td += dist(temp->front, temp);
            if(time > temp->endTime) return false;
            if(time < temp->startTime) time = temp->startTime;
            time += temp->serviceTime;
            td += temp->serviceTime;
            if(td > timeDuration) return false;
            temp = temp->next;
        }
        td += dist(rear->front, rear);
        if(td > timeDuration) return false;
    } else {
        td += dist(cur, rear);
        if(td > timeDuration) return false;
    }
    return true;
}

void Route::computeInsertCost(Spot* cur, float &minValue, Spot *&ref1, 
        float &secondValue, Spot *&ref2, float randomNoise, bool allowNegativeCost){
    // 计算cur在路径中的最小插入代价和次小插入代价
    // 返回其最佳/次佳插入点ref1/ref2
    // Args:
    //   * allowNegativeCost: 为true表示插入代价取非负数，为false表示可取负数
    //   * randomNoise: 随机噪声量
    // Returns:
    //   * ref1: 最小插入代价对应位置，若minValue=MAX_FLOAT，则为NULL
    //   * ref2: 次小插入代价对应位置, 若secondVlue=MAX_FLOAT，则为NULL
    ref1 = NULL;
    ref2 = NULL;
    minValue = MAX_FLOAT;
    secondValue = MAX_FLOAT;
    if(quantity + cur->quantity > capacity) {
        // 超出车容量约束，则无需计算
        return;
    }
    for(Spot* ref=head; ref!=rear; ref=ref->next) {
        if(timeWindowJudge(ref, cur) == true) {
            float diff = dist(ref, cur) + dist(cur, ref->next) -
                    dist(ref, ref->next);
            float cost = diff + randomNoise;
            if(allowNegativeCost == false) {
                cost = max(0.0f, cost);
            }
            if(cost <= minValue) {
                // 找到了新的最小者，更新返回值
                secondValue = minValue;
                ref2 = ref1;
                ref1 = ref;
                minValue = cost;
            }
        }
    }
}

//=============== 其余辅助性质的函数 ================//
bool Route::checkTimeConstraint() {
    // 检查当前路径是否满足时间窗约束
    if(size == 0) return true;
    Spot *pre = head;
    Spot *cur = head->next;
    float time = 0;
    float td = 0;
    while(cur != rear) {
        time += dist(pre, cur);
        td += dist(pre, cur);
        cur->arrivedTime = time;  // 更新arrivedTime
        if(time > cur->endTime) return false;
        if(time < cur->startTime) time = cur->startTime;
        time += cur->serviceTime;
        td += cur->serviceTime;
        if(td > timeDuration) return false;
        pre = pre->next;
        cur = cur->next;
    }
    td += dist(rear, rear->front);
    if(td > timeDuration) return false;
    else return true;
}
