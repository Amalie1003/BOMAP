#include "src/client.hpp"
#include <vector>
#include <iostream>
using namespace std;

#define my_insert 1
#define my_access !my_insert


extern bool is_access;

int main()
{
	vector<bytes<Key>> key1(L);
	for(int i = 0;i < L;i++)
	{
		bytes<Key> tempkey{(uint8_t)i};
		key1[i] = tempkey;
	}	
	std::cout<<"init begin"<<std::endl;
	#if my_access
		is_access = true;
	#endif
	client my_client(N_pairs, key1);
	my_client.init();	
	std::cout<<"init finished!"<<std::endl;
	// insert
    int i = 0;	
    while (true) 
    {
        Bid key = i + 1;
		int value = i + 1;
		std::cout << "begin insert (" << key << "," << value << ")" << std::endl;
		my_client.insert_map(key, value);
        i++;
		if(i == insert_pairs) break;
    }

	Bid del_key = 100;
	std::cout << "begin search : " << del_key << "--- ";
	int ret = my_client.search_map(del_key);
	std::cout << ret << std::endl;

	std::cout << "begin delete (" << del_key << ")" << std::endl;
	my_client.delete_map(del_key);	
    
	std::cout << "begin search : " << del_key << "--- ";
	int ret_2 = my_client.search_map(del_key);
	std::cout << ret_2 << std::endl;
		

	my_client.sendend();
	
	return 0;
}
