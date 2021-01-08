#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <vector>
using namespace std;

class ArrayType
{
public:
    ArrayType() = default;
    ArrayType(int cap)
    {
        flagNumber = (8 << 24) + 0;
        error = 0;
        m_left = -1;
        m_capacity = cap;
    }
    inline int UpdateError(const vector<pair<double, double>> &dataset);
    inline int UpdateError(const int start_idx, const int size);
    inline void SetDataset(const vector<pair<double, double>> &dataset, int cap);
    inline void SetDataset(const int left, const int size, int cap);
    void Train(const vector<pair<double, double>> &dataset);
    void Train(const int start_idx, const int size);
    int Predict(double key);

    int flagNumber; // 4 Byte (flag + 0)

    int m_left; // the left boundary of the leaf node in the global array
    // int m_datasetSize; // the current amount of data
    int m_capacity; // the maximum capacity of this leaf node
    int error;      // the boundary of binary search

    float theta1;
    float theta2;
    pair<float, float> tmppp[5];
};

#endif // !ARRAY_H