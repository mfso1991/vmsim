/*************************
 *	@author : You Zhou   *
 *************************/
 
#include <string>
#include <stdio.h>
#include "pr.hpp"

int main(int argc, char **argv)
{
	if(argc != 6 && argc != 8)
	{
		cout << "Error: invalid command!\nPlease use ./vmsim –n <numframes> -a <opt|clock|aging|lru> [-r <refresh>] <tracefile>" << endl;
		return 0;
	}
	
	unsigned int nframes; 	/** # of Frames **/
	if(sscanf(argv[2], "%u", &nframes) == -1 || (nframes & (nframes - 1)) || nframes > 64)		/** x &(x-1) produces 0 if x is of the form 2^n. **/ 
	{														
		cout << "Error: invalid argument for frame numbers!\nPlease use 8, 16, 32, or 64 instead." << endl;
		return 0;
	}
	
	string alg(argv[4]); 	/** Algorithm **/
	
	if(alg == "aging")
	{
		unsigned int period; /** Refreshing Period **/
		sscanf(argv[6], "%u", &period);
		run_aging(argv[7], nframes, period); 	/** will be using a priority deque to simulate the physical memory. **/
	}
	else
	{
		if(alg == "opt")
			run_optimal(argv[5], nframes);		/** will be using an unordered_set to simulate the physical memory. **/
		else if(alg == "clock")
			run_clock(argv[5], nframes); 		/** will be using a doubly-linked list to simulate the physical memory. **/
		else if(alg == "lru")
			run_lru(argv[5], nframes);   		/** will be using a stack to simulate the physical memory. **/
		else
		{
			cout << "Error: unrecognised algorithm.\nPlease use opt, clock, aging, or lru instead." << endl;
			return 0;
		}
	}
	
	cout << "\n\nAlgorithm:\t" << alg << endl;
	cout << "Number of frames:\t" << nframes << endl;
	cout << "Total memory accesses:\t" << total_accesses << endl;
	cout << "Total page faults:\t" << page_fault_count << endl;
	cout << "Total writes to disk:\t" << write_to_disk_count << "\n\n" << endl;
	
	return 0;
}
