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
float M_DYNAMICISM[4] = {0.2, 0.4, 0.6, 0.8};

// **************** 重规划频率实验参数 ************** //
string RPF_ROOT_PATH = "VariousRPFExperiment/";
string RPF_SETS[4] = {"low_set/", "mid_set/", "higher_set/", "highest_set/"};
int M_SPLIT[4] = {1, 2, 3, 4};

// ******************** 公共参数 ******************** //
string METHODS[6] = {"replan_sampling_evaluation_pos/", "replan_sampling_evaluation_neg/",
                     "replan_sampling_random_pos/", "replan_sampling_random_neg/",
                     "no_replan_sampling_evaluation_pos/", "no_replan_sampling_evaluation_neg/"};
bool M_REPLAN[6] = {true, true, true, true, false, false};
bool M_ASSESSMENT[6] = {true, true, false, false, true, true};
// 仅针对DYN数据集构造时，因为其他数据集的构造都是以DYN数据集为base set
Strategy M_STRATEGY[6] = {Positive, Negative, Positive, Negative, Positive, Negative};
bool HAS_DATASET = true;
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

void experimentEngine(string expRootPath, string *setsName, int setNum, int mode) {
    // 实验引擎
    // mode: 1表示动态实验，2重规划频率实验
    ostringstream ostr;
    srand(unsigned(0));
    for(int i=0; i<setNum; i++) {
        for(int j=0; j<EXP_TIMES; j++) {
            for(int k=0; k<6; ) {
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
                cout << "Set index: " << i << " Experiment times: " << j <<
                    " Methods index: " << k << endl;
                cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
                if(mode == 2) {
                    SPLIT = M_SPLIT[i];
                }
                REPLAN = M_REPLAN[k];
                ASSESSMENT = M_ASSESSMENT[k];
                STRATEGY = M_STRATEGY[k];
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
    if(!HAS_DATASET) {
        constructBaseSet(DYNExpRootPath + "base.xml");
        constructDYNDataSet(DYNExpRootPath, 4);
    }
    //experimentEngine(DYNExpRootPath, DYN_SETS, 4, 1);

    // *************** 重规划频率实验 ************* //
    string RPFExpRootPath = SIMULATION_ROOT_PATH + RPF_ROOT_PATH;
    experimentEngine(RPFExpRootPath, RPF_SETS, 4, 2);
    
    return 0;
}
