#ifndef LOGNORMAL_DISTRIBUTION_H
#define LOGNORMAL_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

class LognormalDataset
{
public:
    LognormalDataset(int total, double initRatio)
    {
        num = 2;
        insertNumber = 100000 * (1 - initRatio);
        if (initRatio == 0)
        { // several leaf nodes are inserted
            insertNumber = 15000;
            totalSize = round(total / 0.85);
            initSize = 0;
        }
        else if (initRatio == 1)
        {
            totalSize = round(total / initRatio);
            num = -1;
            initSize = total;
        }
        else
        {
            totalSize = round(total / initRatio);
            initSize = total;
            num = round(initRatio / (1 - initRatio));
        }
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery);

private:
    int totalSize;
    int initSize;

    int num;
    int insertNumber;
};

void LognormalDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
    if (initSize != 0 && num != -1)
        totalSize *= 1.5;
    float maxValue = 100000000;

    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);
    vector<double> ds;

    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(trainFindQuery);
    vector<pair<double, double>>().swap(trainInsertQuery);
    vector<pair<double, double>>().swap(testInsertQuery);

    for (int i = 0; i < totalSize; i++)
    {
        ds.push_back(distribution(generator));
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
    if (initSize == 0)
    {
        int i = 0;
        totalSize = 67108864;
        for (; i <= 0.6 * totalSize; i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
        for (; i < 0.9 * totalSize; i++)
        {
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            if (trainInsertQuery.size() < 11842741)
                trainInsertQuery.push_back({double((ds[i] + 0.0001) * factor), double((ds[i] + 0.0001) * factor) * 10});
            if (testInsertQuery.size() < insertNumber)
                testInsertQuery.push_back({double((ds[i] + 0.0001) * factor), double((ds[i] + 0.0001) * factor) * 10});
        }
        for (; i < totalSize; i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    else if (num == -1)
    {
        for (int i = 0; i < ds.size(); i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    else
    {
        int cnt = 0;
        for (int i = 0; i < ds.size(); i++)
        {
            cnt++;
            if (cnt <= num)
            {
                if (initDataset.size() == 67108864)
                    break;
                initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            }
            else
            {
                trainInsertQuery.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
                i++;
                if (testInsertQuery.size() < insertNumber)
                    testInsertQuery.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
                cnt = 0;
            }
        }
    }
    trainFindQuery = initDataset;

    cout << "lognormal: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\ttrain insert size:" << trainInsertQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif