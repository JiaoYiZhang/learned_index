#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/histogram_node.h"

#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "reconstruction.h"

#include <algorithm>
#include <random>
#include <windows.h>
#include <iostream>
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"
#include "inner_noded_creator.h"
#include "leaf_node_creator.h"

using namespace std;

int datasetSize = 1000000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

btree::btree_map<double, double> btreemap;

LRNode lr;
NetworkNode nn;
HistogramNode histogram;
BinarySearchNode bin;

AdaptiveLR ada_lr;
AdaptiveNN ada_nn;
AdaptiveHis ada_his;
AdaptiveBin ada_bin;

int kLeafNodeID = 0;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;

void btree_test(double &time0, double &time1, double &time2, double &time3)
{
    LARGE_INTEGER s, e, c;
    QueryPerformanceFrequency(&c);
    QueryPerformanceCounter(&s);
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);
    QueryPerformanceCounter(&e);
    time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Insert time:" << time1 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.find(insertDataset[i].first);
    QueryPerformanceCounter(&e);
    time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Update time:" << time2 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.erase(insertDataset[i].first);
    QueryPerformanceCounter(&e);
    time3 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Delete time:" << time3 << endl;
    cout << endl;
}

template <typename type>
void test(type obj, double &time0, double &time1, double &time2, double &time3)
{
    LARGE_INTEGER s, e, c;
    QueryPerformanceCounter(&s);
    QueryPerformanceFrequency(&c);
    for (int i = 0; i < dataset.size(); i++)
    {
        // cout << "Find : i:" << i << "\tkey:" << dataset[i].first << "\t";
        obj.Find(dataset[i].first);
        // auto res = obj.Find(dataset[i].first);
        // cout << "\tvalue:" << res.second << endl;
        // if (res.second != dataset[i].first * 10)
        //     cout << "Something wrong with find!" << endl;
    }
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    // QueryPerformanceCounter(&s);
    // for (int i = 0; i < insertDataset.size(); i++)
    // {
    //     // cout << "Insert : i:" << i << "\tkey:" << insertDataset[i].first << "\t";
    //     obj.Insert(insertDataset[i]);
    //     // auto res = obj.Find(insertDataset[i].first);
    //     // cout << "\tvalue:" << res.second << endl;
    //     // if (res.second != insertDataset[i].first * 10)
    //     //     cout << "Something wrong with insert!" << endl;
    //     // cout << endl;
    // }
    // QueryPerformanceCounter(&e);
    // time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    // cout << "Insert time:" << time1 << endl;

    // QueryPerformanceCounter(&s);
    // for (int i = 0; i < insertDataset.size(); i++)
    // {
    //     // cout << "Update : i:" << i << "\tkey:" << insertDataset[i].first << "\t";
    //     obj.Update({insertDataset[i].first, 1.11});
    //     // auto res = obj.Find(insertDataset[i].first);
    //     // cout << "\tvalue:" << res.second << endl;
    //     // if (res.second != 1.11)
    //     //     cout << "Something wrong with update!" << endl;
    // }
    // QueryPerformanceCounter(&e);
    // time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    // cout << "Update time:" << time2 << endl;

    // QueryPerformanceCounter(&s);
    // for (int i = 0; i < insertDataset.size(); i++)
    // {
    //     // cout << "Delete : i:" << i << "\tkey:" << insertDataset[i].first << "\t";
    //     obj.Delete(insertDataset[i].first);
    //     // auto res = obj.Find(insertDataset[i].first);
    //     // cout << "\tvalue:" << res.second << endl;
    //     // if (res.second != 0 && res.second != DBL_MIN)
    //     //     cout << "Something wrong with delete!" << endl;
    // }
    // QueryPerformanceCounter(&e);
    // time3 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    // cout << "Delete time:" << time3 << endl;
    // cout << endl;
}

void printResult(int r, double &time0, double &time1, double &time2, double &time3)
{
    cout << "Average time: " << endl;
    cout << "Find time:" << time0 / (float)dataset.size() / float(r) << endl;
    cout << "Insert time:" << time1 / (float)insertDataset.size() / float(r) << endl;
    cout << "Update time:" << time2 / (float)insertDataset.size() / float(r) << endl;
    cout << "Delete time:" << time3 / (float)insertDataset.size() / float(r) << endl;
    cout << "***********************" << endl;
}

void totalTest(bool isStatic, int repetitions)
{
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double lr_time0 = 0.0, lr_time1 = 0.0, lr_time2 = 0.0, lr_time3 = 0.0;
    double nn_time0 = 0.0, nn_time1 = 0.0, nn_time2 = 0.0, nn_time3 = 0.0;
    double his_time0 = 0.0, his_time1 = 0.0, his_time2 = 0.0, his_time3 = 0.0;
    double bin_time0 = 0.0, bin_time1 = 0.0, bin_time2 = 0.0, bin_time3 = 0.0;
    for (int i = 0; i < repetitions; i++)
    {
        btree::btree_map<double, double> btree;
        for (int i = 0; i < dataset.size(); i++)
            btree.insert(dataset[i]);
        btreemap = btree;
        cout << "btree:    " << i << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        printResult((i + 1), btree_time0, btree_time1, btree_time2, btree_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 0;
        if (isStatic)
        {
            lr = LRNode(childNum);
            lr.Initialize(dataset);
        }
        else
        {
            ada_lr = AdaptiveLR(childNum);
            ada_lr.Initialize(dataset);
        }
        cout << "lr init over!" << endl;
        cout << "****************" << endl;

        cout << "lr:    " << i << endl;
        if (isStatic)
            test(lr, lr_time0, lr_time1, lr_time2, lr_time3);
        else
            test(ada_lr, lr_time0, lr_time1, lr_time2, lr_time3);
        cout << endl;
        printResult((i + 1), lr_time0, lr_time1, lr_time2, lr_time3);
        cout << "-------------------------------" << endl;

        // kInnerNodeID = 1;
        // if (isStatic)
        // {
        //     nn = NetworkNode(childNum);
        //     nn.Initialize(dataset);
        // }
        // else
        // {
        //     ada_nn = AdaptiveNN(childNum);
        //     ada_nn.Initialize(dataset);
        // }
        // cout << "nn init over!" << endl;
        // cout << "****************" << endl;

        // cout << "nn:    " << i << endl;
        // if (isStatic)
        //     test(nn, nn_time0, nn_time1, nn_time2, nn_time3);
        // else
        //     test(ada_nn, nn_time0, nn_time1, nn_time2, nn_time3);
        // cout << endl;
        // printResult((i + 1), nn_time0, nn_time1, nn_time2, nn_time3);
        // cout << "-------------------------------" << endl;

        kInnerNodeID = 2;
        if (isStatic)
        {
            histogram = HistogramNode(childNum);
            histogram.Initialize(dataset);
        }
        else
        {
            ada_his = AdaptiveHis(childNum);
            ada_his.Initialize(dataset);
        }
        cout << "his init over!" << endl;
        cout << "****************" << endl;
        cout << "his:    " << i << endl;
        if (isStatic)
            test(histogram, his_time0, his_time1, his_time2, his_time3);
        else
            test(ada_his, his_time0, his_time1, his_time2, his_time3);
        cout << endl;
        printResult((i + 1), his_time0, his_time1, his_time2, his_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 3;
        if (isStatic)
        {
            bin = BinarySearchNode(childNum);
            bin.Initialize(dataset);
        }
        else
        {
            ada_bin = AdaptiveBin(childNum);
            ada_bin.Initialize(dataset);
        }
        cout << "bin init over!" << endl;
        cout << "****************" << endl;

        cout << "bin:    " << i << endl;
        if (isStatic)
            test(bin, bin_time0, bin_time1, bin_time2, bin_time3);
        else
            test(ada_bin, bin_time0, bin_time1, bin_time2, bin_time3);
        cout << endl;
        printResult((i + 1), bin_time0, bin_time1, bin_time2, bin_time3);
        cout << "-------------------------------" << endl;
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "lr:" << endl;
    printResult(repetitions, lr_time0, lr_time1, lr_time2, lr_time3);

    cout << "nn:" << endl;
    printResult(repetitions, nn_time0, nn_time1, nn_time2, nn_time3);

    cout << "his:" << endl;
    printResult(repetitions, his_time0, his_time1, his_time2, his_time3);

    cout << "bin:" << endl;
    printResult(repetitions, bin_time0, bin_time1, bin_time2, bin_time3);
}

int main()
{
    cout << "Theoretical Analysis:" << endl;
    cout << "size of binary search:\t" << sizeof(BinarySearchNode) << endl;
    cout << "size of histogram:\t" << sizeof(HistogramNode) << endl;
    cout << "size of linear regression:\t" << sizeof(LRNode) << endl;
    cout << "size of network:\t" << sizeof(NetworkNode) << endl;
    cout << "bs:\t" << sizeof(BinarySearchModel) << endl;
    cout << "his:\t" << sizeof(HistogramModel) << endl;
    cout << "lr:\t" << sizeof(LinearRegression) << endl;
    cout << "nn:\t" << sizeof(Net) << endl;
    cout << "basicmodel *:\t" << sizeof(BasicModel *) << endl;
    cout << "vector<void *>:\t" << sizeof(vector<void *>) << endl;
    cout << "vector<bool>:\t" << sizeof(vector<bool>) << endl;
    cout << "basic inner node:\t" << sizeof(BasicInnerNode) << endl;
    cout << "vector<double>:\t" << sizeof(vector<double>) << endl;
    vector<double> t;
    for (int i = 0; i < 8; i++)
        t.push_back(0.2);
    cout << "vector<double> t:\t" << sizeof(t) << endl;
    cout << endl;
    int repetitions = 1;
    cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
    LognormalDataset logData = LognormalDataset(datasetSize, 0.9);
    logData.GenerateDataset(dataset, insertDataset);
    cout << "-------------Static inner nodes----------------" << endl;
    totalTest(true, repetitions);
    // cout << "-------------Adaptive inner nodes----------------" << endl;
    // totalTest(false, repetitions);

    dataset = vector<pair<double, double>>();
    insertDataset = vector<pair<double, double>>();
    kNeuronNumber = 1;
    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    UniformDataset uniData = UniformDataset(datasetSize, 0.9);
    uniData.GenerateDataset(dataset, insertDataset);
    cout << "-------------Static inner nodes----------------" << endl;
    totalTest(true, repetitions);
    cout << "-------------Adaptive inner nodes----------------" << endl;
    totalTest(false, repetitions);

    // vector<pair<double, double>> totalData;
    // for (int i = 0; i < dataset.size(); i++)
    //     totalData.push_back(dataset[i]);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     totalData.push_back(insertDataset[i]);
    // std::sort(totalData.begin(), totalData.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });
    // vector<pair<int, int>> cnt;
    // for (int i = 0; i < datasetSize; i++)
    // {
    //     if ((i + 1) % 10 == 0)
    //         cnt.push_back({0, 1});
    //     else
    //         cnt.push_back({1, 0});
    // }
    // reconstruction(totalData, cnt);

    return 0;
}