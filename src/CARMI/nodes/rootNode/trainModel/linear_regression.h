/**
 * @file linear_regression.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
#define SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../../../params.h"

class LinearRegression {
 public:
  LinearRegression() {
    theta1 = 0.0001;
    theta2 = 0.666;
  }
  void Train(const DataVectorType &dataset, int len);
  int Predict(double key) const {
    // return the predicted idx in the children
    int p = theta1 * key + theta2;
    if (p < 0)
      p = 0;
    else if (p > length)
      p = length;
    return p;
  }

 private:
  int length;
  double theta1;
  double theta2;
};

void LinearRegression::Train(const DataVectorType &dataset, int len) {
  length = len - 1;
  int actualSize = 0;
  int idx = 0;
  std::vector<double> index(dataset.size(), 0);
  for (int i = 0; i < dataset.size(); i++) {
    if (dataset[i].first != -1) {
      actualSize++;
    }
    index[idx++] = static_cast<double>(i) / static_cast<double>(dataset.size());
  }
  if (actualSize == 0) return;

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = 0; i < dataset.size(); i++) {
    if (dataset[i].first != -1) {
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i];
      t4 += index[i];
    }
  }
  theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
  theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
  theta1 *= len;
  theta2 *= len;
}
#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
