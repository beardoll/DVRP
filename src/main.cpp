#include<iostream>
#include<set>
#include<algorithm> 
#include<map>
#include<stdlib.h>
#include<sstream>
#include<string>
#include<fstream>
#include<ctime>
#include<cstdlib>

#include "run/SetBench.h"
#include "modules/Timer.h"
#include "run/TxtRecorder.h"
//#include "baseclass/Car.h"
#include "algorithm/ALNS.h"
#include "public/PublicFunction.h"
#include "xml/BenchWrapper.h"
#include "run/Config.h"
#include "baseclass/Spot.h"

using namespace std;

// *************  综合实验参数  ************** //
string INT_ROOT_PATH = "IntegeratedExperiment/";
string INT_SETS[4] = {"small_set/", "medium_set/", "larger_set/", "largest_set/"};
float LAMBDA_FACTOR[4] = {0.5, 1.0, 1.5, 2.0};

// ************* 时间窗实验参数 ************** //
string TWL_ROOT_PATH = "VariousTWLExperiment/";
string TWL_SETS[4] = {"short_set/", "mid_set/", "longer_set/", "longest_set/"};
float M_ALPHA[4] = {1.5, 2.0, 2.5, 3.0};

// ************* 动态性实验参数 ************** //
string DYN_ROOT_PATH = "DynamicismExperiment/";
string DYN_SETS[4] = {"low_set/", "mid_set/", "higher_set/", "highest_set/"};
int M_BEGIN_INDEX[4] = {1, 2, 3, 4};

// ***********  采样率变化实验参数  ********** //
string SPR_ROOT_PATH = "VariousSPRExperiment/";
string SPR_SETS[4] = {"low_set/", "mid_set/", "higher_set", "highest_set/"};
int M_SAMPLE_RATE[4] = {10, 20, 30, 40};

// *************     公共参数   ************** //
string METHODS[4] = {"replan_sampling_evaluation/", "replan_sampling_random/", "replan_no_sampling/", "no_replan_sampling_evaluation/"};
bool M_REPLAN[4] = {true, true, true, false};
bool M_ASSESSMENT[4] = {true, false, false, true};
bool M_SAMPLING[4] = {true, true, false, true};
bool HAS_DATASET = true;
int EXP_TIMES = 10;

void constructIntDataSet(string expRootPath) {
    // 综合实验的数据集构造
    // expRootPath: 实验数据的根目录
    for(int i=0; i<4; i++) {
        string setName = INT_SETS[i];
        string path = expRootPath + setName + "bench.xml";
        float lambdaFactor = LAMBDA_FACTOR[i];
        for(int j=0; j<6; j++) {
            LAMBDA[j] = LAMBDA[j] * lambdaFactor; 
        }
        srand(unsigned(time(0)));
        // 获取benchmark中的数据
        Spot depot;
        float time = BEGIN_SLOT_INDEX * TIME_SLOT_LEN;
        float capacity = 30;

        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Spot*> staticCustomer, dynamicCustomer;
        vector<Spot*> store;
        SetBench sb;
        sb.construct(staticCustomer, dynamicCustomer, store, depot, time);
        BenchWrapper bw;
        bw.saveBench(path, staticCustomer, dynamicCustomer, store, depot, capacity);
        for(int j=0; j<6; j++) {
            LAMBDA[j] = LAMBDA[j] / lambdaFactor;
        }
    }
}

void constructTWLDataSet(string expRootPath, int setNum) {
    // 时间窗长短实验的数据集构造
    // expRootPath: 实验数据存放的根目录
    srand(unsigned(time(0)));
    for(int i=0; i<setNum; i++) {
        float alpha = M_ALPHA[i];
        string setName = TWL_SETS[i];
        // basePath: 基准数据集
        string basePath = expRootPath + "base.xml";  
        string savePath = expRootPath + setName + "bench.xml";
        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Spot*> staticCustomer, dynamicCustomer;
        vector<Spot*> store;
        Spot depot;
        float capacity;
        BenchWrapper bw;
        bw.loadBench(basePath, staticCustomer, dynamicCustomer, store,
                        depot, capacity);
        SetBench sb;
        sb.changeTWL(staticCustomer, &depot, alpha);
        sb.changeTWL(dynamicCustomer, &depot, alpha);
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, store, depot, capacity);
    }
}

void constructDYNDataSet(string expRootPath, int setNum) {
    // 动态性实验的数据集构造
    // expRootPath: 实验数据存放的根目录
    srand(unsigned(time(0)));
    for(int i=0; i<setNum; i++) {
        string setName = DYN_SETS[i];
        float beginIndex = M_BEGIN_INDEX[i];
        // basePath: 基准数据集
        string basePath = expRootPath + "base.xml";  
        string savePath = expRootPath + setName + "bench.xml";
        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Spot*> staticCustomer, dynamicCustomer;
        vector<Spot*> store;
        Spot depot;
        float capacity;
        BenchWrapper bw;
        bw.loadBench(basePath, staticCustomer, dynamicCustomer, store,
                        depot, capacity);
        SetBench sb;
        vector<Spot*> allCustomer = staticCustomer;
        allCustomer.insert(allCustomer.end(), dynamicCustomer.begin(), dynamicCustomer.end());
        staticCustomer.clear();
        dynamicCustomer.clear();
        sb.changeDYN(allCustomer, &depot, beginIndex, staticCustomer, dynamicCustomer);
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, store, depot, capacity);
    }
}

void experimentEngine(string expRootPath, string *setsName, int setNum, int mode) {
    // 实验引擎，适用于综合实验，时间窗实验以及动态性实验
    // mode: 1表示综合实验，2表示时间窗实验，3表示动态性实验
    int vehicleNum = 0;
    ostringstream ostr;
    srand(unsigned(0));
    for(int i=0; i<setNum; i++) {
        for(int j=0; j<EXP_TIMES; j++) {
            for(int k=0; k<4; ) {
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;;
                cout << "Set index: " << i << " Experiment times: " << j << " Methods index: " << k << endl;
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
                if(k==2) {
                    // replan_no_sampling, 使用和replan_evaluation一样的货车数量
                    CONSTRAIN_CAR_NUM = true;
                    VEHICLE_NUM = vehicleNum;
                } else {
                    CONSTRAIN_CAR_NUM = false;
                }
                if(mode == 1) {
                    FACTOR = LAMBDA_FACTOR[i];
                } else if(mode == 2) {
                    // 选择medium作为base benchmark
                    FACTOR = LAMBDA_FACTOR[1];
                    ALPHA = M_ALPHA[i];
                } else if(mode == 3) {
                    // 选择larger作为base benchmark
                    FACTOR = LAMBDA_FACTOR[2];
                    BEGIN_SLOT_INDEX = M_BEGIN_INDEX[i]; 
                } else if(mode == 4) {
                    // 选择medium作为base benchmark
                    FACTOR = LAMBDA_FACTOR[1];
                    SAMPLE_RATE = M_SAMPLE_RATE[i]; 
                }
                REPLAN = M_REPLAN[k];
                ASSESSMENT = M_ASSESSMENT[k];
                SAMPLING = M_SAMPLING[k];
                string method = METHODS[k];
                string setName = setsName[i];
                string benchPath = expRootPath + setName + "bench.xml";
                cout << benchPath << endl;
                string mediumFileBasis = expRootPath + setName + method; 
                ostr.str("");
                ostr << j;
                string txtName = mediumFileBasis + "txt/" + ostr.str() + ".txt"; 
                string xmlName = mediumFileBasis + "xml/" + ostr.str() + ".xml"; 
                BenchWrapper bw;
                vector<Spot*> staticCustomer, dynamicCustomer, store;
                Spot depot;
                float capacity;
                bw.loadBench(benchPath, staticCustomer, dynamicCustomer, store,
                        depot, capacity);
                TxtRecorder::changeFile(txtName);
                CUSTOMER_NUM = staticCustomer.size() + dynamicCustomer.size();
                Timer timer(staticCustomer, dynamicCustomer, store, capacity, depot);
                vector<Spot*> rejectCustomer;
                vector<Car*> finalCarSet;
                float travelDistance = 0;
                float addAveDistance = 0;
                try{
                    timer.run(finalCarSet, rejectCustomer, travelDistance, addAveDistance);
                } catch(exception &e){
                    cout << e.what() << endl;
                    continue;
                }
                if(k==0) vehicleNum = finalCarSet.size();
                k++;
                TxtRecorder::closeFile();
                bw.saveResult(xmlName, finalCarSet, rejectCustomer, dynamicCustomer, depot, 
                    travelDistance, addAveDistance);
                deleteCustomerSet(staticCustomer);
                deleteCustomerSet(dynamicCustomer);
                withdrawPlan(finalCarSet);
            } 
        } 
    }
    return;
}

int main(int argc, char *argv[]){
    // **************** 时间窗实验 **************** //
    // string TWLExpRootPath = SIMULATION_ROOT_PATH + TWL_ROOT_PATH;
    // if(!HAS_DATASET) constructTWLDataSet(TWLExpRootPath, 4);
    // experimentEngine(TWLExpRootPath, TWL_SETS, 4, 2);
    
    // **************** 动态性实验 **************** //
    // string DYNExpRootPath = SIMULATION_ROOT_PATH + DYN_ROOT_PATH;
    // if(!HAS_DATASET) constructDYNDataSet(DYNExpRootPath, 4);
    // experimentEngine(DYNExpRootPath, DYN_SETS, 4, 3);
    
    // ************** 采样率变化实验 ************** //
    string SPRExpRootPath = SIMULATION_ROOT_PATH + SPR_ROOT_PATH;
    experimentEngine(SPRExpRootPath, DYN_SETS, 4, 4);
    return 0;
}
