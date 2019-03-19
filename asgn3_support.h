#ifndef ASGN3_SUPPORT_INCLUDED
#define ASGN3_SUPPORT_INCLUDED
/**************************************************************************
 *
 * File: asgn3_support.h
 * Description: This is the header file for the functions for assignment 3
 * of the CMSC257 course. Functions are implemented in the file 
 * asgn3_support.c.
 *
 * Author: Bharat Venkat
 * Created: 03/17/2019
 * ***********************************************************************/


//function prototypes


struct block_meta *find_free_block(struct block_meta **last, size_t size);
struct block_meta *request_space(struct block_meta* last, size_t size);
void *malloc(size_t size);
void *calloc(size_t nelem, size_t elsize);
struct block_meta *get_block_ptr(void *ptr);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
int getLeakage();


#endif
