#ifndef CONFIG_H
#define CONFIG_h

#include<string>

using namespace std;

enum Strategy{Negative, Positve};
enum DynamicLevel{low, mid, high};

extern DynamicLevel DYNAMIC_LEVEL;
extern Strategy STRATEGY;
extern float ITER_PERCENTANGE;
extern int SAMPLE_RATE;
extern int NUM_OF_CUSTOMER;
extern int TIME_SLOT_LEN;
extern int TIME_SLOT_NUM;
extern int CORE_NUM;

// ******** Path for Solomon Benchmark ********* //
extern string SOLOMON_PATH;
extern string SOLOMON_FILENAME;

// ******** Path for the bench we construct ******** //
extern string BENCH_FILE_PATH;

#endif
