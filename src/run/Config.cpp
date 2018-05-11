#include "Config.h"

// 定义全局变量
float DYNAMICISM = 0.3;
Strategy STRATEGY = Negative;
float ITER_PERCENTAGE = 0.5;
bool SAMPLING = true;
bool ASSESSMENT = true;
bool REPLAN = true;
bool SHOW_DETAIL = false;
int SAMPLE_RATE = 10;
int NUM_OF_CUSTOMER = 100;
float REPLAN_END_TIME = 200;
int TIME_SLOT_NUM = 5;
int SPLIT = 1;
float LATEST_SERVICE_TIME = 260;
int CORE_NUM = 20;
bool CONSTRAIN_CAR_NUM = false;
int VEHICLE_NUM = 20;
float ALPHA = 1.2;

// ******** Path for Solomon Benchmark ********* //
string SOLOMON_FILENAME = "./DVRPSimulation/solomon-1987-rc1/RC103_100.xml";

// ******** Path for the Simulation  ******** //
string SIMULATION_ROOT_PATH = "./DVRPSimulation/";

