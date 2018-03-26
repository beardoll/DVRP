#ifndef TXT_PARSER_H
#define TXT_PARSER_H
#include<string>
#include<vector>
#include "../baseclass/Spot.h"

using namespace std;

class TxtParser{
public:
    TxtParser(const string filename);
    ~TxtParser() {};
    void getDataset(vector<Spot*> &depots, vector<Spot*> &customers,
            int &limitCarNum);
private:
    string filename;
};

#endif
