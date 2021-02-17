
#include <algorithm>
#include <random>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "./experiment/construct_map.h"
#include "./experiment/construct_ycsb.h"
#include "./experiment/construct_synthetic.h"
#include "./experiment/test_synthetic.h"
using namespace std;

int datasetSize = 67108864;
// int datasetSize = 26214400;
// int datasetSize = 1000000;
int initDatasetSize;

vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

vector<pair<double, double>> findActualDataset;
vector<pair<double, double>> insertActualDataset;
vector<int> length;

int kLeafNodeID = 0;
int kInnerNodeID = 0;

ofstream outRes;

LRType lrRoot;
PLRType plrRoot;
HisType hisRoot;
BSType bsRoot;

int main()
{
    outRes.open("res_0217.csv", ios::app);

    time_t timep;
    time(&timep);
    char tmpTime[64];
    strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "\nTest time: " << tmpTime << endl;

    kMaxKeyNum = 2;
    // kLeafNodeID = 1;
    cout << "kLeafNodeID:" << kLeafNodeID << endl;
    // // static structure
    // for (int i = 0; i < 2; i++)
    // {
    //     kLeafNodeID = i;
    //     cout << "kleafnode:" << kLeafNodeID << endl;
    //     // testSynthetic(1);
    //     testSynthetic(0.5);
    // }
    // testSynthetic(0.95);
    // testSynthetic(0);

    kMaxKeyNum = 1024;
    constructSynthetic(1); // read-only
    constructYCSB(1);      // read-only
    kMaxKeyNum = 2048;
    constructMap(1); // read-only

    // kMaxKeyNum = 1024;
    // constructSynthetic(0.5); // balance
    // constructYCSB(0.5);      // balance
    // kMaxKeyNum = 2048;
    // constructMap(0.5); // balance

    // kMaxKeyNum = 1024;
    // constructSynthetic(0); // partial
    // constructYCSB(0);      // partial
    // kMaxKeyNum = 2048;
    // constructMap(0); // partial

    kMaxKeyNum = 1024;
    constructSynthetic(2); // range scan
    constructYCSB(2);      // range scan
    kMaxKeyNum = 2048;
    constructMap(2); // range scan

    kMaxKeyNum = 1024;
    constructSynthetic(0.95);
    constructYCSB(0.95);
    kMaxKeyNum = 2048;
    constructMap(0.95);

    outRes << "----------------------------------------------" << endl;

    return 0;
}