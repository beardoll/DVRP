#include "Config.h"

// 定义全局变量
bool DEBUG = false;
bool SAMPLING = true;
bool SHOW_DETAIL = false;
bool REPLAN = true;
float DYNAMICISM = 0.3;
Strategy STRATEGY = Negative;
float ITER_PERCENTAGE = 1.0;
int SAMPLE_RATE = 10;
int CUSTOMER_NUM = 200;
int TIME_SLOT_LEN = 40;
int TIME_SLOT_NUM = 5;
int BEGIN_SLOT_INDEX = 1;
int CORE_NUM = 10;
float LATEST_SERVICE_TIME = 250;
bool CONSTRAIN_CAR_NUM = true;
int VEHICLE_NUM = 26;
bool ASSESSMENT = true;
float OFF_WORK_TIME = 300;
int STORE_NUM = 20;
int SUBCIRCLE_NUM = 6;
float R1 = 10;
float R2 = 30;
float R3 = 40;
float R4 = 70;
float LAMBDA[6] = {0.1, 0.15, 0.25, 0.1, 0.1, 0.1};
float FACTOR = 1.0;
float ALPHA = 1.5;
float PI = 3.1415926;
int MAX_DEMAND = 5;


// ******** Path for the bench we construct ******** //
string SIMULATION_ROOT_PATH = "./O2OSimulation/";

