#include "node.hpp"

// const int C = 8, P1 = 10, P2 = 10;


// int RandomPos(int maxsize)
// {
// 	std::random_device rd;
// 	std::mt19937 mt(rd());
// 	std::uniform_int_distribution<int> dis(0,pow(2, floor(log2(maxsize / Z))) - 1);
// 	int val = dis(mt);
// 	return val;
// }


bool kppair::operator<(const kppair& b)const
{
	return key < b.key;
}

bool kppair::operator<=(const kppair& b)const
{
	return key <= b.key;
}

bool kppair::operator>(const kppair& b)const
{
	return key > b.key;
}

bool kppair::operator>=(const kppair& b)const
{
	return key >= b.key;
}

bool kppair::operator==(const kppair rhs)const
{
	return key == rhs.key;
}

bool kppair_sort_rule(kppair &k1, kppair& k2)
{
	return k1.key < k2.key;
}

leafNode::leafNode()
{
	c = 0; 
	//pos = RandomPos(P1*pow(P2,(4-2)));
	// pos = RandomPos(2 * P1*pow(P2,(L-2)));
}

leafNode::leafNode(Bid k, int v)
{
	kvpair kv(k,v);
	arr[0] = kv;
	c = 1;
	max_value = k;
	// pos = RandomPos(2 * P1 * pow(P2,(L-2)));
}

leafNode::leafNode(const leafNode &l)
{
	this->arr = l.arr;
	c = l.c;
	this->max_value = l.max_value;
	old_max = l.old_max;
	pos = l.pos;
}

void leafNode::Insert(leafNode &newNode, Bid key, int value){
    kvpair insert_kvpair(key, value);
    if(this->c < C)
    {
        int i = 0;
		for(; i < c; i++)
        {
            if(arr[i].key == key){
                arr[i].value = value;
                return;
            }
            if(arr[i].key > key) break;
        }
		if(i == c) arr[i] = insert_kvpair;
		else
		{
			for(int j = c; j>= i; j--)
			{
				if(j == i) arr[j] = insert_kvpair;
				else arr[j] = arr[j-1];
			}
		}
        c++;
		old_max = max_value;
        max_value = arr[c-1].key;
        return;
    }
    else{
		int count = 0; //记录遍历了多少个元素
		bool written = false;
		int half = (C + 1)/2;
        std::array<kvpair, C+1> full;
        for(int i = 0; i < c; i++)
        {
			if(arr[i].key < key)
			{
				full[count++] = arr[i];
				continue;
			}
			else if(arr[i].key == key){
				arr[i].value = value;
				return;
			}
            else if(arr[i].key > key)
			{
				if(!written)
				{
					full[count++] = insert_kvpair;
					written = true;
				}
				full[count++] = arr[i];
			}
        }
		if(!written) 
			full[count++] = insert_kvpair;
		kvpair kkvv;
		arr.fill(kkvv);

        memcpy(arr.data(), full.data(), half * sizeof(kvpair));
		memcpy(newNode.arr.data(), full.data() + half, (C+1-half)*sizeof(kvpair));
		c = half;
		newNode.c = C + 1 - half;
		old_max = max_value;
		max_value = arr[c-1].key;
		newNode.max_value = newNode.arr[newNode.c-1].key;
    }
}

void leafNode::Search(kvpair &l_node, Bid key)
{
    for(int i = 0; i < c; i++)
    {
        if(arr[i].key == key)
		{
			l_node = arr[i];
			return; 
		}
            
    }
    return;
}

int leafNode::Delete(leafNode &neighbor, int flag, Bid key)
{
	int i = 0;
	for(; i < c; i++)
	{
		if(arr[i].key == key) break;
	}
	if(i == c) return -1;
	if(i == c - 1)
	{
		kvpair kv;
		arr[i] = kv;
	}
	else
	{
		for(int j = i; j < c - 1; j++)
		{
			arr[j] = arr[j+1];
		}
		arr[c-1] = kvpair();
	}
	
	if(c <= (C+1)/2 && neighbor.c != 0)
	{
		if (flag == -1)  // left
		{
			if(neighbor.c + this->c - 1 <= C)
			{
				std::copy(arr.begin(), arr.begin()+(c-1), neighbor.arr.begin()+neighbor.c);
				neighbor.c += c - 1;
				neighbor.old_max = neighbor.max_value;
				neighbor.max_value = neighbor.arr[neighbor.c - 1].key;
				c = 0;
				return 1;
			}
			else //rearrange
			{
				kvpair kvp_temp = neighbor.arr[neighbor.c - 1];
				neighbor.arr[neighbor.c - 1] = kvpair();
				neighbor.c--;
				neighbor.old_max = neighbor.max_value;
				neighbor.max_value = neighbor.arr[neighbor.c - 1].key;
				// insert
				for(int i = c; i > 0; i--)
				{
					arr[i] = arr[i-1];
				}
				arr[0] = kvp_temp;
				old_max = max_value;
				max_value = arr[c-1].key;
				return 0;
			}
		}
		else if (flag == 1)
		{
			if(neighbor.c + this->c - 1 <= C)
			{
				for(int i = neighbor.c + this->c - 2; i >= c - 1; i--)
				{
					neighbor.arr[i] = neighbor.arr[i - c + 1];
				}
				for(int i = c - 2; i >= 0; i--)
				{
					neighbor.arr[i] = arr[i];
				}
				neighbor.c += c-1;
				c = 0;
				return 1;
			}
			else
			{
				kvpair kvp_temp = neighbor.arr[0];
				for(int i = 0; i < neighbor.c - 1; i++)
				{
					neighbor.arr[i] = neighbor.arr[i+1];
				}
				neighbor.arr[neighbor.c - 1] = kvpair();
				neighbor.c--;
				arr[c-1] = kvp_temp;
				old_max = max_value;
				max_value = kvp_temp.key;
				return 0;
			}
		}
		else //flag == 0
		{
			this->c--;
			this->old_max = max_value;
			this->max_value = this->arr[c-1].key;
			return 0;
		}	
	}
	else // 直接删除
	{
		this->c--;
		this->old_max = max_value;
		this->max_value = this->arr[c-1].key;
		return 0;
	}
}

midNode2::midNode2()
{
	p2 = 0; 
	// pos = RandomPos(2 * P1*pow(P2,(L-3))); 
}

midNode2::midNode2(const midNode2 & mid2)
{
	this->childMap = mid2.childMap;
	p2 = mid2.p2;
	max_value = mid2.max_value;
	old_max = mid2.old_max;
	pos = mid2.pos;
}

void midNode2::Insert(midNode2 &mid2, leafNode &leaf)
{
	// std::cout << "----midNode2-Insert()----" << std::endl;
	kppair kp(leaf.max_value, leaf.pos);
    if (this->p2 < P2)
	{
		int i = 0;
		for (; i < p2; i++)
		{
			if(childMap[i].key > leaf.max_value)
			{
				break;
			}
		}
		if(i == p2) childMap[i] = kp;
		else{
			for(int j = p2; j >= i; j--)
			{
				if(j == i) childMap[j] = kp;
				else childMap[j] = childMap[j-1];
			}
		}
		this->p2++;
		leaf.old_max = leaf.max_value;
		//this->old_max = this->max_value;
		max_value = childMap[p2 - 1].key;

		// for(std::vector<kppair*>::iterator iter = childMap.begin(); iter != childMap.end(); iter++)
		// {
		// 	if(*iter != NULL)
		// 		std::cout << (*iter)->key << ":" << (*iter)->pos << " ";
		// }
		// std::cout << std::endl;
		return;
	}
	else
	{
		int count = 0;
		bool written = false;
		int half = (P2 + 1) / 2;
		std::array<kppair, P2 + 1> full;
		for(int i = 0; i < p2; i++)
		{
			if(childMap[i].key < kp.key)
			{
				full[count++] = childMap[i];
				continue;
			}
			else
			{
				if(!written)
				{
					full[count++] = kp;
					written = true;
				}
				full[count++] = childMap[i];
			}
		}
		if(!written)
		{
			full[count++] = kp;
		}
		leaf.old_max = leaf.max_value;
		//this->old_max = this->max_value;
		kppair kkpp;
		childMap.fill(kkpp);
		memcpy(childMap.data(), full.data(), half * sizeof(kppair));
		memcpy(mid2.childMap.data(), full.data() + half, (P2+1-half)*sizeof(kppair));
		p2 = half;
		mid2.p2 = P2+1-half;
		max_value = childMap[p2 - 1].key;
		mid2.max_value = mid2.childMap[mid2.p2 - 1].key; 
		// for(std::vector<kppair*>::iterator iter = childMap.begin(); iter != childMap.end(); iter++)
		// {
		// 	if(*iter != NULL)
		// 		std::cout << (*iter)->key << ":" << (*iter)->pos << " ";
		// }
		// std::cout << std::endl;
		// for(std::vector<kppair*>::iterator iter = newNode->childMap.begin(); iter != newNode->childMap.end(); iter++)
		// {
		// 	if(*iter != NULL)
		// 		std::cout << (*iter)->key << ":" << (*iter)->pos << " ";
		// }
		// std::cout << std::endl;
		//std::map<Bid, int>().swap(this->my_map);
		//malloc_trim(0);
		//this->my_map = copy;
		return;
	}
}


void midNode2::Update(leafNode &l)
{
    //std::map<Bid, int>::iterator it = this->my_map.begin();
	//std::cout<<my_map.begin()->second<<std::endl;
	// for(std::vector<kppair*>::iterator iter = childMap.begin(); iter != childMap.end(); iter++)
	// {
	// 	if(*iter != NULL)
	// 		std::cout << (*iter)->key << " " << (*iter)->pos << std::endl;
	// }
		/*
	while (it != this->my_map.end())
	{
		//if (it->first == l->old_max)
		{
			std::cout << "hi";
			//this->my_map.erase(it->first);
			//break;
		}
		it++;
	}	*/
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key == l.old_max)
		{
			childMap[i].key = l.max_value;
			break;
		}
	}
	if(l.max_value != l.old_max) sort(childMap.begin(), childMap.begin() + p2, kppair_sort_rule);
	l.old_max = l.max_value;
	//this->old_max = this->max_value;
	this->max_value = childMap[p2 - 1].key;
	return;
}




int midNode2::Search(Bid key, Bid& l_key)
{
	for(int i = 0; i < p2; i++)
	{
		if(childMap[i].key >= key)
		{
			l_key = childMap[i].key;
			return childMap[i].pos;
		}
	}
	l_key = max_value;
	return childMap[p2 - 1].pos;
}

int midNode2::Search(Bid key, Bid& l_key, int& l_pos, Bid& neighbor, int& n_pos, int& up_right)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key >= key)
		{
			l_key = childMap[i].key;
			l_pos = childMap[i].pos;
			break;
		}
	}
	if(i == p2)
	{
		l_key = max_value;
		l_pos = childMap[p2 - 1].pos;
		i--;
	}
	if(up_right == -1)
	{
		if(i > 0)
		{
			neighbor = childMap[i-1].key;
			n_pos = childMap[i-1].pos;
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if(up_right == 1)
	{
		if(i < p2 - 1)
		{
			neighbor = childMap[i+1].key;
			n_pos = childMap[i+1].pos;
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else //up_right == 0,如果可以 要设置up_right
	{
		if(p2 != 1)
		{
			if(i == 0)
			{
				up_right = 1;
				neighbor = childMap[i+1].key;
				n_pos = childMap[i+1].pos;
			}
			else
			{
				up_right = -1;
				neighbor = childMap[i-1].key;
				n_pos = childMap[i-1].pos;
			}
		}
		return 0;
	}
}

int midNode2::Delete(midNode2 &neighbor, int flag, Bid key)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key == key) break;
	}
	if(i == p2) return -1;
	if(i == p2 - 1) childMap[i] = kppair();
	else
	{
		for(int j = i; j < p2-1; j++)
		{
			childMap[j] = childMap[j+1];
		}
		childMap[p2-1] = kppair();
	}
	if(p2 <= (P2 + 1) / 2 && neighbor.p2 > 0)
	{
		if(flag == -1)
		{
			if(neighbor.p2 + p2 -1 <= P2)
			{
				std::copy(childMap.begin(), childMap.begin()+(p2-1), neighbor.childMap.begin()+neighbor.p2);
				neighbor.p2 += p2 - 1;
				neighbor.max_value = neighbor.childMap[neighbor.p2 - 1].key;
				p2 = 0;
				return 1;
			}
			else
			{
				kppair kvp_temp = neighbor.childMap[neighbor.p2 - 1];
				neighbor.childMap[neighbor.p2 - 1] = kppair();
				neighbor.p2--;
				neighbor.max_value = neighbor.childMap[neighbor.p2 - 1].key;
				// insert
				for(int i = p2; i > 0; i--)
				{
					childMap[i] = childMap[i-1];
				}
				childMap[0] = kvp_temp;
				max_value = childMap[p2-1].key;
				return 0;
			}
		}
		else if(flag == 1)
		{
			if(neighbor.p2 + this->p2 - 1 <= P2)
			{
				for(int i = neighbor.p2 + this->p2 - 2; i >= p2 - 1; i--)
				{
					neighbor.childMap[i] = neighbor.childMap[i - p2 + 1];
				}
				for(int i = p2 - 2; i >= 0; i--)
				{
					neighbor.childMap[i] = childMap[i];
				}
				neighbor.p2 += p2-1;
				p2 = 0;
				return 1;
			}
			else
			{
				kppair kvp_temp = neighbor.childMap[0];
				for(int i = 0; i < neighbor.p2 - 1; i++)
				{
					neighbor.childMap[i] = neighbor.childMap[i+1];
				}
				neighbor.childMap[neighbor.p2 - 1] = kppair();
				neighbor.p2--;
				childMap[p2-1] = kvp_temp;
				max_value = kvp_temp.key;
				return 0;
			}
		}
		else
		{
			this->p2--;
			this->max_value = this->childMap[p2-1].key;
			return 0;
		}
	}
	else
	{
		this->p2--;
		this->max_value = this->childMap[p2-1].key;
		return 0;
	}
}

midNode1::midNode1()
{
	p2 = 0; 
	// childMap.resize(P2);
}

midNode1::midNode1(int mid1L)
{
	p2 = 0; 
	// pos = RandomPos(2 * P1 * pow(P2,mid1L)); 
	// childMap.resize(P2);
}

midNode1::midNode1(const midNode1 & mid1)
{
	this->childMap = mid1.childMap;
	p2 = mid1.p2;
	max_value = mid1.max_value;
	old_max = mid1.old_max;
	pos = mid1.pos;
}

void midNode1::Insert(midNode1 &m1, midNode2& mid_node2)
{
	kppair kp(mid_node2.max_value, mid_node2.pos);
	if (this->p2 < P2)
	{
		int i = 0;
		for (; i < p2; i++)
		{
			if(childMap[i].key > mid_node2.max_value)
			{
				break;
			}
		}
		if(i == p2) childMap[i] = kp;
		else{
			for(int j = p2; j >= i; j--)
			{
				if(j == i) childMap[j] = kp;
				else childMap[j] = childMap[j-1];
			}
		}
		this->p2++;
		mid_node2.old_max = mid_node2.max_value;
		//this->old_max = this->max_value;
		max_value = childMap[p2 - 1].key;
		return;
	}
	else
	{
		int count = 0;
		bool written = false;
		int half = (P2 + 1) / 2;
		std::array<kppair, P2 + 1> full;
		for(int i = 0; i < p2; i++)
		{
			if(childMap[i].key < kp.key)
			{
				full[count++] = childMap[i];
				continue;
			}
			else
			{
				if(!written)
				{
					full[count++] = kp;
					written = true;
				}
				full[count++] = childMap[i];
			}
		}
		if(!written)
		{
			full[count++] = kp;
		}
		mid_node2.old_max = mid_node2.max_value;
		//this->old_max = this->max_value;
		kppair kkpp;
		childMap.fill(kkpp);
		memcpy(childMap.data(), full.data(), half * sizeof(kppair));
		memcpy(m1.childMap.data(), full.data() + half, (P2+1-half)*sizeof(kppair));
		p2 = half;
		m1.p2 = P2+1-half;
		max_value = childMap[p2 - 1].key;
		m1.max_value = m1.childMap[m1.p2 - 1].key; 
		return;
	}
}

//改
void midNode1::Insert(midNode1 &m1, midNode1 &mid_node1,int mid1L)
{
	kppair kp(mid_node1.max_value, mid_node1.pos);
	if (this->p2 < P2)
	{
		int i = 0;
		for (; i < p2; i++)
		{
			if(childMap[i].key > mid_node1.max_value)
			{
				break;
			}
		}
		if(i == p2) childMap[i] = kp;
		else{
			for(int j = p2; j >= i; j--)
			{
				if(j == i) childMap[j] = kp;
				else childMap[j] = childMap[j-1];
			}
		}
		this->p2++;
		mid_node1.old_max = mid_node1.max_value;
		//this->old_max = this->max_value;
		max_value = childMap[p2 - 1].key;
		return;
	}
	else
	{
		int count = 0;
		bool written = false;
		int half = (P2 + 1) / 2;
		std::array<kppair, P2 + 1> full;
		for(int i = 0; i < p2; i++)
		{
			if(childMap[i].key < kp.key)
			{
				full[count++] = childMap[i];
				continue;
			}
			else
			{
				if(!written)
				{
					full[count++] = kp;
					written = true;
				}
				full[count++] = childMap[i];
			}
		}
		if(!written)
		{
			full[count++] = kp;
		}
		mid_node1.old_max = mid_node1.max_value;
		//this->old_max = this->max_value;
		kppair kkpp;
		childMap.fill(kkpp);
		memcpy(childMap.data(), full.data(), half * sizeof(kppair));
		memcpy(m1.childMap.data(), full.data() + half, (P2+1-half)*sizeof(kppair));
		p2 = half;
		m1.p2 = P2+1-half;
		max_value = childMap[p2 - 1].key;
		m1.max_value = m1.childMap[m1.p2 - 1].key; 
		return;
	}
}



void midNode1::Update(midNode2 &m2)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key == m2.old_max)
		{
			childMap[i].key = m2.max_value;
			break;
		}
	}
	if(m2.max_value != m2.old_max) sort(childMap.begin(), childMap.begin() + p2, kppair_sort_rule);
	m2.old_max = m2.max_value;
	//this->old_max = this->max_value;
	max_value = childMap[p2 - 1].key;
	return;
}


//改
void midNode1::Update(midNode1 &m1)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key == m1.old_max)
		{
			childMap[i].key = m1.max_value;
			break;
		}
	}
	if(m1.max_value != m1.old_max) sort(childMap.begin(), childMap.begin() + p2, kppair_sort_rule);
	m1.old_max = m1.max_value;
	//this->old_max = this->max_value;
	max_value = childMap[p2 - 1].key;
	return;
}

int midNode1::Search(Bid key, Bid& m2_key)
{
	for(int i = 0; i < p2; i++)
	{
		if(childMap[i].key >= key)
		{
			m2_key = childMap[i].key;
			return childMap[i].pos;
		}
	}
	m2_key = max_value;
	return childMap[p2 - 1].pos;
}

// 返回值：-1（需要从左兄弟取），0（从本节点），1（从右兄弟取）
int midNode1::Search(Bid key, Bid& m2_key, int& m2_pos, Bid& neighbor, int& n_pos, int& up_right)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key >= key)
		{
			m2_key = childMap[i].key;
			m2_pos = childMap[i].pos;
			break;
		}
	}
	if(i == p2)
	{
		m2_key = max_value;
		m2_pos = childMap[p2 - 1].pos;
		i--;
	}
	if(up_right == -1)
	{
		if(i > 0)
		{
			neighbor = childMap[i-1].key;
			n_pos = childMap[i-1].pos;
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if(up_right == 1)
	{
		if(i < p2 - 1)
		{
			neighbor = childMap[i+1].key;
			n_pos = childMap[i+1].pos;
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else //up_right == 0,如果可以 要设置up_right
	{
		if(p2 != 1)
		{
			if(i == 0)
			{
				up_right = 1;
				neighbor = childMap[i+1].key;
				n_pos = childMap[i+1].pos;
			}
			else
			{
				up_right = -1;
				neighbor = childMap[i-1].key;
				n_pos = childMap[i-1].pos;
			}
		}
		return 0;
	}
}


int midNode1::Delete(midNode1 &neighbor, int left, Bid key)
{
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i].key == key) break;
	}
	if(i == p2) return -1;
	if(i == p2 - 1) childMap[i] = kppair();
	else
	{
		for(int j = i; j < p2-1; j++)
		{
			childMap[j] = childMap[j+1];
		}
		childMap[p2-1] = kppair();
	}
	if(p2 <= (P2 + 1) / 2 && neighbor.p2 > 0)
	{
		if(left == -1)
		{
			if(neighbor.p2 + p2 -1 <= P2)
			{
				std::copy(childMap.begin(), childMap.begin()+(p2-1), neighbor.childMap.begin()+neighbor.p2);
				neighbor.p2 += p2 - 1;
				neighbor.max_value = neighbor.childMap[neighbor.p2 - 1].key;
				p2 = 0;
				return 1;
			}
			else
			{
				kppair kvp_temp = neighbor.childMap[neighbor.p2 - 1];
				neighbor.childMap[neighbor.p2 - 1] = kppair();
				neighbor.p2--;
				neighbor.max_value = neighbor.childMap[neighbor.p2 - 1].key;
				// insert
				for(int i = p2; i > 0; i--)
				{
					childMap[i] = childMap[i-1];
				}
				childMap[0] = kvp_temp;
				max_value = childMap[p2-1].key;
				return 0;
			}
		}
		else if(left == 1) //right
		{
			if(neighbor.p2 + this->p2 - 1 <= P2)
			{
				for(int i = neighbor.p2 + this->p2 - 2; i >= p2 - 1; i--)
				{
					neighbor.childMap[i] = neighbor.childMap[i - p2 + 1];
				}
				for(int i = p2 - 2; i >= 0; i--)
				{
					neighbor.childMap[i] = childMap[i];
				}
				neighbor.p2 += p2-1;
				p2 = 0;
				return 1;
			}
			else
			{
				kppair kvp_temp = neighbor.childMap[0];
				for(int i = 0; i < neighbor.p2 - 1; i++)
				{
					neighbor.childMap[i] = neighbor.childMap[i+1];
				}
				neighbor.childMap[neighbor.p2 - 1] = kppair();
				neighbor.p2--;
				childMap[p2-1] = kvp_temp;
				max_value = kvp_temp.key;
				return 0;
			}
		}
		else
		{
			this->p2--;
		this->max_value = this->childMap[p2-1].key;
		return 0;
		}
	}
	else
	{
		this->p2--;
		this->max_value = this->childMap[p2-1].key;
		return 0;
	}
}

rootNode::rootNode()
{
	p1 = 0; 
	// childMap.resize(P1);
}

rootNode::rootNode(const rootNode& n)
{
	this->p1 = n.p1;
	this->max_value = n.max_value;
	this->childMap = n.childMap;
}

void rootNode::Update(midNode1 &m1)
{
	int i = 0;
	for(; i < p1; i++)
	{
		if(childMap[i].key == m1.old_max)
		{
			childMap[i].key = m1.max_value;
			break;
		}
	}
	if(m1.max_value != m1.old_max) sort(childMap.begin(), childMap.begin() + p1, kppair_sort_rule);
	m1.old_max = m1.max_value;
	//this->old_max = this->max_value;
	max_value = childMap[p1 - 1].key;
	return;
}

void rootNode::Update(midNode2 &m2)
{
	int i = 0;
	for(; i < p1; i++)
	{
		if(childMap[i].key == m2.old_max)
		{
			childMap[i].key = m2.max_value;
			break;
		}
	}
	if(m2.max_value != m2.old_max) sort(childMap.begin(), childMap.begin() + p1, kppair_sort_rule);
	m2.old_max = m2.max_value;
	//this->old_max = this->max_value;
	max_value = childMap[p1 - 1].key;
	return;
}

bool rootNode::Insert(midNode1 &m1)
{
	kppair kp(m1.max_value, m1.pos);
	if (this->p1 < P1)
	{
		int i = 0;
		for (; i < p1; i++)
		{
			if(childMap[i].key > m1.max_value)
			{
				break;
			}
		}
		if(i == p1) childMap[i] = kp;
		else{
			for(int j = p1; j >= i; j--)
			{
				if(j == i) childMap[j] = kp;
				else childMap[j] = childMap[j-1];
			}
		}
		this->p1++;
		m1.old_max = m1.max_value;
		//this->old_max = this->max_value;
		max_value = childMap[p1 - 1].key;
		return true;
	}
	else return false;
}

bool rootNode::Insert(midNode2 &m2)
{
	kppair kp(m2.max_value, m2.pos);
	if (this->p1 < P1)
	{
		int i = 0;
		for (; i < p1; i++)
		{
			if(childMap[i].key > m2.max_value)
			{
				break;
			}
		}
		if(i == p1) childMap[i] = kp;
		else{
			for(int j = p1; j >= i; j--)
			{
				if(j == i) childMap[j] = kp;
				else childMap[j] = childMap[j-1];
			}
		}
		this->p1++;
		m2.old_max = m2.max_value;
		//this->old_max = this->max_value;
		max_value = childMap[p1 - 1].key;
		return true;
	}
	else return false;
}


int rootNode::Search(Bid key, Bid& m1_key)
{
	for(int i = 0; i < p1; i++)
	{
		if(childMap[i].key >= key)
		{
			m1_key = childMap[i].key;
			return childMap[i].pos;
		}
	}
	m1_key = max_value;
	return childMap[p1 - 1].pos;
}

// right:-1左 0无 1右
void rootNode::Search(Bid key, Bid& m1_key, int& m1_pos, Bid& neighbor, int& n_pos, int& right)
{
	int i = 0;
	for(; i < p1; i++)
	{
		if(childMap[i].key >= key)
		{
			m1_key = childMap[i].key;
			m1_pos = childMap[i].pos;
			break;
		}
	}
	if(i == p1)
	{
		m1_key = max_value;
		m1_pos = childMap[p1 - 1].pos;
		i--;
	}
	if(p1 != 1)
	{
		if(i == 0)
		{
			right = 1;
			neighbor = childMap[i+1].key;
			n_pos = childMap[i+1].pos;
		}
		else
		{
			right = -1;
			neighbor = childMap[i-1].key;
			n_pos = childMap[i-1].pos;
		}
	}
	
}

int rootNode::Delete(Bid key)
{
	int i = 0; 
	for(; i < p1; i++)
	{
		if(childMap[i].key == key) break;
	}
	if(i == p1) return -1;
	if(i == p1 - 1) childMap[i] = kppair();
	else
	{
		for(int j = i; j < p1 - 1; j++)
		{
			childMap[j] = childMap[j+1];
		}
		childMap[p1-1] = kppair();
	}
	return 0;
}