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

sf_free_list_node* get_sf_free_list_node(size_t size);
sf_free_list_node* set_sf_free_list_node(size_t size);
void removeFromFreelist(sf_header* header);
void addFreeHeader(sf_header* header);

sf_prologue* prologue;
sf_epilogue* epilogue;
sf_header* lastFreeHeader;
sf_footer* lastFreeFooter;

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
        lastFreeHeader = (sf_header*) ptrFreeBlock;

        sf_footer free_block_footer;
        free_block_footer.info.requested_size = 0;
        free_block_footer.info.block_size = (remaining_byte >> 4);
        free_block_footer.info.allocated = 0;
        free_block_footer.info.prev_allocated = 1;
        memcpy(ptrFreeBlock + remaining_byte - 8, &free_block_footer,8);
        lastFreeFooter = (sf_footer*) (ptrFreeBlock + remaining_byte - 8);

        sf_free_list_node* freeBlockNode = sf_add_free_list(remaining_byte,&sf_free_list_head);
        freeBlockNode->head.links.next = (sf_header*) ptrFreeBlock;
        freeBlockNode->head.links.prev = (sf_header*) ptrFreeBlock;
        ((sf_header*) ptrFreeBlock)->links.next = &(freeBlockNode->head);
        ((sf_header*) ptrFreeBlock)->links.prev = &(freeBlockNode->head);
        sf_show_heap();
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
        if(get_sf_free_list_node(size) == NULL){
            if (((sf_footer*)(sf_mem_end()-16)) == lastFreeFooter){
                size_t numOfBytesHad = lastFreeFooter->info.block_size << 4;
                while(numOfBytesHad < block_size){
                    sf_mem_grow();
                    epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
                    memset(epilogue,0,8);
                    (*epilogue).footer.info.allocated = 1;
                    numOfBytesHad += 4096;
                }
                //now numOfBytesHad will be the size of the new coallesce free block.
                memset(lastFreeHeader + (lastFreeHeader->info.block_size) -8, 0, 8); //But, is it (calling memset) really necessary?
                memcpy(sf_mem_end() - 8,epilogue,8);
                lastFreeHeader->info.block_size = numOfBytesHad;
                lastFreeFooter = (sf_footer*)(sf_mem_end() - 16);
                lastFreeFooter-> info.block_size = numOfBytesHad;
                lastFreeFooter-> info.prev_allocated = lastFreeHeader->info.prev_allocated;
                lastFreeFooter-> info.allocated = 0;

                // if(block_size == numOfBytesHad){
                //     sf_header block_header = {size, block_size, 0,lastFreeHeader->info.prev_allocated,1};

                // }

            }
            else{
                // sf_mem_grow();
                // sf_free_list_node* ptrNewFreelistNode = set_sf_free_list_node(size);
            }


        }
        else{

        }
    }
    return NULL;
}


/*
** This function removes a sf_header from the circular doubly linked list
*@header the block header you are going to remove from the circular doubly linked list
*/
void removeFromFreelist(sf_header* header){
    sf_header* prev_free_header = header->links.prev;
    sf_header* next_free_header = header->links.next;
    prev_free_header->links.next = next_free_header;
    next_free_header->links.prev = prev_free_header;
    header->links.prev = NULL;
    header->links.next = NULL;

}

/*
*This function coallesce the free block pointed by freeBlockHeader with previous or next block if existed, and return the pointer to the
*coallesced free block. Otherwise, return null if previous block and next block are not free
*/
sf_header* coallesce(sf_header* freeBlockHeader){
        int isLastFreeBlock;
        if((freeBlockHeader + freeBlockHeader->info.block_size) == (sf_mem_end()-8)){
            isLastFreeBlock = 1;
        }
        else{
            isLastFreeBlock = 0;
        }
        if(isLastFreeBlock == 1){
            if(freeBlockHeader->info.prev_allocated == 1){
                return NULL;
            }
            else{
                sf_footer* prevBlKfterPtr = (sf_footer*) freeBlockHeader - 8;
                size_t prevBlkSize = prevBlKfterPtr->info.block_size;
                size_t currBlkSize = freeBlockHeader->info.block_size;
                sf_header* prevBlkHdPtr = (sf_header*)(prevBlKfterPtr - prevBlkSize + 8);
                sf_footer* freeBlockFooter = (sf_footer*)freeBlockHeader + currBlkSize - 8;
                size_t combined_blkSize = currBlkSize+ prevBlkSize;
                prevBlkHdPtr->info.block_size = combined_blkSize;
                freeBlockFooter->info.block_size = combined_blkSize;
                removeFromFreelist(prevBlkHdPtr);
                removeFromFreelist(freeBlockHeader);
                addFreeHeader(prevBlkHdPtr);
                memset(prevBlKfterPtr,0,8);
                memset(freeBlockHeader,0,8);
                return prevBlkHdPtr;
            }
        }
        else{
            sf_footer* freeBlockFooter = (sf_footer*)(freeBlockHeader + freeBlockHeader->info.block_size - 8);
            sf_header* nextBlockHeader = freeBlockHeader + freeBlockHeader->info.block_size;
            unsigned prevBlkAllocated = freeBlockHeader->info.prev_allocated;
            unsigned nextBlkAllocated = nextBlockHeader->info.allocated;

            if(prevBlkAllocated == 0 && nextBlkAllocated == 0){
                //first, I need to get the size of this free block, the size of previous free block, and the size of the next free block
                //and also the header and footer of each of them
                sf_footer* prevBlkFooter = (sf_footer*)(freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)(prevBlkFooter - prevBlkFooter->info.block_size + 8);
                sf_footer* nextBlkFooter = (sf_footer*)(nextBlockHeader + nextBlockHeader->info.block_size - 8);
                size_t totalSize = prevBlkHeader->info.block_size + freeBlockHeader->info.block_size + prevBlkHeader->info.block_size;
                removeFromFreelist(prevBlkHeader);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                memset(prevBlkFooter,0,8);
                memset(freeBlockHeader,0,8);
                memset(freeBlockFooter,0,8);
                memset(nextBlockHeader,0,8);
                prevBlkHeader->info.block_size = totalSize;
                nextBlkFooter->info.block_size = totalSize;
                nextBlkFooter->info.prev_allocated = prevBlkHeader->info.prev_allocated;
                addFreeHeader(prevBlkHeader);
                return prevBlkHeader;
            }
            else if(prevBlkAllocated == 1 && nextBlkAllocated == 0){
                sf_footer* nextBlkFooter = (sf_footer*)(nextBlockHeader + nextBlockHeader->info.block_size - 8);
                size_t totalSize = freeBlockHeader->info.block_size + nextBlockHeader->info.block_size;
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                memset(freeBlockFooter,0,8);
                memset(nextBlockHeader,0,8);
                freeBlockHeader->info.block_size = totalSize;
                nextBlkFooter-> info.block_size = totalSize;
                nextBlkFooter->info.prev_allocated = freeBlockHeader->info.prev_allocated;
                addFreeHeader(freeBlockHeader);
                return freeBlockHeader;
            }
            else if(prevBlkAllocated == 0 && nextBlkAllocated == 1){
                sf_footer* prevBlkFooter = (sf_footer*) (freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)(prevBlkFooter - prevBlkFooter->info.block_size + 8);
                size_t totalSize = prevBlkHeader->info.block_size + freeBlockHeader->info.block_size;
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(prevBlkHeader);
                memset(prevBlkFooter,0,8);
                memset(freeBlockFooter,0,8);
                prevBlkHeader->info.block_size = totalSize;
                freeBlockFooter->info.block_size = totalSize;
                freeBlockFooter->info.prev_allocated = prevBlkHeader->info.prev_allocated;
                addFreeHeader(prevBlkHeader);
                return prevBlkHeader;
            }
            else{
                return NULL;
            }

        }
    }


/** This function add header of a free block to list of free lists
**
**/
void addFreeHeader(sf_header* header){
    size_t block_size = header->info.block_size;
    if(get_sf_free_list_node(block_size) == NULL){
        sf_free_list_node* freeNode = set_sf_free_list_node(block_size);
        freeNode->head.links.next = header;
        freeNode->head.links.prev = header;
        header->links.next = &freeNode->head;
        header->links.prev = &freeNode->head;
    }
    else{
        sf_free_list_node* freeNode = get_sf_free_list_node(block_size);
        sf_header* nextHeader = freeNode->head.links.next;
        freeNode->head.links.next = header;
        header->links.prev = &freeNode->head;
        nextHeader->links.prev = header;
        header->links.next = nextHeader;
    }
}

/* This function add a node that contains the size specified to sf_free_list in the corrsponding spot based on its size, and return the Node being added
** The list needs to be in increasing order.
** @size size that the node to be added contains
*/
sf_free_list_node* set_sf_free_list_node(size_t size){
    if(sf_free_list_head.next == &sf_free_list_head){
        return sf_add_free_list(size,&sf_free_list_head);
    }
    sf_free_list_node* current_node = sf_free_list_head.next;
    while (current_node -> size < size){
        if(current_node == &sf_free_list_head){
            return sf_add_free_list(size,&sf_free_list_head);
        }
        current_node = current_node->next;
    }
    return sf_add_free_list(size,current_node);
}

/* This function returns an address to the sf_free_list_node which contains the size.
**@size size that the sf_free_list_node contains.
**@return the address to the sf_free_list_node that contains the size.
   return NULL if no such node exists.
*/
sf_free_list_node * get_sf_free_list_node(size_t size){
    sf_free_list_node* current_node = sf_free_list_head.next;
    while (current_node != &sf_free_list_head){
        if(((current_node-> size) << 4) <= size){
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
