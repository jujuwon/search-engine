/*
2021 고급자료구조 term_project
[검색 엔진 구현]

1.  문서가 주어졌을 때 문서를 읽어서 그 속에서 발생하는 단어를 분리,
    (단어, 문서번호, 빈도수) 로 구성된 쌍 생성.
2.  이들을 검색이 용이한 자료구조로 저장 -> 색인 (Indexing).
    문서 속에 포함된 단어를 분리하여 단어-문서번호 관계로 만들기.
    입력되는 문서에 적절하게 번호를 할당.
    문서 이름을 그대로 사용하기 보다는 번호로 바꾸어 저장하는 것이 효율적.
    이렇게 만들어진 '단어-문서번호' 관계를 이용하여 Hash 자료구조 이용, 단어 저장.
    이를 '색인 테이블' 이라고 함.
    색인 테이블에는 다음 정보가 담겨야 함.
    1) 단어 (w)
    2) 링크의 길이 (n)
    3) 링크 포인터
    단어는 앞에서 만든 '단어-문서번호'에서 단어.
    링크의 길이는 문서번호 링크드 리스트의 길이. (단어가 나타난 모든 문서 수와 동일)
    발생한 문서번호와 빈도를 담고 있는 링크드 리스트를 가리키는 포인터가 필요.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#define MAX_BUFFER  10000
#define BUFFER_SIZE  1000
#define TRUE    1
#define FALSE   0

// 단어 자료구조
typedef struct WordNode_ {
    int key;
    int count;
} WordNode;

// 문서번호 Linked List Node
typedef struct DocumentLinkNode_ {
    int documentNo;                 //  문서번호 (d)
    int frequency;                  //  단어의 빈도수 (f) -> 해당 단어가 이 문서에서 발생한 빈도
    struct DocumentLinkNode_ *next;   //  다음 노드를 가리키는 포인터
} DocumentLinkNode;

// 색인 자료구조
typedef struct BPlusTreeNode_ {
    int *keys;                              //  단어 (w) Hash Key
    int numOfKeys;                          //  현재 키 수
    int minDegree;                          //  최소 degree
    int isLeaf;                             //  leaf node 여부
    struct BPlusTreeNode_ **children;        //  자식 node 포인터
    int lengthOfLink;                        //  문서번호 링크의 길이 (n)
    struct DocumentLinkNode_ *linkPointer;    //  링크 포인터
} BPlusTreeNode;

// 함수 선언
int ReadFile(char *);
int Hashing(char *, size_t);
BPlusTreeNode *makeNewNode(int, int);
BPlusTreeNode *SearchNode(int, BPlusTreeNode *);
DocumentLinkNode *SearchDocument(BPlusTreeNode *);
void Insert(int, int, int);
void InsertToList(BPlusTreeNode *, DocumentLinkNode *);
void InsertUnfilled(int, BPlusTreeNode *);
void Split(int, BPlusTreeNode *, BPlusTreeNode *);
double CalcWeight(int, int, int);

// 전역 변수
BPlusTreeNode *root = NULL;
int t = 2;

int main(void) {
    // 무한루프, 3 입력 시 종료
    while(TRUE) { 
        char buf[1000];
        int i = 0, result = 0;
        printf("-------------------\n1. Insert\n2. Search\n3. Exit\n");
        printf("Enter the number : ");
        if(fgets(buf, 1000, stdin) == NULL) {
            fprintf(stderr, "Invalid input format !\n");
            continue;
        }
        result = sscanf(buf, "%d", &i);
        if(result != 1) {
            fprintf(stderr, "Invalid input format !\n");
            continue;
        }
        if(i == 1) {
            char path[BUFFER_SIZE] = {0,};
            printf("Enter the document path (based on the current path)) : ");
            
            if(fgets(buf, 1000, stdin) == NULL) {
                fprintf(stderr, "Invalid input format !\n");
                continue;
            }
            result = sscanf(buf, "%s", path);
            if(result != 1) {
                fprintf(stderr, "Invalid input format !\n");
                continue;
            }
            if(ReadFile(path)) {
                printf("The document has been successfully inserted !\n");
            } else {
                fprintf(stderr, "error : Invalid directory path !\n");
            }
        } else if(i == 2) {
            //search
            char word[BUFFER_SIZE];
            memset(word, 0, sizeof(word));
            printf("Search ! \n");
            printf("Enter the word you want to find : ");

            if(fgets(buf, 1000, stdin) == NULL) {
                fprintf(stderr, "Invalid input format (fgets) !\n");
                continue;
            }
            
            result = sscanf(buf, "%s", word);
            if(result != 1) {
                fprintf(stderr, "Invalid input format !\n");
                continue;
            }

            // int offset = 0;
            // while(word[offset] != NULL) {
            //     if(word[offset] == '\r' || word[offset] == '\n') {
            //         word[offset] = '\0';
            //     }
            //     offset++;
            // }

            printf("Searching... Please wait\n");

            int hash = Hashing(word, strlen(word));
            printf("hash : %d\n", hash);
            BPlusTreeNode *node = SearchNode(hash, root);
            if(node == NULL) {
                printf("There are no documents with the word !\n");
            } else {
                DocumentLinkNode *ret = SearchDocument(node);
                if(ret == NULL) {
                    printf("There are no documents with the word !\n");
                } else {
                    printf("[weighted]\n");
                    printf("The document in which the word '%s' appears is %d\n", word, ret->documentNo);
                    printf("and the frequency is %d in that document.\n", ret->frequency);
                }
            }
        } else if(i == 3) {
            printf("Quit the program ! \n");
            // todo 전체 메모리 해제 후 프로그램 종료하기
            exit(0);
        } else {
            printf("Wrong input ! \n\n");
        }
    }

    return 0;
}

int ReadFile(char *path) {
    DIR *dirPointer;
    struct dirent *directory;
    dirPointer = opendir(path);
    WordNode words[MAX_BUFFER];
    char *filePath = (char *)malloc(sizeof(char) * MAX_BUFFER);

    if(dirPointer == NULL) {
        return FALSE;
    }

    int documentNo = 1;
    int isShow = FALSE;

    while((directory = readdir(dirPointer)) != NULL) {
        
        int wordCount = 0;
        int isFirst = TRUE;

        if(directory->d_ino == 0 || !strcmp(directory->d_name, ".") || !strcmp(directory->d_name, "..")) {
            continue;
        }

        // filePath 초기화
        memset(filePath, 0, sizeof(WordNode));
        // filePath = path + /
        strncpy(filePath, path, strlen(path));
        const char *temp = strncat(filePath, "/", 1);
        // fileName = filePath + d_name
        const char *fileName = strncat(filePath, directory->d_name, strlen(directory->d_name));

        FILE *filePointer = fopen(filePath, "r");
        if(filePointer == NULL) {
            fprintf(stderr, "error : Can't open file !\n");
            continue;
        }

        char readBuffer[MAX_BUFFER] = { 0, };
        char *tokPointer = NULL;
        if(isShow == FALSE) {
            printf("Indexing the entered document... Please wait\n");
            isShow = TRUE;
        }

        fread(readBuffer, 1, MAX_BUFFER, filePointer);
        // 문서 하나씩 읽고, 공백 구분해서 split
        tokPointer = strtok(readBuffer, "\n\r.,=() ");
        while(tokPointer != NULL) {
            int key = Hashing(tokPointer, strlen(tokPointer));
            isFirst = TRUE;
            wordCount++;
            for(int i = 0; i < wordCount; i++) {
                if(words[i].key == key) {
                    isFirst = FALSE;
                    words[i].count++;
                    wordCount--;
                    break;
                }
            }
            if(isFirst) {
                words[wordCount-1].key = key;
                words[wordCount-1].count = 1;
            }

            tokPointer = strtok(NULL, "\n\r.,=() ");
        }

        for(int i = 0; i < wordCount; i++) {
            Insert(words[i].key, documentNo, words[i].count);
        }

        printf("Document No : %d\n", documentNo);
        documentNo++;
        fclose(filePointer);
    }

    printf("The document has been entered !\n");
    closedir(dirPointer);
    free(filePath);
    return TRUE;
}

int Hashing(char *input, size_t size) {
    int key, i, count = 0;
    for(key = i = 0; i < size; ++i)
    {
        key += input[i];
        key += (key << 10);
        key ^= (key >> 6);
        count++;
    }
    if(count <= 0) {
        return -1;
    } else {
        key += (key << 3);
        key ^= (key >> 11);
        key += (key << 15);
        return key;
    }
}

BPlusTreeNode *makeNewNode(int t, int leaf) {
    BPlusTreeNode *newNode = (BPlusTreeNode *)malloc(sizeof(BPlusTreeNode));
    newNode->keys = (int *)malloc((2*t-1) * sizeof(int));    
    newNode->minDegree = t;
    newNode->children = (BPlusTreeNode **)malloc((2*t) * sizeof(BPlusTreeNode));
    newNode->numOfKeys = 0;
    newNode->isLeaf = leaf;
    newNode->lengthOfLink = 0;
    newNode->linkPointer = NULL;

    return newNode;
}

void InsertToList(BPlusTreeNode *treeNode, DocumentLinkNode *docNode) {
    if(treeNode->lengthOfLink == 0) {
        treeNode->linkPointer = docNode;
    } else {
        docNode->next = treeNode->linkPointer->next;
        treeNode->linkPointer = docNode;
    }
    treeNode->lengthOfLink++;
    // temp, newNode
    // if(treeNode->linkPointer == NULL) {
    //     printf("treeNode->linkPointer == NULL\n");
    //     treeNode->linkPointer = docNode;
    // }
    // printf("treeNode->lengthOfLink : ");
    // // printf("%d\n", treeNode->lengthOfLink);
    // DocumentLinkNode *cur = treeNode->linkPointer;
    // for(int i = 0; i < treeNode->lengthOfLink - 1; i++) {
    //     printf("[for] i : %d\n", i);
    //     cur = cur->next;
    // }
    // cur->next = docNode;
    // treeNode->lengthOfLink++;
}

void Insert(int key, int documentNo, int frequency) {
    // sleep(1);
    // tree 가 비었을 때
    if(root == NULL) {
        root = makeNewNode(t, TRUE);
        root->keys[0] = key;
        root->numOfKeys = 1;
        // 링크 포인터에 새 문서노드 달기
        DocumentLinkNode *newDocNode = (DocumentLinkNode *)malloc(sizeof(DocumentLinkNode));
        newDocNode->documentNo = documentNo;
        newDocNode->frequency = frequency;
        newDocNode->next = NULL;
        InsertToList(root, newDocNode);
    } else { // tree 가 비어있지 않을 때
        BPlusTreeNode *temp;
        // search 결과 key 존재
        if((temp = SearchNode(key, root)) != NULL) { 
            // 해당 key node 에 문서 노드 하나 만들어서 새로 달기
            DocumentLinkNode *newDocNode = (DocumentLinkNode *)malloc(sizeof(DocumentLinkNode));
            newDocNode->documentNo = documentNo;
            newDocNode->frequency = frequency;
            newDocNode->next = NULL;
            InsertToList(temp, newDocNode);
        } else {
            // key 가 없는 새로운 노드 생성
            if(root->numOfKeys == 2*t-1) { 
                // key 가 다 찼을 때
                temp = makeNewNode(t, FALSE);
                temp->children[0] = root;
                DocumentLinkNode *newDocNode = (DocumentLinkNode *)malloc(sizeof(DocumentLinkNode));
                newDocNode->documentNo = documentNo;
                newDocNode->frequency = frequency;
                newDocNode->next = NULL;
                InsertToList(temp, newDocNode);
                Split(0, temp, root);
                int i = 0, j = 0;
                    i++;
                if(temp->keys[0] < key) {
                    j++;
                }
                InsertUnfilled(key, temp->children[i]);
                root = temp;
            } else {
                DocumentLinkNode *newDocNode = (DocumentLinkNode *)malloc(sizeof(DocumentLinkNode));
                newDocNode->documentNo = documentNo;
                newDocNode->frequency = frequency;
                newDocNode->next = NULL;
                InsertToList(root, newDocNode);
                InsertUnfilled(key, root);
            }
        }
    }
}

void Split(int i, BPlusTreeNode *a, BPlusTreeNode *b) {
    int num = b->minDegree - 1;
    BPlusTreeNode *newNode = makeNewNode(b->minDegree, b->isLeaf);
    newNode->numOfKeys = num;
    int j = 0;
    int count = 0;

    while(j < (b->minDegree - 1)) {
        count++;
        newNode->keys[j] = b->keys[j + b->minDegree];
        j++;
    }
    if(b->isLeaf == FALSE) { // internal node 일 때
        j = 0;
        while(j < (b->minDegree)) {
            count--;
            newNode->children[j] = b->children[j + b->minDegree];
            j++;
        }
    }
    b->numOfKeys = b->minDegree - 1;
    j = a->numOfKeys;
    while(j >= i + 1) {
        count++;
        a->children[j+1] = a->children[j];
        j--;
    }
    a->children[i+1] = newNode;
    j = (a->numOfKeys-1);
    while(j >= i) {
        count--;
        a->keys[j+1] = a->keys[j];
        j--;
    }
    a->keys[i] = b->keys[b->minDegree-1];
    count = 0;
    a->numOfKeys++;
}

void InsertUnfilled(int key, BPlusTreeNode *temp) {
    int index = temp->numOfKeys-1;
    if(temp->isLeaf == TRUE) { // leaf node 일 때
        int count = 0;
        for( ; index >= 0 && temp->keys[index] > key; index--) { // key 비교
            // 입력된 key 가 더 작으면 하나씩 뒤로 이동
            temp->keys[index+1] = temp->keys[index];
            count++;
        }
        temp->keys[index+1] = key;
        temp->numOfKeys++;
    } else { // internal node 일 때
        int count = 0;
        for( ; index > 0 && temp->keys[index] > key; index--) {
            index--;
            count++;
        }
        if(temp->children[index+1]->numOfKeys == 2*t-1) {
            Split(index+1, temp, temp->children[index+1]);
            if(temp->keys[index+1] < key) {
                index++;
            }
        }
        InsertUnfilled(key, temp->children[index+1]);
    }
}

// 해당 key 를 가진 Node 가 있는지 체크 후 Node return
BPlusTreeNode *SearchNode(int key, BPlusTreeNode *node) {
    int i = 0;
    int count = 0;
    while (i < node->numOfKeys && key > node->keys[i]) {
        i++;
    }
 
    if(node->keys[i] == key) {
        count++;
        return node;
    }
    if(node->isLeaf == FALSE) {
    }
    if(node->isLeaf == TRUE) {
        count--;
        return NULL;
    }

    return SearchNode(key, node->children[i]);
}

// Leaf Node 에서 Link 를 순회하며 가장 가중치 높은 문서번호 return
DocumentLinkNode *SearchDocument(BPlusTreeNode *node) {
    if(node->lengthOfLink <= 0) {
        return NULL;
    }

    int N = 0;
    DocumentLinkNode *docNode = node->linkPointer;
    N = docNode->frequency;
    while(docNode->next != NULL) {
        docNode = docNode->next;
        N += docNode->frequency;
    }
    
    DocumentLinkNode *ret = node->linkPointer;
    docNode = node->linkPointer;
    int max = CalcWeight(docNode->frequency, N, docNode->documentNo);
    while(docNode->next != NULL) {
        docNode = docNode->next;
        if(CalcWeight(docNode->frequency, N, docNode->documentNo) > max) {
            ret = docNode;
        }
    }

    return ret;
}

// 가중치 계산 func
double CalcWeight(int f, int N, int D) {
    return f * log10(N/(1+D));
}
