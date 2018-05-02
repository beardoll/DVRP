#include<iostream>
#include<set>
#include<algorithm> 
#include<map>
#include<stdlib.h>
#include<sstream>
#include<string>
#include<ctime>
#include<cstdlib>
#include "algorithm/SSLR.h"
#include "public/PublicFunction.h"
#include "run/Config.h"
#include "baseclass/Spot.h"
#include "dataset/TxtParser.h"
#include "run/TxtRecorder.h"
#include<fstream>

using namespace std;

void experimentEngine(int index) {
    // index可从1-20中选择
    ostringstream ostr;
    if(index < 0 || index > 20) {
        cout << "Benchmark with index " << index << " not exist!" << endl;
        exit(1);
    } else if (index < 10) {
        ostr << 0 << index;
    } else {
        ostr << index;
    }
    string data_file = SIMULATION_PATH + "dataset/pr" + ostr.str(); 
    string txt_file = SIMULATION_PATH + "result/pr" + ostr.str() + ".txt";
    TxtRecorder::changeFile(txt_file);

    TxtParser tp(data_file);
    vector<Spot*> depots, customers;
    tp.getDataset(depots, customers, VEHICLE_NUM);
    SSLR alg(customers, depots, MAX_ITER, true);
    vector<Car*> result;
    float cost;
    alg.run(result, cost);
    // 每辆车搭载的货物量
    vector<float> demands = getDemands(result);
    // 每辆车的time duration
    vector<float> tds = getTimeDurations(result);
    for(int i=0; i<tds.size(); i++) {
        ostr.str("");
        ostr << result[i]->getCarIndex() << " " << result[i]->getDepotIndex() << " "
             << demands[i] << " " << tds[i] << " " << result[i]->getRoute()->getSize() << endl;
        TxtRecorder::addLine(ostr.str());
    }
    ostr.str("");
    ostr << cost << endl;
    TxtRecorder::addLine(ostr.str());
    // showDetailForPlan(result);
    TxtRecorder::closeFile();
}

int main(int argc, char *argv[]){
    experimentEngine(1);
    return 0;
}
