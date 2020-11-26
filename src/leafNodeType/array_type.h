#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H

#include "../params.h"
#include "../trainModel/lr.h"
#include <vector>
#include "../datapoint.h"
using namespace std;

extern pair<double, double> *entireData;
extern int *mark;
extern int entireDataSize;
class ArrayType
{
public:
    ArrayType(){};
    ArrayType(int cap)
    {
        m_datasetSize = 0;
        error = 0;
        m_left = -1;
        m_capacity = cap; // 256 or 512
        rate = 2;
    }
    void SetDataset(const vector<pair<double, double>> &dataset, int cap);

    LinearRegression model; // 20 Byte
    int m_left;             // the left boundary of the leaf node in the global array
    int m_datasetSize;      // the current amount of data
    int m_capacity;         // the maximum capacity of this leaf node
    double rate;            // the ratio of each expansion

    int error; // the boundary of binary search
};

void ArrayType::SetDataset(const vector<pair<double, double>> &dataset, int cap)
{
    m_capacity = cap;
    m_datasetSize = dataset.size();
    while (m_datasetSize > m_capacity)
        m_capacity *= 2;
    m_left = allocateMemory(m_capacity);

    if (m_datasetSize == 0)
        return;

    for (int i = m_left, j = 0; j < m_datasetSize; i++, j++)
        entireData[i] = dataset[j];

    model.Train(dataset, m_datasetSize);
    int sum = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        int p = model.Predict(dataset[i].first);
        int e = abs(i - p);
        sum += e;
    }
    error = float(sum) / m_datasetSize + 1;
}
#endif