#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H


#include "../params.h"
#include "../trainModel/lr.h"
#include <vector>
using namespace std;


class ArrayType
{
public:
    ArrayType(){};
    ArrayType(int maxNumber)
    {
        m_datasetSize = 0;
        error = 0;
        m_maxNumber = maxNumber;
        writeTimes = 0;
    }
    void SetDataset(const vector<pair<double, double>> &dataset);

    vector<pair<double, double>> m_dataset;
    LinearRegression model;  // 20 Byte
    int m_datasetSize;
    int error;

    int m_maxNumber; 
    int writeTimes;
};


void ArrayType::SetDataset(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;

    model.Train(m_dataset, m_datasetSize);
    int sum = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        int p = model.Predict(m_dataset[i].first);
        int e = abs(i - p);
        sum += e;
    }
    error = float(sum) / m_datasetSize + 1;
    writeTimes = 0;
}
#endif