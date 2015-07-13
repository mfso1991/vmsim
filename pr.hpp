/*************************
 *	@author : You Zhou   *
 *************************/
 
#ifndef PR_HPP
#define PR_HPP

#include <iostream>
#include <unordered_map>
#include <deque>
#include <list>
#include <algorithm>
#include <vector>

using namespace std;

extern unsigned int total_accesses, page_fault_count, write_to_disk_count;

struct Page
{
	unsigned int addr;
	bool referenced;
	bool modified;
	unsigned char aging_count;
	
	inline bool operator<(const Page& rhs) const
	{
		return aging_count < rhs.aging_count;
	}

	inline bool operator==(const Page& rhs) const
	{
		return addr == rhs.addr;
	}
};
struct clock_ptr
{
	struct Page page;
	struct clock_ptr* prev;
	struct clock_ptr* next;
};

void run_optimal(const char*, const unsigned int);

void run_clock(const char*, const unsigned int);

void run_lru(const char*, const unsigned int);

void run_aging(const char*, const unsigned int, const unsigned int);

#endif