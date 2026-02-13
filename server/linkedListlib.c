/*  Library : Library functions implemented to to create linkedlist with adding
              and deleting nodes functionality
    Purpose : TO use this custom linkedlist library for my coursera assignment and also
              to learn and implement linkedlist datastructure.
    File : LinkedListlib.c
*/
//Author : Amogh Sadhu

/* =========================================================================
   1. HEADER INCLUDES
   ========================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include "linkeListLib.h"

/* =========================================================================
   2. PREPROCESSOR DEFINES & MACROS
   ========================================================================= */

/* =========================================================================
   3. TYPE DEFINITIONS
   ========================================================================= */

/* =========================================================================
   4. GLOBAL VARIABLES
   ========================================================================= */

/* =========================================================================
   5. STATIC VARIABLES
   ========================================================================= */

/* =========================================================================
   6. STATIC FUNCTION PROTOTYPES
   ========================================================================= */

/* =========================================================================
   8. FUNCTION IMPLEMENTATIONS
   ========================================================================= */
Node* createNode(void* data){
    Node* newNode = (Node*)malloc(sizeof(Node));
    if(newNode == NULL){
        printf("Failed to create node");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

Node* insertAtBeginning(Node* currentHead, void * data){

    Node* newNode = createNode(data); 
    newNode->next = currentHead;

    currentHead = newNode;
    return currentHead;
}

Node* insertAtEnd(Node* head, void * data){
    Node* newNode = createNode(data);

    if(head == NULL){
        return newNode;
    }

    Node* currentNode = head;
    while(currentNode->next != NULL){
        currentNode = currentNode->next;
    }
    currentNode->next = newNode;
    newNode->next = NULL;
    return head;
}

Node* deleteNodeAtBeginning(Node* head){
    if(head == NULL){
        return NULL;
    }

    //storing old heads address to free it up after assigning next
    Node* tmp = head;
    head = head->next;      //assigning new head
    free(tmp);              //deleting old head
    return head;
}

Node* deleteNodeAtEnd(Node* head){
    if(head == NULL){
        return NULL;
    }

    if(head->next == NULL){
        free(head);
        return NULL;
    }
    Node* currentNode = head;
    while(currentNode->next->next != NULL){
        currentNode = currentNode->next;
    }
    free(currentNode->next);
    currentNode->next = NULL;
    return head;
}

void traverseList(Node* head, void(traverseCallback)(void*)){
    Node* currentNode = head;
    while(currentNode != NULL){
        traverseCallback((void*)currentNode->data);
        currentNode = currentNode->next;
    }
}

void printList(Node* head){

    Node* currentNode = head;
    while(currentNode != NULL){
        printf("%d ->", *((int*)currentNode->data));
        currentNode = currentNode->next;
    }
    printf(" NULL\n");
}