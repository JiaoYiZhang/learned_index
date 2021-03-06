/**
 * @file plr.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_PLR_H_
#define SRC_CARMI_NODES_INNERNODE_PLR_H_

#include <iostream>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

class PLRModel {
 public:
  PLRModel() = default;
  void SetChildNumber(int c) { flagNumber = (PLR_INNER_NODE << 24) + c; }
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;  // 4 Byte
  int index[6];   // 24 Byte
  float keys[8];   // 32 Byte
};

#endif  // SRC_CARMI_NODES_INNERNODE_PLR_H_
