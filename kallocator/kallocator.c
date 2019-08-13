#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "kallocator.h"
#include "list.h"
#include <stdbool.h>

struct KAllocator {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
    // Some other data members you want,
    struct nodeStruct *allocBlocksListHead;
    struct nodeStruct *freeBlocksListHead;
};
struct KAllocator kallocator;

static int num_contiguous_blocks(struct nodeStruct *free_list_head);
static void combine_chunk(int size,  struct nodeStruct **_free_HeadRef);

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
    assert(_size > 0);
    kallocator.aalgorithm = _aalgorithm;
    kallocator.size = _size;
    kallocator.memory = malloc((size_t)kallocator.size);

    // Add some other initialization
    kallocator.allocBlocksListHead = NULL;
    kallocator.freeBlocksListHead = NULL;
    struct nodeStruct *node = List_createNode(_size, kallocator.memory);
    List_insertHead(&kallocator.freeBlocksListHead, node);
}

void destroy_allocator() {
    free(kallocator.memory);
    while(kallocator.freeBlocksListHead){
        List_deleteNode(&kallocator.freeBlocksListHead, kallocator.freeBlocksListHead);
    }
    while(kallocator.allocBlocksListHead){
        List_deleteNode(&kallocator.allocBlocksListHead, kallocator.allocBlocksListHead);
    }
    // free other dynamic allocated memory to avoid memory leak
}

void* kalloc(int _size) {
    void* ptr = NULL;
    // Allocate memory from kallocator.memory
    if(kallocator.aalgorithm == FIRST_FIT)
    {
        struct nodeStruct *headRef = kallocator.freeBlocksListHead;
        while(headRef){
            if(headRef->len >= _size){
                struct nodeStruct *allocated_node = List_createNode(_size, kallocator.freeBlocksListHead->address);
                // adjust the memory block
                kallocator.freeBlocksListHead->address += _size;
                kallocator.freeBlocksListHead->len -= _size;
                // remove the node if the size of the node is now 0
                if(kallocator.freeBlocksListHead->len == 0){
                    struct nodeStruct *found_node = List_findNode(kallocator.freeBlocksListHead, kallocator.freeBlocksListHead->address);
                    List_removeNode(&kallocator.freeBlocksListHead, found_node);
                }
                List_insertTail(&kallocator.allocBlocksListHead, allocated_node);
                ptr = allocated_node->address;
                break;
            }
            headRef = headRef->next;
        }
    }

    else if(kallocator.aalgorithm == BEST_FIT)
    {
        struct nodeStruct *headRef = kallocator.freeBlocksListHead;
        struct nodeStruct *smallest_fragment = headRef;
        int fragment_size = 0;
        while(headRef){
            fragment_size = headRef->len - _size;
            if((fragment_size<smallest_fragment->len) && fragment_size >= 0){
                smallest_fragment = headRef;
            }
            headRef=headRef->next;
        }
        if(fragment_size > 0){
            struct nodeStruct *allocated_node = List_createNode(_size, smallest_fragment->address);
            List_insertTail(&kallocator.allocBlocksListHead, allocated_node);
            smallest_fragment->address += _size;
            smallest_fragment->len -= _size;
        }
        else if(fragment_size == 0){
            List_removeNode(&kallocator.freeBlocksListHead, smallest_fragment);
            List_insertHead(&kallocator.allocBlocksListHead, smallest_fragment);
            return smallest_fragment->address;
        }
        ptr = smallest_fragment->address-_size;
    }
    else if(kallocator.aalgorithm == WORST_FIT)
    {
        struct nodeStruct *headRef = kallocator.freeBlocksListHead;
        struct nodeStruct *largest_fragment = headRef;
        while(headRef){
            int fragment_size = headRef->len - _size;
            if((fragment_size>largest_fragment->len) && fragment_size >= 0){
                largest_fragment = headRef;
            }
            headRef=headRef->next;
        }
        if(largest_fragment->len-_size > 0){
            struct nodeStruct *allocated_node = List_createNode(_size, largest_fragment->address);
            List_insertTail(&kallocator.allocBlocksListHead, allocated_node);
            largest_fragment->address += _size;
            largest_fragment->len -= _size;
        }
        else if(largest_fragment->len-_size == 0){
            List_removeNode(&kallocator.freeBlocksListHead, largest_fragment);
            List_insertHead(&kallocator.allocBlocksListHead, largest_fragment);
            return largest_fragment->address;
        }
        ptr = largest_fragment->address-_size;
    }
    return ptr;
}

void kfree(void* _ptr) {
    assert(_ptr != NULL);
    struct nodeStruct *new_free_block = List_findNode(kallocator.allocBlocksListHead, _ptr);
    // call free here -- free the memory buffer
    if(new_free_block){
        List_removeNode(&kallocator.allocBlocksListHead, new_free_block);
        List_insertTail(&kallocator.freeBlocksListHead, new_free_block);
        List_sort(&kallocator.freeBlocksListHead);
        struct nodeStruct *free_HeadRef = kallocator.freeBlocksListHead;
        while(free_HeadRef){
            //int _num_continguous_blocks = num_contiguous_blocks(free_HeadRef);
            int num_next_blocks = num_contiguous_blocks(free_HeadRef);
            //combine_chunk(_num_continguous_blocks);
            for(int i=0;i<num_next_blocks; i++){
                struct nodeStruct *to_be_deleted =free_HeadRef->next;
                free_HeadRef->len += to_be_deleted->len;
                free_HeadRef->next = to_be_deleted->next;
                free(to_be_deleted);
            }
            free_HeadRef=free_HeadRef->next;
        }
    }
    else fprintf(stderr, "%s\n", "Error. Could not find free");
}

int compact_allocation(void** _before, void** _after) {
    int compacted_size = 0;
    // move stuff around
    struct nodeStruct *free_HeadRef = kallocator.freeBlocksListHead;
    struct nodeStruct *alloc_HeadRef = kallocator.allocBlocksListHead;
    if(free_HeadRef && alloc_HeadRef){
        List_sort(&free_HeadRef);
        List_sort(&alloc_HeadRef);
    }

    if(free_HeadRef && alloc_HeadRef)
    {
        int i_before = 0, i_after = 0;
        while(alloc_HeadRef){
            // can you combine free chunks?
            int _num_continguous_blocks = num_contiguous_blocks(free_HeadRef);
            if(_num_continguous_blocks>0){
                free_HeadRef = kallocator.freeBlocksListHead;
                combine_chunk(_num_continguous_blocks, &free_HeadRef);
            }
            if(free_HeadRef->address<alloc_HeadRef->address){
                // adjust _before array
                for(int k=0;k<kallocator.allocBlocksListHead->len;k++){
                    _before[i_before] = alloc_HeadRef->address;
                    i_before++;
                }
                // make adjustments to linked lists
                compacted_size++;
                alloc_HeadRef->address = free_HeadRef->address;
                free_HeadRef->address = alloc_HeadRef->address+alloc_HeadRef->len;
                // adjust _after array
                for(int k=0;k<kallocator.allocBlocksListHead->len;k++){
                    _after[i_after] = alloc_HeadRef->address;
                    i_after++;
                }
            }
             alloc_HeadRef=alloc_HeadRef->next;
        }
    }
    return compacted_size;
}

int available_memory() {
    int available_memory_size = 0;
    struct nodeStruct *headFree = kallocator.freeBlocksListHead;
    while(headFree){
        available_memory_size += headFree->len;
        headFree = headFree->next;
    }
    return available_memory_size;
}

void print_statistics() {
    int allocated_size = 0;
    int allocated_chunks = List_countNodes(kallocator.allocBlocksListHead);
    int free_size = 0;
    int free_chunks = List_countNodes(kallocator.freeBlocksListHead);
    int smallest_free_chunk_size = kallocator.size;
    int largest_free_chunk_size = 0;

    // Calculate the statistics
    // Collect allocated memory stats
    struct nodeStruct *temp = kallocator.allocBlocksListHead;
    while(temp){
        allocated_size += temp->len;
        temp=temp->next;
    }
    // Collect free memory
    temp = kallocator.freeBlocksListHead;
    if(temp == NULL)
        smallest_free_chunk_size = 0;
    else{
        while(temp){
            free_size += temp->len;
            if(largest_free_chunk_size<temp->len)
                largest_free_chunk_size = temp->len;
            if(smallest_free_chunk_size>temp->len)
                smallest_free_chunk_size = temp->len;
            temp=temp->next;
        }
    }
    printf("Allocated size = %d\n", allocated_size);
    printf("Allocated chunks = %d\n", allocated_chunks);
    printf("Free size = %d\n", free_size);
    printf("Free chunks = %d\n", free_chunks);
    printf("Largest free chunk size = %d\n", largest_free_chunk_size);
    printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);
}


static int num_contiguous_blocks(struct nodeStruct *free_list_current_node){
    int count = 0;
    struct nodeStruct *current_node = free_list_current_node;
    if(free_list_current_node){
        while(current_node){
            if(current_node->next){
                if(current_node->address+current_node->len == current_node->next->address)
                    count++;
                else
                    break;
            }
            else break;
            current_node = current_node->next;
        }
    }
    return count;
}

static void combine_chunk(int size, struct nodeStruct **_free_HeadRef){
    struct nodeStruct *free_HeadRef = *_free_HeadRef;
    for(int i=0;i<size; i++){
        struct nodeStruct *to_be_deleted =free_HeadRef->next;
        free_HeadRef->len += to_be_deleted->len;
        free_HeadRef->next = to_be_deleted->next;
        free(to_be_deleted);
    }
}
