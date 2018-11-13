#ifndef GRAPH_H
#define GRAPH_H



typedef struct adjVertex{
    char* fileType;
    struct adjVertex* nextAdjVertex;
}adjVertex;

typedef struct vertex{
    char* fileType;
    struct vertex* nextVertex;
    adjVertex* adjVertex;
} vertex;


void addVertex(char *fileType);
void addAjdVertex(char* vertexType, char* adjVertexType);
int isConvertible(char* fileType1,char* fileType2);
void addUnvisited(char* node);
void removeUnivisted(char* node);
int isVisited(char* node);
void addToPath(char* node);
void getFilePath(char* fileType1,char* fileType2);

vertex* VHead;

typedef struct visitedNode{
    char* value;
    struct visitedNode* nextVisitedNode;
    struct visitedNode* prevVisitedNode;
}visitedNode;

typedef struct unvisitedNode{
    char* value;
    struct unvisitedNode* nextUnvisitedNode;
}unvisitedNode;

typedef struct pathNode{
    char* value;
    struct pathNode* forward;
    struct pathNode* backward;
}pathNode;

visitedNode* visitedHead;
visitedNode* visitedTail;
unvisitedNode* unvisitedHead;
unvisitedNode* unvisitedTail;
pathNode* pathHead;
pathNode* pathTail;



#endif