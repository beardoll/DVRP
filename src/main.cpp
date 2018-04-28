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
//#include "xml/LoadSolomon.h"
#include "xml/BenchWrapper.h"
#include "run/Config.h"
#include "baseclass/Spot.h"

using namespace std;

string ALL_SETS[4] = {"small_set/", "medium_set/", "larger_set/", "largest_set/"};
string METHODS[4] = {"replan_sampling_evaluation/", "replan_sampling_random/", "replan_no_sampling/", "no_replan_sampling_evaluation/"};
bool M_REPLAN[4] = {true, true, true, false};
bool M_ASSESSMENT[4] = {true, false, false, true};
bool M_SAMPLING[4] = {true, true, false, true};
float LAMBDA_FACTOR[4] = {0.5, 1.0, 1.5, 2.0};
bool HAS_DATASET = true;
int EXP_TIMES = 10;

void constructDataSet() {
    for(int i=0; i<4; i++) {
        string setName = ALL_SETS[i];
        string path = BENCH_FILE_PATH + setName + "bench.xml";
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

int main(int argc, char *argv[]){
    if(!HAS_DATASET) constructDataSet();
    int vehicleNum = 0;
    ostringstream ostr;
    srand(unsigned(0));
    for(int i=0; i<4; i++) {
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
                FACTOR = LAMBDA_FACTOR[i];
                REPLAN = M_REPLAN[k];
                ASSESSMENT = M_ASSESSMENT[k];
                SAMPLING = M_SAMPLING[k];
                string method = METHODS[k];
                string setName = ALL_SETS[i];
                string benchPath = BENCH_FILE_PATH + setName + "bench.xml";
                string mediumFileBasis = BENCH_FILE_PATH + setName + method; 
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
                cout << "Travel len is: " << travelDistance << endl;
                cout << "Ave len is: " << addAveDistance << endl;
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
    return 0;
}
