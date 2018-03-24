#include<fstream>
#include<string>
#include<iostream>
#include<cstdlib>
#include "TxtParser.h"

using namespace std;

TxtParser::TxtParser(const string filename) {
    // 初始化函数
    this->filename = filename;
}

void splitString(const string &s, vector<float> &data, const string &sep) {
    // 将字符串s以sep为分隔符切割，并转化成float格式存储于data数组中
    string::size_type pos1, pos2;
    pos2 = s.find(sep);
    pos1 = 0;
    while(string::npos != pos2) {
        string subs = s.substr(pos1, pos2-pos1);
        if(subs != sep) {
            // 处理分隔符连续出现的情况
            data.push_back(atof(subs.c_str()));
        }
        pos1 = pos2 + sep.size();
        pos2 = s.find(sep, pos1);
    }
    if(pos1 != s.length()) {
        string subs = s.substr(pos1);
        if(subs != sep) {
            data.push_back(atof(subs.c_str()));
        }
    }
}
void TxtParser::getDataset(vector<Spot*> &depots, vector<Spot*> &customers, int &limitCarNum) {
    // 读取txt数据，并将其存放在不同的数据体中
    // 数据格式:
    //    * line 1: (都是整数) type, number of vehicles(m), number of customers(n), number of depots(t)
    //    * line 2~(1+t): (都是整数, for vehicles of each depot) time duration, load
    //    * line (2+t)~(2+t+n): (customer数据) id, x, y, serviceTime, demand, freq of visit, _, _, startTime, endTime
    //    * line(2+t+n+1)~(2+t+n+m): (depot数据) id, x, y, _, _, _, _, startTime, endTime
    // Returns:
    //    * depots: 仓库数据
    //    * customers: 顾客数据
    //    * limitCarNum: 车辆数限制
    ifstream in(filename);
    if(!in) {
        cout << "File: " << filename <<  " not exists!" << endl;
        return;
    }
    string line;
    // 读取第一行数据
    getLine(in, line);
    vector<float> data;
    splitString(line, data, " ");
    int vechileNum = (int)data[1];
    int customerNum = (int)data[2];
    limitCarNum = customerNum;
    int depotNum = (int)data[3];
    for(int i=0; i<depotNum; i++) {
        data.clear();
        data.resize(0);
        getLine(in, line);
        splitString(line, data, " ");
        Spot *dp = new Spot();
        dp->timeDuration = data[0];
        dp->quantity = data[1];
        depots.push_back(dp);
    }
    for(int i=0; i<customerNum; i++) {
        data.clear();
        data.resize(0);
        getLine(in, line);
        splitString(line, data, " ");
        Spot *cust = new Spot();
        cust->id = (int)data[0];
        cust->x = data[1];
        cust->y = data[2];
        cust->serviceTime = data[3];
        cust->quantity = data[4];
        cust->startTime = data[8];
        cust->endTime = data[9];
        customers.push_back(cust);
    }
    for(int i=0; i<depotNum; i++) {
        data.clear();
        data.resize(0);
        getLine(in, line);
        splitString(line, data, " ");
        Spot *dp = depots[i];
        dp->id = (int)data[0];
        dp->x = data[1];
        dp->y = data[2];
        dp->startTime = data[7];
        dp->endTime = data[8];
    }
}
