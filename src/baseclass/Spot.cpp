#include "Spot.h"
#include "../run/Config.h"

Spot::Spot() {
    this->id = 0;
    this->type = 'D';
    this->x = 0;
    this->y = 0;
    this->startTime = 0;
    this->endTime = 0;
    this->serviceTime = 0;
    this->arrivedTime = 0; 
    this->timeDuration = 0;
    this->quantity = 0;
    this->priority = 0;
    this->next = NULL;
    this->front = NULL;
}

Spot::Spot(const Spot &s) {
    this->id = s.id;
    this->type = s.type;
    this->x = s.x;
    this->y = s.y;
    this->startTime = s.startTime;
    this->endTime = s.endTime;
    this->arrivedTime = s.arrivedTime;
    this->serviceTime = s.serviceTime;
    this->timeDuration = s.timeDuration;
    this->quantity = s.quantity;
    this->priority = s.priority;
    this->next = s.next;
    this->front = s.front;
}

Spot& Spot::operator= (const Spot &s) {
    // 重载赋值操作
    this->id = s.id;
    this->type = s.type;
    this->x = s.x;
    this->y = s.y;
    this->startTime = s.startTime;
    this->endTime = s.endTime;
    this->arrivedTime = s.arrivedTime;
    this->serviceTime = s.serviceTime;
    this->timeDuration = s.timeDuration;
    this->quantity = s.quantity;
    this->priority = s.priority;
    this->next = s.next;
    this->front = s.front;
	return *this;
}

bool Spot::operator< (Spot &s) {
    // "<"比较符
    return this->id < s.id;
}

