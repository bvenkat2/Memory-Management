/***************************************************************
 * Author: Bharat Venkat
 * Subject: CMSC 257
 * Assignment: Assignment 3: Memory Management
 * Class: asgn3.c
 * Description: Driver to test Malloc, Free, Calloc, and Realloc
 * written in asgn3_support.c
 * ************************************************************/
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
//Used to check if functions work correctly
//#include <stdlib.h>


#include "asgn3_support.h"

/******************************************************
 * Function: main
 * Description: Driver to test functions. Atleast 
 * 10 calls to malloc, realloc, and calloc and 5 calls 
 * to free.
 * Output: returns 0 if successful
 * ****************************************************/


int main(int argc, char* argv[]){
	//store starting address of heap
	void* startHeap = sbrk(0);
	//declare pointers	
	void* a;
	void* b;
	void* c;
	void* d;
	void* e;
	void* f;
	void* g;
	void* h;
	void* i;
	void* j;
	//10 malloc calls
	a = malloc(10);
	b = malloc(2);
	c = malloc(14);
	d = malloc(34);
	e = malloc(53);
	f = malloc(34);
	g = malloc(54);
	h = malloc(42);
	i = malloc(42);
	j = malloc(46);
	

	//6 free calls
	free(b);
	free(c);
	free(d);
	free(a);
	free(e);
	free(f);
	
	//10 calloc calls
	a = calloc(13, 5);
	b = calloc(17,4);
	c = calloc(9,4);
	d = calloc(53,65);
	e = calloc(435,34);
	f = calloc(42,12);
	g = calloc(32, 1);
	h = calloc(42, 42);
	i = calloc(4, 12);
	j = calloc(42,42);
	
	//10 realloc calls
	a = realloc(a,7);
	b = realloc(b, 12);
	c = realloc(c,32);
	d = realloc(d, 2);
	e = realloc(f, 4);
	f = realloc(e, 45);
	g = realloc(a, 42);
	h = realloc(h, 5);
	i = realloc(i, 9);
	j = realloc(j, 6);
	//store ending address of heap
	void* endHeap = sbrk(0);
	//print leakage and heap addresses
	printf("%s, %i\n","Total memory leakage is:", getLeakage());
	printf("%s %p\n","Starting address of heap is:", startHeap);
	printf("%s %p\n","Ending address of heap is:", endHeap);
	return 0;
}


