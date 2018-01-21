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
    int prop;            // 0表示静态顾客，1表示动态顾客
    bool visit;          // 该节点是否被访问过（完成服务）
    Spot *next;     // 指向下一个节点的指针
    Spot *front;    // 指向上一个节点的指针
    Spot *choice;   // 顾客选用的商家（仅"C"有效）
    int priority;   // 优先级，数值大的优先级小
    float tolerantTime; // 最晚得到答复的时间（仅动态顾客）
};

#endif
