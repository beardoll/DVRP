#include "Spot.h"
#include "../run/Config.h"

Spot::Spot() {
    this->id = 0;
    this->type = 'D';
    this->x = 0;
    this->y = 0;
    this->prop = 0;
    this->visit = false;
    this->startTime = 0;
    this->endTime = TIME_SLOT_NUM * TIME_SLOT_LEN;
    this->serviceTime = 0;
    this->arrivedTime = 0;
    this->quantity = 0;
    this->next = NULL;
    this->front = NULL;
    this->priority = 1;
    this->choice = NULL;
    this->tolerantTime = TIME_SLOT_NUM * TIME_SLOT_LEN;
}

Spot::Spot(const Spot &s) {
    this->id = s.id;
    this->type = s.type;
    this->x = s.x;
    this->y = s.y;
    this->startTime = s.startTime;
    this->prop = s.prop;
    this->visit = s.visit;
    this->endTime = s.endTime;
    this->arrivedTime = s.arrivedTime;
    this->serviceTime = s.serviceTime;
    this->quantity = s.quantity;
    this->next = s.next;
    this->front = s.front;
    this->priority = s.priority;
    this->choice = s.choice;
    this->tolerantTime = s.tolerantTime;
}

Spot& Spot::operator= (const Spot &s) {
    // 重载赋值操作
    this->id = s.id;
    this->type = s.type;
    this->x = s.x;
    this->y = s.y;
    this->startTime = s.startTime;
    this->prop = s.prop;
    this->visit = s.visit;
    this->endTime = s.endTime;
    this->arrivedTime = s.arrivedTime;
    this->serviceTime = s.serviceTime;
    this->quantity = s.quantity;
    this->next = s.next;
    this->front = s.front;
    this->priority = s.priority;
    this->choice = s.choice;
    this->tolerantTime = s.tolerantTime;
}

bool Spot::operator< (Spot &s) {
    // "<"比较符
    return this->id < s.id;
}

