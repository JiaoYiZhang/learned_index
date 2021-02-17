#ifndef BASE_NODE_H
#define BASE_NODE_H
#include <iostream>
#include "innerNode/bs.h"
#include "innerNode/his.h"
#include "innerNode/lr.h"
#include "innerNode/nn.h"

#include "leafNodeType/array.h"
#include "leafNodeType/ga.h"
#include "leafNodeType/ycsb_leaf.h"
using namespace std;

union BaseNode
{
    LRModel lr;
    NNModel nn;
    HisModel his;
    BSModel bs;

    ArrayType array;
    GappedArrayType ga;
    YCSBLeaf ycsbLeaf;

    BaseNode(){};
    ~BaseNode(){};
};

#endif // !BASE_NODE_H