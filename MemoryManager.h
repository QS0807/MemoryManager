//
// Created by QSilver on 2022/3/14.
//

#ifndef OS_P2_1_MEMORYMANAGER_H
#define OS_P2_1_MEMORYMANAGER_H
#include <string>
#include <cmath>
#include <array>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "Block.h"
#include <list>
#include <functional>
#include <unistd.h>
#include <unistd.h>
#include <sys/file.h>
#include <cstring>
#include <iomanip>
#include <climits>
#include <algorithm>
using namespace std;
class MemoryManager {
public:
    bool freed;
    int setallocator;
    bool getlistfreed;
    /** for memory leak check **/
    uint16_t *arr; //for get list
    unsigned int wordSize;
    size_t sizeInWords;
    uint64_t * BigMemory;
    vector<char>memory;    //0s and 1s used in bitmap
    vector<Block> list;     //keep track of free and allocated space
    function<int(int, void *)> allocator;

    /**Constructor and Destructor**/
    MemoryManager(unsigned wordSize, function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void *allocate(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void *getMemoryStart();
    unsigned getMemoryLimit();

    /** helper function **/
    bool mergeRight(unsigned int offset, unsigned int length, int location){
        bool merge = false;

        if(list[location+1].used==false){
            list[location].length += list[location+1].length;

            list.erase(list.begin() + location + 1);
            merge = true;
        }
        return merge;
    }
    bool mergeLeft(unsigned int offset, unsigned int length, int location){
        bool merge = false;
        if(list[location-1].used==false){
            list[location-1].length += length;

            list.erase(list.begin() + location);
            merge=true;
        }
        return merge;
    }
};
/**Memory Allocation Algorithm**/
static int bestFit(int sizeInWords, void *list){

//    int* ptr = (int*) list;
    vector<uint16_t>ptr;
    if(list== nullptr){
        return -1;
    }
    uint16_t* getlist = static_cast<uint16_t*>(list);
    uint16_t listLength = *getlist;
    for(unsigned int i =0;i<2*(listLength)+1;i++){
        ptr.push_back(getlist[i]);
    }
    int i =1;
    int holeSize = INT_MAX;
    int index = 0;

    for(int j =0;j<ptr[0];j++){
        if(ptr[2*i]>=sizeInWords && ptr[2*i]<holeSize){
            index = 2*i;
            holeSize = ptr[2*i];
        }
        else if(ptr[index]<sizeInWords){
            return -1;
        }
        i++;
    }
    uint16_t ret = ptr[index-1];
 //   std::free(getlist);
    return ret;
}
static int worstFit(int sizeInWords, void *list){

    vector<uint16_t>ptr;
    if(list== nullptr){
        return -1;
    }
    uint16_t* getlist = static_cast<uint16_t*>(list);
    uint16_t listLength = *getlist;
    for(unsigned int i =0;i<2*(listLength)+1;i++){
        ptr.push_back(getlist[i]);
    }
    int i =1;
    int holeSize = INT_MIN;
    int index = 0;

    for(int j =0;j<ptr[0];j++){
        if(ptr[2*i]>=sizeInWords && ptr[2*i]>holeSize){
            index = 2*i;
            holeSize = ptr[2*i];
        }
        else if(ptr[index]<sizeInWords){
            return -1;
        }
        i++;
    }
 //   std::free(getlist);
    return ptr[index-1];
}


#endif //OS_P2_1_MEMORYMANAGER_H
