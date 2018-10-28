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
sf_header* coallesce(sf_header* freeBlockHeader);
void removeFromFreelist(sf_header* header);
void addFreeHeader(sf_header* header);
void setNextHeader(sf_header* header);
sf_free_list_node * get_sf_free_list_node_of_size(size_t size);

sf_prologue* prologue;
sf_epilogue* epilogue;
void *sf_malloc(size_t size) {
    if(size == 0)
        return NULL;

    if(sf_mem_start() == sf_mem_end()){
        void* ptrBeginning = sf_mem_grow();
        if(ptrBeginning == NULL){
            sf_errno = ENOMEM;
            return NULL;
        }
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
            block_size = 32;

        //gotta check if there is enough bytes to allocate
        size_t remaining_byte = 4096 - 48;

        while (remaining_byte < block_size){
            if(sf_mem_grow() == NULL){
                sf_errno = ENOMEM;
                return NULL;
            }
            memset(epilogue,0,8);
            epilogue = (sf_epilogue*)(sf_mem_end()  - 8);
            memset(epilogue,0,8);
            (*epilogue).footer.info.allocated = 1;
            remaining_byte += 4096;
        }

        //the allocated bit of the first allocate block is always 1.
        sf_header* block_header = ptrBeginning + 40;
        block_header->info.requested_size = size;
        block_header->info.block_size = (block_size >> 4);
        block_header->info.prev_allocated = 1;
        block_header->info.allocated = 1;


        //make a free block header and footer
        //but, what if the function takes up exactly all the memory in the heap, not more and not less?

        remaining_byte -= block_size;

        if(remaining_byte < 32){
            block_size = block_size + remaining_byte;
            block_header->info.block_size = (block_size >> 4);
            epilogue->footer.info.prev_allocated = 1;
            return (ptrBeginning + 48);

        }

        void* ptrFreeBlock = ptrBeginning + 40 + block_size;
        sf_header* free_block_header = (sf_header*)ptrFreeBlock;
        free_block_header->info.block_size = (remaining_byte >> 4);
        free_block_header->info.prev_allocated = 1;
        free_block_header->info.allocated = 0;


        sf_footer* free_block_footer = (sf_footer*)(ptrFreeBlock + remaining_byte - 8);
        free_block_footer->info.requested_size = 0;
        free_block_footer->info.block_size = (remaining_byte >> 4);
        free_block_footer->info.allocated = 0;
        free_block_footer->info.prev_allocated = 1;



        epilogue->footer.info.prev_allocated = 0;
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
            block_size = 32;
        sf_free_list_node* freeNode = get_sf_free_list_node(size);
        if(freeNode != NULL && freeNode->head.links.next != &freeNode->head){
            if(freeNode->size == block_size){
                sf_header* freeHeader = freeNode->head.links.next;
                removeFromFreelist(freeHeader);
                freeHeader->info.allocated = 1;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.requested_size = size;
                if((freeHeader + block_size) == (sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                return freeHeader;
            }
            else{
                sf_header* freeHeader = freeNode->head.links.next;
                sf_footer* freeFooter = (sf_footer*)((void*)freeHeader + (freeHeader->info.block_size << 4) - 8);
                removeFromFreelist(freeHeader);
                freeHeader->info.allocated = 1;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.requested_size = size;
                if((freeFooter->info.block_size << 4) - block_size < 32){
                    freeHeader->info.block_size = freeFooter->info.block_size;
                    if(((void*)freeFooter + 8) == ((void*)sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                    return freeHeader;
                }
                sf_header* newFreeBlock = (sf_header*)((void*)freeHeader + block_size);
                newFreeBlock->info.block_size = (freeFooter->info.block_size << 4) - block_size;
                newFreeBlock->info.two_zeroes = 0;
                newFreeBlock->info.requested_size = 0;
                newFreeBlock->info.allocated = 0;
                newFreeBlock->info.prev_allocated = 1;

                freeFooter->info.block_size = newFreeBlock->info.block_size;
                freeFooter->info.prev_allocated = 1;
                addFreeHeader(freeHeader);
                if(((void*)freeFooter + 8) == (sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                return freeHeader;
            }
        }
        else{
            void* newPagePtr;
            while(freeNode == NULL ||freeNode->head.links.next != &freeNode->head){
                newPagePtr = sf_mem_grow();
                if(newPagePtr == NULL){
                    sf_errno = ENOMEM;
                    return NULL;
                }
                memset(sf_mem_end()  - 8,0,8);
                epilogue = sf_mem_end() - 8;
                epilogue->footer.info.allocated = 1;
                epilogue->footer.info.prev_allocated = 0;
                sf_header* freeHeader = (sf_header*)(newPagePtr - 8);
                freeHeader->info.requested_size = 0;
                freeHeader->info.block_size = 4096 >> 4;
                freeHeader->info.two_zeroes = 0;
                freeHeader->info.allocated = 0;
                if(epilogue->footer.info.prev_allocated == 1){
                    freeHeader->info.prev_allocated = 1;
                }
                else{
                    freeHeader->info.allocated = 0;
                }

                sf_footer* freeFooter = (sf_footer*)((void*)epilogue - 8);
                freeFooter->info.requested_size = 0;
                freeFooter->info.block_size = 4096 >> 4;
                freeFooter->info.two_zeroes = 0;
                freeFooter->info.allocated = 0;
                freeFooter->info.prev_allocated = freeHeader->info.prev_allocated;
                addFreeHeader(freeHeader);
                coallesce(freeHeader);

                freeNode = get_sf_free_list_node(size);
            }
            if(freeNode->size == block_size){
                sf_header* freeHeader = freeNode->head.links.next;
                removeFromFreelist(freeHeader);
                freeHeader->info.allocated = 1;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.requested_size = size;
                if(((void*)freeHeader + (freeHeader->info.block_size << 4)) == ((void*)sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                else{
                    setNextHeader(freeHeader);
                }
                return freeHeader;
            }
            else{
                sf_header* freeHeader = freeNode->head.links.next;
                sf_footer* freeFooter = (sf_footer*)((void*)freeHeader + (freeHeader->info.block_size << 4) - 8);
                removeFromFreelist(freeHeader);
                freeHeader->info.allocated = 1;
                freeHeader->info.block_size = block_size >> 4;
                freeHeader->info.requested_size = size;
                if((freeFooter->info.block_size << 4) - block_size < 32){
                    freeHeader->info.block_size = freeFooter->info.block_size;
                    if(((void*)freeHeader + (freeHeader->info.block_size << 4)) == ((void*)sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                    return freeHeader;
                }
                sf_header* newFreeBlock = freeHeader + block_size;
                newFreeBlock->info.block_size = (freeFooter->info.block_size << 4) - block_size;
                newFreeBlock->info.two_zeroes = 0;
                newFreeBlock->info.requested_size = 0;
                newFreeBlock->info.allocated = 0;
                newFreeBlock->info.prev_allocated = 1;

                freeFooter->info.block_size = newFreeBlock->info.block_size;
                freeFooter->info.prev_allocated = 1;
                addFreeHeader(freeHeader);
                if(((void*)freeFooter + 8) == (sf_mem_end() - 8)){
                        epilogue->footer.info.prev_allocated = 1;
                    }
                return freeHeader;
            }

        }

    }//this ending bracket belongss to the outermost else statement
}

void setNextHeader(sf_header* header){
    sf_header* nextHeader = (sf_header*)((void*)header + (header->info.block_size << 4));
    nextHeader->info.prev_allocated = 1;
    if(nextHeader->info.allocated == 0){
        sf_footer* nextFooter = (sf_footer*)((void*)nextHeader + (nextHeader->info.block_size <<4 ) - 8);
        nextFooter->info.prev_allocated = 1;
    }
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
}

/*
*This function coallesce the free block pointed by freeBlockHeader with previous or next block if existed, and return the pointer to the
*coallesced free block. Otherwise, return pointer to the current sf_header if previous block and next block are not free
*/
sf_header* coallesce(sf_header* freeBlockHeader){

        int isLastFreeBlock;
        if(((void*)freeBlockHeader + (freeBlockHeader->info.block_size << 4)) == ((void*)epilogue)){
            isLastFreeBlock = 1;
        }
        else{
            isLastFreeBlock = 0;
        }
        if(isLastFreeBlock == 1){
            if(freeBlockHeader->info.prev_allocated == 1){
                return freeBlockHeader;
            }
            else{
                sf_footer* prevBlKfterPtr = (sf_footer*) ((void*)freeBlockHeader - 8);
                size_t prevBlkSize = (prevBlKfterPtr->info.block_size) << 4;
                size_t currBlkSize = (freeBlockHeader->info.block_size) << 4;
                sf_header* prevBlkHdPtr = (sf_header*)((void*)prevBlKfterPtr - prevBlkSize + 8);
                sf_footer* freeBlockFooter = (sf_footer*)((void*)freeBlockHeader + currBlkSize - 8);
                size_t combined_blkSize = currBlkSize + prevBlkSize;
                prevBlkHdPtr->info.block_size = combined_blkSize >> 4;
                freeBlockFooter->info.block_size = combined_blkSize >> 4;
                freeBlockFooter->info.prev_allocated = prevBlkHdPtr->info.prev_allocated;
                memset(prevBlKfterPtr,0,8);
                memset(freeBlockHeader,0,8);
                removeFromFreelist(prevBlkHdPtr);
                removeFromFreelist(freeBlockHeader);
                addFreeHeader(prevBlkHdPtr);
                return prevBlkHdPtr;
            }
        }
        else{
            sf_footer* freeBlockFooter = (sf_footer*)(((void*)freeBlockHeader) + ((freeBlockHeader->info.block_size << 4) - 8));
            sf_header* nextBlockHeader = (sf_header*)(((void*)freeBlockHeader) + (freeBlockHeader->info.block_size << 4));
            unsigned prevBlkAllocated = freeBlockHeader->info.prev_allocated;
            unsigned nextBlkAllocated = nextBlockHeader->info.allocated;

            if(prevBlkAllocated == 0 && nextBlkAllocated == 0){
                //first, I need to get the size of this free block, the size of previous free block, and the size of the next free block
                //and also the header and footer of each of them
                sf_footer* prevBlkFooter = (sf_footer*)((void*)freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)((void*)prevBlkFooter - (prevBlkFooter->info.block_size << 4) + 8);
                sf_footer* nextBlkFooter = (sf_footer*)((void*)nextBlockHeader + (nextBlockHeader->info.block_size << 4) - 8);
                size_t totalSize = (prevBlkHeader->info.block_size << 4) + (freeBlockHeader->info.block_size << 4) + (nextBlockHeader->info.block_size << 4);
                removeFromFreelist(prevBlkHeader);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                prevBlkHeader->info.block_size = totalSize >> 4;
                nextBlkFooter->info.block_size = totalSize >> 4;
                nextBlkFooter->info.prev_allocated = prevBlkHeader->info.prev_allocated;
                addFreeHeader(prevBlkHeader);
                return prevBlkHeader;
            }
            else if(prevBlkAllocated == 1 && nextBlkAllocated == 0){
                sf_footer* nextBlkFooter = (sf_footer*)((void*)nextBlockHeader + (nextBlockHeader->info.block_size << 4) - 8);
                size_t totalSize = (freeBlockHeader->info.block_size << 4) + (nextBlockHeader->info.block_size << 4);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(nextBlockHeader);
                freeBlockHeader->info.block_size = totalSize >> 4;
                nextBlkFooter-> info.block_size = totalSize >> 4;
                nextBlkFooter->info.prev_allocated = freeBlockHeader->info.prev_allocated;
                addFreeHeader(freeBlockHeader);
                return freeBlockHeader;
            }
            else if (prevBlkAllocated == 0 && nextBlkAllocated == 1){
                sf_footer* prevBlkFooter = (sf_footer*) ((void*)freeBlockHeader - 8);
                sf_header* prevBlkHeader = (sf_header*)((void*) prevBlkFooter - (prevBlkFooter->info.block_size << 4) + 8);
                size_t totalSize = (prevBlkHeader->info.block_size << 4)+ (freeBlockHeader->info.block_size << 4);
                removeFromFreelist(freeBlockHeader);
                removeFromFreelist(prevBlkHeader);
                prevBlkHeader->info.block_size = totalSize >> 4;
                freeBlockFooter->info.block_size = totalSize >> 4;
                freeBlockFooter->info.prev_allocated = prevBlkHeader->info.prev_allocated;
                addFreeHeader(prevBlkHeader);
                return prevBlkHeader;
            }
            else{
                return freeBlockHeader;
            }

        }
    }


/** This function add header of a free block to list of free lists
**
**/
void addFreeHeader(sf_header* header){
    size_t block_size = header->info.block_size << 4;
    if(get_sf_free_list_node_of_size(block_size) == NULL){
        sf_free_list_node* freeNode = set_sf_free_list_node(block_size);
        freeNode->head.links.next = header;
        freeNode->head.links.prev = header;
        header->links.next = &freeNode->head;
        header->links.prev = &freeNode->head;
    }
    else{
        sf_free_list_node* freeNode = get_sf_free_list_node_of_size(block_size);
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

sf_free_list_node * get_sf_free_list_node_of_size(size_t size){
    sf_free_list_node* current_node = sf_free_list_head.next;
    while (current_node != &sf_free_list_head){
        if(size == (current_node-> size)){
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

/* This function returns an address to the sf_free_list_node which contains the size.
**@size size that the sf_free_list_node contains.
**@return the address to the sf_free_list_node that contains the size.
   return NULL if no such node exists.
*/
sf_free_list_node * get_sf_free_list_node(size_t size){
    sf_free_list_node* current_node = sf_free_list_head.next;
    while (current_node != &sf_free_list_head){
        if(size <= (current_node-> size)){
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

void sf_free(void *pp) {
    if(pp == NULL){
        abort();
    }
    else if(pp < ((void*)(sf_mem_start() + 40))) {
        abort();
    }
    else if(pp > ((void*)(sf_mem_end() - 8))){
        abort();
    }
    sf_header* allocBlkHeader =(sf_header*)(pp - 8);
    if(allocBlkHeader->info.allocated == 0){
        abort();
    }
    size_t block_size = allocBlkHeader->info.block_size << 4;
    if(block_size % 16 != 0 || block_size < 32){
        abort();
    }
    size_t required_size = allocBlkHeader->info.requested_size;
    if(required_size % 16 != 0){
            required_size += (16 - (required_size % 16));
        }
    if(required_size < 32)
        required_size = 32;
    if(required_size > block_size)
        abort();

    sf_footer* allocBlkFooter = (sf_footer*)((void*)allocBlkHeader + block_size - 8);
    if(allocBlkHeader->info.allocated == 0){
        abort();
    }
    if(allocBlkHeader->info.prev_allocated == 0){
        sf_footer* prevBlkFooter = (sf_footer*)(pp - 16);
        if(prevBlkFooter->info.allocated == 1)
            abort();
        sf_header* prevBlkHeader = (sf_header*)((void*)prevBlkFooter - (prevBlkFooter->info.block_size << 4) + 8);
        if(prevBlkHeader->info.allocated == 1)
            abort();
    }
    allocBlkHeader->info.allocated = 0;
    allocBlkHeader->info.requested_size = 0;

    allocBlkFooter->info.requested_size = 0;
    allocBlkFooter->info.block_size = block_size >> 4;
    allocBlkFooter->info.prev_allocated = allocBlkHeader->info.prev_allocated;
    allocBlkFooter->info.two_zeroes = 0;
    allocBlkFooter->info.allocated = 0;
    addFreeHeader(allocBlkHeader);
    coallesce(allocBlkHeader);
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    if(pp == NULL){
        sf_errno = ENOMEM;
    }
    else if(pp < ((void*)(sf_mem_start() + 40))) {
        sf_errno = ENOMEM;
    }
    else if(pp > ((void*)(sf_mem_end() - 8))){
        sf_errno = ENOMEM;
    }
    sf_header* allocBlkHeader =(sf_header*)(pp - 8);
    if(allocBlkHeader->info.allocated == 0){
        sf_errno = ENOMEM;
    }
    size_t block_size = allocBlkHeader->info.block_size << 4;
    if(block_size % 16 != 0 || block_size < 32){
        sf_errno = ENOMEM;
    }
    size_t required_size = allocBlkHeader->info.requested_size;
    if(required_size % 16 != 0){
            required_size += (16 - (required_size % 16));
        }
    if(required_size < 32)
        required_size = 32;
    if(required_size > block_size)
        sf_errno = ENOMEM;

    if(allocBlkHeader->info.allocated == 0){
        sf_footer* prevBlkFooter = (sf_footer*)(pp - 16);
        if(prevBlkFooter->info.allocated == 1)
            sf_errno = ENOMEM;
        sf_header* prevBlkHeader = (sf_header*)((void*)prevBlkFooter - prevBlkFooter->info.block_size + 8);
        if(prevBlkHeader->info.allocated == 1)
            sf_errno = ENOMEM;
    }
    if(rsize == 0){
        sf_free(pp);
        return NULL;
    }

    size_t newBlkSize = rsize;
    newBlkSize = newBlkSize + 8;
    if(newBlkSize % 16 != 0){
        newBlkSize += (16 - (newBlkSize % 16));
    }
    if(newBlkSize < 32)
        newBlkSize = 32;

    if(newBlkSize > block_size){
        void* ptrNewBlock = sf_malloc(newBlkSize);
        memcpy(ptrNewBlock,pp,block_size);
        sf_free(pp);
        return ptrNewBlock;
    }
    else if(newBlkSize < block_size){
        if((block_size - newBlkSize) < 32){
            allocBlkHeader->info.requested_size = rsize;
            return pp;
        }
        else{
            allocBlkHeader->info.requested_size = rsize;
            allocBlkHeader->info.block_size = newBlkSize >> 4;
            void* newFreeBlock = (void*)allocBlkHeader + newBlkSize + 8;
            sf_free(newFreeBlock);
            return pp;
        }
    }
    else{
        return pp;
    }
}
