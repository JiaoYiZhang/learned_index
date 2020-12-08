#ifndef DP_H
#define DP_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../function.h"
#include "../inlineFunction.h"
#include "params_struct.h"
#include <float.h>
#include <algorithm>
#include <vector>
using namespace std;

extern map<int, double> COST;
extern map<int, LeafParams> leafMap;
extern map<int, InnerParams> innerMap;

extern vector<pair<double, double>> findDataset;
extern vector<pair<double, double>> insertDataset;

// return {cost, type(0:inner, 1:leaf)}
pair<double, int> Construct(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    if (findSize == 0)
    {
        auto it = COST.find(findLeft);
        if (it != COST.end())
        {
            double cost = it->second;
            return {cost, 1};
        }
        else
        {
            double cost = kRate * sizeof(ArrayType) / 1024 / 1024;
            COST.insert({findLeft, cost});
            LeafParams leafP = {4, 2};
            leafMap.insert({findLeft, leafP});
            return {cost, 1};
        }
    }

    // construct a leaf node
    if (isLeaf)
    {
        auto it = COST.find(findLeft);
        if (it != COST.end())
        {
            auto cost = it->second;
            return {cost, 1};
        }
        vector<pair<double, double>> findData;
        vector<pair<double, double>> insertData;
        for (int l = findLeft; l < findLeft + findSize; l++)
            findData.push_back(findDataset[l]);
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
            insertData.push_back(insertDataset[l]);

        double OptimalValue = DBL_MAX;
        LeafParams optimalStruct = {-1, -1};
        double space, time, cost;

        // choose an array node as the leaf node
        time = 0.0;
        space = float(sizeof(ArrayType) + 16 * findSize) / 1024 / 1024;

        auto tmp = ArrayType(kMaxKeyNum);
        tmp.model.Train(findData, findData.size());
        auto error = tmp.UpdateError(findData);
        for (int i = 0; i < findData.size(); i++)
        {
            auto predict = tmp.model.Predict(findData[i].first);
            auto d = abs(i - predict);
            time += 16.36;
            if (d <= error)
                time += log(error) / log(2) * findData[i].second * 4.11;
            else
                time += log(findData.size()) / log(2) * findData[i].second * 4.11;
        }

        for (int i = 0; i < insertData.size(); i++)
        {
            auto predict = tmp.model.Predict(insertData[i].first);
            auto actual = TestArrayBinarySearch(findData[i].first, findData);
            auto d = abs(actual - predict);
            time += 16.36 + 5.25 * (insertData.size() - actual) * insertData[i].second;
            if (d <= error)
                time += log(error) / log(2) * insertData[i].second * 4.11;
            else
                time += log(findData.size()) / log(2) * insertData[i].second * 4.11;
        }
        time = time / (findData.size() + insertData.size());

        cost = time + space * kRate; // ns + MB * kRate
        if (cost <= OptimalValue)
        {
            OptimalValue = cost;
            optimalStruct.type = 4;
            optimalStruct.density = 2;
        }

        // choose a gapped array node as the leaf node
        float Density[4] = {0.5, 0.7, 0.8, 0.9}; // data/capacity
        for (int i = 0; i < 4; i++)
        {
            time = 0.0;
            auto tmpNode = GappedArrayType(kMaxKeyNum);
            tmpNode.density = Density[i];
            space = float(sizeof(GappedArrayType) + 16.0 / tmpNode.density * findData.size()) / 1024 / 1024;

            tmpNode.model.Train(findData, findData.size());
            auto errorGA = tmpNode.UpdateError(findData);
            for (int t = 0; t < findData.size(); t++)
            {
                auto predict = tmpNode.model.Predict(findData[t].first);
                auto d = abs(t - predict);
                time += 16.36;
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * findData[t].second * 4.11 * (2 - Density[i]);
                else
                    time += log(findData.size()) / log(2) * findData[t].second * 4.11 * (2 - Density[i]);
            }
            for (int t = 0; t < insertData.size(); t++)
            {
                auto predict = tmpNode.model.Predict(insertData[t].first);
                auto actual = TestGABinarySearch(findData[t].first, findData);
                time += 16.36;
                auto d = abs(actual - predict);
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * insertData[t].second * 4.11 * (2 - Density[i]);
                else
                    time += log(findData.size()) / log(2) * insertData[t].second * 4.11 * (2 - Density[i]);
            }
            time = time / (findData.size() + insertData.size());

            cost = time + space * kRate; // ns + MB * kRate
            if (cost <= OptimalValue)
            {
                OptimalValue = cost;
                optimalStruct.type = 5;
                optimalStruct.density = Density[i];
            }
        }
        COST.insert({findLeft, cost});
        leafMap.insert({findLeft, optimalStruct});
        return {OptimalValue, 1};
    }
    else
    {
        double OptimalValue = DBL_MAX;
        double space;
        InnerParams optimalStruct = {0, 32, vector<pair<int, int>>()};
        vector<pair<double, double>> findData;
        vector<pair<double, double>> insertData;
        for (int l = findLeft; l < findLeft + findSize; l++)
            findData.push_back(findDataset[l]);
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
            insertData.push_back(insertDataset[l]);
        for (int c = 16; c < findData.size(); c *= 2)
        {
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                switch (type)
                {
                case 0:
                {
                    space = float(4 * c + sizeof(LRType)) / 1024 / 1024; // MB
                    double time = 8.1624;                                // ns
                    double RootCost = time + kRate * space;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = LRType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<int, int>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, subFindData[i].first});
                        RootCost += res.first;
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 0;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                    }
                    break;
                }
                case 1:
                {
                    space = float(4 * c + 192 + sizeof(NNType)) / 1024 / 1024; // MB
                    double time = 20.2689;                                     // ns
                    double RootCost = time + kRate * space;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = NNType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<int, int>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, subFindData[i].first});
                        RootCost += res.first;
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 1;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                    }
                    break;
                }
                case 2:
                {
                    space = float(5 * c + sizeof(HisType)) / 1024 / 1024; // MB
                    double time = 19.6543;
                    double RootCost = time + kRate * space;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = HisType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<int, int>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, subFindData[i].first});
                        RootCost += res.first;
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 2;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                    }
                    break;
                }
                case 3:
                {
                    space = float(12 * c + sizeof(BSType)) / 1024 / 1024;
                    double time = 4 * log(c) / log(2);
                    double RootCost = time + kRate * space;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = BSType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<int, int>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, subFindData[i].first});
                        RootCost += res.first;
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 3;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                    }
                    break;
                }
                }
            }
        }
        COST.insert({findLeft, OptimalValue});
        innerMap.insert({findLeft, optimalStruct});
        return {OptimalValue, 0};
    }
}

#endif // !DP_H