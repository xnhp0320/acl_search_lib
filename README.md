# acl_search_lib

This is a rewrite for the HyperSplit packet classification algorithm. It can be used as a lib for IPv4 based 
five tuples or three tuples acl matching. HyperSplit is one of the best packet algorithms. It has faster lookup speed 
as well as small memory size compared to the DPDK ACL algorithm.

The public available code for the HyperSplit algorithm has some minor issues:  

1. using too many malloc/free in its tree building process, but does not check if the memory allocation succeeds or not. 

2. The heuristic the author uses to build trees has some problems. In fact, this implementation results in better trees
(fewer memory accesses and fewer nodes) than the original implementation.

3. The original code does not use a common optimization technique in many Decision-Tree based packet classification algorithms. 
This has been fixed in the current version. I checked that sometimes this technique can save up to 10x memory. 


