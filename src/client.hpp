#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <map>
#include <unordered_map>
#include <string>
#include <set>
#include <unordered_set>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <random>
#include <cassert>
#include <iomanip>
#include <vector>
#include <chrono>

#include "Types.hpp"
#include "node.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "../protos/bomap.grpc.pb.h"
#else
#include "bomap.grpc.pb.h"
#endif


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std;

#define debug 0
#define debug_randompos 0
extern size_t write_communication_size, read_communication_size;
extern double_t mean_write_communication_size, mean_read_communication_size;
extern double_t pad_communication, mean_pad_communication;
extern std::chrono::duration<double> random_path_time, mean_random_path_time;
extern std::chrono::duration<double> deserial_time, mean_deserial_time;
extern std::chrono::duration<double> serial_time, mean_serial_time;
extern std::chrono::duration<double> dec_time, mean_dec_time;
extern std::chrono::duration<double> enc_time, mean_enc_time;
extern std::chrono::duration<double> insert_time, mean_insert_time;
extern std::chrono::duration<double> write_communication_time, read_communication_time, mean_write_communication_time, mean_read_communication_time, end_to_end_time;

class client
{
private:
    /* data */
public:
    client(int maxSize, vector<bytes<Key>> k1);
    ~client();
    bool init();
    void insert_map(Bid key, int value);
    int search_map(Bid key);
    bool delete_map(Bid key);
    void sendend(std::string str);

    size_t get_stash_size();

    bool insert(Bid key, int value);
    void search(kvpair &kv, Bid key);
    bool Delete(Bid key);
    void part_init();
    void finish(bool find);

    void Readmid1Node(midNode1 &m1, Bid key, int leaf, int mid1L);
    void Readmid2Node(midNode2 &m2, Bid key, int leaf);
    void ReadleafNode(leafNode &l, Bid key, int leaf);
    void WriteRootNode();
    int Writemid1Node(bool incache, Bid bid, midNode1 &node, int mid1L);
    int Writemid2Node(bool incache, Bid bid, midNode2 &node);
    int WriteleafNode(bool incache, Bid bid, leafNode &node);
    void Fetchmid1Path(int leaf, int mid1L);
    void Fetchmid2Path(int leaf);
    void FetchleafPath(int leaf);
    std::string Writemid1Path(int leaf, int d, int mid1L);
    std::string Writemid2Path(int leaf, int d);
    std::string WriteleafPath(int leaf, int d);
    void finalizemid1(bool find, int mid1L);
    void finalizemid2(bool find);
    void finalizeleaf(bool find);
    void finalize2mid1(int mid1L);
    void finalize2mid2();
    void finalize2leaf();

    void m1Update(int m1L, midNode1 &newNode, vector<midNode1>& m1);
    void m1Delete(int m1L, vector<midNode1>& m1, vector<midNode1>& m1_neighbor, vector<int>& f, vector<bool>& brother);
    std::vector<Bid> GetIntersectingBlocks1(int x, int curDepth, int mid1L);
    std::vector<Bid> GetIntersectingBlocks2(int x, int curDepth);
    std::vector<Bid> GetIntersectingBlocks3(int x, int curDepth);
    int RandomPath(int mt);
    int leafRandomPos();
    int mid2RandomPos();
    int mid1RandomPos(int mid1L);
    int GetNodeOnPath(int maxSize, int leaf, int curDepth);
    block SerialiseBucket(Bucket bucket);

    template <class Node>
    Bucket DeserialiseBucket(block buffer);
    template <class Node>
    Bucket DeserialiseBucket(block buffer, int mid1L);

    template <class Node>
    void convert(Node &node, block b);
    template <class Node>
    void convertBlockToNode(Node &node, block b);
    template <class Node>
    void convertBlockToNode(Node &node, block b, int mid1L);
    template <class Node>
    block convertNodeToBlock(Node &node);

    std::string init_dummy(std::string buf, int32_t p, int32_t oram_index);
    std::string read_bucket(const int32_t path, const int32_t oram_index);
    std::string write_bucket(std::vector<std::string> buf_vec,std::vector<int32_t> p, int32_t oram_index);
    std::string end_signal(std::string msg);
    std::string Setup(const int32_t level, const int32_t maxsize, const int32_t oram_index);

    rootNode root;
    
    
    std::vector<std::unordered_map<Bid, midNode1>>  mid1cache;  // search, insert and delete
    std::unordered_map<Bid, midNode2> mid2cache;
    std::unordered_map<Bid, leafNode> leafcache;
    
    std::vector<unordered_set<Bid>> mid1modified;
    std::unordered_set<Bid> mid2modified;    // search and insert
    std::unordered_set<Bid> leafmodified;
    
    std::vector<unordered_set<int>> leafList1;   //search and insert
    std::unordered_set<int> leafList2;
    std::unordered_set<int> leafList3;
    
    std::vector<unordered_set<int>> writeviewmap1;  // search and insert
    std::unordered_set<int> writeviewmap2;
    std::unordered_set<int> writeviewmap3;
    
    vector<bytes<Key>> key1;
    
    
    
    vector<int> readCntmid1;
    int readCntmid2;
    int readCntleaf;
    

    
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    
private:
  std::unique_ptr<bomap::Stub> stub_;
};

#endif