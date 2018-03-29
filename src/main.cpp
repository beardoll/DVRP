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
#include<fstream>

using namespace std;

int main(int argc, char *argv[]){
    cout << "================ Simulation Start =============" << endl;
    cout << "Please choose benchmark file index: (1-20):";
    int index;
    cin >> index;
    ostringstream ostr;
    if(index < 0 || index > 20) {
        cout << "Benchmark with index " << index << " not exist!" << endl;
    } else if (index < 10) {
        ostr << 0 << index;
    } else {
        ostr << index;
    }
    string data_file = "C:/Users/beardoll/Documents/DVRP/src/pr02"; 
	//static ofstream outfile;
	//outfile.open(data_file.c_str(), ofstream::trunc);
	//outfile << "test" << endl;
	//outfile.close();

    TxtParser tp(data_file);
    vector<Spot*> depots, customers;
    tp.getDataset(depots, customers, VEHICLE_NUM);
    SSLR alg(customers, depots, 15000, true);
    vector<Car*> result;
    float cost;
    alg.run(result, cost);
    cout << "The final cost is " << cost << endl;
    return 0;
}
