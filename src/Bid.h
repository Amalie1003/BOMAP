#ifndef BID_H
#define BID_H
#include "Types.hpp"
#include <iostream>
#include <functional>
#include <array>
using namespace std;

class Bid {
public:
    std::array< byte_t, ID_SIZE> id;
    Bid();
    Bid(int value);
    Bid(std::array< byte_t, ID_SIZE> value);
    Bid(string value);
    ~Bid();
    Bid operator++ ();
    Bid& operator=(int other);
    bool operator!=(const int rhs) const ;
    bool operator!=(const Bid rhs) const ;
    bool operator==(const int rhs)const ;
    bool operator==(const Bid rhs)const ;
    Bid& operator=(std::vector<byte_t> other);
    bool operator<(const Bid& b) const ;
    bool operator>(const Bid& b) const ;
    bool operator<=(const Bid& b) const ;
    bool operator>=(const Bid& b) const ;    
    friend ostream& operator<<(ostream &o, Bid& id);
};

struct Block {
    Bid id;
    block data;
};

using Bucket = std::array<Block, Z>;

namespace std {
    template <>
    struct hash<Bid> {
        size_t operator()(const Bid& b) const {
            return hash<string>()(std::string(b.id.begin(), b.id.end())); // 哈希逻辑
        }
    };
}

#endif /* BID_H */

