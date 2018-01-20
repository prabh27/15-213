/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
  queue_t* q;
  q = (queue_t*) malloc (sizeof(queue_t));
  if(q != NULL) {
    q->head = NULL;
    q->size = 0;
    q->tail = NULL;
    return q;
  }
  return NULL;

    /* Remember to handle the case if malloc returned NULL */
    
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
  if(q == NULL)
    return;
  list_ele_t* head = q->head;
  while(head != NULL) {
    list_ele_t* next = head->next;
    free(head);
    head = next;
  }
  free(q);
  return;
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, int v)
{
    if(q == NULL)
      return false;
    list_ele_t* element;
    element = (list_ele_t*) malloc (sizeof(list_ele_t));
    if(element == NULL) {
      return false;
    }
    element->value = v;
    if(q->head == NULL) {
      element->next = NULL;
      q->tail = element;
    } else {
      element->next = q->head;
    }
    q->head = element;
    q->size++;
    /* What should you do if the q is NULL? */
    /* What if malloc returned NULL? */
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    if(q == NULL)
      return false;
    list_ele_t* element; 
    element = (list_ele_t*) malloc (sizeof(list_ele_t));
    if(element == NULL) {
      return false;
    }
    element->value = v;
    element->next = NULL;
    if(q->tail == NULL) {
      q->head = element;
      q->tail = element;
    } else {
      q->tail->next = element;
      q->tail = element;
    }
    q->size++;
    /* Remember: It should operate in O(1) time */
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
    if (q == NULL) 
      return false;
    if(q->size == 0)
      return false;
    if(vp != NULL)
      *vp = q->head->value;
    list_ele_t* head = q->head->next;
    free(q->head);
    q->head = head;
    if(q->head == NULL)
      q->tail = NULL;
    q->size--;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    if(q == NULL)
      return 0;
    /* Remember: It should operate in O(1) time */
    return q->size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
  if(q == NULL)
      return;
    if(q->size <= 1)
      return;
    list_ele_t* first = q->head;
    list_ele_t* second = q->head->next;
    first->next = NULL;
    q->tail = first;
    while(second != NULL) {
      list_ele_t* temp = second->next;
      second->next = first;
      first = second;
      second = temp;
    }
    
    q->head = first;
    return;
}

