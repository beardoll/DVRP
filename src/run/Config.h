#ifndef CONFIG_H
#define CONFIG_h

#include<string>
#include<vector>

using namespace std;

enum Strategy{Negative, Positive};
enum DynamicLevel{low, mid, high};

extern bool DEBUG;
extern bool SAMPLING;  // 是否使用采样算法
extern bool SHOW_DETAIL; // 是否在控制台打印细节信息
extern bool REPLAN;   // 是否对路径进行重新规划
extern float DYNAMICISM;
extern Strategy STRATEGY;
extern float ITER_PERCENTAGE;
extern int SAMPLE_RATE;
extern int CUSTOMER_NUM;
extern int TIME_SLOT_LEN;
extern int TIME_SLOT_NUM;
extern int BEGIN_SLOT_INDEX;      // 仿真开始的时间段
extern float OFF_WORK_TIME;       // 骑手收工时间
extern float LATEST_SERVICE_TIME; // 最晚的仍在服务的时间
extern bool CONSTRAIN_CAR_NUM;    // 是否限制车辆使用
extern int VEHICLE_NUM;           // 允许使用的货车数量
extern bool ASSESSMENT;           // 是否使用评价函数在采样情景解中选择
// R1-R2是store区
// R3-R4是customer区
extern float R1;
extern float R2;
extern float R3;
extern float R4;
extern int CORE_NUM;
extern int STORE_NUM;
extern int SUBCIRCLE_NUM;
extern float LAMBDA[6];
extern float FACTOR;
extern float ALPHA;
extern float PI;
extern int MAX_DEMAND;

// ******** Path for Solomon Benchmark ********* //
extern string SOLOMON_PATH;
extern string SOLOMON_FILENAME;

// ******** Path for the bench we construct ******** //
extern string BENCH_FILE_PATH;

#endif
