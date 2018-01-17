#ifndef CONFIG_H
#define CONFIG_h

#include<string>
#include<vector>

using namespace std;

enum Strategy{Negative, Positive};
enum DynamicLevel{low, mid, high};

extern float DYNAMICISM;
extern Strategy STRATEGY;
extern float ITER_PERCENTAGE;
extern int SAMPLE_RATE;
extern int NUM_OF_CUSTOMER;
extern int TIME_SLOT_LEN;
extern int TIME_SLOT_NUM;
extern float R1;
extern float R2;
extern float R3;
extern int CORE_NUM;
extern int STORE_NUM;
extern int SUBCIRCLE_NUM;
extern vector<float> LAMBDA;
extern float ALPHA;

// ******** Noise param for Algorithm ******** //
extern float* RANDOM_RANGE_ALNS;
extern float* RANDOM_RANGE_SSLR;

// ******** Path for Solomon Benchmark ********* //
extern string SOLOMON_PATH;
extern string SOLOMON_FILENAME;

// ******** Path for the bench we construct ******** //
extern string BENCH_FILE_PATH;

#endif
