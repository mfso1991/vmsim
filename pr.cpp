/*************************
 *	@author : You Zhou   *
 *************************/

#include <fstream>
#include "pr.hpp"

unsigned int total_accesses, page_fault_count, write_to_disk_count;

void run_optimal(const char* kfilename, const unsigned int knframe)
{
	ifstream in(kfilename);
	
	/** VA as key is mapped to a deque holding distances of a sequence of occurrences. **/
	unordered_map<unsigned int, deque<unsigned int>> distance_map; 
	unsigned int addr, distance = 0;							   
	char mode; 						
	while(in >> hex >> addr >> mode)
	{
		addr >>= 12;
		if(distance_map.find(addr) == distance_map.end())
			distance_map.insert(make_pair(addr, deque<unsigned int>())); 	/** Note : emplace can be used with gcc v 4.8.*. **/
		distance_map.at(addr).push_back(distance);	/** addr -> [occur1 -- occur2 -- occur3 -- ...] **/
		distance++;
	}
	total_accesses = distance;
	
	in.clear();		/** unsets EOF flag. **/
	in.seekg(0, ios::beg);		/** resets the position to the beginning. **/
	
	vector<Page> memory;
	while(in >> hex >> addr >> mode)
	{
		addr >>= 12;
		if(!distance_map.at(addr).empty())
			distance_map.at(addr).pop_front();
		struct Page page = {addr, false, mode == 'W', 0};
		vector<Page>::iterator itr = find(memory.begin(), memory.end(), page);
		if(itr != memory.end())	/** Found **/
		{
			cout << "HIT!" << endl;
			page.modified = page.modified || (itr->modified);
			memory.erase(itr);
		}
		else /** Not Found **/
		{
			page_fault_count++;
			if(memory.size() == knframe) /** has to evict. **/
			{
				vector<Page>::iterator victim = memory.begin();
				unsigned int furthest = 0;//distance_map.at(victim->addr).front();
				for(vector<Page>::iterator itr = memory.begin(); itr != memory.end() ; itr++)
				{
					if(distance_map.at(itr->addr).empty())
					{
						victim = itr;
						break;
					}
					unsigned int next = distance_map.at(itr->addr).front(); 
					if(next > furthest)
					{
						victim = itr;
						furthest = next;
					}
				}
				
				if(victim->modified)
				{
					cout << "Page Fault -- Evict Dirty." << endl;
					write_to_disk_count++;
				}
				else
					cout << "Page Fault -- Evict Clean." << endl;
				
				memory.erase(victim);
			}
			else
				cout << "Page Fault -- No Eviction." << endl;
		}
		memory.push_back(page);
	}
	
	in.close();
}

void run_clock(const char* kfilename, const unsigned int knframe)
{
	ifstream in(kfilename);
	struct clock_ptr* hand;// = new clock_ptr();
	unsigned int addr, size(0);							   
	char mode; 
	while(in >> hex >> addr >> mode)
	{
		total_accesses++;
		addr >>= 12;
		struct clock_ptr* node = new clock_ptr({{addr, false, mode == 'W', 0}, NULL, NULL}); /** &node <- prev and next -> &node**/
		node->prev = node;
		node->next = node;
		if(size == 0)
		{
			cout << "Initialization" << endl;
			size = 1;
			page_fault_count = 1;
		    hand = node; /** ROOT **/
			cout << "Page Fault -- No Eviction." << endl;
		}
		else
		{
			bool found = false;
			struct clock_ptr* temp = hand;
			do
			{
				if((temp->page).addr == addr) /** Found **/
				{
					cout << "HIT!" << endl;
					found = true;
					(temp->page).modified = (temp->page).modified || (node->page).modified;
					(temp->page).referenced = true;
				}
				temp = temp->next;
			}
			while(temp != hand); /** True if a circular search has been done. **/
			
			if(!found)
			{
				page_fault_count++;
				
				if(size == knframe) /** hand position potentially change. **/
				{
					bool success = false;
					while(!success)
					{
						if( (hand->page).referenced ) /** Deserve a second chance. **/
							(hand->page).referenced = false;
						else
						{
							if((hand->page).modified)
							{
								cout << "Page Fault -- Evict Dirty." << endl;
								write_to_disk_count++;
							}
							else
								cout << "Page Fault -- Evict Clean." << endl;
							
							/** over-write the content. **/
							hand->page = {(node->page).addr, (node->page).referenced, (node->page).modified, (node->page).aging_count};
							delete node;
							success = true;
						}
						hand = hand->prev; /** Searching in the previous direction. **/
					}
				}
				else /** hand position won't change. **/
				{
					cout << "Page Fault -- No Eviction." << endl;
					struct clock_ptr* formernext = hand->next;
					hand->next = node;
					node->prev = hand;
					node->next = formernext;
					formernext->prev = node;
					size++;
				}
			}
		}
	}

	while(hand != hand->prev)
	{
		struct clock_ptr* nextnext = hand->next->next;
		nextnext->prev = hand;
		delete hand->next;
		hand->next = nextnext;
		hand = hand->prev;
	}
	delete hand;
	in.close();
}

void run_lru(const char* kfilename, const unsigned int knframe)
{
	ifstream in(kfilename);

	list<Page> memory;
	unsigned int addr;							   
	char mode; 
	while(in >> hex >> addr >> mode)
	{
		total_accesses++;
		addr >>= 12;
		struct Page page = {addr, false, mode == 'W', 0};
		list<Page>::iterator itr = find(memory.begin(), memory.end(), page);
		if(itr != memory.end())	/** Found **/
		{
			cout << "HIT!" << endl;
			itr->modified = (itr->modified) || page.modified;
			itr->aging_count = (itr->aging_count) | 0x80;
			struct Page reinsert = {itr->addr, itr->referenced, itr->modified, itr->aging_count};
			memory.erase(itr);
			memory.push_front(reinsert);
		}
		else
		{
			page_fault_count++;
			if(memory.size() == knframe)
			{
				struct Page victim = memory.back();
				if(victim.modified)
				{
					cout << "Page Fault -- Evict Dirty." << endl;
					write_to_disk_count++;
				}
				else			
					cout << "Page Fault -- Evict Clean." << endl;
				
				memory.pop_back();
			}
			else
				cout << "Page Fault -- No Eviction." << endl;
			
			memory.push_front(page);
		}
	}
	in.close();
}

void run_aging(const char* kfilename, const unsigned int knframe, const unsigned int kperiod)
{
	ifstream in(kfilename);
	
	unsigned int count(0);
	list<Page> memory;
	unsigned int addr;							   
	char mode; 
	while(in >> hex >> addr >> mode)
	{
		total_accesses++;
		if(count%kperiod == 0)
			for(list<Page>::iterator itr = memory.begin(); itr != memory.end() ; itr++)
				itr->aging_count >>= 1;
		
		addr >>= 12;
		struct Page page = {addr, false, mode == 'W', 0};
		list<Page>::iterator itr = find(memory.begin(), memory.end(), page);
		if(itr != memory.end())	/** Found **/
		{
			cout << "HIT!" << endl;
			itr->modified = (itr->modified) || page.modified;
			itr->aging_count = (itr->aging_count) | 0x80;
		}
		else
		{
			page_fault_count++;
			if(memory.size() == knframe)
			{
				memory.sort();
				struct Page victim = memory.front();
				if(victim.modified)
				{
					cout << "Page Fault -- Evict Dirty." << endl;
					write_to_disk_count++;
				}
				else			
					cout << "Page Fault -- Evict Clean." << endl;
				
				memory.pop_front();
			}
			else
				cout << "Page Fault -- No Eviction." << endl;
			
			memory.push_front(page);
		}
		count++;
	}
	
	in.close();
}