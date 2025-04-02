#include "src/node.hpp"


int main()
{
    std::cout << "Bid " << sizeof(Bid) << std::endl;
    std::cout << "kvpair " << sizeof(kvpair) << std::endl;
    std::cout << "leafNode " << sizeof(leafNode) << std::endl;
    std::cout << "mid2Node " << sizeof(midNode2) << std::endl;
    std::cout << "mid1Node " << sizeof(midNode1) << std::endl;
    size_t total_size = 0;
    size_t maxSize = P1*2;
    size_t depth = floor(log2(maxSize / Z));
    size_t bucketCount = pow(2, depth + 1) - 1;
    size_t blockSize = sizeof (midNode1); 
    cout << "Bid size:" << sizeof(Bid) << "\nkvpair size:" << sizeof(kvpair) << "\narray size:" << sizeof(std::array<kvpair, C>) << "\n";
    cout << "bucket size:" << blockSize*Z << "\n# of levels:" << depth +1 << "\ntotal bucket: " <<
    bucketCount << "\ntotal size:" << (float)(depth+1)*blockSize*Z/1024 << "KB\n\n";
    total_size += (float)(depth+1)*blockSize*Z/1024;

    maxSize = P1*P1*2;
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    cout << "\n# of levels:" << depth +1 << "\ntotal bucket: " <<
    bucketCount << "\ntotal size:" << (float)(depth+1)*blockSize*Z/1024 << "KB\n\n";
    total_size += (float)(depth+1)*blockSize*Z/1024;

    maxSize = P1*P1*P1*2;
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    cout << "\n# of levels:" << depth +1 << "\ntotal bucket: " <<
    bucketCount << "\ntotal size:" << (float)(depth+1)*blockSize*Z/1024 << "KB\n\n";
    total_size += (float)(depth+1)*blockSize*Z/1024;

    maxSize = P1*P1*P1*P1*2;
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof(leafNode);
    cout << "\n# of levels:" << depth +1 << "\ntotal bucket: " <<
    bucketCount << "\ntotal size:" << (float)(depth+1)*blockSize*Z/1024 << "KB\n\n";
    total_size += (float)(depth+1)*blockSize*Z/1024;
    cout << total_size << "\n";
    std::cout <<sizeof(string) << "\n";
    return 0;
}