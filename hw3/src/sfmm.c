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

void *sf_malloc(size_t size) {
    if(size == 0)
        return NULL;

    if(sf_mem_start() == sf_mem_end()){
        void* ptrNewPage = sf_mem_grow();
        sf_prologue prologue;
        prologue.header = *((sf_header*)(ptrNewPage + 8));
        prologue.footer = *((sf_footer*)(ptrNewPage + 24));
        sf_epilogue epilogue;
        epilogue.footer = *((sf_footer*)(ptrNewPage - 7));
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
        //node is the sentinel of the free list.
        sf_free_list_node *node = &sf_free_list_head;
        node = (*node).next; //now, it is the first node of the list
    }
    return NULL;

}





void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}
