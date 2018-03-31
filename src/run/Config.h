#ifndef CONFIG_H
#define CONFIG_H

#include<string>
#include<vector>

using namespace std;

enum Strategy{Negative, Positive};
enum DynamicLevel{low, mid, high};

extern int CUSTOMER_NUM;
extern int VEHICLE_NUM;

// 数据路径
extern string SIMULATION_PATH;

#endif
