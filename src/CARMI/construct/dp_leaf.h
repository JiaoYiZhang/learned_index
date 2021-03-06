/**
 * @file dp_leaf.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_DP_LEAF_H_
#define SRC_CARMI_CONSTRUCT_DP_LEAF_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "./structures.h"

/**
 * @brief calculate the time cost of find queries
 * @param node this leaf node
 * @param range the range of find queries
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @return the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafFindTime(int actualSize, double density, const TYPE &node,
                              const IndexPair &range) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    auto predict = node.Predict(findQuery[i].first) + range.left;
    auto d = abs(i - predict);
    time_cost += (CostBaseTime * findQuery[i].second) / querySize;
    if (d <= node.error)
      time_cost +=
          (log(node.error + 1) / log(2) * findQuery[i].second * CostBSTime) *
          (2 - density) / querySize;
    else
      time_cost += (log(actualSize) / log(2) * findQuery[i].second *
                    CostBSTime * (2 - density)) /
                   querySize;
  }
  return time_cost;
}

/**
 * @brief calculate the time cost of insert queries
 * @param node this leaf node
 * @param range the range of insert queries
 * @param findRange the range of find queries
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @return the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafInsertTime(int actualSize, double density,
                                const TYPE &node, const IndexPair &range,
                                const IndexPair &findRange) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    int predict = node.Predict(insertQuery[i].first) + findRange.left;
    int d = abs(i - predict);
    time_cost += (CostBaseTime * insertQuery[i].second) / querySize;
    int actual;
    if (density == 1) {
      actual = TestBinarySearch(insertQuery[i].first, findRange.left,
                                findRange.left + findRange.size);
      time_cost +=
          CostMoveTime * findRange.size / 2 * insertQuery[i].second / querySize;
    } else {
      actual = TestBinarySearch(insertQuery[i].first, findRange.left,
                                findRange.left + findRange.size);
      time_cost += CostMoveTime * density / (1 - density) *
                   insertQuery[i].second / querySize;
    }
    d = abs(actual - predict);

    if (d <= node.error)
      time_cost +=
          (log(node.error + 1) / log(2) * insertQuery[i].second * CostBSTime) *
          (2 - density) / querySize;
    else
      time_cost += (log(actualSize) / log(2) * insertQuery[i].second *
                    CostBSTime * (2 - density)) /
                   querySize;
  }
  return time_cost;
}

/**
 * @brief traverse all possible settings to find the optimal leaf node
 * @param dataRange the range of data points in this node
 * @return the optimal cost of this subtree
 */
NodeCost CARMI::dpLeaf(const DataRange &dataRange) {
  NodeCost nodeCost;
  auto it = COST.find(dataRange.initRange);
  if (it != COST.end()) {
    nodeCost = it->second;
    nodeCost.isInnerNode = false;
    return nodeCost;
  }

  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX, false};
  BaseNode optimal_node_struct;

  if (kPrimaryIndex) {
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    YCSBLeaf tmp;
    Train(&tmp, dataRange.initRange.left, dataRange.initRange.size);
    auto error = tmp.error;
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    for (int i = dataRange.findRange.left; i < findEnd; i++) {
      auto predict = tmp.Predict(findQuery[i].first) + dataRange.findRange.left;
      auto d = abs(i - predict);
      nodeCost.time += (CostBaseTime * findQuery[i].second) / querySize;
      if (d <= error)
        nodeCost.time +=
            (log(error + 1) / log(2) * findQuery[i].second * CostBSTime) /
            querySize;
      else
        nodeCost.time += (log(dataRange.initRange.size) / log(2) *
                          findQuery[i].second * CostBSTime) /
                         querySize;
    }

    int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
    for (int i = dataRange.insertRange.left; i < insertEnd; i++) {
      nodeCost.time +=
          ((CostBaseTime + CostMoveTime) * insertQuery[i].second) / querySize;
    }

    nodeCost.cost = nodeCost.time + nodeCost.space * kRate;  // ns + MB * kRate
    optimalCost = {nodeCost.time, nodeCost.space, nodeCost.cost, false};
    optimal_node_struct.ycsbLeaf = tmp;

    nodeCost.isInnerNode = false;
    COST.insert({dataRange.initRange, optimalCost});
    structMap.insert({dataRange.initRange, optimal_node_struct});
    return nodeCost;
  }

  int actualSize = kThreshold;
  while (dataRange.initRange.size >= actualSize) actualSize *= kExpansionScale;

  if (actualSize > 4096) actualSize = 4096;

  // choose an array node as the leaf node
  double time_cost = 0.0;
  double space_cost = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

  ArrayType tmp(actualSize);
  Train(dataRange.initRange.left, dataRange.initRange.size, &tmp);
  CalLeafFindTime<ArrayType>(actualSize, 1, tmp, dataRange.findRange);
  CalLeafInsertTime<ArrayType>(actualSize, 1, tmp, dataRange.insertRange,
                               dataRange.findRange);

  double cost = time_cost + space_cost * kRate;  // ns + MB * kRate
  if (cost <= optimalCost.cost) {
    optimalCost = {time_cost, space_cost, cost, false};
    optimal_node_struct.array = tmp;
  }

  // choose a gapped array node as the leaf node
  float Density[3] = {0.5, 0.7, 0.8};  // data/capacity
  for (int i = 0; i < 3; i++) {
    // calculate the actual space
    int actualSize = kThreshold;
    while ((static_cast<float>(dataRange.initRange.size) /
                static_cast<float>(actualSize) >=
            Density[i]))
      actualSize = static_cast<float>(actualSize) / Density[i] + 1;
    if (actualSize > 4096) actualSize = 4096;

    GappedArrayType tmpNode(actualSize);
    tmpNode.density = Density[i];

    time_cost = 0.0;
    space_cost = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

    Train(dataRange.initRange.left, dataRange.initRange.size, &tmpNode);
    CalLeafFindTime<GappedArrayType>(actualSize, Density[i], tmpNode,
                                     dataRange.findRange);
    CalLeafInsertTime<GappedArrayType>(actualSize, Density[i], tmpNode,
                                       dataRange.insertRange,
                                       dataRange.findRange);
    cost = time_cost + space_cost * kRate;  // ns + MB * kRate
    if (cost <= optimalCost.cost) {
      optimalCost = {time_cost, space_cost, cost, false};
      optimal_node_struct.ga = tmpNode;
    }
  }

  COST.insert({dataRange.initRange, optimalCost});
  structMap.insert({dataRange.initRange, optimal_node_struct});

  return optimalCost;
}

#endif  // SRC_CARMI_CONSTRUCT_DP_LEAF_H_
