#ifndef PARAMS
#define PARAMS

#define ZIPFIAN 1
#define PARAM_ZIPFIAN 0.99
#define DEBUG

// const bool kPrimaryIndex = false;
bool kPrimaryIndex = false;

const double kDensity = 0.5;        // the density of gap in gapped array
const double kExpansionScale = 1.5; // scale of array expansion

const double BaseNodeSpace = 64.0 / 1024 / 1024;

const double LRRootTime = 12.7013;
const double PLRRootTime = 39.6429;
const double HisRootTime = 44.2824;

const double LRInnerTime = 92.4801;
const double PLRInnerTime = 97.1858;
const double HisInnerTime = 109.8874;
const double BSInnerTime = 114.371;

const double CostMoveTime = 6.25;
const double CostBaseTime = 161.241;
const double CostBSTime = 10.9438;

const int reservedSpace = 1024 * 512;

#endif