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

sf_free_list_node * getBlockOfFitSize(sf_free_list_node * head, sf_header header);
sf_free_list_node * get_sf_free_list_node(size_t size);

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
        block_size = block_size + 8;
        if(block_size % 16 != 0){
            block_size += (16 - (block_size % 16));
        }
        if(block_size < 32)
            block_size += 16;

        //gotta check if there is enough bytes to allocate
        size_t remaining_byte = 4096 - 48;
        while (remaining_byte <= block_size){
            sf_mem_grow();
            memset(epilogue,0,8);
            epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
            memset(epilogue,0,8);
            (*epilogue).footer.info.allocated = 1;
            remaining_byte += 4096;
        }

        //the allocated bit of the first allocate block is always 1.
        sf_header block_header;
        block_header.info.requested_size = size;
        block_header.info.block_size = (block_size >> 4);
        block_header.info.prev_allocated = 1;
        block_header.info.allocated = 1;
        memcpy(ptrBeginning + 40, &block_header,8);

        //make a free block header and footer
        //but, what if the function takes up exactly all the memory in the heap, not more and not less?
        remaining_byte -= block_size;
        if(remaining_byte < 32){
            sf_mem_grow();
            memset(epilogue,0,8);
            epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
            memset(epilogue,0,8);
            (*epilogue).footer.info.allocated = 1;
            remaining_byte += 4096;
        }
        void* ptrFreeBlock = ptrBeginning + 40 + block_size;
        sf_header free_block_header;
        free_block_header.info.block_size = (remaining_byte >> 4);
        free_block_header.info.prev_allocated = 1;
        free_block_header.info.allocated = 0;
        memcpy(ptrFreeBlock, &free_block_header,8);

        sf_footer free_block_footer;
        free_block_footer.info.requested_size = 0;
        free_block_footer.info.block_size = (remaining_byte >> 4);
        free_block_footer.info.allocated = 0;
        memcpy(ptrFreeBlock + remaining_byte -8, &free_block_footer,8);

        sf_free_list_node* freeBlockNode = sf_add_free_list(remaining_byte,&sf_free_list_head);
        freeBlockNode->head.links.next = (sf_header*) ptrFreeBlock;
        freeBlockNode->head.links.prev = (sf_header*) ptrFreeBlock;
        ((sf_header*) ptrFreeBlock)->links.next = &(freeBlockNode->head);
        ((sf_header*) ptrFreeBlock)->links.prev = &(freeBlockNode->head);

        return (ptrBeginning + 48);
    }

    else{
        size_t block_size = size;
        block_size = block_size + 8;
        if(block_size % 16 != 0){
            block_size += (16 - (block_size % 16));
        }
        if(block_size < 32)
            block_size += 16;



    }
    return NULL;
}



/* This function returns an address to the sf_free_list_node which contains the size.
**@size size that the sf_free_list_node contains.
**@return the address to the sf_free_list_node that contains the size.
*/
sf_free_list_node * get_sf_free_list_node(size_t size){
    sf_free_list_node* current_node = sf_free_list_head.next;
    while (current_node != &sf_free_list_head){
        if(((current_node-> size) << 4) >= size){
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
