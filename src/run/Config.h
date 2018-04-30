#ifndef CONFIG_H
#define CONFIG_h

#include<string>

using namespace std;

enum Strategy{Negative, Positive};
enum DynamicLevel{low, mid, high};

extern bool SHOW_DETAIL;
extern float DYNAMICISM;
extern bool SAMPLING;
extern bool ASSESSMENT;
extern bool REPLAN;
extern Strategy STRATEGY;
extern float ITER_PERCENTAGE;
extern int SAMPLE_RATE;
extern int NUM_OF_CUSTOMER;
extern int TIME_SLOT_LEN;
extern int TIME_SLOT_NUM;
extern int CORE_NUM;
extern bool CONSTRAIN_CAR_NUM;
extern int VEHICLE_NUM;
extern float ALPHA;

// ******** Path for Solomon Benchmark ********* //
extern string SOLOMON_PATH;
extern string SOLOMON_FILENAME;

// ******** Path for the bench we construct ******** //
extern string BENCH_FILE_PATH;

#endif
