#include<iostream>
#include<set>
#include<algorithm> 
#include<map>
#include "LoadFile.h"
#include "SetBench.h"
#include "Timer.h"
#include "TxtRecorder.h"
#include "Car.h"
#include "ALNS.h"
#include "OutputFile.h"
#include "PublicFunction.h"
#include "LoadBench.h"
#include "SaveBench.h"
#include <stdlib.h>
#include<sstream>
#include<string>
#include<fstream>
#include<ctime>
#include<cstdlib>

using namespace std;
// ********** Simulation Version ********** //
string predictMethod[2] = {"Negative", "Positive"};
float iter_percentage[3] = {0.3, 0.6, 1.0};
string dynamicLevel[3] = {"low", "mid", "high"};
float dynamicism[3] = {0.1, 0.3, 0.5};
int sampleRate[4] = {5, 10, 20, 30};


// ********** Path for benchmark ********** //
static const string FILE_PATH = "./solomon-1987-rc1/";
static const string FILE_NAME = "RC103_100.xml";
//static const string FILE_PATH2 = "./Simulation_Result/10_sample/xml/";

// ********** Path for sample that we construct ******* //
static const string BENCH_FILE_PATH = "./Simulation_2_version/";    // the root directory for global bench

// ********** Path for repeated experiment ************ //
//static const string SIMULATION_PATH = "./Simulation_Result_low_dynamicism/";
//static const string SIMULATION_PATH = "./Fault_check/";

static const string FILE_NAME2 = FILE_NAME;
static const string BENCH_ROUTE = BENCH_FILE_PATH + "bench.xml";              // the routine for original sample

// ********** parametre settings ********** //
static const int NUM_OF_CUSTOMER = 100;
static const int timeSlotLen = 40;
static const int timeSlotNum = 6;


int main(int argc, char *argv[]){
    cout << "================ Preparation =============" << endl;
    cout << "Please choose the mode (0 for set bench, 1 for run):";
    int condition;
    cin >> condition;
    if(condition == 0) {
        cout << "You choose to set bench, then the old version will be covered, yes or no? (Y/N):";
        char verification;
        cin >> verification;
        if(verification == 'Y') {
                    // construct global bench
            srand(unsigned(time(0)));
            LoadFile lf(FILE_PATH, FILE_NAME, NUM_OF_CUSTOMER);
            vector<Customer*> allCustomer;
            Customer depot;
            float capacity;
            bool mark = lf.getData(allCustomer, depot, capacity);

            // first, save the simulation sample
            vector<Customer*> staticCustomer, dynamicCustomer;
            SetBench sb(allCustomer, timeSlotLen, timeSlotNum, 0);
            sb.construct(staticCustomer, dynamicCustomer);
            SaveBench s1(staticCustomer, dynamicCustomer, depot, capacity, timeSlotNum);
            s1.save(BENCH_ROUTE); 

            // from here we can also construct the sample for local simulation
            LoadBench L2(BENCH_ROUTE);
            vector<Customer*> tempSet1;
            vector<Customer*> tempSet2;
            L2.load(tempSet1, tempSet2, depot, capacity);
            for(int i=0; i<3; i++) {
                float currentDynamicism = dynamicism[i];
                string save_path = BENCH_FILE_PATH + dynamicLevel[i] + "_dynamicism/bench.xml";
                vector<Customer*> staticSet;
                vector<Customer*> dynamicSet;
                seperateCustomer(tempSet1, staticSet, dynamicSet, currentDynamicism);
                SaveBench s2(staticSet, dynamicSet, depot, capacity, timeSlotNum);
                s2.save(save_path);
                deleteCustomerSet(staticSet);
                deleteCustomerSet(dynamicSet);
            }
            TxtRecorder::closeFile();
            cout << "OK, new version of bench has been established!" << endl;
        } 
        else if(verification == 'N') {
            cout << "Cancel the set of bench!!" << endl;
        }
    }
    else if(condition == 1) {
        cout << "Please input the dynamic count (0 for low, 1 for mid, 2 for high):";
        int dynamicCount;
        cin >> dynamicCount;
        cout << "Please input the method count (0 for negative, 1 for positive):";
        int methodCount;
        cin >> methodCount;
        cout << "Verify your settings? (Y/N):";
        char verification;
        cin >> verification;
        if(verification == 'Y') {
            // run repeated simulation for one sample, and get the average performance 
            int experimentNum = 30;

            // the directory for saving txt and xml files, respectively
            //string txtName = SIMULATION_PATH + "txt/";
            //string xmlName = SIMULATION_PATH + "xml/";

            string currentDynamicLevel = dynamicLevel[dynamicCount];
            string current_simulation_sample = BENCH_FILE_PATH + currentDynamicLevel + "_dynamicism/bench.xml";
            cout << current_simulation_sample << endl;
            LoadBench L1(current_simulation_sample);
            vector<Customer*> staticCustomer, dynamicCustomer;
            Customer depot;
            float capacity;
            L1.load(staticCustomer, dynamicCustomer, depot, capacity);
            for(int sampleCount = 2; sampleCount < 3; sampleCount++) {
                for(int percentageCount = 2; percentageCount < 3; percentageCount++) {
                    for(int experimentCount = 22; experimentCount <= experimentNum; experimentCount++) {
                        string currentMethod = predictMethod[methodCount]; 
                        int currentSampleRate = sampleRate[sampleCount];
                        float currentPercentage = iter_percentage[percentageCount];

                        string dirName = "";
                        stringstream sss;
                        sss.str("");
                        sss << currentSampleRate;
                        string sampleRate_string = sss.str();
                        sss.str("");
                        sss << (int)(currentPercentage*100);
                        string percentage_string = sss.str();    

                        string baseDir = BENCH_FILE_PATH + currentDynamicLevel + "_dynamicism/" + currentMethod + "_random/" + sampleRate_string +
                            "_sample/" + percentage_string + "_percentage/"; 
                        string recordDir = baseDir + "record/";
                        string dynamicResultDir = baseDir + "dynamicResult/";
                        string staticResultDir = baseDir + "staticResult/";

                        srand(unsigned(time(0)));
                        TxtRecorder::closeFile();
                        sss.str("");
                        sss << experimentCount;
                        string experimentCountString = sss.str();

                        string pp = recordDir + sampleRate_string + "_" + experimentCountString + ".txt";
                        cout << "***************** new experiment ****************" << endl;
                        cout << "---- txt file name: " << pp << endl;
                        TxtRecorder::changeFile(pp);
                        Timer timer(staticCustomer, dynamicCustomer, timeSlotLen, timeSlotNum, capacity, depot, currentSampleRate, currentPercentage, methodCount);
                        vector<Customer*> rejectCustomer;
                        vector<Car*> finalCarSet;
                        float travelDistance = 0;
                        float addAveDistance = 0;
                        timer.run(finalCarSet, rejectCustomer, travelDistance, addAveDistance);
                        TxtRecorder::closeFile();
                        string name = dynamicResultDir+ sampleRate_string + "_" + experimentCountString + ".xml";
                        OutputFile of(finalCarSet, rejectCustomer, dynamicCustomer, name, depot, travelDistance, addAveDistance);
                        of.exportData();

                        // using ALNS to serve customers in finalCarSet to get best results
                        vector<Car*> bestCarSet;
                        float bestCost = 0;
                        computeBest(finalCarSet, bestCarSet, bestCost);
                        vector<Customer*> temp1, temp2;
                        string name2 = staticResultDir + sampleRate_string + "_" + experimentCountString + ".xml";
                        OutputFile of2(bestCarSet, temp1, temp2, name2, depot, bestCost);
                        of2.exportData();
                        
                        withdrawPlan(finalCarSet);
                        withdrawPlan(bestCarSet);
                        cout << endl;
                    }
                }
            }
            cout << "-----------------------------------------------------" << endl;
            cout << "**************** result presentation ****************" << endl;
            cout << "****** The dynamic count is " << dynamicCount << endl;
            cout << "******  The method count is " << methodCount  << endl;
            cout << "*****************************************************" << endl;
            cout << "-----------------------------------------------------" << endl;
        }
        else if (verification == 'N') {
            cout << "stop running!" << endl;
        }
    }
    cout << "==========================================" << endl;
    return 0;
}
