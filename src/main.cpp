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

#include "LoadFile.h"
#include "run/SetBench.h"
#include "baseclass/Timer.h"
#include "run/TxtRecorder.h"
#include "baseclass/Car.h"
#include "algorithm/ALNS.h"
#include "public/PublicFunction.h"
#include "xml/LoadSolomon.h"
#include "xml/BenchWrapper.h"

using namespace std;



// ********** Path for sample that we construct ******* //
static const string BENCH_FILE_PATH = "./Simulation_2_version/";    // the root directory for global bench

static const string FILE_NAME2 = FILE_NAME;
static const string BENCH_ROUTE = BENCH_FILE_PATH + "bench.xml";              // the routine for original sample


int main(int argc, char *argv[]){
    cout << "================ Preparation =============" << endl;
    cout << "Please choose the mode (0 for set bench, 1 for run):";
    int condition;
    cin >> condition;
    if(condition == 0) {
        // 建立仿真的Benchmark
        srand(unsigned(time(0)));
        
        // 获取benchmark中的数据
        vector<Customer*> allCustomer;
        Customer depot;
        float capacity;
        bool mark = getData(filename, allCustomer, depot, capacity);

        // 建立新的benchmark（修改服务时间以及时间窗 + 分static和dynamic）
        vector<Customer*> staticCustomer, dynamicCustomer;
        SetBench sb(allCustomer);
        sb.construct(staticCustomer, dynamicCustomer);
        string savePath = ;
        BenchWrapper bw();
        bw.saveBench(savePath, staticCustomer, dynamicCustomer, depot, capacity);

        cout << "OK, new version of bench has been established!" << endl;
    }
    else if(condition == 1) {
        BenchWrapper bw();
        // run repeated simulation for one sample, and get the average performance 
        int experimentNum = 30;
        srand(unsigned(time(0)));
        
        // the directory for saving txt and xml files, respectively
        //string txtName = SIMULATION_PATH + "txt/";
        //string xmlName = SIMULATION_PATH + "xml/";
        string loadFileName = ;

        vector<Customer*> staticCustomer, dynamicCustomer;
        Customer depot;
        float capacity;
        bw.loadBench(loadFileName, staticCustomer, dynamicCustomer, depot, capacity);

        // txt文件地址
        string pp = recordDir + sampleRate_string + "_" + experimentCountString + ".txt";
        TxtRecorder::changeFile(pp);

        Timer timer(staticCustomer, dynamicCustomer, capacity, depot);
        vector<Customer*> rejectCustomer;
        vector<Car*> finalCarSet;
        float travelDistance = 0;
        float addAveDistance = 0;
        timer.run(finalCarSet, rejectCustomer, travelDistance, addAveDistance);
        TxtRecorder::closeFile();
        
        // xml文件地址
        string name = dynamicResultDir+ sampleRate_string + "_" + experimentCountString + ".xml";
        bw.saveResult(name, finalCarSet, rejectCustomer, dynamicCustomer, depot, travelDistance, 
                addAveDistance);

        // using ALNS to serve customers in finalCarSet to get best results
        vector<Car*> bestCarSet;
        float bestCost = 0;
        computeBest(finalCarSet, bestCarSet, bestCost);
        vector<Customer*> temp1, temp2;
        // ALNS结果的存放地
        string name2 = staticResultDir + sampleRate_string + "_" + experimentCountString + ".xml";
        bw.saveResult(name2, bestCarSet, temp1, temp2, name2, depot, bestCost, 0);
                        
        withdrawPlan(finalCarSet);
        withdrawPlan(bestCarSet);
    }
    return 0;
}
