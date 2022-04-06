#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>
#include <iomanip>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>

using namespace std;

#define LEN 32
char L1_NAME_STR[] ="L1";
char L2_NAME_STR[] ="L2";
char READ_STR[] ="r";
char WRITE_STR[] ="w";
char INVALIDATE_STR[] ="i";

// Constants
const int LRU = 0;
const int PLRU = 1;
const int OPTIMAL = 2;

const int NON_INCLUSIVE = 0;
const int INCLUSIVE = 1;



// for read, set write = 0, for write, set write = 1
struct AddressData
{
	int tags, index, blockOffset, write;
	char address[50];
};

struct CacheAccessReply
{
	// 1: hit, 0: miss
	int tags, set_no, status;

	// 1: WB, 0: No WB
	int writeback;

	int back_invalidation;

	char address[50];
};



struct CacheBlock
{

	int tags;
	int isDirty =0;
	int isInvalid;
	int accessCounter;
	char address[50];

};

void printSimulationResults(int L1_readsp, int L1_read_missesp, int L1_writesp, int L1_write_missesp, int L1_write_backsp, int L2_readsp, int L2_read_missesp, int L2_writesp, int L2_write_missesp, int L2_write_backsp , int isInclusiveCache, int extra_memoryp)
{
	// STAT Variables
	int L1_reads = L1_readsp;
	int L1_read_misses = L1_read_missesp;
	int L1_writes = L1_writesp;
	int L1_write_misses = L1_write_missesp;
	float L1_miss_rate = 0.0;
	int L1_write_backs = L1_write_backsp;

	int L2_reads = L2_readsp;
	int L2_read_misses = L2_read_missesp;
	int L2_writes = L2_writesp;
	int L2_write_misses = L2_write_missesp;
	float L2_miss_rate = 0.0;
	int L2_write_backs = L2_write_backsp;

	int totalMemoryTraffic = 0;


/*
cout << "a. number of "<< cacheName <<" reads:\t\t" << std::dec << Cache_reads << endl;
		cout << "b. number of "<< cacheName <<" read misses:\t" << Cache_read_misses << endl;
		cout << "c. number of "<< cacheName <<" writes:\t\t" << Cache_writes << endl;
		cout << "d. number of "<< cacheName <<" write misses:\t" << Cache_write_misses << endl;
		
		float a,b;
		if(strcmp(CacheNAME, "L1") == 0){
			a = (Cache_read_misses + Cache_write_misses);//(float)((Cache_read_misses + Cache_write_misses)/(Cache_reads + Cache_writes));
			b = (Cache_reads + Cache_writes);	
		}
		else {
			a = Cache_read_misses;
			b = Cache_reads;
		}
		

		//cout <<"AB::" << a<<"::"<<b<<"::"<<a/b<<"##"<<endl;

		cout << "e. "<< cacheName <<" miss rate:\t\t"<<std::fixed<<std::setprecision(6)<< a/b << endl;
		cout << "f. number of "<< cacheName <<" writebacks:\t" << Cache_write_backs << endl;

*/	
	float a =0.0,b =0.0, c =0.0, d =0.0;
	//if(strcmp(CacheNAME, "L1") == 0){
	a = (L1_read_misses + L1_write_misses);//(float)((Cache_read_misses + Cache_write_misses)/(Cache_reads + Cache_writes));
	b = (L1_reads + L1_writes);	
	L1_miss_rate = a / b;

	totalMemoryTraffic += (L1_read_misses + L1_write_misses + L1_write_backs);
	//}
	if(L2_reads !=0 && L2_read_misses!=0) {
		c = L2_read_misses;
		d = L2_reads;

		L2_miss_rate = c / d;

		totalMemoryTraffic = (L2_read_misses + L2_write_misses + L2_write_backs);
		
		if(isInclusiveCache == 1)
		{
			// update this value
			totalMemoryTraffic += extra_memoryp;
		}	
	}

	cout << "===== Simulation results (raw) =====" << endl;
	cout << "a. number of L1 reads:\t\t"<< std::dec << L1_reads << endl;
	cout << "b. number of L1 read misses:\t" << L1_read_misses << endl;
	cout << "c. number of L1 writes:\t\t" << L1_writes << endl;
	cout << "d. number of L1 write misses:\t" << L1_write_misses << endl;
	cout << "e. L1 miss rate:\t\t" <<std::fixed<<std::setprecision(6)<< L1_miss_rate << endl;
	cout << "f. number of L1 writebacks:\t" << L1_write_backs << endl;
	cout << "g. number of L2 reads:\t\t" << L2_reads << endl;
	cout << "h. number of L2 read misses:\t" << L2_read_misses << endl;
	cout << "i. number of L2 writes:\t\t" << L2_writes << endl;
	cout << "j. number of L2 write misses:\t" << L2_write_misses << endl;
	
	if (L2_miss_rate == 0) {
		cout << "k. L2 miss rate:\t\t0"<< endl;
	}
	else {
		cout << "k. L2 miss rate:\t\t" <<std::fixed<<std::setprecision(6)<< L2_miss_rate << endl;
	}
	cout << "l. number of L2 writebacks:\t" << L2_write_backs << endl;

	

	cout << "m. total memory traffic:\t" << totalMemoryTraffic << endl;
}

bool checkPowerOfTwo(int num)
{

	float log2ofn = log2(num);
	//cout << "log2: " << log2ofn;
	return ceil(log2ofn) == floor(log2ofn);
}

uint64_t extract(uint64_t value, int start, int end)
{
    uint64_t mask = (1 << (end - start)) - 1;//~(~0 << (end - begin + 1));//

    //printf("MAKS: %lu\n", mask);
    return (value >> start) & mask;
}


void bin(uint64_t n) 
{ 
    /* step 1 */
    if (n > 1) 
        bin(n/2); 
  
    /* step 2 */
    cout << n % 2; 
}

class CACHE
{


	public:
	int Cache_BLOCKSIZE;
    
    int Cache_SIZE;
    int Cache_ASSOC;

    // 0:LRU, 1:PLRU, 2:Optimal.
    int Cache_REPLACEMENT_POLICY;
    int Cache_INCLUSION_PROPERTY;

    int n_of_sets;

    int cache_tag_bits;
    int cache_index_bits;

    // Tag Array of the Cache
    //int **tag_array;
    CacheBlock **tag_array;

    int **btreeArray;

    // Optimal Replacement

    std::map<uint64_t,vector<uint64_t>> futureAccess;
    uint64_t futureAccessCounter = 0 ;
    uint64_t currentFutureCounter = 0;

	// STAT Variables
	int Cache_reads;
	int Cache_read_misses;
	int Cache_writes;
	int Cache_write_misses;
	float Cache_miss_rate;
	int Cache_write_backs;
	char CacheNAME[50];

	int extra_memory = 0;

	int level;
	int zeros;

	int getIndexFromTreeLoc(int treeIndex, int zeros)
	{
		return treeIndex - zeros;

	}

	int toIndexOfTreeLoc(int treeIndex, int zeros)
	{
		return treeIndex + zeros;

	}

	void initTreeArray(int len, int zeros){

		
		//cout << "Initializing BTree Array." << endl;
		//cout <<"Zeros: " << zeros <<endl;

		// assings zeros
		


		//int k, j;
		for(int k=0; k<n_of_sets; k++)
		{
			//cout << "Set\t" << std::dec << k <<":\t";
			
			int i;
			for(i=0; i< zeros; i++){
				btreeArray[k][i] = 0;
			}
			for(int j=0; j<len; j++) {
				btreeArray[k][i] = j;
				i++;
			}


			//cout << endl;
		}

		

		/*cout << "PRINT BTREE ARR:\n";
		for(int k=0; k<n_of_sets; k++)
		{
			for(int i=0; i<50; i++) {
				cout << btreeArray[k][i] << " " ;
			}
			cout << endl;
		}

		cout << "BTREE PRint done\n";*/
		


	}

	void accessTree(int set_no, int childIndex, int level)
	{
		float parentIndex = (float)childIndex;

		while(level > 0){

			//int childIndex = 5;
			parentIndex = (parentIndex - 1)/2;//parentIndex/2;

			//cout << "Parent::" << parentIndex <<" FL:" <<floor(parentIndex)<<endl;
			int fl_index = floor(parentIndex);
			if(parentIndex== fl_index){
				//cout << "Left child" << endl;

				// Set parent 0
				btreeArray[set_no][fl_index] = 0;

			}
			else {
				//cout << "Right child" << endl;

				// Set parent 1
				btreeArray[set_no][fl_index] = 1;
			}

			parentIndex = (float) fl_index ;
			level --;

		}
	}

	int replaceTree(int set_no, int level){
		int rootIndex = 0;
		
		while(level > 0){

			if(btreeArray[set_no][rootIndex] == 0)
			{
				rootIndex = (2 * rootIndex) + 2;
				// Go RIGHT

				// toggle the prev value
				//arr[rootIndex] = 1;
			}
			else {
				rootIndex = (2 * rootIndex) + 1;
				// Go LEFT

				// toggle the prev value
				//arr[rootIndex] = 0;
			}

			level --;

		}

		// child of root Index:
		//int child = (2 * rootIndex)
		
		//cout << "FINAL Root Index:" << rootIndex << endl;
		//cout << "FINAL VALUE Index:" << btreeArray[set_no][rootIndex]<<endl;
		//cout << "FINAL Data Index:" << data[arr[rootIndex]]<<endl;

		//print_tree();
		return btreeArray[set_no][rootIndex];
	}

	int replaceTree11(int set_no, int rootIndex, int level){

		
		while(level > 0){

			if(btreeArray[set_no][rootIndex] == 0)
			{
				rootIndex = (2 * rootIndex) + 1;
				// Go LEFT

				// toggle the prev value
				//btreeArray[set_no][rootIndex] = 1;
			}
			else {
				rootIndex = (2 * rootIndex) + 2;
				// Go Right

				// toggle the prev value
				//btreeArray[set_no][rootIndex] = 0;
			}

			level --;

		}

		// child of root Index:
		//int child = (2 * rootIndex)
		
		cout << "FINAL Root Index:" << rootIndex << endl;
		cout << "FINAL VALUE Index:" << btreeArray[set_no][rootIndex]<<endl;
		//cout << "FINAL Data Index:" << data[arr[rootIndex]]<<endl;
	
		return btreeArray[set_no][rootIndex];
	}

	CACHE()
	{
		//size = 11;
		//CACHE(32, 0, 0, 0, 0);
	}

	CACHE(int blocksize, int size, int assoc, int replacement_policy, int inclusion_property, char * CacheNameP)
	{	
		extra_memory = 0;
		// Optimal
		futureAccessCounter = 1;
		//int maxCounterArray[1000];

		Cache_BLOCKSIZE = blocksize;
		Cache_SIZE = size;
		Cache_ASSOC = assoc;
		Cache_REPLACEMENT_POLICY = replacement_policy;
		Cache_INCLUSION_PROPERTY = inclusion_property;
		

		n_of_sets = Cache_SIZE / (Cache_BLOCKSIZE * Cache_ASSOC);

		//tag_array = new int[n_of_sets][Cache_ASSOC]
		tag_array = new CacheBlock*[n_of_sets];
		for(int i = 0; i < n_of_sets; i++)
		    tag_array[i] = new CacheBlock[Cache_ASSOC];

		// Declare Btree Array;
		// Change the size later
		btreeArray = new int*[n_of_sets];
		for(int i = 0; i < n_of_sets; i++)
		    btreeArray[i] = new int[100];


		strcpy(CacheNAME, CacheNameP);

		initializeTagArray();

		// Initialize Btree Array
		level = log2(Cache_ASSOC);
		zeros = pow(2,level) - 1;
		initTreeArray(Cache_ASSOC, zeros);

		currentFutureCounter = -1;
		// Optimal
		futureAccessCounter = 0;
		//cout << "Cache Initialized";
		//cout << "value: "<< checkPowerOfTwo(asize) << endl;
	}

	void printCacheSimulationResults(char *cacheName)
	{
		cout << "a. number of "<< cacheName <<" reads:\t\t" << std::dec << Cache_reads << endl;
		cout << "b. number of "<< cacheName <<" read misses:\t" << Cache_read_misses << endl;
		cout << "c. number of "<< cacheName <<" writes:\t\t" << Cache_writes << endl;
		cout << "d. number of "<< cacheName <<" write misses:\t" << Cache_write_misses << endl;
		
		float a,b;
		if(strcmp(CacheNAME, "L1") == 0){
			a = (Cache_read_misses + Cache_write_misses);//(float)((Cache_read_misses + Cache_write_misses)/(Cache_reads + Cache_writes));
			b = (Cache_reads + Cache_writes);	
		}
		else {
			a = Cache_read_misses;
			b = Cache_reads;
		}
		

		//cout <<"AB::" << a<<"::"<<b<<"::"<<a/b<<"##"<<endl;

		cout << "e. "<< cacheName <<" miss rate:\t\t"<<std::fixed<<std::setprecision(6)<< a/b << endl;
		cout << "f. number of "<< cacheName <<" writebacks:\t" << Cache_write_backs << endl;
	}


	void setObjectValues(int blocksize, int size, int assoc, int replacement_policy, int inclusion_property, char * CacheNameP)
	{	
		// Optimal
		futureAccessCounter = 1;

		Cache_BLOCKSIZE = blocksize;
		Cache_SIZE = size;
		Cache_ASSOC = assoc;
		Cache_REPLACEMENT_POLICY = replacement_policy;
		Cache_INCLUSION_PROPERTY = inclusion_property;
		

		n_of_sets = Cache_SIZE / (Cache_BLOCKSIZE * Cache_ASSOC);

		//tag_array = new int[n_of_sets][Cache_ASSOC]
		tag_array = new CacheBlock*[n_of_sets];
		for(int i = 0; i < n_of_sets; i++)
		    tag_array[i] = new CacheBlock[Cache_ASSOC];


		// Declare Btree Array;
		// Change the size later
		btreeArray = new int*[n_of_sets];
		for(int i = 0; i < n_of_sets; i++)
		    btreeArray[i] = new int[100];


		strcpy(CacheNAME,CacheNameP);

		initializeTagArray();

		// Initialize Btree Array
		level = log2(Cache_ASSOC);
		zeros = pow(2,level) - 1;
		initTreeArray(Cache_ASSOC, zeros);

		currentFutureCounter = -1;
		// Optimal
		futureAccessCounter = 0;
		//cout << "Cache Object value Initialized";
		//cout << "value: "<< checkPowerOfTwo(asize) << endl;
	}

	AddressData processMemoryAddress(char *str, char *accessType)
	{
		//cout << "\n###CACHE:: <<"<< CacheNAME<<" Processing Address:" << str << "#$#"<<endl;
		// #index_bits is log2(#sets)
	    int indexbits = log2(n_of_sets);

	    // #block_offset_bits is log2(block_size)
	    int blockoffsetbits = log2(Cache_BLOCKSIZE);

	    // #tag_bits is 32 - #index_bits - #block_offset_bits
		int tagbits = LEN - indexbits - blockoffsetbits;
		
		//cout << "bit numbers::" << "tagbits:"<<tagbits <<" indexbits:"<<indexbits<<" offsetbits:"<<blockoffsetbits<<endl;

		int tags=0, index=0, blockOffset=0, write = -1;

		// write: 0=read, 1=write, 2=back_invalidate
		if(strcmp(accessType, "w") == 0){
			write = 1;
		}
		else if (strcmp(accessType, "r") == 0) {
			write = 0; 
			//exit(1);
		}
		else if (strcmp(accessType, "i") == 0) {
			//exit(1);
			write = 2; 
		}
		else {
			exit(1);
		}

		//cout << "ACCESS TYPE: "<<accessType <<" Bit: "<<write<<" #$#" <<endl;
		

		uint64_t num = (uint64_t) strtoul(str, NULL, 16);

		//blockOffset = num << 
		//bin(num); printf("\n");



		tags = extract(num, (LEN-tagbits), LEN);
		index = extract(num, (LEN-(tagbits+indexbits)), (LEN-tagbits));
		blockOffset = extract(num, 0, blockoffsetbits);

		//struct AddressData addr = {tags, index, blockOffset, write, str};
		struct AddressData addr = {tags, index, blockOffset, write};
		
		// Not changing the input address string
		strcpy(addr.address, str);

		return addr;
	}

	void processFutureAccessMap(char *str)
	{
		//cout << "\n###Processing Future Access for CACHE:: <<"<< CacheNAME<<" Processing Address:" << str << "#$#"<<endl;
		// #index_bits is log2(#sets)
	    int indexbits = log2(n_of_sets);
	    cache_index_bits = indexbits;

	    // #block_offset_bits is log2(block_size)
	    int blockoffsetbits = log2(Cache_BLOCKSIZE);

	    // #tag_bits is 32 - #index_bits - #block_offset_bits
		int tagbits = LEN - indexbits - blockoffsetbits;
		cache_tag_bits = tagbits;
		
		//cout << "bit numbers::" << "tagbits:"<<tagbits <<" indexbits:"<<indexbits<<" offsetbits:"<<blockoffsetbits<<endl;

		//int tags, index=0, blockOffset=0, write = -1;
		uint64_t tags_and_index = 0;

		

		uint64_t num = (uint64_t) strtoul(str, NULL, 16);

		//tags = extract(num, (LEN-tagbits), LEN);
		//index = extract(num, (LEN-(tagbits+indexbits)), (LEN-tagbits));

		tags_and_index = extract(num, (LEN-(tagbits+indexbits)), LEN);
		
		//cout <<"FNC: tagbits::" << tagbits <<" indexBits:" << indexbits << " tags_and_index:"<<tags_and_index<< endl;
		//if(futureAccessCounter == 10)
		//	exit(1);

		//futureAccess[tags + index] = futureAccessCounter++;
		//cout << "FUTURE ACCESS COUNTER:: "<<futureAccessCounter <<endl;


		// Set key value with tag_and_index


		vector<uint64_t> accessVector = futureAccess[tags_and_index];
		accessVector.push_back(futureAccessCounter);
		futureAccess[tags_and_index] = accessVector;
		futureAccessCounter++;


	}

	CacheAccessReply accessCache(char * addressStr, char *accessType)
	{
		AddressData addr;
		currentFutureCounter++;
		//cout << "\nCurrent Counter" << currentFutureCounter << endl;

		//if(currentFutureCounter == 10){
		//	exit(1);
		//}

		addr = processMemoryAddress(addressStr, accessType);

		struct CacheAccessReply reply = {-1, -1, -1, -1};
		reply.writeback = 0;
		reply.status = -1;
		reply.back_invalidation = 0;

		if(addr.write == 2){
			//cout << "INVALIDATE access:: cache name:"<< CacheNAME << endl;
			//exit(1);
		}

		//cout << "CACHE::accessCache" << endl;
		//cout << "values:Tag:" << addr.tags << " Index:" << addr.index << " Offset:" << addr.blockOffset<<" Write:"<<addr.write<<" Addr Str:"<<addr.address<<"#$#" <<endl;
		//cout << "Access type: " << accessType << endl;

		int hit = 0;
		int maxAccessCounter= 0;
		int LRUReplaceCounter = 10000000;
		int replaceIndex = -1;
		int isCacheFull = 0;
		int invalidBlockIndex = -1;

		// Find invalid block
		for(int i=0; i<Cache_ASSOC; i++)
		{
			if(tag_array[addr.index][i].isInvalid == 1)
			{
				invalidBlockIndex = i;
				break;
			}
		}

		maxAccessCounter = 0;
		
		replaceIndex = -1;

		if (Cache_REPLACEMENT_POLICY == LRU) { 

			int LRUReplaceIndex = -1;


			LRUReplaceCounter = 10000000;
			// Find max access counter in the set
			for(int i=0; i<Cache_ASSOC; i++)
			{
				if(tag_array[addr.index][i].isInvalid == 0)
				{
					isCacheFull++;

					// Finding Highest Counter
					if(tag_array[addr.index][i].accessCounter > maxAccessCounter)
						maxAccessCounter = tag_array[addr.index][i].accessCounter;

					// Finding Lowest Counter
					if(tag_array[addr.index][i].accessCounter < LRUReplaceCounter){

						LRUReplaceCounter = tag_array[addr.index][i].accessCounter;
						LRUReplaceIndex = i;
					}
				}
				
			}

			replaceIndex = LRUReplaceIndex;

		}

		

		if(strcmp(CacheNAME, "L1")==0 && addr.write == 2 ) {
			//cout << "DO NOT MODIFY COUNTER" << "reads/writes" << endl;
			exit(1);
		}
		else {
			if(addr.write == 0){
				Cache_reads++;
			}
			else {
				Cache_writes++;
			}
		}

		

		//cout << "MAX access counter:: "<<maxAccessCounter <<" Filledup Content:"<< isCacheFull <<" LRUReplaceCounter:"<<LRUReplaceCounter<<" replaceIndex:"<<replaceIndex<<"#$#"<< endl;

		hit =0;
		reply.status = 0;
		// Check the tag value in the specified set (using index) in the Cache
		for(int i=0; i<Cache_ASSOC; i++)
		{
			// If tag match found and value is not invalid, then 
			// CACHE HIT
			if(tag_array[addr.index][i].tags == addr.tags && tag_array[addr.index][i].isInvalid == 0){
				hit = 1;


				//if accessType is write, then modify the block and set the dirty bit to 1
				// As we are not working with data, just make the dirty bit to 1
				
				if(addr.write == 1){
					//cout<<"WRITE HIT -Dirty Set- ########################" << endl;
					tag_array[addr.index][i].isDirty = 1;
				}
				else {
					//tag_array[addr.index][i].isDirty = 0;
				}

				if (Cache_REPLACEMENT_POLICY == LRU) {
					// update maxcounter
					tag_array[addr.index][i].accessCounter = maxAccessCounter + 1;	
				}
				else if (Cache_REPLACEMENT_POLICY == PLRU) {

					//cout << endl << endl;
					// Access 'C', tree index = 5
					// Here array Index is i

					int treeArrayIndex = toIndexOfTreeLoc(i, zeros);//getIndexFromTreeLoc(5, zeros);
					//cout << "Access Hit data to btree: Tree index: "<< i <<" ::Array Index" << treeArrayIndex << endl;

					accessTree(addr.index, treeArrayIndex, level);
				} 
				/*else {
					cout << "Wrong replacement" << endl;
					exit(1);
				}*/
				
				
				// set the status to One, 1: hit
				reply.status = 1;
	

				break;
			}
		}

		if(hit == 1){
			//cout << "HIT" << endl;

		}
		else {

			//cout << "MISS" << endl;
			
			if(strcmp(CacheNAME, "L1")==0 && addr.write == 2 ) {
				//cout << "DO NOT UPDATE:: Returning from here:: " << endl;
				exit(1);
			}
			else {
				if(addr.write == 0)
					Cache_read_misses++;
				else if(addr.write == 1)
					Cache_write_misses++;
			}
			

			// status 0: miss
			reply.status = 0;
			reply.writeback = 0;

			/*if(strcmp(accessType, "w") == 0){
					cout<<"########################" << endl;
					tag_array[addr.index][i].isDirty = 1;
				}*/

			// add the value in Cache
			// First check if the cache is already full. Then use the specified replacement policy to evict the element and update cache.
			// Otherwise, add the element in the cache.
			//int bothblock = 0;

			if(invalidBlockIndex == -1)//isCacheFull >= Cache_ASSOC)
			{
				// Cache is full. Use eviction
				// If LRU, replace Index already updated
				//cout<<"CACHE FULL: EVICTION: replaceIndex::" << replaceIndex << endl;

				if (Cache_REPLACEMENT_POLICY == PLRU) 
				{

					// Update replaceIndex

					// NOW REPLACE 'B'
					// Check whether algo also returns B
					//cout << endl << endl;
					replaceIndex = replaceTree(addr.index, level);
				
					//cout << "REPLACE INDEX FOR PLRU:::"<<replaceIndex << endl;

					//cout << "READ again with plru" << endl;
					int treeArrayIndex = toIndexOfTreeLoc(replaceIndex, zeros);//getIndexFromTreeLoc(5, zeros);
					//cout << "Access Hit data to btree: Tree index: "<< replaceIndex <<" ::Array Index" << treeArrayIndex << endl;

					accessTree(addr.index, treeArrayIndex, level);

				}
				else if (Cache_REPLACEMENT_POLICY == OPTIMAL) 
				{
					// Optimal replacement policy
					int highestFutureCounter = -1;
					int tempReplacingIndex= -1;

					//int repCounterForNofuture = -1;;

					// Find max access counter in the set
					//cout <<"#@# OPTIMAL EVICTION: \n";


					for(int i=0; i<Cache_ASSOC; i++)
					{
						
						if(tag_array[addr.index][i].isInvalid == 0)
						{
							//isCacheFull++;
							
							//int futureCounter = futureAccess[(tag_array[addr.index][i].tags + addr.index)];
							

							uint64_t tag_and_index = 0;
							uint64_t num1 = (uint64_t) strtoul(tag_array[addr.index][i].address, NULL, 16);
							tag_and_index = extract(num1, (LEN-(cache_tag_bits + cache_index_bits)), LEN);

							//cout << "tag_bits::" << cache_tag_bits <<" "<< cache_index_bits<< " "<< tag_and_index <<endl;
							

							vector<uint64_t> accessVector = futureAccess[tag_and_index];//futureAccess[(tag_array[addr.index][i].tags + addr.index)];
							
							if(accessVector.empty()){
								cout <<"EMPTY FOUND:"<<endl;
								exit(1);

							}

							//accessVector.push_back(futureAccessCounter++);
							
							//int noFuture = 0;
							//int tempFutureCounter=-1;
							
							int futureCounter = -1;

							for(auto it = accessVector.begin(); it!= accessVector.end(); ++it)
							{
							    uint64_t a = *it;
							    if(a > currentFutureCounter){
							      futureCounter = a;
							    	break;
							    }
							    

							}	

							if(futureCounter == -1)
							{
								tempReplacingIndex = i;
								break;
								//repCounterForNofuture = i;
								//flag = 0;
								//exit(1);
								//futureCounter = tempFutureCounter;
							}
							//cout << "Future Count:"<<futureCounter<<" Index:"<<i<<endl;

							if(futureCounter > highestFutureCounter)
							{
								highestFutureCounter = futureCounter;
								tempReplacingIndex = i;
								//cout << "higher taken:::" << endl;
							}

							// Finding Highest Counter
							/*if(tag_array[addr.index][i].accessCounter > maxAccessCounter)
								maxAccessCounter = tag_array[addr.index][i].accessCounter;

							// Finding Lowest Counter
							if(tag_array[addr.index][i].accessCounter < LRUReplaceCounter){

								LRUReplaceCounter = tag_array[addr.index][i].accessCounter;
								LRUReplaceIndex = i;
							}*/
						}
						
					}

					if(tempReplacingIndex == -1){
						cout <<"YES::";
						exit(1);

						replaceIndex = 0;
					}
					else {
						replaceIndex = tempReplacingIndex;	
					}
					/*else{
						cout <<"HERE";
						exit(1);
					}*/

					

					//cout << "REPLACE INDEX::" << replaceIndex << endl;

					//cout << "FINAL:::::Future Count:"<<highestAccessCounter<<" Index:"<<replaceIndex<<endl;


				}

				if(replaceIndex == -1)
				{
					cout << "Negetive Replace Index" << endl;
					exit(1);
				}
				//exit(1);

				//cout << "ReplaceIndex: "<< replaceIndex << " REPLACEMENT_POLICY" << Cache_REPLACEMENT_POLICY << endl;

				// If the victim block is dirty, do Write Back to the next level
				if(tag_array[addr.index][replaceIndex].isDirty == 1) 
				{

					//cout << "INVALID REPLACE INDEX::" << replaceIndex << endl;
					// If the replaced block (victim block) is dirty, then do WB
					// send this info to L2
					// Send status, tag, set_no
					reply.status = 0;
					

					strcpy(reply.address, tag_array[addr.index][replaceIndex].address);
					//reply.address = tag_array[addr.index][LRUReplaceIndex].address;
					//reply.set_no = addr.index; 
					reply.writeback = 1;

					if(strcmp(CacheNAME, "L1")==0 && addr.write == 2 ) {
						//cout << "DO NOT MODIY\n";
						exit(1);
					}
					else {
						Cache_write_backs++;
					}
					
					//cout<<"Victim BLOCK EVICTION########################" << endl;

					//cout << "EVICTED ADDRESS::" << tag_array[addr.index][replaceIndex].address << endl;

					//return reply;



					// For inclusive cache, set reply:
					// back_invalidation to 1
					// Evicted address in the reply.address
					if (Cache_INCLUSION_PROPERTY == INCLUSIVE){

						if(strcmp(CacheNAME, "L2") == 0) {
							//cout << "Eviction (for READ/WRITE MISS) happened in L2 address::" <<tag_array[addr.index][replaceIndex].address<< endl;

							reply.status = -1;
							reply.writeback = -1;
							//cout << "\n\nTEST";

							strcpy(reply.address, tag_array[addr.index][replaceIndex].address);
							reply.back_invalidation = 1;
							//reply.address = tag_array[addr.index][LRUReplaceIndex].address;
							//reply.set_no = addr.index; 
							
						}
					}

				}


				//cout << "ALLOCATING WITH Replacement" << endl;
				tag_array[addr.index][replaceIndex].tags = addr.tags;
				tag_array[addr.index][replaceIndex].accessCounter = maxAccessCounter + 1;
				//tag_array[addr.index][LRUReplaceIndex].isDirty = 0;
				
				strcpy(tag_array[addr.index][replaceIndex].address, addr.address);
				//tag_array[addr.index][LRUReplaceIndex].address = addr.address;//addressStr;
				tag_array[addr.index][replaceIndex].isInvalid = 0;



				//tag_array[addr.index][LRUReplaceIndex].isDirty = 0;
				if(addr.write == 1){
					//cout<<"Dirty Bit SET ########################" << endl;
					tag_array[addr.index][replaceIndex].isDirty = 1;
				}
				else {
					tag_array[addr.index][replaceIndex].isDirty = 0;
				}


			}
			else if (invalidBlockIndex != -1){

				if (Cache_REPLACEMENT_POLICY == PLRU) {

					//cout << endl << endl;
					// Access 'C', tree index = 5
					// Here array Index is i

					int treeArrayIndex = toIndexOfTreeLoc(invalidBlockIndex, zeros);//getIndexFromTreeLoc(5, zeros);
					//cout << "Access Hit data to btree: Tree index: "<< invalidBlockIndex <<" ::Array Index" << treeArrayIndex << endl;

					accessTree(addr.index, treeArrayIndex, level);
				} 

				//cout << "ALLOCATING WITH OUT Replacement" << endl;
				// Cache not full. Add the element in cache
				tag_array[addr.index][invalidBlockIndex].tags = addr.tags;
				tag_array[addr.index][invalidBlockIndex].accessCounter = maxAccessCounter + 1;
				strcpy(tag_array[addr.index][invalidBlockIndex].address, addr.address);//addressStr;
				tag_array[addr.index][invalidBlockIndex].isInvalid = 0;

				//tag_array[addr.index][invalidBlockIndex].isDirty = 0;
				
				if(addr.write == 1){
					//cout<<"Dirty Bit SET ########################" << endl;
					tag_array[addr.index][invalidBlockIndex].isDirty = 1;
				}
				else {
					tag_array[addr.index][invalidBlockIndex].isDirty = 0;
				}

			}
			else {
				//cout << "Exited at line 602" << endl; 
				exit(1);
			}			
			
		} 
		return reply;
	}

	CacheAccessReply backInvalidateL1Cache(char * addressStr, char *accessType)
	{


		AddressData addr;

		addr = processMemoryAddress(addressStr, accessType);

		struct CacheAccessReply reply = {-1, -1, -1, -1};
		reply.writeback = 0;
		reply.status = -1;
		reply.back_invalidation = 0;

		///cout << "Back Invaldation::: req::"<<addr.address << endl;

		if(addr.write == 2){
			//cout << "Back Invaldation:::" <<endl;
			//cout << "INVALIDATE access:: cache name:"<< CacheNAME << endl;
			
		}

		

		//int hit = 0;

		// Find invalid block
		for(int i=0; i<Cache_ASSOC; i++)
		{
			if(tag_array[addr.index][i].tags == addr.tags && tag_array[addr.index][i].isInvalid == 0)
			{
				if ( tag_array[addr.index][i].isDirty == 1){
					//tag_array[addr.index][i].isInvalid = 1;
					// Direct Read to memory needed
					
					extra_memory++;
				}

				tag_array[addr.index][i].isInvalid = 1;

				//cout << "FOUND:: invalidated;\n";
				//exit(1);
				//return reply;
				break;
			}
		}

		//cout << "MISS::not invalidated;\n";
		//exit(1);
		return reply;

		
	}

	void initializeTagArray()
	{

		int i, j;
		for(i=0; i<n_of_sets; i++)
		{
			//cout << "Set\t" << std::dec << i <<":\t";
			for(j=0; j<Cache_ASSOC; j++)
			{
				//cout <<"$" << Cache_ASSOC<< " ";
				tag_array[i][j].isDirty = 0;
				tag_array[i][j].isInvalid = 1;
				
			}
			//cout << endl;
		}

		Cache_reads = 0;
		Cache_read_misses = 0;
		Cache_writes = 0;
		Cache_write_misses = 0;
		Cache_miss_rate = 0.0;
		Cache_write_backs = 0;
	}

	void printTagArray()
	{

		int i, j;
		for(i=0; i<n_of_sets; i++)
		{
			cout << "Set\t" << std::dec << i <<":\t";
			for(j=0; j<Cache_ASSOC; j++)
			{
				//cout <<"$" << Cache_ASSOC<< " ";

				//cout << std::hex << tag_array[i][j].tags<< ((tag_array[i][j].isDirty == 1)?" D":"")<< "\t";
				cout << std::hex << std::left << setw(6) << tag_array[i][j].tags;

				if((tag_array[i][j].isDirty == 1))
					cout <<" D";
				else cout << "  ";
				
				cout << "\t";
			}
			cout << endl;
		}
	}

	void printCacheProperties(){


		//cout << "Params:"<<Cache_BLOCKSIZE << " " << Cache_SIZE << " " << Cache_ASSOC << " " << Cache_REPLACEMENT_POLICY << " " <<Cache_INCLUSION_PROPERTY << endl;
		//cout << "Other: Sets:"<< n_of_sets <<endl;
	}


	void printid()
	{

		cout << "print cache\n";
	}
};



int main(int argc, char *argv[])
{
	//CACHE L1_cache, L2_cache(8);

	if(argc <= 8) {
        printf("%s should be sim_cache <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_PROPERTY> <trace_file>\n", argv[0]);
        return -1;
    }

    int BLOCKSIZE = atoi(argv[1]);

    int L1_SIZE = atoi(argv[2]);
    int L1_ASSOC = atoi(argv[3]);

    int L2_SIZE = atoi(argv[4]);
    int L2_ASSOC = atoi(argv[5]);

    // 0:LRU, 1:PLRU, 2:Optimal.
    int REPLACEMENT_POLICY = atoi(argv[6]);
    int INCLUSION_PROPERTY = atoi(argv[7]);
    char *trace_file = argv[8];


    // Initialize L1_CACHE
    CACHE L1_CACHE(BLOCKSIZE, L1_SIZE, L1_ASSOC, REPLACEMENT_POLICY, INCLUSION_PROPERTY, L1_NAME_STR);


    CACHE *L2_CACHE;

    if (L2_SIZE != 0){
    	// Initialize L2_CACHE
	    L2_CACHE = new CACHE(BLOCKSIZE, L2_SIZE, L2_ASSOC, REPLACEMENT_POLICY, INCLUSION_PROPERTY, L2_NAME_STR);
    }
    //exit(1);
    
    
	//L1_cache.printid();
	L1_CACHE.printCacheProperties();

	

	
	FILE *fp;
	char inputLine1[30];
	fp = fopen(trace_file, "r");

	if(fp == NULL)
	{

		perror("Unable to open file!");
		exit(1);
	}

	//char *line;
	//char ch;
	int l = 0;

	// int WRITE_COUNTER = 0;
	// int READ_COUNTER = 0;
	int Total_LINES = 0;
	int ERROR_LINES = 0;
	// int ERROR_LINES1 = 0;
	int ERROR_LINES2 = 0;
	// int ERROR_LINE_TRACK =0;

	// Preprocess input trace for future access
	// Optimal replacement policy
	if(REPLACEMENT_POLICY == OPTIMAL){
//		cout << "OPTIMAL";

		while (fgets(inputLine1, 20, fp) != NULL){

			
			Total_LINES++;
			// cout << "READ LINE::" << inputLine1 << endl;

			//if (Total_LINES == 10)
			//	break;
			string s1 = inputLine1;
			boost::trim_right(s1);
			boost::trim_left(s1);

			if (s1.length() == 0)
			{

				ERROR_LINES++;
				continue;
			}

			// cout << "*\n***********************\n";
			// cout <<"LINE FROM INPUT FILE::"<< s1 << endl;

			string sub1 = s1.substr(0,1);
			string sub2 = s1.substr(3);

			if(s1.length() == 13){
				s1 = s1.substr(3);
			}


			strcpy(inputLine1, s1.c_str()); 

			char *token1 = strtok(inputLine1, " ");
			char *token2 = strtok(NULL, "\n");

			

			if(token1!=NULL && token2 != NULL){
	

				//cout << "PARSED VALUES:" << token1 <<"#"<<token2<<"#" << endl;

				L1_CACHE.processFutureAccessMap(token2);

				if(L2_SIZE != 0)
				{
					L2_CACHE->processFutureAccessMap(token2);
				}


			}
			//else {
			//	ERROR_LINES1++;
			//	
			//}
		}
	}

	if(fp!=NULL){
		fclose(fp);
	}

	//exit(1);

	FILE *fp1;
	char inputLine[30];
	fp1 = fopen(trace_file, "r");

	if(fp1 == NULL)
	{

		perror("Unable to open file!");
		exit(1);
	}

	while (fgets(inputLine, 20, fp1) != NULL){

		Total_LINES++;
		//cout << "READ LINE::" << inputLine << endl;

		//if (Total_LINES == 10)
		//	break;
		string s1 = inputLine;
		boost::trim_right(s1);
		boost::trim_left(s1);

		if (s1.length() == 0)
		{

			ERROR_LINES++;
			continue;
		}
		//cout << "*\n***********************\n";
		//cout <<"LINE FROM INPUT FILE::"<< s1 << endl;
		//cout <<"@@LEN:"<<s1.length()<<endl;
		string sub1 = s1.substr(0,1);
		string sub2 = s1.substr(3);
		//cout <<"NEW$$"<<sub1<<":"<<sub2<<"##LEN:"<<s1.length()<<endl;

		if(s1.length() == 13){
			s1 = s1.substr(3);
		}

		/*string token1 = s1.substr(0,1);
		string token2 = s1.substr(2);*/

		strcpy(inputLine, s1.c_str()); 

		char *token1 = strtok(inputLine, " ");
		char *token2 = strtok(NULL, "\n");

		if(token2 == NULL){
			ERROR_LINES2++;
		}
		

		if(token1!=NULL && token2 != NULL){
			//cout << "TOKEN1::::::::" << token1<< endl;


			//cout << "PARSED VALUES:" << token1 <<"#"<<token2<<"#" << endl;
			//if(token1=="w")
				
			//CacheAccessReply emptyReply = {-1,-1,-1};

			CacheAccessReply L1_reply = L1_CACHE.accessCache(token2, token1);

			//cout << "REPLY:: tag:" << L1_reply.tags << " status:"<<L1_reply.status<<endl;
			// L1_reply:: 1=hit, 2=miss, 3=WB;

			// The L2 Cache call will only depend on L1 Reply, if L1 reply is hit, then not call to L2
			// If L1_Miss (read or write miss), L2 access will call.
			// If during L1 access, any eviction causes WB, then L2 access will call.
			
			int L1_reply_status = -1;

			L1_reply_status = L1_reply.status;

			if(L2_SIZE != 0)
			{
				//char invalidateStr[] = "i";

				if (L1_reply_status == 0 && L1_reply.writeback == 1) // L1 Write Back		
				{
					//char writeStr[] = WRITE_STR;
					CacheAccessReply L2_reply1 = L2_CACHE->accessCache(L1_reply.address, WRITE_STR);
				
					if(INCLUSION_PROPERTY == INCLUSIVE){
						if(L2_reply1.back_invalidation == 1)
						{
							char addressNew[50]; 
							strcpy(addressNew, L2_reply1.address);
							// Invalidate the block in L1 cache
							L1_CACHE.backInvalidateL1Cache(addressNew, INVALIDATE_STR);
						}
					}
				}

				//L1_reply.writeback = 0;
				if(L1_reply_status == 0 ) // L1 cache miss -- READ/WRITE MISS
				{
					//char readStr[] = READ_STR;

					CacheAccessReply L2_reply2 = L2_CACHE->accessCache(token2, READ_STR);
					//CacheAccessReply L2_reply = L2_CACHE->accessCache(token2, carr, emptyReply);
					//int L2_reply;	
					if(INCLUSION_PROPERTY == INCLUSIVE) 
					{
						if(L2_reply2.back_invalidation == 1){
							// Invalidate the block in L1 cache
							char addressNew[50]; 
							strcpy(addressNew, L2_reply2.address);
							L1_CACHE.backInvalidateL1Cache(addressNew, INVALIDATE_STR);
						}
					}
					
				}
				
			}
			

			//printf("TOKENS:: ::%s::%s::%s\n", token1, token2, inputLine);

			l++;
		}
		//else {
		//	ERROR_LINES1++;
		//	
		//}
	}

	if(fp1 != NULL) {
		fclose(fp1);
	}
/*
  int BLOCKSIZE = atoi(argv[1]);

    int L1_SIZE = atoi(argv[2]);
    int L1_ASSOC = atoi(argv[3]);

    int L2_SIZE = atoi(argv[4]);
    int L2_ASSOC = atoi(argv[5]);

    // 0:LRU, 1:PLRU, 2:Optimal.
    int REPLACEMENT_POLICY = atoi(argv[6]);
    int INCLUSION_PROPERTY = atoi(argv[7]);
    char *trace_file = argv[8];
    */
    char replaceSTR[3][15] ={"LRU", "Pseudo-LRU", "OPTIMAL"};
    char inclusionSTR[2][15] ={"non-inclusive", "inclusive"};

	cout <<"===== Simulator configuration =====" << endl;
	cout <<"BLOCKSIZE:\t\t" << BLOCKSIZE << endl;
	cout <<"L1_SIZE:\t\t"<<  L1_SIZE << endl;
	cout <<"L1_ASSOC:\t\t"<< L1_ASSOC << endl;
	cout <<"L2_SIZE:\t\t"<< L2_SIZE << endl;
	cout <<"L2_ASSOC:\t\t"<< L2_ASSOC << endl;
	cout <<"REPLACEMENT POLICY:\t"<< replaceSTR[REPLACEMENT_POLICY] << endl;
	cout <<"INCLUSION PROPERTY:\t" << inclusionSTR[INCLUSION_PROPERTY] << endl;
	cout <<"trace_file:\t\t" << trace_file <<endl;


	// Set Stat values in the global stat variables;

	cout << "===== L1 Contents =====" << endl;

	L1_CACHE.printTagArray();

	if (L2_SIZE !=0 )
	{
		cout << "===== L2 Contents =====" << endl;
		L2_CACHE->printTagArray();
	}

/*
	int Cache_reads;
	int Cache_read_misses;
	int Cache_writes;
	int Cache_write_misses;
	float Cache_miss_rate;
	int Cache_write_backs;
*/
	int extra_mem = L1_CACHE.extra_memory;

	if(L2_SIZE ==0){
		printSimulationResults(L1_CACHE.Cache_reads, L1_CACHE.Cache_read_misses, L1_CACHE.Cache_writes, L1_CACHE.Cache_write_misses, L1_CACHE.Cache_write_backs, 0,0,0,0,0,INCLUSION_PROPERTY, extra_mem);
	}
	else {
		printSimulationResults(L1_CACHE.Cache_reads, L1_CACHE.Cache_read_misses, L1_CACHE.Cache_writes, L1_CACHE.Cache_write_misses, L1_CACHE.Cache_write_backs, L2_CACHE->Cache_reads, L2_CACHE->Cache_read_misses, L2_CACHE->Cache_writes, L2_CACHE->Cache_write_misses, L2_CACHE->Cache_write_backs, INCLUSION_PROPERTY, extra_mem);
		//cout << "L2 zero";
	}
	/*cout << endl <<endl<<endl;

	cout << "===== Simulation results (raw) =====" << endl;
	
	char l1name[] = "L1";
	char l2name[] = "L2";
	L1_CACHE.printCacheSimulationResults(l1name);
	if (L2_SIZE != 0)
	L2_CACHE->printCacheSimulationResults(l2name);
	*/



	
	return 0;
}