#ifndef LONGITUDES_H
#define LONGITUDES_H

#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
using namespace std;

class LongitudesDataset
{
public:
	LongitudesDataset(double initRatio)
	{
		init = initRatio;
		insertNumber = 100000 * (1 - initRatio);
		if (initRatio == 0)
		{
			num = 0;
			init = 0.85;
			insertNumber = 15000;
		}
		else if (initRatio == 1)
			num = -1;
		else
			num = round(initRatio / (1 - initRatio));
	}

	void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery);

private:
	int num;
	float init;
	int insertNumber;
};

void LongitudesDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
	vector<pair<double, double>>().swap(initDataset);
	vector<pair<double, double>>().swap(trainFindQuery);
	vector<pair<double, double>>().swap(trainInsertQuery);
	vector<pair<double, double>>().swap(testInsertQuery);

	vector<pair<double, double>> insertDataset;

	vector<pair<double, double>> ds;
	ifstream inFile("../src/experiment/dataset/longitude.csv", ios::in);
	if (!inFile)
	{
		cout << "打开文件失败！" << endl;
		exit(1);
	}
	string line;
	while (getline(inFile, line))
	{
		if (line.empty())
			continue;
		istringstream sin(line);
		vector<string> fields;
		string field;
		while (getline(sin, field, ','))
			fields.push_back(field);
		string key = fields[0];
		string value = fields[1];
		double k = stod(key);
		double v = stod(value);
		ds.push_back({k, v});
	}
	cout << "longitude size:" << ds.size() << endl;

	std::sort(ds.begin(), ds.end());
	if (num == 0)
	{
		int totalSize = round(67108864 / 0.85);
		int i = 0;
		for (; i <= 0.6 * totalSize; i++)
			initDataset.push_back(ds[i]);
		for (; i < 0.9 * totalSize; i += 2)
		{
			initDataset.push_back(ds[i]);
			trainInsertQuery.push_back(ds[i + 1]);
			if (testInsertQuery.size() < insertNumber)
				testInsertQuery.push_back(ds[i + 1]);
		}
        for (; i < totalSize; i++)
			initDataset.push_back(ds[i]);
	}
	else if (num == -1)
	{
		for (int i = 0; i < 67108864; i++)
			initDataset.push_back(ds[i]);
	}
	else
	{
		int cnt = 0;
		for (int i = 0; i < ds.size(); i++)
		{
			if (initDataset.size() == 67108864)
				break;
			cnt++;
			if (cnt <= num)
				initDataset.push_back(ds[i]);
			else
			{
				trainInsertQuery.push_back(ds[i]);
				if (testInsertQuery.size() < insertNumber)
					testInsertQuery.push_back(ds[i++]);
				cnt = 0;
			}
		}
	}
	trainFindQuery = initDataset;

	cout << "longitudes: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\ttrain insert size:" << trainInsertQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif