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
	ofstream fout("client-03.csv", ios::app);
	// cout << "读-通信时间：" << mean_read_communication_time.count() << "\n";
	// cout << "读-通信量：" << mean_read_communication_size << "\n";
	// cout << "写-通信时间：" << mean_write_communication_time.count() << "\n";
	// cout << "写-通信量：" << mean_write_communication_size << "\n";
	// cout << "端到端时间：" << end_to_end_time.count() << "\n";
	// cout << "客户端处理时间：" << end_to_end_time.count()-mean_read_communication_time.count()-mean_write_communication_time.count() << "\n"; 
	// cout << "随机重分配地址时间：" << mean_random_path_time.count() << "\n";
	// cout << "解密时间：" << mean_dec_time.count() << "\n";
	// cout << "反序列化时间：" << mean_deserial_time.count() << "\n";
	// // cout << "客户端纯插入时间：" << mean_insert_time.count() << "\n";
	// cout << "加密时间：" << mean_enc_time.count() << "\n";
	// cout << "序列化时间：" << mean_serial_time.count() << "\n";
	// cout << std::endl;
	fout << N_pairs << ",";
	fout << L << ",";
    // fout << init_time.count() << ",";
    fout << mean_read_communication_time.count() << ",";
	fout << mean_read_communication_size << ",";
	fout << mean_write_communication_time.count() << ",";
	fout << mean_write_communication_size << ",";
	fout << end_to_end_time.count() << ",";
    // << "客户端处理时间："
	fout << end_to_end_time.count()-mean_read_communication_time.count()-mean_write_communication_time.count() << ","; 
	fout << mean_deserial_time.count()+ mean_dec_time.count() << ",";
	fout << mean_serial_time.count()+mean_enc_time.count() << ",";
	
    for(int i = 100; i > 0; i--)
    {
        Bid f_str = i;
        std::cout << "find " << i << ": " << my_client.search_map(f_str) << "\n";
        // h->del(f_str);
        // std::cout << "find " << i << ": " << h->lookup(f_str) << "\n";
    }
    fout << mean_pad_communication << "\n";
    // fout << std::endl;
    fout.close();
	
	Bid del_key = 100;
	std::cout << "begin search : " << del_key << "--- ";
	int ret = my_client.search_map(del_key);
	std::cout << ret << std::endl;
	/*
	std::cout << "begin delete (" << del_key << ")" << std::endl;
	my_client.delete_map(del_key);	
    
	std::cout << "begin search : " << del_key << "--- ";
	int ret_2 = my_client.search_map(del_key);
	std::cout << ret_2 << std::endl;
	*/
		

	my_client.sendend("exit");
	
	return 0;
}
