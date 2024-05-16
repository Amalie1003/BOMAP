#include "src/ORAM.hpp"

int main()
{
    bytes<Key> key{2};
    ORAM<midNode1>* oram1 = new ORAM<midNode1>(100,key);
    return 0;
}