
// A C Program to demonstrate adjacency list
// representation of graphs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

void addVertex(char *fileType){
    if (VHead == NULL){
        vertex* vertex = malloc(sizeof(vertex));
        char* ft = malloc(strlen(fileType) * sizeof(char));
        strcpy(ft,fileType);
        vertex->fileType = ft;
        vertex->nextVertex = NULL;
        vertex->adjVertex = NULL;
        VHead = vertex;
    }
    else{
        vertex* newVertex;
        vertex* vertex = VHead;
        while(vertex->nextVertex != NULL){
            if(strcmp(fileType,vertex->fileType) == 0){
                return;
            }
            vertex = vertex->nextVertex;
        }
        if(strcmp(fileType,vertex->fileType) == 0){
            return;
        }
        newVertex =  malloc(sizeof(vertex));
        char* ft = malloc(strlen(fileType) * sizeof(char));
        strcpy(ft,fileType);
        newVertex->fileType = ft;
        newVertex->nextVertex = NULL;
        newVertex->adjVertex=NULL;
        vertex->nextVertex = newVertex;
    }
}

void addAjdVertex(char* vertexType, char* adjVertexType){
    adjVertex* newVertex;
    vertex* vertex = VHead;
    while(vertex != NULL){
        if(strcmp(vertexType,vertex->fileType) == 0){
            break;
        }
        vertex = vertex->nextVertex;
    }
    if(vertex == NULL){
        return;
    }
    if(vertex->adjVertex == NULL){
        adjVertex* adjVertex = malloc(sizeof(adjVertex));
        char* vertexType = malloc(strlen(adjVertexType)*sizeof(char));
        strcpy(vertexType,adjVertexType);
        adjVertex->fileType = vertexType;
        adjVertex->nextAdjVertex = NULL;
        vertex->adjVertex = adjVertex;
    }
    else{
        adjVertex* adjVertex = vertex->adjVertex;
        while(adjVertex->nextAdjVertex != NULL){
            if(strcmp(adjVertexType,adjVertex->fileType) == 0){
                return;
            }
            adjVertex = adjVertex->nextAdjVertex;
        }
        if(strcmp(adjVertexType,adjVertex->fileType) == 0){
            return;
        }
        newVertex = malloc(sizeof(adjVertex));
        char* vertexType = malloc(strlen(adjVertexType)*sizeof(char));
        strcpy(vertexType,adjVertexType);
        newVertex->fileType = vertexType;
        newVertex->nextAdjVertex = NULL;
        adjVertex->nextAdjVertex = newVertex;
    }
}

void addUnvisited(char* node){
    if(unvisitedHead == NULL && unvisitedTail == NULL){
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        unvisitedHead = malloc(sizeof(unvisitedNode));
        unvisitedHead->value = nodeValue;
        unvisitedHead->nextUnvisitedNode = NULL;
        unvisitedTail = unvisitedHead;
    }
    else if(unvisitedHead == unvisitedTail && unvisitedHead != NULL && unvisitedTail != NULL){
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        unvisitedNode* newNode = malloc(sizeof(unvisitedNode));
        newNode->value = nodeValue;
        newNode->nextUnvisitedNode = NULL;
        unvisitedHead->nextUnvisitedNode = newNode;
        unvisitedTail = newNode;
    }
    else{
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        unvisitedNode* newNode = malloc(sizeof(unvisitedNode));
        newNode->value = nodeValue;
        newNode->nextUnvisitedNode = NULL;
        unvisitedTail->nextUnvisitedNode = newNode;
        unvisitedTail = newNode;
    }
}

void removeUnivisted(char* node){
    if(unvisitedHead == NULL && unvisitedTail == NULL){
        return;
    }
    else if(unvisitedHead == unvisitedTail && unvisitedHead != NULL && unvisitedTail != NULL){
        unvisitedHead = NULL;
        unvisitedTail = NULL;
    }
    else{
        unvisitedHead = unvisitedHead->nextUnvisitedNode;
    }
}

void addVisited(char* node){
    if(visitedHead == NULL && visitedTail == NULL){
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        visitedHead = malloc(sizeof(visitedNode));
        visitedHead->value = nodeValue;
        visitedHead->nextVisitedNode = NULL;
        visitedHead->prevVisitedNode = NULL;
        visitedTail = visitedHead;
    }
    else{
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        visitedNode* newNode = malloc(sizeof(visitedNode));
        newNode->value = nodeValue;
        newNode->nextVisitedNode = NULL;
        newNode->prevVisitedNode = visitedTail;
        visitedTail->nextVisitedNode = newNode;
        visitedTail = newNode;
    }
}

int isVisited(char* node){
    visitedNode* visitedNode = visitedHead;
    while(visitedNode != NULL){
        if(strcmp(visitedNode->value,node) == 0){
            return 1;
        }
        visitedNode = visitedNode->nextVisitedNode;
    }
    return 0;
}

int isConvertible(char* fileType1,char* fileType2){
    vertex* currentVertex = VHead;
    while(strcmp(currentVertex->fileType,fileType1) != 0){
        if(currentVertex == NULL)
            return 0;
        currentVertex = currentVertex->nextVertex;
    }
    if(currentVertex->adjVertex == NULL){
        return 0;
    }
    addVisited(currentVertex->fileType);
    adjVertex* adjVer = currentVertex->adjVertex;
    while(adjVer != NULL){
        addUnvisited(adjVer->fileType);
        adjVer = adjVer->nextAdjVertex;
    }
    while(unvisitedHead != NULL && unvisitedTail != NULL){
        unvisitedNode* unvisitedNode = unvisitedHead;
        while(unvisitedNode != NULL){
            if(isVisited(unvisitedNode->value) == 0){
                addVisited(unvisitedNode->value);
                removeUnivisted(unvisitedNode->value);
                if(strcmp(unvisitedNode->value,fileType2) == 0){
                    return 1;
                }
            }
            vertex* currentVertex = VHead;
            while(strcmp(currentVertex->fileType,unvisitedNode->value) != 0){
                if(currentVertex == NULL)
                    return 0;
                currentVertex = currentVertex->nextVertex;
            }
            adjVertex* adjVer = currentVertex->adjVertex;
            while(adjVer != NULL){
                addUnvisited(adjVer->fileType);
                adjVer = adjVer->nextAdjVertex;
            }
            unvisitedNode = unvisitedNode->nextUnvisitedNode;
        }
    }
    return 0;
}

void addToPath(char* node){
    if(pathHead == NULL && pathTail == NULL){
        pathTail = malloc(sizeof(pathNode));
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        pathTail->value = nodeValue;
        pathTail->backward = NULL;
        pathTail->forward = NULL;
        pathHead = pathTail;
    }
    else if(pathHead == pathTail && pathHead != NULL && pathTail != NULL){
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        pathNode* newNode = malloc(sizeof(pathNode));
        newNode->value = nodeValue;
        newNode->forward = pathTail;
        newNode->backward = NULL;
        pathTail->backward = newNode;
        pathHead = newNode;
    }
    else{
        char* nodeValue = malloc(strlen(node)*sizeof(char));
        strcpy(nodeValue,node);
        pathNode* newNode = malloc(sizeof(pathNode));
        newNode->value = nodeValue;
        newNode->forward = pathHead;
        newNode->backward = NULL;
        pathHead->backward = newNode;
        pathHead = newNode;
    }
}

void getFilePath(char* fileType1,char* fileType2){
    if(isConvertible(fileType1,fileType2) == 1){
        visitedNode* destType = visitedTail;
        visitedNode* srcType = visitedTail->prevVisitedNode;
        addToPath(destType->value);
        while(srcType != NULL){
            vertex* currentVertex = VHead;
            while(currentVertex != NULL){
                if(strcmp(currentVertex->fileType,srcType->value) ==0)
                    break;
                currentVertex = currentVertex->nextVertex;
            }
            adjVertex* adjVertex = currentVertex->adjVertex;
            while(adjVertex != NULL){
                if(strcmp(adjVertex->fileType,destType->value) == 0){
                    addToPath(srcType->value);
                    destType = srcType;
                    break;
                }
                adjVertex = adjVertex->nextAdjVertex;
            }
            srcType = srcType->prevVisitedNode;
        }
    }
}