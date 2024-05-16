#ifndef ORAM_H
#define ORAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <bits/stdc++.h>
#include "node.hpp"
#include "mongoConnector.hpp"


using namespace std;

template <class Node>
class ORAM {
private:
    
    
    size_t depth;    
    int readCnt = 0;    
    size_t bucketCount;
    bool batchWrite = false;

    int GetNodeOnPath(int leaf, int curDepth);
    block SerialiseBucket(Bucket bucket);
public:
    ORAM(){};
    ORAM(int maxSize, int oram_index, mongoConnector* mc, bool is_acc);
    ~ORAM();
 
    void start();
    void FetchPath(int oram_index, std::string &result, int leaf);
    void WriteBucket(int oram_index, int pos, std::string bucket);
    void InitBucket(int oram_index, int pos, std::string bucket);
    void bulkInitBucket(int oram_index, vector<int> pos, vector<std::string> bucket);
    bytes<Key> key;
    std::vector<int> readviewmap;
    mongoConnector* mc;
};

#endif 
