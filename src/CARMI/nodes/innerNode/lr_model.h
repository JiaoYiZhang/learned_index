#ifndef LR_MODEL_H
#define LR_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
using namespace std;

extern vector<BaseNode> entireChild;
extern vector<pair<double, double>> findActualDataset;

inline void LRModel::Initialize(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }

    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].array = tmp;
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GappedArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].ga = tmp;
        }
        break;
    }
}

inline void LRModel::Train(const vector<pair<double, double>> &dataset)
{
    int actualSize = 0;
    int childNumber = flagNumber & 0x00FFFFFF;
    double maxValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            minValue = dataset[i].first;
            break;
        }
    }
    for (int i = dataset.size() - 1; i >= 0; i--)
    {
        if (dataset[i].first != -1)
        {
            maxValue = dataset[i].first;
            break;
        }
    }
    divisor = float(maxValue - minValue) / 6;

    vector<double> index;
    int j = 0, cnt = 0;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            actualSize++;
            int idx = float(dataset[i].first - minValue) / divisor;
            if (idx < 0)
                idx = 0;
            else if (idx >= 6)
                idx = 5;
            if (idx != cnt)
            {
                cnt = idx;
                j = 0;
            }
        }
        index.push_back(double(j));
        j++;
    }
    if (actualSize == 0)
        return;

    int i = 0;
    cnt = 0;
    for (int k = 1; k <= 6; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        for (; i < dataset.size() - 1; i++)
        {
            if (dataset[i].first != -1)
            {
                if (float(dataset[i].first - minValue) / divisor >= k)
                    break;
                cnt++;
                t1 += dataset[i].first * dataset[i].first;
                t2 += dataset[i].first;
                t3 += dataset[i].first * index[i];
                t4 += index[i];
            }
        }
        t3 /= cnt;
        t4 /= cnt;
        if (t1 * cnt - t2 * t2 != 0)
        {
            auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
            auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
            theta1 *= childNumber;
            theta2 *= childNumber;
            theta[k - 1] = {theta1, theta2};
        }
        else
        {
            theta[k - 1] = {childNumber, 0};
        }
    }
}

inline int LRModel::Predict(double key)
{
    int idx = float(key - minValue) / divisor;
    if (idx < 0)
        idx = 0;
    else if (idx >= 6)
        idx = 5;
    // return the predicted idx in the children
    int p = theta[idx].first * key + theta[idx].second;
    int bound = (flagNumber & 0x00FFFFFF) / 6;
    int left = bound * idx;
    if (p < left)
        p = left;
    else if (p >= left + bound)
        p = left + bound - 1;
    return p;
}

inline void LRModel::Train(const int left, const int size)
{
    if (size == 0)
        return;
    int childNumber = flagNumber & 0x00FFFFFF;
    int end = left + size;
    double maxValue;
    for (int i = left; i < end; i++)
    {
        minValue = findActualDataset[i].first;
        break;
    }
    for (int i = end - 1; i >= left; i--)
    {
        maxValue = findActualDataset[i].first;
        break;
    }
    divisor = float(maxValue - minValue) / 6;

    vector<double> index;
    int j = 0;
    int cnt = 0;
    for (int i = left; i < end; i++)
    {
        int idx = float(findActualDataset[i].first - minValue) / divisor;
        if (idx < 0)
            idx = 0;
        else if (idx >= 6)
            idx = 5;
        if (idx != cnt)
        {
            cnt = idx;
            j = 0;
        }
        index.push_back(double(j));
        j++;
    }

    int i = left;
    cnt = 0;
    for (int k = 1; k <= 6; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        for (; i < end - 1; i++)
        {
            if (float(findActualDataset[i].first - minValue) / divisor >= k)
                break;
            cnt++;
            t1 += findActualDataset[i].first * findActualDataset[i].first;
            t2 += findActualDataset[i].first;
            t3 += findActualDataset[i].first * index[i - left];
            t4 += index[i - left];
        }
        t3 /= cnt;
        t4 /= cnt;
        if (t1 * cnt - t2 * t2 != 0)
        {
            auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
            auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
            theta1 *= childNumber;
            theta2 *= childNumber;
            theta[k - 1] = {theta1, theta2};
        }
        else
        {
            theta[k - 1] = {childNumber, 0};
        }
    }
}

#endif