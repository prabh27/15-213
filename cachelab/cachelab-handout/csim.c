#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

typedef struct line {
	int tag;
	int time;
	int valid;
}line;


char *quads[16] = { "0000", "0001", "0010", "0011",
				   "0100", "0101", "0110", "0111",
				   "1000", "1001", "1010", "1011",
				   "1100", "1101", "1110", "1111" };

int hits = 0, misses = 0, evictions = 0;

line** create_cache(int setBits, int lines, int blocks) {
	int sets = 2<<(setBits - 1);
	line ** cache = (line **)malloc((sets) * sizeof(line**));
	for (int i = 0; i < sets; i++) {
		cache[i] = (line *)malloc(lines * sizeof(line));
		for(int j = 0; j < lines; j++) {
			cache[i][j].valid = 0;
		}
	}
	return cache;
}

void get_hex(char *buf, char *hex) {
	char *token;
	const char s[2] = ",";
	token = strtok(buf, s);
	int tokenLen = strlen(token);
	strncpy(hex, token+3, tokenLen - 2);
}

int binary_to_decimal (char *bin) {
	int binLen = strlen(bin);
	int decimal = 0;
	for(int i = 0; i < binLen; i++) {
		char c = bin[i];
		if(c == '0')
			decimal = decimal*2;
		else
			decimal = decimal*2 + 1;
	}
	return decimal;
}

void load_address(int set, int tag, int lines, line ** cache, int verbose, int time) {
	// Check hit.
	for(int i = 0; i < lines; i++) {
		if(cache[set][i].valid == 1) {
			if(cache[set][i].tag == tag) {
				hits++;
				cache[set][i].time = time;
				printf("hit\n");
				return;
			}
		}
	}

	// Check if any line empty.
	for(int i = 0; i < lines; i++) {
		if(cache[set][i].valid == 0) {
			cache[set][i].valid = 1;
			cache[set][i].tag = tag;
			cache[set][i].time = time;
			misses++;
			printf("miss\n");
			return;
		}
	}

	// Evict if no line empty.
	line least_recent = cache[set][0];
	int evicted_line = 0;
	for(int i = 0; i < lines; i++) {
		if(cache[set][i].time < least_recent.time) {
			least_recent = cache[set][i];
			evicted_line = i;
		}
	}
	misses++;
	evictions++;
	least_recent.time = time;
	least_recent.tag = tag;
	cache[set][evicted_line] = least_recent;
	printf("miss eviction\n");

	return;
}

void print_cache(line ** cache, int lines, int setBits) {
	int sets = 2<<(setBits - 1);
	for(int i = 0; i < sets; i++) {
		printf("Set number: %d\n", i);
		for(int j = 0; j < lines; j++) {
			printf("Line number: %d\n", j);
			printf("Valid: %d\n ", cache[i][j].valid);
			printf("Tag: %d\n", cache[i][j].tag);
			printf("Time: %d\n", cache[i][j].time);
		}
		printf("\n");
	}
	return;
}

int get_set(char *binary, char *set, int setBits, int blocks) {
	int binaryLen = strlen(binary);
	int start = binaryLen - setBits - blocks;
	strncpy(set, binary + start, setBits);
	set[setBits] = '\0';
	// printf("Set is: %s\n", set);
	int decimalSet = binary_to_decimal(set);
	printf("Decimal set is %d\n", decimalSet);
	return decimalSet;
}

int get_tag(char *binary, char *tag, int setBits, int blocks) {
	int binaryLen = strlen(binary);
	int end = binaryLen - setBits - blocks;
	if(end == 0) {
		tag = "0";
	}
	else {
		strncpy(tag, binary, end);
		tag[end] = '\0';
	}
	// printf("Tag is: %s\n", tag);
	int decimalTag = binary_to_decimal(tag);
	printf("Tag is: %d\n", decimalTag);
	return decimalTag;
}

void get_binary(char *hex, char *binary) {
	int j = 0;
	int hexLen = strlen(hex);
	for(int i = 0; i < hexLen; i++) {
		char c = hex[i];
		if (c >= '0' && c <= '9') memcpy(binary + j,     quads[     c - '0'], 4 * sizeof(char));
  		if (c >= 'A' && c <= 'F') memcpy(binary + j,     quads[10 + c - 'A'], 4 * sizeof(char));
  		if (c >= 'a' && c <= 'f') memcpy(binary + j,     quads[10 + c - 'a'], 4 * sizeof(char));
  		j += 4;
	}
	binary[j] = '\0';
	// printf("Binary is: %s\n", binary);
	return;
}

void read_file(char traceFile[100], int setBits, int lines, int blocks, int verbose) {
	
	line ** cache = create_cache(setBits, lines, blocks);
	
	int time = 0;

	// cache[0][0].tag = 0;
	FILE * fp;
    char buf[1024];

    fp = fopen(traceFile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while (fgets(buf, sizeof(buf), fp) != NULL){
    	buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
    	time++;
    	if(verbose)
    		printf("Line is: %s\n", buf);

    	char hex[40];
    	get_hex(buf, hex);

    	char binary[40];
    	get_binary(hex, binary);
    	
    	char set[40];
    	int decimalSet = get_set(binary, set, setBits, blocks);
    	
    	char tag[40];
    	int decimalTag = get_tag(binary, tag, setBits, blocks);

    	if(buf[1] == 'L') {
    		load_address(decimalSet, decimalTag, lines, cache, verbose, time);
    		// print_cache(cache, lines, setBits);
    	}
    	if(buf[1] == 'S') {
    		load_address(decimalSet, decimalTag, lines, cache, verbose, time);
    		// print_cache(cache, lines, setBits);
    	}
    	if(buf[1] == 'M') {
    		load_address(decimalSet, decimalTag, lines, cache, verbose, time);
    		printf("hit\n");
    		hits++;
    	}
    	print_cache(cache, lines, setBits);
    }

    fclose(fp);
    return;
}

int main(int argc, char *argv[])
{
	int verbose = 0;
	int setBits = 0, lines = 0, blocks = 0;
	char traceFile[100];

	for(int i = 1; i < argc; i++) {
		if(strcmp( argv[i], "-h") == 0) {
			// optional help flag.
		}
		if(strcmp( argv[i], "-v") == 0) 
			verbose = 1;
		
		if(strcmp(argv[i], "-s") == 0) 
			setBits = atoi(argv[i+1]);
		
		if(strcmp( argv[i], "-E") == 0)
			lines = atoi(argv[i+1]);

		if(strcmp( argv[i], "-b") == 0)
			blocks = atoi(argv[i+1]);

		if(strcmp( argv[i], "-t") == 0)
			strcpy(traceFile, argv[i+1]);
		
	}

	read_file(traceFile, setBits, lines, blocks, verbose);
	printSummary(hits, misses, evictions);
    return 0;
}

