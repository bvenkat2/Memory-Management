/***************************************************************
 * File: asgn3_support.c
 * Description: Support file containing method definitions for 
 * functions defined in asgn3_support.h. asgn3.c used to test
 * functions.
 * Author: Bharat Venkat
 * Date: 03/17/19
 * ************************************************************/

//imports
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <asgn3_support.h>
// Don't include stdlb since the names will conflict?



struct block_meta {
  	size_t size;
	struct block_meta *next;
	struct block_meta *prev;
	int slack;	//used to keep track of slack bytes. neccesary to calculate total fragmentation
  	int free;
  	int magic;    // For debugging only. TODO: remove this in non-debug mode.
};

#define META_SIZE sizeof(struct block_meta)

void *global_base = NULL;

/**************************************************************************
 * Function: find_free_block
 * Finds free block using best fit algorithm. Split if block found is too big
 * Inputs: Pointer to pointer to last block, size needed for block as size_t
 * Outputs: Returns pointer to best fit block
 * ***********************************************************************/
struct block_meta *find_free_block(struct block_meta **last, size_t size) {
	struct block_meta *current = global_base;
	//temporary pointer to store current best fit
	struct block_meta *best_fit = NULL;
	//if current is NULL aka no blocks
	//Take care of dealing with this case later, we return NULL
	//for this or for not finding a valid block all at once
	if(!current){
		best_fit = current;
	}
	//Still have blocks to loop through	
	while(current){
		//Don't have a best fit, create candidate best fit with any free block big enough
		if( !best_fit && current->size > size && current->free){
			//only actaully executes once
			best_fit = current;
		}
		//If we have a best fit but current free block is a better fit
		if(current->size > size && best_fit && current->size < best_fit->size && current->free){
			best_fit = current;
		}
		//Optimum fit, no need to loop anymore
		//No need to check if free, only assign best_fit if block is free
		if(best_fit && best_fit->size == size){
			return best_fit;
		}			
		*last=current;
		current = current->next;
	}
	//if we can't find a free block big enough return NULL
	if(!best_fit){
		return NULL;
	}
	//split blocks if too big
	if(best_fit ->size > size + META_SIZE + 8){
		//new block to add in 
		struct block_meta *split_block = (void*)best_fit + META_SIZE + size;
		//set size to leftover
		split_block->size = best_fit->size - META_SIZE -size;
		//set pointers appropriately
		if(best_fit->next){
			best_fit->next->prev = split_block;
			split_block->next = best_fit->next;
		}
		best_fit->next = split_block;
		split_block->prev = best_fit;
		//update size of best_fit
		best_fit->size = size;
		//mark the new block as unused
		split_block->free = 1;
	}
	return best_fit;
}
/****************************************************************************
 * Function request_space
 * Creates new block of requested size and adds it to list
 * Inputs: Pointer to pointer to last block, size needed for block as size_t
 * Outputs: returns new block of size requested
 * *************************************************************************/
struct block_meta *request_space(struct block_meta* last, size_t size) {
	struct block_meta *block;
	//get address for top of heap
	block = sbrk(0);
	//allocate memory
	void *request = sbrk(size + META_SIZE);
	assert((void*)block == request); // Not thread safe.
	//
	if (request == (void*) -1) {
		return NULL; // sbrk failed.
	}
  
	if (last) { // NULL on first request.
		last->next = block;
		block->prev = last;
	}
	block->size = size;
	block->next = NULL;
	block->free = 0;
	block->magic = 0x12345678;
	return block;
}

/******************************************************
 * Function: malloc
 * Allocate a segment of memory in the heap, of at
 * least the given size, and return a pointer to 
 * this segment. Always returns a segement with size
 * that is a multiple of 8
 * Input: Size of desired segment as a size_t
 * Output: allocates segment of memory and returns
 * a pointer to it
 * ****************************************************/
void *malloc(size_t size) {
	struct block_meta *block;
	int temp_size = size;
	if (size <= 0) {
		return NULL;
	}
	//always want to return a segment that is of size
	//which is a multiple of 8
	if(size%8 !=0){
		size = size + 8-size%8;
	}
	if (!global_base) { // First call.
		block = request_space(NULL, size);
  		if (!block) {
  			return NULL;
  		}
		global_base = block;
	} 
	else {
  		struct block_meta *last = global_base;
  		//splitting handled here
		block = find_free_block(&last, size);
		if (!block) { // Failed to find free block.
  		block = request_space(last, size);
  			if (!block) {
				return NULL;
  			}
		} 
		else {   
		// Found free block
			block->free = 0;
			block->magic = 0x77777777;
		}
	}
	//set slack bytes to unused memory
 	block->slack = block->size -temp_size; 
	return(block+1);
}
/**********************************************************
 * Function: calloc
 * Allocates a segment of memory big enough to hold 
 * n items of x size each. Zeros out memory. Returns
 * a pointer to the beginning of the segment.
 * Input: number of elements and the size of each element
 * as size_t
 * Output: Pointer to segment of memory
 * ******************************************************/
void *calloc(size_t nelem, size_t elsize) {
	//size of desired memory block
	size_t size = nelem * elsize;
	//malloc memory
	void *ptr = malloc(size);
	//zero out memory
	memset(ptr, 0, size);
	return ptr;
}


/********************************************
 * function get_block_ptr
 * description: return a pointer to the 
 * metadata of the block
 * input: pointer to memory address
 * output: pointer to block's metadata
 * *****************************************/
struct block_meta *get_block_ptr(void *ptr) {
  return (struct block_meta*)ptr - 1;
}


/********************************************************
 * Function: free
 * Deallocates memory segment. If consecutive free blocks
 * exist they are merged.
 * Input: Pointer to memory to free
 * Output: Deallocates memory segment pointed to  by ptr
 * *****************************************************/
void free(void *ptr) {
	if (!ptr) {
		return;
	}

	struct block_meta* block_ptr = get_block_ptr(ptr);
	assert(block_ptr->free == 0);
	//Merging consecutive free blocks
	struct block_meta* prevPointer = NULL;
	struct block_meta* nextPointer = NULL;
	//Get pointer to previous block if it is free
	if(block_ptr->prev && block_ptr->prev->free){
		prevPointer = block_ptr->prev;
	}
	//Get pointer to next block if it is free
	if(block_ptr->next && block_ptr->next->free){
		nextPointer = block_ptr->next;
	}
	//nextPointer absorbs block_ptr
	if(prevPointer && !nextPointer){
		//combine blocks
		
		//increment size
		prevPointer->size += block_ptr->size + META_SIZE;
		//set pointers appropriately if next block exists
		if(block_ptr->next){	
			prevPointer->next = block_ptr->next;
			block_ptr->next->prev = prevPointer;
		}
		else{
			prevPointer->next = NULL;
		}
		//reset slack bytes and mark as free
		prevPointer->slack = 0;
		prevPointer->free = 1;	

		block_ptr = NULL;
	}
	//block_ptr absorbs nextPointer
	else if(nextPointer && !prevPointer){
		//combine blocks

		//increment size
		block_ptr->size += nextPointer->size + META_SIZE;
		//set pointers appropriately if we have a block after nextPointer
		if(nextPointer->next){
			//remove nextPointer from list
			nextPointer->next->prev = block_ptr;
			block_ptr->next = nextPointer->next;
		}
		else{
			//if second to last element in list, block_ptr becomes last
			block_ptr->next = NULL;
		}
		//reset slack bytes and free block
		block_ptr->slack = 0;
		block_ptr->free = 1;
	
	}
	//prevPointer absorbs both block_ptr and nextPointer
	else if(nextPointer && prevPointer){
		//increment size
		//absorb both metadatas and sizes
		prevPointer->size += META_SIZE*2 + block_ptr->size + nextPointer->size;
		//set pointers appropriately if we have a block after nextPointer
		if(nextPointer-> next){
			nextPointer->next->prev = prevPointer;
			prevPointer->next = nextPointer->next;
		}
		else{
			prevPointer->next = NULL;
		}
		//reset slack bytes and free block
		prevPointer->slack = 0;
		prevPointer->free = 1;

	}

}
/********************************************************************
 * function: realloc
 * description: given a pointer to a block of memory, resize it
 * if requested size is bigger than current size, malloc new and free old
 * if less than current size, split blocks and return new allocated chunk
 * inputs: pointer to memory segment, desired size as a size_t
 * outputs: pointer returned points to a block of memory of a given size
 * *********************************************************************/
void *realloc(void *ptr, size_t size) {
	if (!ptr) { 
  		// NULL ptr. realloc should act like malloc.
		return malloc(size);
	}

	struct block_meta* block_ptr = get_block_ptr(ptr);
	//if we can reuse same block
	if (block_ptr->size >= size) {
		//check if we can split block
		if(block_ptr->size > size + 8 + META_SIZE){
			struct block_meta *node = (void*)block_ptr + size + META_SIZE;
			//set size of new split block
			node->size = block_ptr->size - META_SIZE-size;
			//set size of originial block
			block_ptr->size = size;
			//set pointers
			if(block_ptr->next){
				block_ptr->next->prev = node;
				node->next = block_ptr->next;
			}
			
			node->prev = block_ptr;
			block_ptr->next = node;	
			//mark new node as free
			node->free = 1;
			//return pointer to start of memory
			//+1 because block_ptr points to the metadata
			return (block_ptr+1);
		}

		// return original pointer if we can't split block
  		return ptr;
	}

	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
	void *new_ptr;
	new_ptr = malloc(size);
	if (!new_ptr) {
		return NULL; // TODO: set errno on failure.
	}
	memcpy(new_ptr, ptr, block_ptr->size);
	free(ptr);  
	return new_ptr;
}

/******************************************************
 * function: getLeakage
 * description: determine total memory leakage
 * inputs: none
 * outputs: return sum of internal and external fragmentation
 * ****************************************************/
int getLeakage(){
	int leakage = 0;
	struct block_meta* temp = global_base;
	while(temp){
		//check for external fragmentation
		if(temp->free && (temp->prev != NULL || temp->next != NULL)){
			//append amount of external fragmentation to total count
			leakage+=temp->size;
		}
		//if block is allocated check for internal fragmentation	
		else{
			//append amount of internal fragementation to total count
			leakage+=temp->slack;
		}
		temp = temp->next;
	}
	return leakage;
}
