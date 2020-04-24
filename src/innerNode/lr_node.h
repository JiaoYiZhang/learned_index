#ifndef LR_NODE_H
#define LR_NODE_H

#include "inner_node.h"
#include "../trainModel/lr.h"

template <typename lowerType>
class LRNode : public BasicInnerNode
{
public:
    LRNode() : BasicInnerNode(){};
    LRNode(params firstStageParams, params secondStageParams, int childNum) : BasicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
    }
    LRNode(params firstStageParams, params secondStageParams, int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        for (int i = 0; i < childNumber; i++)
            children.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key)
    {
        double p = m_firstStageNetwork.Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->IsLeaf() == false)
            return ((LRNode *)children[preIdx])->Find(key);
        return children[preIdx]->Find(key);
    }
    bool Insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->Insert(data);
    }
    bool Delete(double key)
    {
        double p = m_firstStageNetwork.Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->IsLeaf() == false)
            return ((LRNode *)children[preIdx])->Delete(key);
        return children[preIdx]->Delete(key);
    }
    bool Update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->IsLeaf() == false)
            return ((LRNode *)children[preIdx])->Update(data);
        return children[preIdx]->Update(data);
    }

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    params m_firstStageParams;                                 // parameters of lr
    params m_secondStageParams;                                // parameters of lower nodes
    LinearRegression m_firstStageNetwork = LinearRegression(); // lr of the first stage
};

template <typename lowerType>
void LRNode<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    m_firstStageNetwork.Train(dataset, m_firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = m_firstStageNetwork.Predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return Initialize(dataset);

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
        children[i]->Train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
long double LRNode<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 2;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpNet = LinearRegression();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.Train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.Predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
        totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class AdaptiveLR : public LRNode<lowerType>
{
public:
    AdaptiveLR() : LRNode<lowerType>(){};
    AdaptiveLR(params firstStageParams, params secondStageParams, int maxKey, int childNum, int cap) : LRNode<lowerType>(firstStageParams, secondStageParams, childNum)
    {
        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    bool Insert(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data
};

template <typename lowerType>
void AdaptiveLR<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    // first train the node's linear moDelete using its assigned keys
    this->m_firstStageNetwork.Train(dataset, this->m_firstStageParams);
    //  use the moDelete to divide the keys into some number of partitions
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < this->childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = this->m_firstStageNetwork.Predict(dataset[i].first);
        p = p * (this->childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < this->childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return Initialize(dataset);

    // then iterate through the partitions in sorted order
    cout << "train second stage" << endl;
    for (int i = 0; i < this->childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            AdaptiveLR *child = new AdaptiveLR(this->m_firstStageParams, this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
            child->Initialize(perSubDataset[i]);
            this->children.push_back((lowerType *)child);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            lowerType *child = new lowerType(maxKeyNum, this->m_secondStageParams, capacity);
            child->Train(perSubDataset[i]);
            this->children.push_back(child);
        }
    }
    cout << "End train" << endl;
}

template <typename lowerType>
bool AdaptiveLR<lowerType>::Insert(pair<double, double> data)
{
    double p = this->m_firstStageNetwork.Predict(data.first);
    int preIdx = static_cast<int>(p * (this->childNumber - 1));
    int size = this->children[preIdx]->GetSize();

    // if an Insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[preIdx]->IsLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level moDelete in RMI
        // now becomes an inner level moDelete
        AdaptiveLR *newNode = new AdaptiveLR(this->m_firstStageParams,this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[preIdx]->GetDataset(dataset);
        newNode->m_firstStageNetwork.Train(dataset, this->m_firstStageParams);

        // a number of children leaf level moDeletes are created
        for (int i = 0; i < this->childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, this->m_secondStageParams, capacity);
            newNode->children.push_back(temp);
        }

        // The data from the original leaf node is then
        // distributed to the newly created children leaf nodes
        // according to the original nodeÃ¢â‚¬â„¢s moDelete.
        vector<vector<pair<double, double>>> perSubDataset;
        vector<pair<double, double>> temp;
        for (int i = 0; i < this->childNumber; i++)
            perSubDataset.push_back(temp);
        for (int i = 0; i < dataset.size(); i++)
        {
            double pre = newNode->m_firstStageNetwork.Predict(dataset[i].first);
            pre = pre * (this->childNumber - 1);
            int pIdx = static_cast<int>(pre);
            perSubDataset[pIdx].push_back(dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // moDelete on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->Train(perSubDataset[i]);
        this->children[preIdx] = (lowerType *)newNode;
        return ((AdaptiveLR *)this->children[preIdx])->Insert(data);
    }
    else if (this->children[preIdx]->IsLeaf() == false)
        return ((AdaptiveLR *)this->children[preIdx])->Insert(data);
    return this->children[preIdx]->Insert(data);
}

template <typename lowerType>
long double AdaptiveLR<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpNet = LinearRegression();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.Train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.Predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxNum)
            totalCost += AdaptiveLR<lowerType>::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif