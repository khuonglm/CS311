#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

// word size: 32 bits / 4B
#define WORD_SIZE 32
#define VALID_BIT 0
#define DIRTY_BIT 1

const int inf = (int)1e9;

typedef struct {
	int read;
	int write;
	int writeback;
	int readhit;
	int writehit;
	int readmiss;
	int writemiss;
} cache_stat;

// floor(log2(n))
int get_log(int n) {
	if (n < 0) {
		return 0;
	}

	int cnt = 0;
	while (n) {
		n = n >> 1;
		++cnt;
	}

	return cnt - 1;
}

// return value of i-th bit of x
int get_bit(int x, int i) {
	return (x >> i) & 1;
}

typedef struct {
	// store log2(num_set)
	// easy and efficient to recover 
	// number of sets by 1 << lg_set
	int lg_set;
	// store log2(num_way)
	int lg_way;
	// store log2(block_size)
	int lg_bsize;
	// address = tag + set index + block offset
	// data[num_set][num_way] = address of block + valid + dirty
	// note: block offset is always set to 0 in data[i][j]
	// as block size >= 4, at least 2 LSB are not used 
	// -> use them for valid and dirty 
	int** data;
	// frq[num_set][num_way] = frequency of accessing to data[i][j]
	int** frq;
	// lst[num_set][num_way] = order ways of a specific index 
	// by the last access time (1 is most recent access, 0 for invalid)
	int** lst;
} cache_struct;

// TODO: free cache at the end of program
cache_struct* build_cache(int num_set, int num_way, int block_size)
{
	cache_struct* cache = malloc(sizeof(cache_struct));
	cache->lg_set = get_log(num_set);
	cache->lg_way = get_log(num_way);
	cache->lg_bsize = get_log(block_size);
	cache->data = (int**) malloc(num_set * sizeof(int*));
	cache->frq = (int**) malloc(num_set * sizeof(int*));
	cache->lst = (int**) malloc(num_set * sizeof(int*));

	for(int i = 0; i < num_set; ++i) {
		cache->data[i] = (int*) malloc(num_way * sizeof(int));
		cache->frq[i] = (int*) malloc(num_way * sizeof(int));
		cache->lst[i] = (int*) malloc(num_way * sizeof(int));

		for(int j = 0; j < num_way; ++j) {
			cache->data[i][j] = 0;
			cache->frq[i][j] = 0;
			cache->lst[i][j] = 0;
		}
	}
	return cache;
}

// in this problem, data is address with block offset = 0
// then data = tag + index + 0 => this function return tag + index
int get_data(int val, int lg_bsize) {
	return val >> lg_bsize;
}

// return the value of tag
int get_tag(int val, int lg_bsize, int lg_set) {
	return val >> (lg_bsize + lg_set);
}

// return the value of index
int get_index(int val, int lg_bsize, int lg_set) {
	return (val >> lg_bsize) & ((1 << lg_set) - 1);
}

// op = 0 -> read from cache
// op = 1 -> write to cache
void access_cache(cache_struct *cache, cache_stat *stat, int op, uint32_t addr, int lru_only_flag)
{
	// printf("%x\n", get_data(addr, cache->lg_bsize) << cache->lg_bsize);
	int tag_index = get_data(addr, cache->lg_bsize);
	int index = get_index(addr, cache->lg_bsize, cache->lg_set);
	int num_way = 1 << cache->lg_way;

	// check if addr is already in cache
	int way_index = -1;
	for(int i = 0; i < num_way; ++i) {
		if(get_bit(cache->data[index][i], VALID_BIT) && 
			get_data(cache->data[index][i], cache->lg_bsize) == tag_index) {
			way_index = i;
			break;
		}
	}

	if(way_index == -1) { // miss
		// check if any space left to write in
		for(int i = 0; i < num_way; ++i) {
			if(!get_bit(cache->data[index][i], VALID_BIT)) {
				way_index = i;
				break;
			}
		}
		if(way_index == -1) {
			// have to evict a block
			int minf = inf, lst = 0;
			if(lru_only_flag) {
				for(int i = 0; i < num_way; ++i) {
					if(cache->lst[index][i] > lst) {
						lst = cache->lst[index][i];
						way_index = i;
					}
				}
			} else {
				for(int i = 0; i < num_way; ++i) {
					if(cache->frq[index][i] < minf || 
						(cache->frq[index][i] == minf && lst < cache->lst[index][i])) {
							minf = cache->frq[index][i];
							lst = cache->lst[index][i];
							way_index = i;
						}
				}
			}

			stat->writeback += get_bit(cache->data[index][way_index], DIRTY_BIT);
		}

		cache->data[index][way_index] = (tag_index << cache->lg_bsize) | (op << DIRTY_BIT) | (1 << VALID_BIT);
		cache->frq[index][way_index] = 0;

		if(op) { // write
			++stat->write;
			++stat->writemiss;
		} else { // read
			++stat->read;
			++stat->readmiss;
		}
	} else { // hit
		if(op) { // write
			++stat->write;
			++stat->writehit;
			cache->data[index][way_index] = cache->data[index][way_index] | (1 << DIRTY_BIT);
		} else { // read
			++stat->read;
			++stat->readhit;
		}
	}

	// update frequency and last access
	cache->lst[index][way_index] = 0;
	++cache->frq[index][way_index];
	for(int i = 0; i < num_way; ++i) {
		if(!get_bit(cache->data[index][i], VALID_BIT)) continue;
		++cache->lst[index][i];
	}
}

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */
/*                                                             */
/***************************************************************/
void cdump(int capacity, int num_way, int block_size, int lru_only_flag)
{

	printf("Cache Configuration:\n");
	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", num_way);
	printf("Block Size: %dB\n", block_size);
	if (lru_only_flag)
		printf("Replacement Policy: LRU\n");
	else
		printf("Replacement Policy: LFLRU\n");
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat                                 */
/*                                                             */
/***************************************************************/
void sdump(cache_stat *stat)
{
	printf("Cache Stat:\n");
	printf("-------------------------------------\n");
	printf("Total reads: %d\n", stat->read);
	printf("Total writes: %d\n", stat->write);
	printf("Write-backs: %d\n", stat->writeback);
	printf("Read hits: %d\n", stat->readhit);
	printf("Write hits: %d\n", stat->writehit);
	printf("Read misses: %d\n", stat->readmiss);
	printf("Write misses: %d\n", stat->writemiss);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */
/*                                                             */
/***************************************************************/
void xdump(cache_struct *cache, int num_set, int num_way, int block_size)
{
	printf("Cache Content:\n");
	printf("-------------------------------------\n");
	for (int i = 0; i < num_way; i++)
	{
		if (i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]", i);
	}
	printf("\n");

	for (int i = 0; i < num_set; i++)
	{
		printf("SET[%d]:   ", i);

		for (int j = 0; j < num_way; j++)
		{
			// include only the information of tag and index (block offset must be 0)
			uint32_t cache_block_addr = get_data(cache->data[i][j], cache->lg_bsize) << cache->lg_bsize; 
			printf("0x%08x  ", cache_block_addr);
		}

		printf("\n");
	}

	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf("Usage: %s -c cap:assoc:block_size [-x] [-r] input_trace \n", argv[0]);
		exit(1);
	}

	int capacity;
	int num_way;
	int block_size;
	int xflag = 0;
	int lru_only_flag = 0;

	{
		char *token;
		int option_flag = 0;

		while ((option_flag = getopt(argc, argv, "c:xr")) != -1)
		{
			switch (option_flag)
			{
			case 'c':
				token = strtok(optarg, ":");
				capacity = atoi(token);
				token = strtok(NULL, ":");
				num_way = atoi(token);
				token = strtok(NULL, ":");
				block_size = atoi(token);
				break;
			case 'x':
				xflag = 1;
				break;
			case 'r':
				lru_only_flag = 1;
				break;
			default:
				printf("Usage: %s -c cap:assoc:block_size [-x] [-r] input_trace \n", argv[0]);
				exit(1);
			}
		}
	}

	char *trace_name;
	trace_name = argv[argc - 1];

	FILE *fp;
	fp = fopen(trace_name, "r"); // read trace file

	if (fp == NULL)
	{
		printf("\nInvalid trace file: %s\n", trace_name);
		return 1;
	}

	cdump(capacity, num_way, block_size, lru_only_flag);

	// cache statistics initialization
	cache_stat* stat = malloc(sizeof(cache_stat));
	stat->read = 0;
	stat->write = 0;
	stat->writeback = 0;
	stat->readhit = 0;
	stat->writehit = 0;
	stat->readmiss = 0;
	stat->writemiss = 0;

	// initialize following variables
	int num_set = capacity / (num_way * block_size);
	cache_struct* cache = build_cache(num_set, num_way, block_size);

	char c[100], op;
	char* token;
	uint32_t addr;
	while(fgets(c, sizeof(c), fp)) {
		token = strtok(c, " ");
		op = token[0];
		token = strtok(NULL, " ");
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';
		addr = (uint32_t)strtol(token, NULL, 16);
		access_cache(cache, stat, op == 'W', addr, lru_only_flag);
	}

	sdump(stat);
	if (xflag)
	{
		xdump(cache, num_set, num_way, block_size);
	}

	return 0;
}