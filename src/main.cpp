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
#include "baseclass/Car.h"
#include "algorithm/ALNS.h"
#include "public/PublicFunction.h"
#include "xml/LoadSolomon.h"
#include "xml/BenchWrapper.h"
#include "run/Config.h"

using namespace std;

// ***************** 动态性实验参数 ***************** //
string DYN_ROOT_PATH = "DynamicismExperiment/";
string DYN_SETS[4] = {"low_set/", "mid_set/", "higher_set/", "highest_set/"};
float M_DYNAMICISM[4] = {0.1, 0.3, 0.5, 0.7};
bool HAS_DYNSET = true;

// ***************** 时间窗实验参数 ***************** //
string TWL_ROOT_PATH = "VariousTWLExperiment/";
string TWL_SETS[5] = {"short_set/", "mid_set/", "long_set/", "longer_set/", 
                      "longest_set/"};
float M_ALPHA[5] = {1.0, 1.5, 2.0, 2.5, 3.0};
bool HAS_TWLSET = false;

// *************** 采样率变化实验参数 *************** //
string SPR_ROOT_PATH = "VariousSPRExperiment/";
string SPR_SETS[4] = {"low_set/", "mid_set/", "higher_set/", "highest_set/"};
int M_SAMPLE_RATE[4] = {10, 20, 30, 40};
bool HAS_SPRSET = false;

// *************** 预测策略变化实验参数 ************* //
string STG_ROOT_PATH = "VariousSTGExperiment/";
string STG_SETS[2] = {"pos_set/", "neg_set/"};
Strategy M_STRATEGY[2] = {Positive, Negative};
bool HAS_STGSET = false;

// ******************** 公共参数 ******************** //
string METHODS[4] = {"replan_sampling_evaluation/", "replan_sampling_random/",
                     "replan_no_sampling/", "no_replan_sampling_evaluation/"};
bool M_REPLAN[4] = {true, true, true, false};
bool M_ASSESSMENT[4] = {true, false, false, true};
bool M_SAMPLING[4] = {true, true, false, true};
// 仅针对DYN数据集构造时，因为其他数据集的构造都是以DYN数据集为base set
bool HAS_BASESET = true;
int EXP_TIMES = 10;

void constructBaseSet(string path) {
    // 构造基准数据集（更改时间窗、服务时间等属性）
    // 获取SOLOMON_FILENAME中的原始数据
    vector<Customer*> allCustomer;
    Customer depot;
    float capacity;
    try {
        getData(SOLOMON_FILENAME, allCustomer, depot, capacity);
    } catch (exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
    vector<Customer*> staticCustomer, dynamicCustomer;
    SetBench sb;
    sb.construct(allCustomer, staticCustomer, dynamicCustomer, depot);
    BenchWrapper bw;
    bw.saveBench(path, staticCustomer, dynamicCustomer, depot, capacity);
}

void constructDYNDataSet(string expRootPath, int setNum) {
    // 动态性实验数据构造
    // expRootPath: 实验数据的根目录
    srand(unsigned(time(0)));
    for(int i=0; i<setNum; i++) {
        string setName = DYN_SETS[i];
        // basePath: 基准数据集
        string basePath = expRootPath + "base.xml";
        string savePath = expRootPath + setName + "bench.xml";
        // 获取base set中的数据
        BenchWrapper bw;
        vector<Customer*> staticCustomer, dynamicCustomer;
        Customer depot;
        float capacity;
        try {
            bw.loadBench(basePath, staticCustomer, dynamicCustomer, depot, capacity);
        } catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
        } 
        vector<Customer*> allCustomer = staticCustomer;
        allCustomer.insert(allCustomer.end(), dynamicCustomer.begin(), dynamicCustomer.end());
        staticCustomer.clear();
        dynamicCustomer.clear();
        DYNAMICISM = M_DYNAMICISM[i];
        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        SetBench sb;
        sb.changeDYN(allCustomer, depot, DYNAMICISM, staticCustomer, dynamicCustomer);
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, depot, capacity);
    }
}

void constructTWLDataSet(string expRootPath, int setNum) {
    // 时间窗变化数据构造
    // expRootPath: 实验数据的根目录
    srand(unsigned(time(0)));
    for(int i=0; i<setNum; i++) {
        float alpha = M_ALPHA[i];
        string setName = TWL_SETS[i];
        // basePath: 基准数据集
        string basePath = expRootPath + "base.xml";
        string savePath = expRootPath + setName + "bench.xml";
        vector<Customer*> staticCustomer, dynamicCustomer;
        Customer depot;
        float capacity;
        BenchWrapper bw;
        try {
            bw.loadBench(basePath, staticCustomer, dynamicCustomer, depot, capacity);
        } catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
        }
        SetBench sb;
        sb.changeTWL(staticCustomer, depot, alpha);
        sb.changeTWL(dynamicCustomer, depot, alpha);
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, depot, capacity);
    }
}

void experimentEngine(string expRootPath, string *setsName, int setNum, int mode) {
    // 实验引擎
    // mode: 1表示动态实验，2表示时间窗实验，3表示采样率实验，4表示采样策略实验
    int vehicleNum = 0;
    ostringstream ostr;
    srand(unsigned(0));
    for(int i=0; i<setNum; i++) {
        for(int j=0; j<EXP_TIMES; j++) {
            for(int k=0; k<4; ) {
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
                cout << "Set index: " << i << " Experiment times: " << j <<
                    " Methods index: " << k << endl;
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
                if(k==2) {
                    // replan_no_sampling，使用和replan_evaluation一样的货车数量
                    CONSTRAIN_CAR_NUM = true;
                    VEHICLE_NUM = vehicleNum;
                } else {
                    CONSTRAIN_CAR_NUM = false;
                }
                if(mode == 2) {
                    ALPHA = M_ALPHA[i];
                } else if(mode == 3) {
                    SAMPLE_RATE = M_SAMPLE_RATE[i];
                } else if(mode == 4) {
                    STRATEGY = M_STRATEGY[i];
                }
                REPLAN = M_REPLAN[k];
                SAMPLING = M_SAMPLING[k];
                ASSESSMENT = M_ASSESSMENT[k];
                string method = METHODS[k];
                string setName = setsName[i];
                string benchPath = expRootPath + setName + "bench.xml";
                string mediumFileBasis = expRootPath + setName + method;
                ostr.str("");
                ostr << j;
                string txtName = mediumFileBasis + "txt/" + ostr.str() + ".txt";
                string xmlName = mediumFileBasis + "xml/" + ostr.str() + ".xml";
                TxtRecorder::changeFile(txtName);
                Customer depot;
                float capacity;
                BenchWrapper bw;
                vector<Customer*> staticCustomer, dynamicCustomer;
                try {
                    bw.loadBench(benchPath, staticCustomer, dynamicCustomer, depot, capacity);
                } catch (exception &e) {
                    cerr << e.what() << endl;
                    exit(1);
                }
                Timer timer(staticCustomer, dynamicCustomer, capacity, depot);
                vector<Customer*> rejectCustomer;
                vector<Car*> finalCarSet;
                float travelDistance = 0;
                float addAveDistance = 0;
                try {
                    timer.run(finalCarSet, rejectCustomer, travelDistance, addAveDistance);
                } catch (exception &e) {
                    cout << e.what() << endl;
                    continue;
                }
                TxtRecorder::closeFile();
                if(k==0) vehicleNum = finalCarSet.size();
                k++;
                bw.saveResult(xmlName, finalCarSet, rejectCustomer, dynamicCustomer, depot, travelDistance, addAveDistance);
                withdrawPlan(finalCarSet);
                deleteCustomerSet(dynamicCustomer);
                deleteCustomerSet(staticCustomer);
            }
        }
    }
}

int main(int argc, char *argv[]){
    // **************** 动态性实验 **************** //
    string DYNExpRootPath = SIMULATION_ROOT_PATH + DYN_ROOT_PATH;
    if(!HAS_BASESET) constructBaseSet(DYNExpRootPath + "base.xml");
    if(!HAS_DYNSET) constructDYNDataSet(DYNExpRootPath, 4);
    experimentEngine(DYNExpRootPath, DYN_SETS, 4, 1);

    // **************** 时间窗实验 **************** //
    // string TWLExpRootPath = SIMULATION_ROOT_PATH + TWL_ROOT_PATH;
    // if(!HAS_TWLSET) constructTWLDataSet(TWLExpRootPath);
    // experimentEngine(TWLExpRootPath, TWL_SETS, 5, 2);

    // **************** 采样率实验 **************** //
    // string SPRExpRootPath = SIMULATION_ROOT_PATH + SPR_ROOT_PATH;
    // if(!HAS_SPRSET) {
    //     cout << "Need base dataset in SPR Experiment!!" << endl;
    //     return 0;
    // }
    // experimentEngine(SPRExpRootPath, SPR_SETS, 4, 3);

    // ****************  策略实验  **************** //
    // string STGExpRootPath = SIMULATION_ROOT_PATH + STG_ROOT_PATH;
    // if(!HAS_STGSET) {
    //     cout << "Need base dataset in STG Experiment!!" << endl;
    //     return 0;
    // }
    // experimentEngine(STGExpRootPath, STG_SETS, 2, 4);
    
    return 0;
}
