#ifndef WORKLOAD_D_H
#define WORKLOAD_D_H
#include <vector>
#include "zipfian.h"
#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
using namespace std;
extern ofstream outRes;

// write partially workload
// a mix of 85/15 reads and writes
void WorkloadD(CARMI *carmi, vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    auto init = initDataset;
    auto insert = insertDataset;

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(init.begin(), init.end(), engine);

    if (!kPrimaryIndex)
    {
        unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed1);
        shuffle(insert.begin(), insert.end(), engine);
    }

    int end = 5000;
    int findCnt = 0;
    int insertCnt = 0;
    Zipfian zip;
    zip.InitZipfian(PARAM_ZIPFIAN, init.size());
    vector<int> index(85000, 0);
    for (int i = 0; i < 85000; i++)
    {
        int idx = zip.GenerateNextIndex();
        index[i] = idx;
    }

    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 17 && findCnt < init.size(); j++)
        {
            carmi->Find(init[index[findCnt]].first);
            findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
        {
            carmi->Insert(insert[insertCnt]);
            insertCnt++;
        }
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 17 && findCnt < 85000; j++)
        {
            carmi->Find(init[findCnt].first);
            findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < 15000; j++)
        {
            carmi->Insert(insert[insertCnt]);
            insertCnt++;
        }
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    findCnt = 0;
    insertCnt = 0;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 17 && findCnt < init.size(); j++)
        {
            // init[index[findCnt]].first;
            findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
        {
            // insert[insertCnt];
            insertCnt++;
        }
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 17 && findCnt < 85000; j++)
        {
            // init[findCnt].first;
            findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < 15000; j++)
        {
            // insert[insertCnt];
            insertCnt++;
        }
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    tmp -= tmp0;
    cout << "total time:" << tmp / 85000.0 * 1000000000 << endl;
    outRes << tmp / 85000.0 * 1000000000 << ",";
}

#endif // !WORKLOAD_D_H
