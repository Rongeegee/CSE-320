/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>


sf_prologue* prologue;
sf_epilogue* epilogue;

void *sf_malloc(size_t size) {
    if(size == 0)
        return NULL;

    if(sf_mem_start() == sf_mem_end()){
        void* ptrBeginning = sf_mem_grow();
        memset(ptrBeginning,0,48);
        memset((sf_mem_end()  - 16),0,16);

        prologue = (sf_prologue*)(ptrBeginning);
        (*prologue).header.info.allocated = 1;
        (*prologue).footer.info.allocated = 1;
        epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
        (*epilogue).footer.info.allocated = 1;

        size_t block_size = size;
        size = size + 8;
        if(block_size % 16 != 0){
            block_size += (16 - (block_size % 16));
        }
        if(block_size < 32)
            block_size += 16;

        //gotta check if there is enough bytes to allocate
        size_t remaining_byte = 4096 - 64;
        while (remaining_byte <= block_size){
            sf_mem_grow();
            memset(epilogue,0,8);
            epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
            memset(epilogue,0,8);
            (*epilogue).footer.info.allocated = 1;
            remaining_byte += 4096;
        }

        //prev_allocated bit hasn't been figured out yet.
        sf_header block_header;
        block_header.info.requested_size = size;
        block_header.info.block_size = (block_size >> 4);
        block_header.info.prev_allocated = 1;
        block_header.info.allocated = 1;
        memcpy(ptrBeginning + 40, &block_header,8);



        sf_footer block_footer;
        block_footer.info.requested_size = size;
        block_footer.info.block_size = (block_size >> 4);
        block_footer.info.allocated = 1;
        memcpy(ptrBeginning + 48 + block_size,&block_footer,8);

    }

    else{
        sf_header header;
        header.info.block_size += size;
        if(header.info.block_size % 16 != 0){
            header.info.block_size += (16 - (header.info.block_size % 16));
        }
        if(header.info.block_size < 32)
            header.info.block_size += 16;

        //now search the free list to obtain the first block on the first free list.
        //head is the sentinel of the free list.
        // sf_free_list_node *head = &sf_free_list_head;
        // while (getBlockOfFitSize(head,header) == NULL){
        //     void* ptrBeginning = sf_mem_grow();
        //     if(sf_mem_grow() == NULL){
        //         sf_errno = ENOMEM;
        //         return NULL;
        //     }
        //     else{

        //     }

        // }
    }
    return NULL;
}

/*
*sf_free_list_node function returns the sf_free_list_node that fits the size requirement, or NULL if no such block exists
*But, I need to find the best fit block header to reduce fragmentation.
*/
sf_free_list_node * getBlockOfFitSize(sf_free_list_node * head, sf_header header){
        //now, head is the first node of the list
        sf_free_list_node* current_node = (*head).next;
        while(current_node != &sf_free_list_head){
            if(((current_node->size) << 4) >= header.info.block_size){
                    return current_node;
                }
            current_node = (*current_node).next;
            }
        return NULL;
}




void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}
