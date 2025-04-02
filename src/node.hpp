#pragma once
#ifndef _NODE_H_
#define _NODE_H_
#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include "Bid.h"
#include <iostream>
#include <map>
#include <set>
#include <bits/stdc++.h>
#include <malloc.h>
using namespace std;

const int C = 57, P1 = 132, P2 = 132;
extern int L;
extern bool is_access;
extern int N_pairs;
extern int insert_pairs;

class kvpair
{
public:
    Bid key;
    int value;
    kvpair(){value = 0;}
    kvpair(Bid k, int v){key = k; value = v;}
    kvpair(const kvpair &kv){key = kv.key; value = kv.value;}
    ~kvpair(){}
};

class kppair
{
public:
    Bid key;
    int pos;
    kppair(){ pos = -1;}
    kppair(Bid k, int p){key = k; pos = p;}
    kppair(const kppair &kp){key = kp.key; pos = kp.pos;}
    ~kppair(){}

    bool operator<(const kppair& b) const ;
    bool operator>(const kppair& b) const ;
    bool operator<=(const kppair& b) const ;
    bool operator>=(const kppair& b) const ;   
    bool operator==(const kppair rhs)const ; 
    friend ostream& operator<<(ostream &o, kppair& id);
};

class leafNode
{
//private:
    /* data */
    
public:
    leafNode();
    leafNode(Bid k, int v);
    leafNode(const leafNode &l);
    ~leafNode(){}
    void Insert(leafNode &node, Bid key, int value);
    void Search(kvpair &l_node, Bid key);
    // neightbor: 邻居节点
    // left：是否是左邻居
    // 返回值：-1（没有该删除值）；0（删除，保持两个节点）；1（删除，合并节点） 
    int Delete(leafNode &neighbor, int flag, Bid key);
    std::array<kvpair, C> arr;
    Bid old_max; //old max_value to index the element in its parent node
    Bid max_value; 
    int pos;
    int c;
    // std::array<kppair, C> childMap;
};

class midNode2
{
private:
    /* data */
    
public:
    midNode2();
    midNode2(const midNode2 & mid2);
    ~midNode2(){};
    void Insert(midNode2 &mid2, leafNode& l);
    void Update(leafNode &l);
    //return pos of leafNode
    int Search(Bid key, Bid& l_key);
    int Search(Bid key, Bid& l_key, int& l_pos, Bid& neighbor, int& n_pos, int& up_right);
    int Delete(midNode2 &neighbor, int left, Bid key);
    std::array<kppair, P2> childMap;
    Bid old_max;
    Bid max_value;
    int pos;
    int p2;
};

class midNode1
{
private:
    /* data */
public:
    midNode1();
    midNode1(int mid1L);
    midNode1(const midNode1 & mid1);
    ~midNode1(){};
    void Insert(midNode1 &mid1, midNode2& m2);
    void Update(midNode2 &m2);
    int Delete(midNode1 &neighbor, int left, Bid key);
    
    void Insert(midNode1 &ret_m1, midNode1& insert_m1, int mid1L);
    void Update(midNode1 &m1);
    int Search(Bid key, Bid& m2_key);
    int Search(Bid key, Bid& m2_key, int& m2_pos, Bid& neighbor, int& n_pos, int& up_right);
    std::array<kppair, P2> childMap;
    Bid old_max;
    Bid max_value;
    int pos;
    int p2;
};

class rootNode
{
public:
    std::array<kppair, P1> childMap;
    Bid max_value;
    int p1;
    rootNode();
    rootNode(const rootNode& n);
    ~rootNode(){}
    void Update(midNode1 &m1);
    void Update(midNode2 &m2);
    bool Insert(midNode1 &m1);
    bool Insert(midNode2 &m2);
    int Search(Bid key, Bid& m1_key);
    void Search(Bid key, Bid& m1_key, int& m1_pos, Bid& neighbor, int& n_pos, int& right);
    int Delete(Bid key);
};

#endif
