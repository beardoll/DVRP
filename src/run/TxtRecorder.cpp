#include "TxtRecorder.h"

string path = "./outputtxt/20_sample.txt";
ofstream TxtRecorder::outfile(path.c_str(), ofstream::trunc);
