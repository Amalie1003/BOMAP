#include "ORAM.hpp"

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <random>
#include <cmath>
#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>

template <class Node>
void convertBlockToNode(Node &node, block b) {
    
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, node);
    return;
}


template <class Node>
Bucket DeserialiseBucket(block buffer) {
    int blockSize = sizeof(Node);
    assert(buffer.size() == Z * (blockSize));

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node node;
        convertBlockToNode<Node>(node, block.data);
        Bid tmp = node.max_value;
        block.id = tmp;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

template<class Node>
ORAM<Node>::ORAM(int maxSize, int oram_index, Connector* mconnector, bool is_acc)
{
    mc = mconnector;    
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    size_t blockSize = sizeof (Node); 
    cout << "Bid size:" << sizeof(Bid) << "\nkvpair size:" << sizeof(kvpair) << "\narray size:" << sizeof(std::array<kvpair, C>) << "\n";
    cout << "bucket size:" << blockSize*Z << "\n# of levels:" << depth +1 << "\ntotal bucket: " <<
    bucketCount << "\ntotal size:" << bucketCount*blockSize*Z/1024/1024 << "MB\n\n";

    if(!is_acc)
    {
        size_t clen_size = AES::GetCiphertextLength((blockSize) * Z);
        size_t plaintext_size = (blockSize) * Z;
        bytes<Key> tempkey{(uint8_t)oram_index};
        vector<int> position;
        vector<string> cipher;
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        
        block b = SerialiseBucket(bucket);
        block ciphertext = AES::Encrypt(tempkey, b, clen_size, plaintext_size);
        std::string buffer(ciphertext.begin(), ciphertext.end());
        for (size_t j = 0; j < bucketCount; j++) {
            position.push_back(j);
            cipher.push_back(buffer);
            if(j % 10000 == 0)
            {
                mc->Bulk_Insert(oram_index, position, cipher);
                position.clear();
                cipher.clear();
            }
        }
        mc->Bulk_Insert(oram_index, position, cipher);
        position.clear();
        cipher.clear();
    }
}
template<class Node>
ORAM<Node>::~ORAM() {
}

template<class Node>
void ORAM<Node>::WriteBucket(int oram_index, int index, std::string ciphertext) {
    
    
    
    mc->Update(oram_index, index, ciphertext);
    

}

template<class Node>
void ORAM<Node>::InitBucket(int oram_index, int pos, std::string bucket)
{
    mc->Insert(oram_index, pos, bucket);
}

template<class Node>
void ORAM<Node>::bulkInitBucket(int oram_index, vector<int> pos, vector<std::string> bucket)
{
    mc->Bulk_Insert(oram_index, pos, bucket);
}

template <class Node>
int ORAM<Node>::GetNodeOnPath(int leaf, int curDepth) {
    
    
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}


template<class Node>
void ORAM<Node>::FetchPath(int oram_index, std::string &result, int leaf) {
    readCnt++;
    
    
    for (size_t d = 0; d <= depth; d++) {
        int node = GetNodeOnPath(leaf, d);
        
        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }       
        std::string ciphertext = mc->Query(oram_index, node);       
        result += ciphertext;
        
    }   
}

template<class Node>
void ORAM<Node>::start() {
    readviewmap.clear();
    readCnt = 0;
}

template <class Node>
block ORAM<Node>::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];        
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }
    return buffer;
}