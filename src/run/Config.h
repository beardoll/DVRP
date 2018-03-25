#ifndef CONFIG_H
#define CONFIG_h

#include<string>
#include<vector>

using namespace std;

enum Strategy{Negative, Positive};
enum DynamicLevel{low, mid, high};

extern int CUSTOMER_NUM;
extern int VEHICLE_NUM;
extern float CAPACITY;
extern float TIME_DURATION;

// ******** Noise param for Algorithm ******** //
extern float* RANDOM_RANGE_ALNS;
extern float* RANDOM_RANGE_SSLR;

// ******** Path for Solomon Benchmark ********* //
extern string SOLOMON_PATH;
extern string SOLOMON_FILENAME;

// ******** Path for the bench we construct ******** //
extern string BENCH_FILE_PATH;

#endif
