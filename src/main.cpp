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

string ALL_SETS[4] = {"low_set/", "mid_set/", "higher_set/", "highest_set/"};
string METHODS[4] = {"replan_sampling_evaluation/", "replan_sampling_random/",
                     "replan_no_sampling/", "no_replan_sampling_evaluation/"};
bool M_REPLAN[4] = {true, true, true, false};
bool M_ASSESSMENT[4] = {true, false, false, true};
bool M_SAMPLING[4] = {true, true, false, true};
float M_DYNAMICISM[4] = {0.1, 0.3, 0.5, 0.7};

bool HAS_DATASET = true;
int EXP_TIMES = 10;

void constructDataSet() {
    for(int i=0; i<4; i++) {
        string setName = ALL_SETS[i];
        string path = BENCH_FILE_PATH + setName + "bench.xml";
        srand(unsigned(time(0)));
        
        // 获取benchmark中的数据
        vector<Customer*> allCustomer;
        Customer depot;
        float capacity;
        string solomonFileName = SOLOMON_FILENAME;
        try {
            getData(solomonFileName, allCustomer, depot, capacity);
        } catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
        } 
        DYNAMICISM = M_DYNAMICISM[i];
        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Customer*> staticCustomer, dynamicCustomer;
        SetBench sb(allCustomer);
        sb.construct(staticCustomer, dynamicCustomer, depot);
        BenchWrapper bw;
        bw.saveBench(path, staticCustomer, dynamicCustomer, depot, capacity);
    }
}

int main(int argc, char *argv[]){
    if(!HAS_DATASET) constructDataSet();
    int vehicleNum = 20;
    ostringstream ostr;
    srand(unsigned(0));
    for(int i=0; i<4; i++) {
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
                REPLAN = M_REPLAN[k];
                SAMPLING = M_SAMPLING[k];
                ASSESSMENT = M_ASSESSMENT[k];
                string method = METHODS[k];
                string setName = ALL_SETS[i];
                string benchPath = BENCH_FILE_PATH + setName + "bench.xml";
                string mediumFileBasis = BENCH_FILE_PATH + setName + method;
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
                bw.saveResult(xmlName, finalCarSet, rejectCustomer, dynamicCustomer, depot, travelDistance, addAveDistance);
                withdrawPlan(finalCarSet);
                deleteCustomerSet(dynamicCustomer);
                deleteCustomerSet(staticCustomer);
            }
        }
    }
    return 0;
}
