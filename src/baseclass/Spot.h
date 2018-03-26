#ifndef _SPOT_H
#define _SPOT_H

// 节点类

class Spot{
public:
    Spot();
    ~Spot(){};
    Spot(const Spot &s);
    Spot& operator= (const Spot &s);
    bool operator< (Spot &s);

    // 公有成员变量（请勿在程序中随意更改）
    int id;
    int quantity;
    char type;  // "D"表示外卖提供商，"S"表示商家，"C"表示顾客
    float x;    // x坐标
    float y;    // y坐标
    float startTime;     // 时间窗开始时间
    float endTime;       // 时间窗结束时间
    float arrivedTime;   // 货车到达此节点时间，对于"D"，此值为0
    float serviceTime;   // 服务时间
    float timeDuration;  // 持续时间（可以参与的时间）
    int priority;   // depot为0，而customer为1
    Spot *next;     // 指向下一个节点的指针
    Spot *front;    // 指向上一个节点的指针
};

#endif
