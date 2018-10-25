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
sf_header* isFreeHeader(sf_header* ending_header);

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
        sf_free_list_node* freeNode = get_sf_free_list_node(size);
        if(freeNode == NULL ||freeNode->head.links.next == &freeNode->head){ //if I cannot find a free block of memory that's large enough
            if (((sf_footer*)(sf_mem_end()-16)) == lastFreeFooter){ //if this is the last free block of memory in heap that precedes the epilogue
                size_t numOfBytesHad = lastFreeFooter->info.block_size << 4;
                while(numOfBytesHad < block_size){
                    sf_mem_grow();
                    epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
                    memset(epilogue,0,8);
                    (*epilogue).footer.info.allocated = 1;
                    numOfBytesHad += 4096;
                }
                //now numOfBytesHad will be the size of the new coallesce free block.
                memset(lastFreeHeader + (lastFreeHeader->info.block_size << 4) -8, 0, 8); //But, is it (calling memset) really necessary?
                memcpy(sf_mem_end() - 8,epilogue,8);
                lastFreeHeader->info.block_size = numOfBytesHad >>4;
                lastFreeFooter = (sf_footer*)(sf_mem_end() - 16);
                lastFreeFooter-> info.block_size = numOfBytesHad >> 4;
                lastFreeFooter-> info.prev_allocated = lastFreeHeader->info.prev_allocated;
                lastFreeFooter-> info.allocated = 0;

                removeFromFreelist(lastFreeHeader);
                if(block_size == numOfBytesHad){
                    sf_header block_header;
                    block_header.info.requested_size = size;
                    block_header.info.block_size = numOfBytesHad >> 4;
                    block_header.info.two_zeroes = 0;
                    block_header.info.prev_allocated = lastFreeHeader->info.prev_allocated;
                    block_header.info.allocated = 1;
                    if(lastFreeHeader->info.prev_allocated == 0){
                        size_t prev_block_size = ((lastFreeHeader - 8)->info.block_size) << 4;
                        memcpy(lastFreeHeader,&block_header,8);
                        lastFreeFooter = (sf_footer*) (lastFreeHeader - 8);
                        lastFreeHeader = (sf_header*) (lastFreeFooter- prev_block_size + 8);
                        //function return void*, but not sure if I can return sf_footer*
                        return (lastFreeFooter + 16);
                    }

                    sf_header* poFreeHeader = lastFreeHeader - 32;
                    sf_header* lastestFreeHeader = NULL;
                    while(poFreeHeader != (sf_header*)(&prologue->footer)){
                        lastestFreeHeader = isFreeHeader(poFreeHeader);
                        if(lastestFreeHeader != NULL){
                            break;
                        }
                        poFreeHeader = poFreeHeader - 8;
                    }
                    if(lastestFreeHeader == NULL){
                        lastFreeHeader = NULL;
                        lastFreeFooter = NULL;
                    }
                    else{
                        lastFreeHeader = lastestFreeHeader;
                        lastFreeFooter = (sf_footer*)(lastFreeHeader + (lastFreeHeader->info.block_size << 4) - 8);
                    }
                }
                else{
                    lastFreeHeader->info.requested_size = size;
                    lastFreeHeader->info.block_size = block_size >> 4;
                    lastFreeHeader->info.allocated = 1;
                    lastFreeHeader = lastFreeHeader + (lastFreeHeader->info.block_size << 4);
                    lastFreeFooter = (sf_footer*)(lastFreeHeader+ (lastFreeHeader->info.block_size << 4) - 8);
                    lastFreeHeader->info.block_size = (numOfBytesHad - block_size) >> 4;
                    lastFreeHeader->info.prev_allocated = 1;
                    lastFreeHeader->info.two_zeroes = 0;
                    lastFreeHeader->info.allocated = 0;
                    lastFreeFooter->info.block_size = (numOfBytesHad - block_size) >> 4;
                    lastFreeFooter->info.prev_allocated = 1;
                    lastFreeFooter->info.two_zeroes = 0;
                    lastFreeFooter->info.allocated = 0;
                    addFreeHeader(lastFreeHeader);
                    return (lastFreeHeader + 8);
                }

            }
            else{
                //if the last memory block is allocated
                void* ptrNewPage = sf_mem_grow();
                size_t numOfBytesHad = 4096;
                sf_header* newBlkHeader =  (sf_header*) (ptrNewPage - 8);
                memset(epilogue,0,8);
                epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
                while(numOfBytesHad < block_size){
                    sf_mem_grow();
                    numOfBytesHad += 4096;
                    memset(epilogue,0,8);
                    epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
                }
                if(numOfBytesHad == block_size){
                    newBlkHeader->info.requested_size = size;
                    newBlkHeader->info.block_size = block_size >> 4;
                    newBlkHeader->info.prev_allocated = 1;
                    newBlkHeader->info.two_zeroes = 0;
                    newBlkHeader->info.allocated = 1;
                    return newBlkHeader;
                }
                else{
                    newBlkHeader->info.requested_size = size;
                    newBlkHeader->info.block_size = block_size >> 4;
                    newBlkHeader->info.prev_allocated = 1;
                    newBlkHeader->info.two_zeroes = 0;
                    newBlkHeader->info.allocated = 1;

                    sf_header* freeBlkHeader = newBlkHeader + block_size;
                    size_t remaining_byte = numOfBytesHad - block_size;
                    freeBlkHeader->info.block_size = remaining_byte >> 4;
                    freeBlkHeader->info.prev_allocated = 1;
                    freeBlkHeader->info.two_zeroes = 0;
                    freeBlkHeader->info.allocated = 0;
                    lastFreeHeader = freeBlkHeader;
                    sf_footer* freeBlockFooter = (sf_footer*)(freeBlkHeader + remaining_byte - 8);
                    freeBlockFooter->info.block_size = remaining_byte >> 4;
                    freeBlockFooter->info.prev_allocated = 1;
                    freeBlockFooter->info.two_zeroes = 0;
                    freeBlockFooter->info.allocated = 0;
                    lastFreeFooter = freeBlockFooter;
                    addFreeHeader(freeBlkHeader);
                    return newBlkHeader;
                }
            }

        }
      //if there exists a free block that's large enough
        sf_free_list_node* freeListNode = get_sf_free_list_node(size);
            if(freeListNode->size == size){
                //if the free list is not empty, get the header of the first free block
                sf_header* freeHeader = freeListNode->head.links.next;
                sf_footer* freeFooter = (sf_footer*)(freeHeader + (freeHeader->info.block_size << 4) - 8);
                removeFromFreelist(freeHeader);
                freeHeader->info.requested_size = size;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.allocated = 1;
                memset(freeFooter,0,8);
                if(freeHeader->info.prev_allocated == 0){
                    sf_footer* prev_footer =  (sf_footer*)(freeHeader - 8);
                    lastFreeFooter = prev_footer;
                    sf_header* prev_header = (sf_header*) (prev_footer - (prev_footer->info.block_size << 4) + 8);
                    lastFreeHeader = prev_header;
                }
                else{
                    sf_header* poFreeHeader = lastFreeHeader - 32;
                    sf_header* lastestFreeHeader = NULL;
                    while(poFreeHeader != (sf_header*)(&prologue->footer)){
                        lastestFreeHeader = isFreeHeader(poFreeHeader);
                        if(lastestFreeHeader != NULL){
                            break;
                        }
                        poFreeHeader = poFreeHeader - 8;
                    }
                    if(lastestFreeHeader == NULL){
                        lastFreeHeader = NULL;
                        lastFreeFooter = NULL;
                    }
                    else{
                        lastFreeHeader = lastestFreeHeader;
                        lastFreeFooter = (sf_footer*)(lastFreeHeader + (lastFreeHeader->info.block_size << 4) - 8);
                    }
                }
                return (freeHeader + 8);
            }
            else{
                sf_header* freeHeader = freeListNode->head.links.next;
                sf_footer* freeFooter = (sf_footer*)(freeHeader + (freeHeader->info.block_size << 4) - 8);
                removeFromFreelist(freeHeader);
                freeHeader->info.requested_size = size;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.allocated = 1;


                size_t newFreeBlkSize = (freeFooter->info.block_size << 4) - block_size;
                if(newFreeBlkSize < 32){
                    if(freeHeader->info.prev_allocated == 0){
                    sf_footer* prev_footer =  (sf_footer*)(freeHeader - 8);
                    lastFreeFooter = prev_footer;
                    sf_header* prev_header = (sf_header*) (prev_footer - (prev_footer->info.block_size << 4) + 8);
                    lastFreeHeader = prev_header;
                     }
                      else{
                        sf_header* poFreeHeader = lastFreeHeader - 32;
                        sf_header* lastestFreeHeader = NULL;
                        while(poFreeHeader != (sf_header*)(&prologue->footer)){
                            lastestFreeHeader = isFreeHeader(poFreeHeader);
                            if(lastestFreeHeader != NULL){
                                break;
                            }
                            poFreeHeader = poFreeHeader - 8;
                        }
                        if(lastestFreeHeader == NULL){
                            lastFreeHeader = NULL;
                            lastFreeFooter = NULL;
                        }
                        else{
                            lastFreeHeader = lastestFreeHeader;
                            lastFreeFooter = (sf_footer*)(lastFreeHeader + (lastFreeHeader->info.block_size << 4) - 8);
                        }
                    }
                        memset(freeFooter,0,8);
                        return (freeHeader + 8);
                    }

                sf_header* newFreeHeader = freeHeader + (freeHeader->info.block_size << 4);

                newFreeHeader->info.block_size = newFreeBlkSize >> 4;
                newFreeHeader->info.two_zeroes = 0;
                newFreeHeader->info.prev_allocated = 1;
                newFreeHeader->info.allocated = 0;

                freeFooter->info.block_size = newFreeBlkSize >> 4;
                freeFooter->info.prev_allocated = 1;
                addFreeHeader(freeHeader);
                if(freeFooter == lastFreeFooter){
                    lastFreeHeader = newFreeHeader;
                }
                return (freeHeader + 8);
            }

    }//this ending bracket belongss to the outermost else statement
}


/**
** This function checks if ending_header is a free header. It returns the free header if it is. NULL if it's not.
**
**/

sf_header* isFreeHeader(sf_header* ending_header){
    sf_free_list_node* current_node = sf_free_list_head.next;
    while(current_node != &sf_free_list_head){
        sf_header* current_header = current_node->head.links.next;
        while (current_header != &current_node->head){
            if(ending_header == current_header){
                return current_header;
            }
            current_header = current_node->head.links.next;
        }
        current_node = sf_free_list_head.next;
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
        if((freeBlockHeader + (freeBlockHeader->info.block_size << 4)) == (sf_mem_end()-8)){
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
                size_t prevBlkSize = (prevBlKfterPtr->info.block_size) << 4;
                size_t currBlkSize = (freeBlockHeader->info.block_size) << 4;
                sf_header* prevBlkHdPtr = (sf_header*)(prevBlKfterPtr - prevBlkSize + 8);
                sf_footer* freeBlockFooter = (sf_footer*)freeBlockHeader + currBlkSize - 8;
                size_t combined_blkSize = currBlkSize+ prevBlkSize;
                prevBlkHdPtr->info.block_size = combined_blkSize >> 4;
                freeBlockFooter->info.block_size = combined_blkSize >> 4;
                removeFromFreelist(prevBlkHdPtr);
                removeFromFreelist(freeBlockHeader);
                addFreeHeader(prevBlkHdPtr);
                memset(prevBlKfterPtr,0,8);
                memset(freeBlockHeader,0,8);
                return prevBlkHdPtr;
            }
        }
        else{
            sf_footer* freeBlockFooter = (sf_footer*)(freeBlockHeader + (freeBlockHeader->info.block_size << 4) - 8);
            sf_header* nextBlockHeader = freeBlockHeader + (freeBlockHeader->info.block_size << 4);
            unsigned prevBlkAllocated = freeBlockHeader->info.prev_allocated;
            unsigned nextBlkAllocated = nextBlockHeader->info.allocated;

            if(prevBlkAllocated == 0 && nextBlkAllocated == 0){
                //first, I need to get the size of this free block, the size of previous free block, and the size of the next free block
                //and also the header and footer of each of them
                sf_footer* prevBlkFooter = (sf_footer*)(freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)(prevBlkFooter - (prevBlkFooter->info.block_size << 4) + 8);
                sf_footer* nextBlkFooter = (sf_footer*)(nextBlockHeader + (nextBlockHeader->info.block_size << 4) - 8);
                size_t totalSize = (prevBlkHeader->info.block_size << 4) + (freeBlockHeader->info.block_size << 4) + (prevBlkHeader->info.block_size << 4);
                removeFromFreelist(prevBlkHeader);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                memset(prevBlkFooter,0,8);
                memset(freeBlockHeader,0,8);
                memset(freeBlockFooter,0,8);
                memset(nextBlockHeader,0,8);
                prevBlkHeader->info.block_size = totalSize >> 4;
                nextBlkFooter->info.block_size = totalSize >> 4;
                nextBlkFooter->info.prev_allocated = prevBlkHeader->info.prev_allocated;
                addFreeHeader(prevBlkHeader);
                return prevBlkHeader;
            }
            else if(prevBlkAllocated == 1 && nextBlkAllocated == 0){
                sf_footer* nextBlkFooter = (sf_footer*)(nextBlockHeader + (nextBlockHeader->info.block_size << 4) - 8);
                size_t totalSize = (freeBlockHeader->info.block_size << 4) + (nextBlockHeader->info.block_size << 4);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                memset(freeBlockFooter,0,8);
                memset(nextBlockHeader,0,8);
                freeBlockHeader->info.block_size = totalSize >> 4;
                nextBlkFooter-> info.block_size = totalSize >> 4;
                nextBlkFooter->info.prev_allocated = freeBlockHeader->info.prev_allocated;
                addFreeHeader(freeBlockHeader);
                return freeBlockHeader;
            }
            else if(prevBlkAllocated == 0 && nextBlkAllocated == 1){
                sf_footer* prevBlkFooter = (sf_footer*) (freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)(prevBlkFooter - (prevBlkFooter->info.block_size << 4) + 8);
                size_t totalSize = (prevBlkHeader->info.block_size << 4)+ (freeBlockHeader->info.block_size << 4);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(prevBlkHeader);
                memset(prevBlkFooter,0,8);
                memset(freeBlockFooter,0,8);
                prevBlkHeader->info.block_size = totalSize >> 4;
                freeBlockFooter->info.block_size = totalSize >> 4;
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
