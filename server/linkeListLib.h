/*  Library : Library functions implemented to to create linkedlist with adding
              and deleting nodes functionality
    Purpose : TO use this custom linkedlist library for my coursera assignment and also
              to learn and implement linkedlist datastructure.
    File : LinkedListlib.h
*/
//Author : Amogh Sadhu
#ifndef LINKEDLISTLIB_H
#define LINKEDLISTLIB_H
/* =========================================================================
   1. HEADER INCLUDES
   ========================================================================= */

/* =========================================================================
   2. PREPROCESSOR DEFINES & MACROS
   ========================================================================= */

/* =========================================================================
   3. TYPE DEFINITIONS
   ========================================================================= */
typedef struct Node
{
    void* data;
    struct Node* next;
}Node;

/* =========================================================================
   4. GLOBAL VARIABLES
   ========================================================================= */

/* =========================================================================
   6. GLOBAL FUNCTION PROTOTYPES
   ========================================================================= */
Node* createNode(void* data);
Node* insertAtBeginning(Node* currentHead, void* data);
Node* insertAtEnd(Node* head, void * data);
Node* deleteNodeAtBeginning(Node* head);
Node* deleteNodeAtEnd(Node* head);
void traverseList(Node* head, void(traverseCallback)(void*));

#endif

