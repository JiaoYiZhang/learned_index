#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <chrono>
#include <vector>
using namespace std;

class UniformDataset
{
public:
    UniformDataset(int total, double initRatio)
    {
        num = 2;
        totalSize = total / initRatio;
        insertNumber = 100000 * (1 - initRatio);
        if (initRatio == 0)
        { // several leaf nodes are inserted
            initSize = 0;
            totalSize = round(total / 0.85);
            insertNumber = 15000;
        }
        else if (initRatio == 1)
        {
            num = -1;
            initSize = total;
        }
        else
        {
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

void UniformDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(trainFindQuery);
    vector<pair<double, double>>().swap(trainInsertQuery);
    vector<pair<double, double>>().swap(testInsertQuery);

    if (initSize != 0 && num != -1)
        totalSize *= 1.5;
    int cnt = 0;
    if (initSize == 0)
    {
        int i = 0;
        totalSize = 67108864;
        for (; i <= 0.6 * totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
        for (; i < 0.9 * totalSize; i++)
        {
            initDataset.push_back({double(i), double(i) * 10});
            if (trainInsertQuery.size() < 11842741)
                trainInsertQuery.push_back({double(i + 0.5), double(i + 0.5) * 10});
            if (testInsertQuery.size() < insertNumber)
                testInsertQuery.push_back({double(i + 0.5001), double(i + 0.5001) * 10});
        }
        for (; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
    }
    else if (num == -1)
    {
        for (int i = 0; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
    }
    else
    {
        for (int i = 0; i < totalSize; i++)
        {
            cnt++;
            if (cnt <= num)
            {
                if (initDataset.size() == 67108864)
                    break;
                initDataset.push_back({double(i), double(i) * 10});
            }
            else
            {
                trainInsertQuery.push_back({double(i), double(i) * 10});
                i++;
                if (testInsertQuery.size() < insertNumber)
                    testInsertQuery.push_back({double(i), double(i) * 10});
                cnt = 0;
            }
        }
    }
    trainFindQuery = initDataset;

    cout << "uniform: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\ttrain insert size:" << trainInsertQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif