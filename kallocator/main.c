#include <stdio.h>
#include "kallocator.h"

int main(int argc, char* argv[]) {
     initialize_allocator(100, FIRST_FIT);
    // initialize_allocator(100, BEST_FIT);
    // initialize_allocator(100, WORST_FIT);

    printf("Using first fit algorithm on memory size 100\n");

    int* p[50] = {NULL};
    for(int i=0; i<25; ++i) { // must be 10
        p[i] = kalloc(sizeof(int));
        if(p[i] == NULL) {
            printf("Allocation failed\n");
            continue;
        }
        *(p[i]) = i;
        printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }

    print_statistics();
    
    // freeing up contiguously
    for(int i=0; i<7; ++i) {
        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL;
    }

    // freeing up oddly from elem 15
    for(int i=15; i<25; ++i) {
        if(i%2 == 0)
            continue;
        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL;
    }
    print_statistics();
    void* before[100] = {NULL};
    void* after[100] = {NULL};
    int count = compact_allocation(before, after);
    printf("Number of ptrs moved : %d\n", count);
    
    printf("available_memory: %d\n", available_memory());
    printf("\n");
    // allocating some new memory now
    int* ptr = kalloc(sizeof(int));
    char *cptr = kalloc(sizeof(char));

    *ptr = 100;
    *cptr = 'a';
    // freeing our memory of one of the items
    kfree(cptr);
    print_statistics();
    
    // You can assume that the destroy_allocator will always be the
    // last funciton call of main function to avoid memory leak
    // before exit
    destroy_allocator();

    return 0;
}
