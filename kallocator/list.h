// Linked list module.

#ifndef LIST_H_
#define LIST_H_

struct nodeStruct {
    int len;
    void* address;
    struct nodeStruct *next;
};

/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int item, void *address);

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head);

/*
 * Return the first node holding the value item, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, void *address);

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set (by for example
 * calling List_findNode()) to a valid node in the list. If the list contains
 * only node, the head of the list should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);

void List_removeNode (struct nodeStruct **headRef, struct nodeStruct *node);
/*
 * Sort the list in ascending order based on the item field.
 * Any sorting algorithm is fine.
 */
void List_sort (struct nodeStruct **headRef);


#endif
