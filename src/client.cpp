#include "client.hpp"
int L = 5;
int N_pairs = pow(2,30);
int insert_pairs = 1000;
bool is_access = false;
size_t write_communication_size = 0, read_communication_size = 0;
double_t mean_write_communication_size = 0, mean_read_communication_size = 0;
double_t pad_communication = 0, mean_pad_communication = 0;
std::chrono::duration<double> write_communication_time(0), read_communication_time(0);
std::chrono::duration<double> random_path_time(0), mean_random_path_time(0);
std::chrono::duration<double> deserial_time(0), mean_deserial_time(0);
std::chrono::duration<double> serial_time(0), mean_serial_time(0);
std::chrono::duration<double> dec_time(0), mean_dec_time(0);
std::chrono::duration<double> enc_time(0), mean_enc_time(0);
std::chrono::duration<double> insert_time(0), mean_insert_time(0);
std::chrono::duration<double> mean_write_communication_time(0), mean_read_communication_time(0);
std::chrono::duration<double> end_to_end_time(0);

client::client(int maxSize, vector<bytes<Key>> k1): rd(), mt(rd()), 
    dis(0, pow(2, floor(log2(maxSize / Z))) - 1),mid1cache(L-3),mid1modified(L-3), leafList1(L-3),
    writeviewmap1(L-3),key1(L),readCntmid1(L-3)
{
    std::cout << "begin client init" << std::endl;
    AES::Setup();
    for(unsigned int i = 0; i < k1.size(); i++) key1[i]=k1[i];
    stub_ = bomap::NewStub(grpc::CreateChannel("localhost:1123", grpc::InsecureChannelCredentials()));
    std::cout << "end client init" << std::endl;  
}

client::~client()
{
    AES::Cleanup();
    
}

size_t client::get_stash_size()
{
    size_t ret;
    ret = leafcache.size()*sizeof(leafNode) + mid2cache.size() * sizeof(midNode2);
	for(int i = 0; i < L-3; i++)
	{
		ret += mid1cache[i].size() * sizeof(midNode1);
	}
    
    return ret;
}

int client::search_map(Bid key)
{
    read_communication_size = 0;
    write_communication_size = 0;
    read_communication_time = std::chrono::milliseconds(0);
    write_communication_time = std::chrono::milliseconds(0);
    enc_time = std::chrono::milliseconds(0);
    dec_time = std::chrono::milliseconds(0);
    serial_time = std::chrono::milliseconds(0);
    deserial_time = std::chrono::milliseconds(0);
    pad_communication = 0;

    part_init();
    end_signal("search");
    kvpair ret;
    search(ret, key);
    pad_communication = read_communication_size;
    finish(true);
    pad_communication = read_communication_size - pad_communication;
    mean_pad_communication += pad_communication/100;
    if(ret.key != 0) return ret.value;
    else return -1;
}

void client::insert_map(Bid key, int value)
{
    read_communication_size = 0;
    write_communication_size = 0;
    read_communication_time = std::chrono::milliseconds(0);
    write_communication_time = std::chrono::milliseconds(0);
    enc_time = std::chrono::milliseconds(0);
    dec_time = std::chrono::milliseconds(0);
    serial_time = std::chrono::milliseconds(0);
    deserial_time = std::chrono::milliseconds(0);
    auto begin = std::chrono::high_resolution_clock::now();

    part_init();
    
    insert(key, value);
    size_t read1_size = read_communication_size;
    
    finish(false);

    auto end = std::chrono::high_resolution_clock::now();
    end_to_end_time += (end - begin)/insert_pairs;
    mean_read_communication_size += (double_t)read_communication_size/insert_pairs;
    mean_read_communication_time += read_communication_time/insert_pairs;
    mean_write_communication_size += (double_t)write_communication_size/insert_pairs;
    mean_write_communication_time += write_communication_time/insert_pairs;
    mean_dec_time += dec_time/insert_pairs;
    mean_enc_time += enc_time/insert_pairs;
    mean_deserial_time += deserial_time/insert_pairs;
    mean_serial_time += serial_time/insert_pairs;
}

bool client::delete_map(Bid key)
{
    part_init();
    bool ret = Delete(key);
    finish(false);
    return ret;
}

bool client::init()
{
    std::vector<size_t> maxsize;
    std::vector<size_t> blockSize;
    maxsize.push_back(2 * P1 * pow(P2,L-2));
    maxsize.push_back(2 * P1 * pow(P2,L-3));
    blockSize.push_back(sizeof(leafNode));
    blockSize.push_back(sizeof(midNode2));
    for(int i = 0; i < L-1; i++)
    {
        if(i >= 2) {
            maxsize.push_back(2 * P1 * pow(P2, i-2));
            blockSize.push_back(sizeof(midNode1));
        }
        Setup(L, maxsize[i], i);
    }
    if(is_access==0){
        block b = convertNodeToBlock(root);
        block rootcipher = AES::Encrypt(key1[L-1], b, AES::GetCiphertextLength(sizeof(rootNode)), sizeof(rootNode));
        string buffer(rootcipher.begin(), rootcipher.end());
        init_dummy(buffer, 0, L-1);
    }
    else
    {
        std::string reply = read_bucket(0, L-1);
        std::cout << "reply = " << reply.length() << std::endl;
        block ciphertext;
        ciphertext.insert(ciphertext.end(), reply.begin(), reply.end());
        block buffer = AES::Decrypt(key1[L-1], ciphertext, AES::GetCiphertextLength(sizeof(rootNode)));
        convertBlockToNode(root, buffer);
        if(root.p1 == 0) is_access = false;
        
    }
    return true;
}

void client::sendend(std::string str)
{
    std::string reply = end_signal(str);
    if(reply == "ok") std::cout << "[client]send over" <<std::endl;
    else std::cout << reply << std::endl;
}

void client::part_init()
{
    
    std::fill(readCntmid1.begin(),readCntmid1.end(),0);
    readCntmid2 = 0;
    readCntleaf = 0;
    for(int i=0;i<L-3;i++)
    {
        writeviewmap1[i].clear();
    }
    writeviewmap2.clear();
    writeviewmap3.clear();
}

void client::search(kvpair &kv, Bid key)
{
    std::vector<Bid> m1_key(L-3);
    std::vector<midNode1> m1(L-3);
    Bid m2_key, l_key;

    midNode2 m2;
    if((L-3)==0)
    {
        int m2_pos = root.Search(key, m2_key);
        #if debug_randompos
        std::cout << "m2pos: " << m2_pos << std::endl;
        #endif
        Readmid2Node(m2, m2_key, m2_pos);
    }
    else
    {
        int m1_pos = root.Search(key, m1_key[0]);
        Readmid1Node(m1[0], m1_key[0], m1_pos, 0);
        for(int i=1;i<L-3;i++)
        {
            int m1_pos2 = m1[i-1].Search(key, m1_key[i]);
            Readmid1Node(m1[i], m1_key[i], m1_pos2, i);
        }
        int m2_pos = m1[L-4].Search(key, m2_key);
        Readmid2Node(m2, m2_key, m2_pos);
    }
    int l_pos = m2.Search(key, l_key);
    #if debug_randompos
    std::cout << "lpos: " << l_pos << std::endl;
    #endif
    leafNode l;
    ReadleafNode(l, l_key, l_pos);
    l.Search(kv, key);
}


void client::m1Update(int m1L, midNode1& newNode, vector<midNode1>& m1)
{
    if(m1L >= 0) 
        m1[m1L].Update(m1[m1L + 1]);
    else 
        root.Update(m1[m1L + 1]);
    
    if(m1L >= 0)
    {
        if(newNode.p2 != 0)
        {
            midNode1 newNode4(m1L);
            newNode4.pos = mid1RandomPos(m1L);
            m1[m1L].Insert(newNode4, newNode, m1L);
            Writemid1Node(false, newNode.max_value, newNode, m1L + 1);      
            m1Update(m1L-1, newNode4,m1);
        }
        else
        {
            for(int j=m1L-1;j>=0;j--)
            {
                m1[j].Update(m1[j+1]);
            }
            root.Update(m1[0]);
        }

    }
    else
    {
        if(newNode.p2 != 0)
        {
            /*flag = */root.Insert(newNode);
            Writemid1Node(false, newNode.max_value, newNode, 0);
        }
    }
}

void client::m1Delete(int m1L, vector<midNode1>& m1, vector<midNode1>& m1_neighbor, vector<int>& f, vector<bool>& brother)
{
    if(m1L >= 0)
    {
        if(m1_neighbor[m1L].p2 > 0 && brother[m1L+1] == false) m1_neighbor[m1L].Update(m1_neighbor[m1L+1]);
        else if(m1_neighbor[m1L+1].p2 > 0 && brother[m1L+1] == true) m1[m1L].Update(m1_neighbor[m1L+1]);
        Writemid1Node(true, m1_neighbor[m1L+1].max_value, m1_neighbor[m1L+1], m1L+1);
    }
    else 
    {
        if(m1_neighbor[m1L+1].p2 > 0) root.Update(m1_neighbor[m1L+1]);
        root.Delete(m1[m1L+1].max_value);
        Writemid1Node(true, m1_neighbor[m1L+1].max_value, m1_neighbor[m1L+1], m1L+1);
        return;
    }
    int sub_flag = m1[m1L].Delete(m1_neighbor[m1L], f[m1L], m1[m1L+1].max_value);
    
    if(sub_flag == 1)
    {
        
        m1Delete(m1L - 1, m1, m1_neighbor, f, brother);
    }
    else if (sub_flag == 0)
    {
        for(int i = m1L -1; i >= 0; i++)
        {
            m1[i].Update(m1[i+1]);
            if(m1_neighbor[i].p2 > 0 && brother[i+1] == false) m1_neighbor[i].Update(m1_neighbor[i+1]);
            else if(m1_neighbor[i+1].p2 > 0 && brother[i+1] == true) m1[i].Update(m1_neighbor[i+1]);
            Writemid1Node(true, m1[i+1].max_value, m1[i+1], i+1);
            Writemid1Node(true, m1_neighbor[i+1].max_value, m1_neighbor[i+1], i+1);
        }
        root.Update(m1[0]);
        if(m1_neighbor[0].p2 > 0) root.Update(m1_neighbor[0]);
        Writemid1Node(true, m1[0].max_value, m1[0], 0);
        Writemid1Node(true, m1_neighbor[0].max_value, m1_neighbor[0], 0);
    }
}

bool client::insert(Bid key, int value)
{
    if (root.p1 == 0){
        #if debug
            std::cout << "---开始第一个数据插入---" << std::endl;
        #endif
        leafNode leaf(key, value);
        leaf.pos = leafRandomPos();
        midNode2 mid2, nmid2;
        mid2.pos = mid2RandomPos();
        nmid2.pos = mid2RandomPos();
        mid2.Insert(nmid2, leaf);
        WriteleafNode(false, key, leaf);
        
        if((L-3)==0)
        {
            root.Insert(mid2);
            Writemid2Node(false, mid2.max_value, mid2);
        }
        else
        {
            vector<midNode1> mid1(L-3);
            for(int i=0;i<L-3;i++)
            {
                mid1[i] = midNode1(i);
                mid1[i].pos = mid1RandomPos(i);
            }
            midNode1 nmid1(L - 4);
            nmid1.pos = mid1RandomPos(L-4);
            mid1[L-4].Insert(nmid1, mid2);
            Writemid2Node(false, mid2.max_value, mid2);

            for(int i=L-5;i>=0;i--)
            {
                midNode1 nmid1(i);
                nmid1.pos = mid1RandomPos(i);
                mid1[i].Insert(nmid1, mid1[i+1],i);
                Writemid1Node(false, mid1[i+1].max_value, mid1[i+1], i+1);
            }
            root.Insert(mid1[0]);
            Writemid1Node(false, mid1[0].max_value, mid1[0], 0);
        }
        return true;
        
        /*---code---*/
    }
    
    Bid m2_key, l_key;
    std::vector<Bid> m1_key(L-3);
    
    
    
    
    vector<midNode1> m1(L-3);
    for(int i = 0; i < L-3; i++) m1[i].pos = mid1RandomPos(i);
    midNode2 m2;
    m2.pos = mid2RandomPos();
    if((L-3)>0)
    {
        int m1_pos = root.Search(key, m1_key[0]);
        Readmid1Node(m1[0], m1_key[0], m1_pos, 0);
        if(mid1cache[0].count(m1_key[0]) != 0) mid1cache[0].erase(m1_key[0]);
        for(int i=1;i<L-3;i++)
        {
            int m1_pos2 = m1[i-1].Search(key, m1_key[i]);
            
            Readmid1Node(m1[i], m1_key[i], m1_pos2, i);
            if(mid1cache[i].count(m1_key[i]) != 0) mid1cache[i].erase(m1_key[i]);
            
        }
        int m2_pos = m1[L-4].Search(key, m2_key);
        
        
        Readmid2Node(m2, m2_key, m2_pos);
        if(mid2cache.count(m2_key) != 0) mid2cache.erase(m2_key);
    }
    else
    {
        int m2_pos = root.Search(key, m2_key);
        
        
        Readmid2Node(m2, m2_key, m2_pos);
        if(mid2cache.count(m2_key) != 0) mid2cache.erase(m2_key);
    }
    
    int l_pos = m2.Search(key, l_key);
    
    leafNode l;
    l.pos = leafRandomPos();
    ReadleafNode(l, l_key, l_pos);
    if(leafcache.count(l_key) != 0) leafcache.erase(l_key);

    // begin insert
    auto begin = std::chrono::high_resolution_clock::now();
    leafNode newNode;
    newNode.pos = leafRandomPos();
    l.Insert(newNode, key, value);
    
    m2.Update(l);
    WriteleafNode(true, l.max_value, l);
    if((L-3)>0)
    {
        if(newNode.c != 0)
        {
            midNode2 newNode2;
            newNode2.pos = mid2RandomPos();
            m2.Insert(newNode2, newNode);
            WriteleafNode(false, newNode.max_value,newNode);
            m1[L-4].Update(m2);
            if(newNode2.p2 != 0)
            {         
                midNode1 newNode3(L - 4);
                newNode3.pos = mid1RandomPos(L-4);
                m1[L-4].Insert(newNode3, newNode2);
                Writemid2Node(false, newNode2.max_value, newNode2);
                
                if(L==4)
                {
                    root.Update(m1[0]);
                    if(newNode3.p2 != 0)
                    {
                        /*flag = */root.Insert(newNode3);
                        Writemid1Node(false, newNode3.max_value, newNode3, 0);
                    }
                }
                else
                {
                    
                    m1Update(L-5, newNode3,m1);
                }
            }
            else 
            {
                for(int i = L-5; i >= 0; i--)
                {
                    m1[i].Update(m1[i+1]);
                }    
                root.Update(m1[0]);
            }
        }
        else{
            m1[L-4].Update(m2);
            for(int i = L-5; i >= 0; i--)
            {
                m1[i].Update(m1[i+1]);
            }    
            root.Update(m1[0]);
        }

        Writemid2Node(true, m2.max_value, m2);
        
        for(int i=L-4;i>=0;i--)
        {
            Writemid1Node(true, m1[i].max_value, m1[i], i);
        }
    }
    else
    {
        if(newNode.c != 0)
        {
            midNode2 newNode2;
            newNode2.pos = mid2RandomPos();
            m2.Insert(newNode2, newNode);
            WriteleafNode(false, newNode.max_value,newNode);
            root.Update(m2);
            if(newNode2.p2 != 0)
            {
                root.Insert(newNode2);
                Writemid2Node(false, newNode2.max_value, newNode2);    
            }
        }
        else
        {
            root.Update(m2);
        }
        Writemid2Node(true, m2.max_value, m2);
    }
    auto end = std::chrono::high_resolution_clock::now();
    insert_time = end-begin;
    mean_insert_time += insert_time/insert_pairs;
    return true;
}

bool client::Delete(Bid key)
{
    if(root.p1 == 0) return false;
    
    std::vector<Bid> node_key(L-1);
    std::vector<Bid> neighbor_key(L-1);
    std::vector<int> node_pos(L-1);
    std::vector<int> neighbor_pos(L-1);
    leafNode l, l_neighbor;
    l.pos = leafRandomPos();
    l_neighbor.pos = leafRandomPos();
    midNode2 m2, m2_neighbor;
    m2.pos = mid2RandomPos();
    m2_neighbor.pos = mid2RandomPos();
    std::vector<midNode1> m1(L-3);
    std::vector<midNode1> m1_neighbor(L-3);
    for(int i = 0; i < L-3; i++) m1[i].pos = mid1RandomPos(i);
    for(int i = 0; i < L-3; i++) m1_neighbor[i].pos = mid1RandomPos(i);
    std::vector<bool> brother(L-1); 
    std::vector<int> f(L-1);
    int flag = 0; 
    if(L == 3)
    {
        root.Search(key, node_key[0], node_pos[0], neighbor_key[0], neighbor_pos[0], flag);
        Readmid2Node(m2, node_key[0], node_pos[0]);
        if(mid2cache.count(node_key[0]) != 0) mid2cache.erase(node_key[0]);
        if(flag != 0)
        {
            Readmid2Node(m2_neighbor, neighbor_key[0], neighbor_pos[0]);
            if(mid2cache.count(neighbor_key[0]) != 0) mid2cache.erase(neighbor_key[0]);
        }
        else
        {
            neighbor_key[0] = node_key[0];
            neighbor_pos[0] = node_pos[0];
        }
        f[0] = flag;
        brother[0] = true;
    }
    else if(L > 3)
    {
        root.Search(key, node_key[0], node_pos[0], neighbor_key[0], neighbor_pos[0], flag);
        Readmid1Node(m1[0], node_key[0], node_pos[0], 0);
        if(mid1cache[0].count(node_key[0]) != 0) mid1cache[0].erase(node_key[0]);
        f[0] = flag;
        brother[0] = true;
        if(flag != 0) 
        {
            Readmid1Node(m1_neighbor[0], neighbor_key[0], neighbor_pos[0], 0);
            if(mid1cache[0].count(neighbor_key[0]) != 0) mid1cache[0].erase(neighbor_key[0]);
        }
        else 
        {
            neighbor_key[0] = node_key[0];
            neighbor_pos[0] = node_pos[0];
        }
        int sub_flag = 0;
        for(int i = 1; i < L - 3; i++)
        {
            sub_flag = m1[i - 1].Search(key, node_key[i], node_pos[i], neighbor_key[i], neighbor_pos[i], flag);
            if(sub_flag == 0) brother[i] = true;
            else brother[i] = false;
            Readmid1Node(m1[i], node_key[i], node_pos[i], i);
            if(mid1cache[i].count(node_key[i]) != 0) mid1cache[i].erase(node_key[i]);
            f[i] = flag;
            if(sub_flag == 0) 
            {
                if(flag == 0)
                {
                    neighbor_key[i] = node_key[i];
                    neighbor_pos[i] = node_pos[i];
                }
                else
                {
                    Readmid1Node(m1_neighbor[i], neighbor_key[i], neighbor_pos[i], i);
                    if(mid1cache[i].count(neighbor_key[i]) != 0) mid1cache[i].erase(neighbor_key[i]);
                }
            }
            else if(sub_flag == -1 && flag == -1) 
            {
                neighbor_key[i] = m1_neighbor[i-1].max_value;
                neighbor_pos[i] = m1_neighbor[i-1].childMap[m1_neighbor[i-1].p2 - 1].pos;
                Readmid1Node(m1_neighbor[i], neighbor_key[i], neighbor_pos[i], i);
                if(mid1cache[i].count(neighbor_key[i]) != 0) mid1cache[i].erase(neighbor_key[i]);
            }
            else if(sub_flag == 1 && flag == 1) 
            {
                neighbor_key[i] = m1_neighbor[i-1].childMap[0].key;
                neighbor_pos[i] = m1_neighbor[i-1].childMap[0].pos;
                Readmid1Node(m1_neighbor[i], neighbor_key[i], neighbor_pos[i], i);
                if(mid1cache[i].count(neighbor_key[i]) != 0) mid1cache[i].erase(neighbor_key[i]);
            }
            else
            {
                
                std::cout << "000000000"<<std::endl;
            }
        }
        sub_flag = 0;
        sub_flag = m1[L-4].Search(key, node_key[L-3], node_pos[L-3], neighbor_key[L-3], neighbor_pos[L-3], flag);
        if(sub_flag == 0) brother[L-3] = true;
        else brother[L-3] = false;
        Readmid2Node(m2, node_key[L-3], node_pos[L-3]);
        if (mid2cache.count(node_key[L-3]) != 0) mid2cache.erase(node_key[L-3]);
        f[L-3] = flag;
        if(sub_flag == 0 || (sub_flag == -1 && flag == -1) || (sub_flag == 1 && flag == 1))
        {
            if(sub_flag == 0 && flag == 0)
            {
                neighbor_key[L-3] = node_key[L-3];
                neighbor_pos[L-3] = node_pos[L-3];
            }
            else
            {
                if(sub_flag == -1 && flag == -1)
                {
                    neighbor_key[L-3] = m1_neighbor[L-4].max_value;
                    neighbor_pos[L-3] = m1_neighbor[L-4].childMap[m1_neighbor[L-4].p2-1].pos;
                }
                else if(sub_flag == 1 && flag == 1)
                {
                    neighbor_key[L-3] = m1_neighbor[L-4].childMap[0].key;
                    neighbor_pos[L-3] = m1_neighbor[L-4].childMap[0].pos;
                }
                Readmid2Node(m2_neighbor, neighbor_key[L-3], neighbor_pos[L-3]);
                if(mid2cache.count(neighbor_key[L-3]) != 0) mid2cache.erase(neighbor_key[L-3]);
            }            
        }       
    }
    int sub_flag = m2.Search(key, node_key[L-2], node_pos[L-2], neighbor_key[L-2], neighbor_pos[L-2], flag);
    if(sub_flag == 0) brother[L-2] = true;
    else brother[L-2] = false;
    ReadleafNode(l, node_key[L-2], node_pos[L-2]);
    if(leafcache.count(node_key[L-2]) != 0) leafcache.erase(node_key[L-2]);
    f[L-2] = flag;
    if(sub_flag == 0 || (sub_flag == -1 && flag == -1) || (sub_flag == 1 && flag == 1))
    {
        if(sub_flag == 0 && flag == 0)
        {
            neighbor_key[L-3] = node_key[L-3];
            neighbor_pos[L-3] = node_pos[L-3];
        }
        else
        {
            if(sub_flag == -1 && flag == -1)
            {
                neighbor_key[L-2] = m2_neighbor.max_value;
                neighbor_pos[L-2] = m2_neighbor.childMap[m2_neighbor.p2-1].pos;
            }
            else if(sub_flag == 1 && flag == 1)
            {
                neighbor_key[L-2] = m2_neighbor.childMap[0].key;
                neighbor_pos[L-2] = m2_neighbor.childMap[0].pos;
            }
            ReadleafNode(l_neighbor, neighbor_key[L-2], neighbor_pos[L-2]);
            if(leafcache.count(neighbor_key[L-2]) != 0) leafcache.erase(neighbor_key[L-2]);
        }
    }

    
    int d_flag = l.Delete(l_neighbor, f[L-2], key);
    if(d_flag == -1) 
    {
        std::cout << "[bbbuggg]" << std::endl;
        return false;
    }
    if(d_flag == 0)
    {
        m2.Update(l);
        if(brother[L-2] == false && m2_neighbor.p2 > 0) m2_neighbor.Update(l_neighbor);
        else if (l_neighbor.c > 0 && brother[L-2] == true) m2.Update(l_neighbor);
        WriteleafNode(true, l.max_value, l);
        WriteleafNode(true, l_neighbor.max_value, l_neighbor);
        for(int i = L-4; i >= 0; i--)
        {
            if(i == L-4) 
            {
                m1[i].Update(m2);
                if(m1_neighbor[i].p2 > 0 && brother[L-3] == false) m1_neighbor[i].Update(m2_neighbor);
                else if(m2_neighbor.p2 >0 && brother[L-3] == true) m1[i].Update(m2_neighbor);
                Writemid2Node(true, m2.max_value, m2);
                Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
            }
            else
            {
                m1[i].Update(m1[i+1]);
                if(m1_neighbor[i].p2 > 0 && brother[i+1] == false) m1_neighbor[i].Update(m1_neighbor[i+1]);
                else if(m1_neighbor[i+1].p2 > 0 && brother[i+1] == true) m1[i].Update(m1_neighbor[i+1]);
                Writemid1Node(true, m1[i+1].max_value, m1[i+1], i+1);
                Writemid1Node(true, m1_neighbor[i+1].max_value, m1_neighbor[i+1], i+1);
            }
        }
        if(L == 3)
        {
            root.Update(m2);
            if(m2_neighbor.p2 > 0) root.Update(m2_neighbor);
            Writemid2Node(true, m2.max_value, m2);
            Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
        }
        else
        {
            root.Update(m1[0]);
            if(m1_neighbor[0].p2 > 0) root.Update(m1_neighbor[0]);
            Writemid1Node(true, m1[0].max_value, m1[0], 0);
            Writemid1Node(true, m1_neighbor[0].max_value, m1_neighbor[0], 0);
        }
    }
    if(d_flag == 1)
    {
        if(m2_neighbor.p2 > 0 && brother[L-2] == false) m2_neighbor.Update(l_neighbor);
        else if(l_neighbor.c > 0 && brother[L-2] == true) m2.Update(l_neighbor);
        int d_flag2 = m2.Delete(m2_neighbor, f[L-3], l.max_value);
        WriteleafNode(true, l_neighbor.max_value, l_neighbor);
        if(d_flag2 == 1)
        {           
            if(L == 3)
            {
                root.Update(m2_neighbor);
                root.Delete(m2.max_value);
                Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
            }
            else if(L > 3)
            {
                if(m1_neighbor[L-4].p2 > 0 && brother[L-3] == false) m1_neighbor[L-4].Update(m2_neighbor);
                else if(m2_neighbor.p2 > 0 && brother[L-3] == true) m1[L-4].Update(m2_neighbor);
                int d_flag3 = m1[L-4].Delete(m1_neighbor[L-4], f[L-4], m2.max_value);
                Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
                if(d_flag3 == 1)
                {
                    
                    
                    m1Delete(L-5, m1, m1_neighbor, f, brother);
                }
                else if(d_flag3 == 0)
                {
                    
                    for(int i = L-5; i >= 0; i--)
                    {                   
                        m1[i].Update(m1[i+1]);
                        if(m1_neighbor[i].p2 > 0 && brother[i+1] == false) m1_neighbor[i].Update(m1_neighbor[i+1]);
                        else if(m1_neighbor[i+1].p2 > 0 && brother[i+1]==true) m1[i].Update(m1_neighbor[i+1]);
                        
                        Writemid1Node(true, m1[i+1].max_value, m1[i+1], i+1);
                        Writemid1Node(true, m1_neighbor[i+1].max_value, m1_neighbor[i+1], i+1);
                    }
                    
                    root.Update(m1[0]);
                    if(m1_neighbor[0].p2 > 0) root.Update(m1_neighbor[0]);
                    Writemid1Node(true, m1[0].max_value, m1[0], 0);
                    Writemid1Node(true, m1_neighbor[0].max_value, m1_neighbor[0], 0);
                }
            }
        }
        else if(d_flag2 == 0)
        {
            for(int i = L-4; i >= 0; i--)
            {
                if(i == L-4)
                {
                    m1[i].Update(m2);
                    if(m1_neighbor[i].p2 > 0 && brother[i+1] == false) m1_neighbor[i].Update(m2_neighbor);
                    else if(m2_neighbor.p2 > 0 && brother[i+1] == true) m1[i].Update(m2_neighbor);
                    Writemid2Node(true, m2.max_value, m2);
                    Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
                }
                else
                {
                    m1[i].Update(m1[i+1]);
                    if(m1_neighbor[i].p2 > 0 && brother[i+1] == false) m1_neighbor[i].Update(m1_neighbor[i+1]);
                    else if(m1_neighbor[i+1].p2 > 0 && brother[i+1] == true) m1[i].Update(m1_neighbor[i+1]);
                    Writemid1Node(true, m1[i+1].max_value, m1[i+1], i+1);
                    Writemid1Node(true, m1_neighbor[i+1].max_value, m1_neighbor[i+1], i+1);
                }
                
            }
            if(L == 3)
            {
                root.Update(m2);
                if(m2_neighbor.p2 > 0) root.Update(m2_neighbor);
                Writemid2Node(true, m2.max_value, m2);
                Writemid2Node(true, m2_neighbor.max_value, m2_neighbor);
            }
            else
            {
                root.Update(m1[0]);
                if(m1_neighbor[0].p2 > 0) root.Update(m1_neighbor[0]);
                Writemid1Node(true, m1[0].max_value, m1[0], 0);
                Writemid1Node(true, m1_neighbor[0].max_value, m1_neighbor[0], 0);
            }
        }
    }
    return true;
}

void client::finish(bool find)
{
    
    
    if((L-3)>0)
    {
        for(int i=0;i<L-3;i++)
        {
            finalizemid1(find, i);
        }
    }
    finalizemid2(find);
    finalizeleaf(find);
    
    auto begin = std::chrono::high_resolution_clock::now();
    for (auto &t : leafcache) {
        if (t.second.c != 0) {
            leafNode& tmp = t.second;
            if (leafmodified.count(tmp.max_value)) {
                tmp.pos = RandomPath(2 * P1 * pow(P2,L-2));
            }
            #if debug_randompos
                cout << "leaf " << tmp.max_value << ": pos=" << tmp.pos << std::endl;
            #endif
        }
    }
    for(auto &t: mid2cache){
        if(t.second.p2 != 0){
            midNode2& tmp = t.second;
            if(mid2modified.count(tmp.max_value)){
                tmp.pos = RandomPath(2 * P1 * pow(P2,L-3));
            }
            #if debug_randompos
                cout << "mid2 " << tmp.max_value << ": pos=" << tmp.pos << std::endl;
            #endif
            for(int i = 0; i < tmp.p2; i++)
            {
                Bid temp_key = tmp.childMap[i].key;
                #if debug
                
                #endif
                if(leafcache.count(temp_key))
                {
                    tmp.childMap[i].pos = leafcache[temp_key].pos;
                }
            }
            
            
            
            
            
            
            
        }
    }
    if(L-3>0)
    {
        for(auto &t: mid1cache[L-4])
        {
            if(t.second.p2 != 0){
                midNode1& tmp = t.second;
                if(mid1modified[L-4].count(tmp.max_value)){
                    tmp.pos = RandomPath(2 * P1 * pow(P2,L-4));
                }
                #if debug_randompos
                    cout << "mid1 " << tmp.max_value << ": pos=" << tmp.pos << std::endl;
                #endif
                for(int i = 0; i < tmp.p2; i++)
                {
                    Bid tmp_key = tmp.childMap[i].key;
                    #if debug
                    
                    #endif
                    if(mid2cache.count(tmp_key))
                    {
                        tmp.childMap[i].pos = mid2cache[tmp_key].pos;
                    }
                }           
            }
        }
        for(int j=L-5;j>=0;j--)
        {
            for(auto &t: mid1cache[j])
            {
                if(t.second.p2 != 0){
                    midNode1& tmp = t.second;
                    if(mid1modified[j].count(tmp.max_value)){
                        tmp.pos = RandomPath(2 * P1 * pow(P2,j));
                    }
                    #if debug_randompos
                        cout << "*mid1 " << tmp.max_value << ": pos=" << tmp.pos << std::endl;
                    #endif
                    for(int i = 0; i < tmp.p2; i++)
                    {
                        Bid tmp_key = tmp.childMap[i].key;
                        #if debug
                            cout << setw(4);
                        #endif
                        if(mid1cache[j+1].count(tmp_key))
                        {
                            tmp.childMap[i].pos = mid1cache[j+1][tmp_key].pos;
                            
                        }
                    }
                    
                }
            }
        }
    }
    
    #if debug
    
    #endif
    if(L>3)
    {
        for(int i = 0 ; i < root.p1; i++)
        {
            Bid tmp_key = root.childMap[i].key;
            #if debug
            
            #endif
            if(mid1cache[0].count(tmp_key))
            {
                root.childMap[i].pos = mid1cache[0][tmp_key].pos;
            }
        }
    }
    else
    {
        for(int i = 0 ; i < root.p1; i++)
        {
            Bid tmp_key = root.childMap[i].key;
            #if debug
            
            #endif
            if(mid2cache.count(tmp_key))
            {
                root.childMap[i].pos = mid2cache[tmp_key].pos;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    mean_random_path_time += (end-begin)/insert_pairs;

    #if debug
    
    #endif
    WriteRootNode();
    if(L>3)
    {
        for(int i=0;i<L-3;i++)
        {
            finalize2mid1(i);
        }
    }
    #if debug
    
    #endif
    finalize2mid2();
    #if debug
    
    #endif
    finalize2leaf();
}

void client::Readmid1Node(midNode1 &m1, Bid bid, int leaf, int mid1L) {
    if (bid == 0) {
        return;
    }
    
    if (mid1cache[mid1L].count(bid) == 0) {
        
        
        Fetchmid1Path(leaf, mid1L);
        
        
        if (mid1cache[mid1L].find(bid) != mid1cache[mid1L].end())
        {
            m1 = mid1cache[mid1L][bid];
        }
        
        if (m1.p2 != 0) {
            m1.pos = leaf;
            if (mid1cache[mid1L].count(bid) != 0) {
                mid1cache[mid1L].erase(bid);
            }
            mid1cache[mid1L][bid] = m1;
            // if (find(leafList1[mid1L].begin(), leafList1[mid1L].end(), leaf) == leafList1[mid1L].end()) {
                leafList1[mid1L].insert(leaf);
            // }
        }
        
        if (m1.p2 != 0) {
            mid1modified[mid1L].insert(bid);
        }
    } 
    else 
    {
        mid1modified[mid1L].insert(bid);
        m1 = mid1cache[mid1L][bid];
        
    }
}

void client::Readmid2Node(midNode2 &node, Bid bid, int leaf) {
    if (bid == 0) {
        return;
    }
    if (mid2cache.count(bid) == 0) {
        Fetchmid2Path(leaf);
        
        
        if (mid2cache.find(bid) != mid2cache.end()) {
            node = mid2cache[bid];
        }
        
        if (node.p2 != 0) {
            node.pos = leaf;
            if (mid2cache.count(bid) != 0) {
                mid2cache.erase(bid);
            }
            mid2cache[bid] = node;
            // if (find(leafList2.begin(), leafList2.end(), leaf) == leafList2.end()) {
                leafList2.insert(leaf);
            // }
        }
        
        if (node.p2 != 0) {
            mid2modified.insert(bid);
        }
    } 
    else {
        mid2modified.insert(bid);
        node = mid2cache[bid];
        node.pos = leaf;
    }
}

void client::ReadleafNode(leafNode &node, Bid bid, int leaf) {
    if (bid == 0) {
        return;
    }
    if (leafcache.count(bid) == 0) {
        
        FetchleafPath(leaf);
        
        
        if (leafcache.find(bid) != leafcache.end()) {
            node = leafcache[bid];
        }
        
        if (node.c != 0) {
            node.pos = leaf;
            // if (leafcache.count(bid) != 0) {
            //     leafcache.erase(bid);
            // }
            leafcache[bid] = node;
            // if (find(leafList3.begin(), leafList3.end(), leaf) == leafList3.end()) {
                leafList3.insert(leaf);
            // }
        }
        
        if (node.c != 0) {
            leafmodified.insert(bid);
        }
    } else {
        leafmodified.insert(bid);
        node = leafcache[bid];
        node.pos = leaf;
    }
}

template <class Node>
Bucket client::DeserialiseBucket(block buffer) {
    int blockSize = sizeof(Node);

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

template <class Node>
Bucket client::DeserialiseBucket(block buffer, int mid1L) {
    int blockSize = sizeof(Node);

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node node(mid1L);
        convertBlockToNode<Node>(node, block.data, mid1L);
        block.id = node.max_value;
        
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

template <class Node>
void client::convert(Node &node, block b)
{
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, node);
}

template <class Node>
void client::convertBlockToNode(Node &node, block b) {
    
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, node);
    return;
}

template <class Node>
void client::convertBlockToNode(Node &node, block b, int mid1L) {
    
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, node);
    return;
}

template <class Node>
block client::convertNodeToBlock(Node &node)
{
    
    
    
    
    
    
    
    
    std::array<byte_t, sizeof (Node) > data = to_bytes(node);
    block b(data.begin(), data.end());
    return b;
}

void client::WriteRootNode()
{
    block b = convertNodeToBlock(root);
    block rootcipher = AES::Encrypt(key1[L-1], b, AES::GetCiphertextLength(sizeof(rootNode)), sizeof(rootNode));
    
    
    
    
    
    string buffer(rootcipher.begin(), rootcipher.end());
    std::vector<std::string> buffer_vec;
    buffer_vec.push_back(buffer);
    std::vector<int32_t> p;
    p.push_back(0);
    auto begin = std::chrono::high_resolution_clock::now();
    write_bucket(buffer_vec, p, L-1);
    auto end = std::chrono::high_resolution_clock::now();
    write_communication_time += end - begin;
    write_communication_size += buffer.length();      
}

int client::Writemid1Node(bool incache, Bid bid, midNode1 &node, int mid1L) {
    if (bid == 0) {
        return -1;
        
    }
    if (!incache) {
        mid1modified[mid1L].insert(bid);
        
        
        
        Fetchmid1Path(node.pos, mid1L);
        
        mid1cache[mid1L][bid] = node;
        
        // if (find(leafList1[mid1L].begin(), leafList1[mid1L].end(), node.pos) == leafList1[mid1L].end()) {
            leafList1[mid1L].insert(node.pos);
        // }
        
        return node.pos;
    } 
    else {
        mid1modified[mid1L].insert(bid);
        mid1cache[mid1L][bid] = node;
        return node.pos;
    }
}

int client::Writemid2Node(bool incache, Bid bid, midNode2 &node) {
    if (bid == 0) {
        return -1;
        
    }
    if (!incache) {
        mid2modified.insert(bid);
        
        
        Fetchmid2Path(node.pos);
        
        mid2cache[bid] = node;
        
        // if (find(leafList2.begin(), leafList2.end(), node.pos) == leafList2.end()) {
            leafList2.insert(node.pos);
        // }
        return node.pos;
    } else {
        mid2modified.insert(bid);
        mid2cache[bid] = node;
        return node.pos;
    }
}

int client::WriteleafNode(bool incache, Bid bid, leafNode &node) {
    if (bid == 0) {
        return -1;
        
    }
    if (!incache) {
        leafmodified.insert(bid);
        
        
        FetchleafPath(node.pos);
        
        leafcache[bid] = node;
        
        // if (find(leafList3.begin(), leafList3.end(), node.pos) == leafList3.end()) {
            leafList3.insert(node.pos);
        // }
        return node.pos;
    } else {
        leafmodified.insert(bid);
        leafcache[bid] = node;
        return node.pos;
    }
}

void client::Fetchmid1Path(int leaf, int mid1L) 
{
    readCntmid1[mid1L]++;
    
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = read_bucket(leaf, mid1L + 2);
    auto end = std::chrono::high_resolution_clock::now();
    read_communication_time += end - begin;
    read_communication_size += reply.length();
    cout << "mid1:" << reply.length() << "\n";

    #if debug 
    std::cout << "[mid1]send and fetch mid1path:" << leaf << ", mid1L= " << mid1L << std::endl;
    #endif
    
    if(reply == "rpc failed")
    {
        std::cout << "ERROR " + reply << std::endl;
        return;
    }
    int len = reply.length();
    if(len == 0) {
        #if debug
        std::cout << "[mid1]receive NONE." << std::endl;
        #endif
        return;
    }
    #if debug
    cout << "[mid1]receive mid1oram bits:" << len << std::endl;
    #endif
    // auto be = std::chrono::high_resolution_clock::now();
    int i = 0;
    int blockSize = sizeof(midNode1);
    int clen = AES::GetCiphertextLength(blockSize*Z);
    while (i * (clen + 16) < len)
    {
        
        
        block ciphertext;
        ciphertext.insert(ciphertext.end(), reply.begin() + i * (clen + 16), reply.begin() + (i + 1) * (clen + 16));
        auto be = std::chrono::high_resolution_clock::now();
        block buffer = AES::Decrypt(key1[mid1L+2], ciphertext, clen);
        // std::cout << ciphertext.size() <<" " << clen << " " << buffer.size() << "\n";
        auto en = std::chrono::high_resolution_clock::now();
        dec_time += en-be;
        // Bucket bucket = DeserialiseBucket<midNode1>(buffer, mid1L);
        Bucket bucket;

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];
            block.data.assign(buffer.begin()+z*blockSize, buffer.begin() + (z+1)*blockSize);
            midNode1 node(mid1L);
            convertBlockToNode<midNode1>(node, block.data, mid1L);
            block.id = node.max_value;           
            // buffer.erase(buffer.begin(), buffer.begin() + blockSize);
            if(block.id != 0)
            {
                // if (mid1cache[mid1L].count(block.id) == 0) {
                    mid1cache[mid1L][block.id] = node;
                    
                // } else {
                    
                // }
            }
        }
        auto en2 = std::chrono::high_resolution_clock::now();
        deserial_time += en2-en;
        i++;
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // deserial_dec_time += en-be;
}

void client::Fetchmid2Path(int leaf)
{
    readCntmid2++;
    
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = read_bucket(leaf, 1);
    auto end = std::chrono::high_resolution_clock::now();
    read_communication_time += end - begin;
    read_communication_size += reply.length();
    cout << "mid2:" << reply.length() << "\n";

    #if debug
        cout << "[mid2]send and fetch mid2path:" << leaf << std::endl; 
    #endif
    
    if(reply == "rpc failed")
    {
        std::cout << "ERROR " + reply << std::endl;
        return;
    }
    int len = reply.length();
    if(len == 0) {
        #if debug
        std::cout << "[mid2]receive NONE." << std::endl;
        #endif
        return;
    }
    #if debug
    cout << "[mid2]receive mid2oram bits:" << len << std::endl;
    #endif
    // auto be = std::chrono::high_resolution_clock::now();
    int i = 0;
    int blockSize = sizeof(midNode2);
    int clen = AES::GetCiphertextLength(blockSize*Z);
    while (i * (clen + 16) < len)
    {
        
        
        block ciphertext;
        ciphertext.insert(ciphertext.end(), reply.begin() + i * (clen + 16), reply.begin() + (i + 1) * (clen + 16));
        auto be = std::chrono::high_resolution_clock::now();
        block buffer = AES::Decrypt(key1[1], ciphertext, clen);
        // std::cout << ciphertext.size() <<" " << clen << " " << buffer.size() << "\n";
        auto en = std::chrono::high_resolution_clock::now();
        dec_time += en-be;
        // Bucket bucket = DeserialiseBucket<midNode2>(buffer);
        Bucket bucket;

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];
            block.data.assign(buffer.begin()+z*blockSize, buffer.begin() + (z+1)*blockSize);
            midNode2 node;
            convertBlockToNode<midNode2>(node, block.data);
            block.id = node.max_value;           
            // buffer.erase(buffer.begin(), buffer.begin() + blockSize);
            if(block.id != 0)
            {
                // if (mid2cache.count(block.id) == 0) {
                    mid2cache[block.id] = node;
                    
                // } else {
                    
                // }
            }
        }
        auto en2 = std::chrono::high_resolution_clock::now();
        deserial_time += en2-en;
        i++;
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // deserial_dec_time += en-be;
}

void client::FetchleafPath(int leaf)
{
    readCntleaf++;
    if(readCntleaf > 2)
    {
        cout << "read overflow!!!";
    }
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = read_bucket(leaf, 0);
    auto end = std::chrono::high_resolution_clock::now();
    read_communication_time += end - begin;
    read_communication_size += reply.length();
    cout << "leaf:" << reply.length() << "\n";

    #if debug
    cout << "[leaf]send and fetch leafpath:" << leaf << std::endl; 
    #endif
    if(reply == "rpc failed")
    {
        std::cout << "ERROR " + reply << std::endl;
        return;
    }
    size_t len = reply.length();
    if(len == 0) {
        #if debug
        std::cout << "[leaf]receive NONE." << std::endl;
        #endif
        return;
    }
    #if debug
        cout << "[leaf]receive leaforam bits:" << len << std::endl;
    #endif
    // auto be = std::chrono::high_resolution_clock::now();
    size_t i = 0;
    size_t blockSize = sizeof(leafNode);
    size_t clen = AES::GetCiphertextLength(blockSize*Z);
    while (i * (clen + 16) < len)
    {
        
        
        block ciphertext;
        ciphertext.insert(ciphertext.end(), reply.begin() + i * (clen + 16), reply.begin() + (i + 1) * (clen + 16));
        auto be = std::chrono::high_resolution_clock::now();
        block buffer = AES::Decrypt(key1[0], ciphertext, clen);
        auto en = std::chrono::high_resolution_clock::now();
        dec_time += en- be;
        // Bucket bucket = DeserialiseBucket<leafNode>(buffer);
        Bucket bucket;

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];
            block.data.assign(buffer.begin()+z*blockSize, buffer.begin() + (z+1)*blockSize);
            leafNode node;
            convertBlockToNode<leafNode>(node, block.data);
            block.id = node.max_value;           
            // buffer.erase(buffer.begin(), buffer.begin() + blockSize);
            if(block.id != 0)
            {
                // if (leafcache.count(block.id) == 0) {
                    leafcache[block.id] = node;
                    
                // } else {
                    
                // }
            }
        }
        auto en2 = std::chrono::high_resolution_clock::now();
        deserial_time += en2-en;
        i++;
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // deserial_dec_time += en- be;
}

std::string client::Writemid1Path(int leaf, int d, int mid1L)
{
    int node = GetNodeOnPath(2 * P1 *  pow(P2,mid1L), leaf, d);
    if (find(writeviewmap1[mid1L].begin(), writeviewmap1[mid1L].end(), node) == writeviewmap1[mid1L].end()) {

        auto validBlocks = GetIntersectingBlocks1(leaf, d, mid1L);
        auto be0 = std::chrono::high_resolution_clock::now();
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            midNode1 curnode = mid1cache[mid1L][block.id];
            block.data = convertNodeToBlock<midNode1>(curnode);
            
            mid1cache[mid1L].erase(block.id);
        }
        
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(sizeof (midNode1), 0);
        }

        
        writeviewmap1[mid1L].insert(node);
        #if debug
        std::cout << "[mid1]plan to write bucket to store: " << node << std::endl;
        #endif
        
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(midNode1) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        auto be = std::chrono::high_resolution_clock::now();
        block ciphertext = AES::Encrypt(key1[mid1L+2], b, clen_size, plaintext_size);
        // std::cout << ciphertext.size() <<" " << clen_size << " " << plaintext_size << "\n";
        auto en = std::chrono::high_resolution_clock::now();
        enc_time += en-be;
        serial_time += be - be0;
        std::string buffer(ciphertext.begin(), ciphertext.end());
        return buffer;
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    else return "";
}

std::string client::Writemid2Path(int leaf, int d)
{
    int node = GetNodeOnPath(2 * P1 * pow(P2,L-3), leaf, d);
    
    if (find(writeviewmap2.begin(), writeviewmap2.end(), node) == writeviewmap2.end()) {
        
        auto validBlocks = GetIntersectingBlocks2(leaf, d);
        auto be0 = std::chrono::high_resolution_clock::now();
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            midNode2 curnode = mid2cache[block.id];
            block.data = convertNodeToBlock<midNode2>(curnode);
            
            mid2cache.erase(block.id);
        }
        
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(sizeof(midNode2), 0);
        }

        
        writeviewmap2.insert(node);
        #if debug
        std::cout << "[mid2]plan to write bucket to store: " << node << std::endl;
        #endif
        
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(midNode2) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        auto be = std::chrono::high_resolution_clock::now();
        block ciphertext = AES::Encrypt(key1[1], b, clen_size, plaintext_size);
        // std::cout << ciphertext.size() <<" " << clen_size << " " << plaintext_size << "\n";
        auto en = std::chrono::high_resolution_clock::now();
        enc_time += en-be;
        serial_time += be-be0;
        std::string buffer(ciphertext.begin(), ciphertext.end());
        return buffer;
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    else return "";
}

std::string client::WriteleafPath(int leaf, int d)
{
    int node = GetNodeOnPath(2 * P1 * pow(P2,L-2), leaf, d);
    
    if (find(writeviewmap3.begin(), writeviewmap3.end(), node) == writeviewmap3.end()) {

        auto validBlocks = GetIntersectingBlocks3(leaf, d);
        auto be0 = std::chrono::high_resolution_clock::now();
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            leafNode curnode = leafcache[block.id];
            block.data = convertNodeToBlock<leafNode>(curnode);
            
            leafcache.erase(block.id);
            
        }
        
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(sizeof(leafNode), 0);
        }

        
        writeviewmap3.insert(node);
        #if debug
        std::cout << "[leaf]plan to write bucket to store: " << node << std::endl;
        #endif
        
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(leafNode) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        auto be = std::chrono::high_resolution_clock::now();
        block ciphertext = AES::Encrypt(key1[0], b, clen_size, plaintext_size);
        auto en = std::chrono::high_resolution_clock::now();
        enc_time += en-be;
        serial_time += be - be0;
        std::string buffer(ciphertext.begin(), ciphertext.end());
        return buffer;
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    else return "";
}

void client::finalizemid1(bool find, int mid1L)
{
    if (find) {
        for (unsigned int i = readCntmid1[mid1L]; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,mid1L));
            // if (std::find(leafList1[mid1L].begin(), leafList1[mid1L].end(), rnd) == leafList1[mid1L].end()) {
                leafList1[mid1L].insert(rnd);
            // }
            
            Fetchmid1Path(rnd, mid1L);
        }
    } else {
        for (int i = readCntmid1[mid1L]; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,mid1L));
            
            // if (std::find(leafList1[mid1L].begin(), leafList1[mid1L].end(), rnd) == leafList1[mid1L].end()) {
                leafList1[mid1L].insert(rnd);
            // }
            
            Fetchmid1Path(rnd, mid1L);
        }
    }
}

void client::finalizemid2(bool find)
{
    if (find) {
        for (unsigned int i = readCntmid2; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,L-3));
            // if (std::find(leafList2.begin(), leafList2.end(), rnd) == leafList2.end()) {
                leafList2.insert(rnd);
            // }
            Fetchmid2Path(rnd);
        }
    } else {
        for (int i = readCntmid2; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,L-3));
            // if (std::find(leafList2.begin(), leafList2.end(), rnd) == leafList2.end()) {
                leafList2.insert(rnd);
            // }
            Fetchmid2Path(rnd);
        }
    }
}

void client::finalizeleaf(bool find)
{
    if (find) {
        for (unsigned int i = readCntleaf; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,L-2));
            // if (std::find(leafList3.begin(), leafList3.end(), rnd) == leafList3.end()) {
                leafList3.insert(rnd);
            // }
            FetchleafPath(rnd);
        }
    } else {
        for (int i = readCntleaf; i < 2; i++) {
            int rnd = RandomPath(2 * P1 * pow(P2,L-2));
            // if (std::find(leafList3.begin(), leafList3.end(), rnd) == leafList3.end()) {
                leafList3.insert(rnd);
            // }
            FetchleafPath(rnd);
        }
    }
}

int client::RandomPath(int maxsize) {
    decltype(dis)::param_type params{0, (pow(2, floor(log2(maxsize / Z))) - 1)/2};
    
    dis.param(params);
    int val = dis(mt);
    
    return val;
}

int client::leafRandomPos()
{
    int32_t maxsize = 2 * P1*pow(P2,(L-2));
    decltype(dis)::param_type params{0, (pow(2, floor(log2(maxsize / Z))) - 1)/2};
    
    dis.param(params);
    int val = dis(mt);
    
    return val;
}
int client::mid2RandomPos()
{
    int32_t maxsize = 2 * P1*pow(P2,(L-3));
    decltype(dis)::param_type params{0, (pow(2, floor(log2(maxsize / Z))) - 1)/2};
    
    dis.param(params);
    int val = dis(mt);
    
    return val;
}
int client::mid1RandomPos(int mid1L)
{
    int32_t maxsize = 2 * P1 * pow(P2,mid1L);
    decltype(dis)::param_type params{0, (pow(2, int(floor(log2(maxsize / Z)))) - 1)/2};
    
    dis.param(params);
    int val = dis(mt);
    
    return val;
}

int client::GetNodeOnPath(int maxSize, int leaf, int curDepth) {
    int depth = floor(log2(maxSize / Z));
    int bucketCount = pow(2, depth + 1) - 1;
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

block client::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    

    return buffer;
}

void client::finalize2mid1(int mid1L)
{
    // auto be = std::chrono::high_resolution_clock::now();
    int depth = floor(log2(2 * P1 *pow(P2,mid1L)/ Z));
    int cnt = 0;
    std::vector<std::string> buffer_vec;
    std::vector<int32_t> position_vec;
    for (int d = depth; d >= 0; d--) {
        // for (unsigned int i = 0; i < leafList1[mid1L].size(); i++) {
        for (auto &leaf: leafList1[mid1L]){
            cnt++;
            
            
            
            std::string buffer = Writemid1Path(leaf, d, mid1L);
            int p = GetNodeOnPath(2 * P1 *  pow(P2,mid1L), leaf, d);
            if(buffer != "")
            {
                buffer_vec.push_back(buffer);
                position_vec.push_back(p);
            }
        }
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // serial_enc_time += en-be;
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = write_bucket(buffer_vec, position_vec, mid1L+2); 
    auto end = std::chrono::high_resolution_clock::now();
    write_communication_time += end - begin;
    for(int i = 0; i < buffer_vec.size(); i++)
    {
        write_communication_size += buffer_vec[i].length();
    }
    
    if(reply == "ok") 
    {
        #if debug
        std::cout << "[mid1]Send Success!!!" << std::endl;
        #endif
    }
    else std::cout << "[mid1]Send WRONG!" << std::endl;
    leafList1[mid1L].clear();
    mid1modified[mid1L].clear();
}

void client::finalize2mid2()
{
    // auto be = std::chrono::high_resolution_clock::now();
    int depth = floor(log2(2 * P1 * pow(P2,L-3) / Z));
    int cnt = 0;
    std::vector<std::string> buffer_vec;
    std::vector<int32_t> position_vec;
    for (int d = depth; d >= 0; d--) {
        // for (unsigned int i = 0; i < leafList2.size(); i++) {
        for (auto &leaf: leafList2){
            cnt++;
            
            
            
            std::string buffer = Writemid2Path(leaf, d);
            int p = GetNodeOnPath(2 * P1 * pow(P2,L-3), leaf, d);
            if(buffer != "")
            {
                buffer_vec.push_back(buffer);
                position_vec.push_back(p);
            }
        }
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // serial_enc_time += en-be;
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = write_bucket(buffer_vec, position_vec, 1);
    auto end = std::chrono::high_resolution_clock::now();
    write_communication_time += end - begin;
    for(int i = 0; i < buffer_vec.size(); i++)
    {
        write_communication_size += buffer_vec[i].length();
    }
    
    if(reply == "ok") 
    {
        #if debug
        std::cout << "[mid1]Send Success!!!" << std::endl;
        #endif
    }
    else std::cout << "[mid1]Send WRONG!" << std::endl;
    leafList2.clear();
    mid2modified.clear();
}

void client::finalize2leaf()
{
    // auto be = std::chrono::high_resolution_clock::now();
    int depth = floor(log2(2 * P1 * pow(P2,L-2)  / Z));
    int cnt = 0;
    std::vector<std::string> buffer_vec;
    std::vector<int32_t> position_vec;
    for (int d = depth; d >= 0; d--) {
        // for (unsigned int i = 0; i < leafList3.size(); i++) {
        for(auto &leaf: leafList3){
            cnt++;
            
            
            
            std::string buffer = WriteleafPath(leaf, d);
            int p = GetNodeOnPath(2 * P1 * pow(P2,L-2), leaf, d);
            if(buffer != "")
            {
                buffer_vec.push_back(buffer);
                position_vec.push_back(p);
            }
        }
    }
    // auto en = std::chrono::high_resolution_clock::now();
    // serial_enc_time += en - be;
    auto begin = std::chrono::high_resolution_clock::now();
    std::string reply = write_bucket(buffer_vec, position_vec, 0);
    auto end = std::chrono::high_resolution_clock::now();
    write_communication_time += end - begin;
    for(int i = 0; i < buffer_vec.size(); i++)
    {
        write_communication_size += buffer_vec[i].length();
    }

    if(reply == "ok") 
    {
        #if debug
        std::cout << "[mid1]Send Success!!!" << std::endl;
        #endif
    }
    else std::cout << "[mid1]Send WRONG!" << std::endl;
    leafList3.clear();
    leafmodified.clear();
}

std::vector<Bid> client::GetIntersectingBlocks1(int x, int curDepth,int mid1L) {
    std::vector<Bid> validBlocks;

    int node = GetNodeOnPath(2 * P1 * pow(P2,mid1L), x, curDepth);
    for (auto b : mid1cache[mid1L]) {
        Bid bid = b.first;
        if (b.second.p2 != 0 && GetNodeOnPath(2 * P1 * pow(P2,mid1L), b.second.pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

std::vector<Bid> client::GetIntersectingBlocks2(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    int node = GetNodeOnPath(2 * P1 * pow(P2,L-3), x, curDepth);
    for (auto b : mid2cache) {
        Bid bid = b.first;
        if (b.second.p2 != 0 && GetNodeOnPath(2 * P1 * pow(P2,L-3), b.second.pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

std::vector<Bid> client::GetIntersectingBlocks3(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    int node = GetNodeOnPath(2 * P1 * pow(P2,L-2), x, curDepth);
    for (auto b : leafcache) {
        Bid bid = b.first;
        if (b.second.c != 0 && GetNodeOnPath(2 * P1 * pow(P2,L-2), b.second.pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

std::string client::init_dummy(std::string buf, int32_t p, int32_t oram_index)
{
    BucketMessage request;
    request.set_buffer(buf);
    request.set_position(p);
    request.set_oram_index(oram_index);
    google::protobuf::Empty e;
    ClientContext context;
    Status status = stub_->init_dummy(&context, request, &e);

    if(status.ok()) {
        return "ok";
    } else {
        std::cout << status.error_code() << ": " <<status.error_message() << std::endl;
        return "RPC failed";
    }
}


std::string client::read_bucket(const int32_t path, const int32_t oram_index) {
    
    BucketReadMessage request;
    request.set_path(path);
    request.set_oram_index(oram_index);

    
    BucketReadResponse reply;

    
    
    ClientContext context;

    
    Status status = stub_->read_bucket(&context, request, &reply);

    
    if (status.ok()) {
      return reply.buffer();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
}

std::string client::write_bucket(std::vector<std::string> buf_vec,std::vector<int32_t> p, int32_t oram_index) {
    BucketWriteMessage request;
    for(unsigned int i = 0; i < buf_vec.size(); i++)
    {
        request.add_buffer(buf_vec[i]);
        request.add_position(p[i]);
    }
    
    request.set_oram_index(oram_index);
    request.set_num(buf_vec.size());
    google::protobuf::Empty e;
    ClientContext context;
    Status status = stub_->write_bucket(&context, request, &e);

    if(status.ok()) {
        return "ok";
    } else {
        std::cout << status.error_code() << ": " <<status.error_message() << std::endl;
        return "RPC failed";
    }
}

std::string client::end_signal(std::string msg) {
    endMessage request;
    request.set_end(msg);
    google::protobuf::Empty e;
    ClientContext context;
    Status status = stub_->end_signal(&context, request, &e);

    if(status.ok()) {
        return "ok";
    } else {
        std::cout << status.error_code() << ": " <<status.error_message() << std::endl;
        return "RPC failed";
    }
}

std::string client::Setup(const int32_t level, const int32_t maxsize, const int32_t oram_index) {
    SetupRequest request;
    request.set_level(level);
    request.set_maxsize(maxsize);
    request.set_oramindex(oram_index);
    request.set_is_find(is_access);
    SetupResponse e;
    
    ClientContext context;
    Status status = stub_->Setup(&context, request, &e);
    if(status.ok())
    {
        return "ok";
    }
    else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "rpc failed";
    }
}
