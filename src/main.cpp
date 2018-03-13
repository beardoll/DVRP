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

int main(int argc, char *argv[]){
    cout << "================ Preparation =============" << endl;
    cout << "Please choose the mode (0 for set bench, 1 for ALNS, 2 for whole simulation):";
    int condition;
    cin >> condition;
    if(condition == 0) {
        // 建立仿真的Benchmark
        srand(unsigned(time(0)));
        
        // 获取benchmark中的数据
        vector<Spot*> allCustomer;
        Spot depot;
        float capacity = 40.0f;

        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Spot*> staticCustomer, dynamicCustomer;
        vector<Spot*> store;

        SetBench sb;
        sb.construct(staticCustomer, dynamicCustomer, store, depot, capacity);
        string savePath = BENCH_FILE_PATH + "bench_exp.xml";
        BenchWrapper bw;
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, store, depot, capacity);
        cout << "OK, new version of bench has been established!" << endl;
    } else if(condition == 1) {
        // 调试ALNS
        string savePath = BENCH_FILE_PATH + "bench_exp.xml";
        vector<Spot*> staticCustomer, dynamicCustomer, store;
        Spot depot;
        float capacity;
        BenchWrapper bw;
        bw.loadBench(savePath, staticCustomer, dynamicCustomer, store, depot, capacity);
        vector<Spot*> allCustomer = mergeCustomer(staticCustomer, dynamicCustomer);
        ALNS alg(allCustomer, depot, capacity, 15000, true);
        vector<Car*> finalCarSet;
        float finalCost;
        alg.run(finalCarSet, finalCost);
        vector<Car*>::iterator carIter;
        for(carIter = finalCarSet.begin(); carIter < finalCarSet.end(); carIter++) {
            if((*carIter)->checkTimeConstraint() == false) {
                cerr << "Error Result in car: #" << (*carIter)->getCarIndex() << endl;
                exit(1);
            }
        }
    }
    else if(condition == 2) {
        BenchWrapper bw;
        srand(unsigned(time(0)));
        
        // the directory for saving txt and xml files, respectively
        //string txtName = SIMULATION_PATH + "txt/";
        //string xmlName = SIMULATION_PATH + "xml/";
        string loadFileName = BENCH_FILE_PATH + "bench_exp.xml";

        vector<Spot*> staticCustomer, dynamicCustomer, store;
        Spot depot;
        float capacity;
        try {
            bw.loadBench(loadFileName, staticCustomer, dynamicCustomer, store, 
                    depot, capacity);
        } catch (exception &e) {
            cerr << e.what() << endl;
            exit(1);
        }
        // txt文件地址
        string txtFileName = BENCH_FILE_PATH + "result.txt";
        TxtRecorder::changeFile(txtFileName);

        cout << "There are " << staticCustomer.size() << " static customers and " << 
            dynamicCustomer.size() << " dynamic customers" << endl;

        Timer timer(staticCustomer, dynamicCustomer, store, capacity, depot);
        vector<Spot*> rejectCustomer;
        vector<Car*> finalCarSet;
        float travelDistance = 0;
        float addAveDistance = 0;
        timer.run(finalCarSet, rejectCustomer, travelDistance, addAveDistance);
        TxtRecorder::closeFile();
        
        // xml文件地址
        string name = BENCH_FILE_PATH + "dynamicResult.xml";
        bw.saveResult(name, finalCarSet, rejectCustomer, dynamicCustomer, depot, travelDistance, 
                addAveDistance);

        // using ALNS to serve customers in finalCarSet to get best results
        vector<Car*> bestCarSet;
        float bestCost = 0;
        computeBest(finalCarSet, bestCarSet, bestCost);
        vector<Spot*> temp1, temp2;
        // ALNS结果的存放地
        string name2 = BENCH_FILE_PATH + "staticResult.xml";
        bw.saveResult(name2, bestCarSet, temp1, temp2, depot, bestCost, 0);
        withdrawPlan(finalCarSet);
        withdrawPlan(bestCarSet);
    }
    return 0;
}
