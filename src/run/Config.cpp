#include "Config.h"

// 定义全局变量
float DYNAMICISM = 0.3;
Strategy STRATEGY = Negative;
float ITER_PERCENTAGE = 1.0;
int SAMPLE_RATE = 1;
int NUM_OF_CUSTOMER = 100;
int TIME_SLOT_LEN = 40;
int TIME_SLOT_NUM = 6;
int CORE_NUM = 10;
int STORE_NUM = 20;
int SUBCIRCLE_NUM = 6;
int CUSTOMER_NUM = 150;
float R1 = 30;
float R2 = 60;
float R3 = 150;
float lambda[6] = {0.1, 0.2, 0.3, 0.1, 0.1, 0.1}
float ALPHA = 1.5;

// ******** Path for Solomon Benchmark ********* //
string SOLOMON_FILENAME = "./solomon-1987-rc1/RC103_100.xml";

// ******** Path for the bench we construct ******** //
string BENCH_FILE_PATH = "./simulation/";

