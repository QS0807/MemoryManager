//
// Created by QSilver on 2022/3/14.
//
#include "MemoryManager.h"
using namespace std;

MemoryManager::MemoryManager(unsigned int wordSize, std::function<int(int, void *)> allocator) {
    this->allocator = allocator;
    this->wordSize = wordSize;
    freed = true;
}
MemoryManager::~MemoryManager() {


}
void MemoryManager::shutdown() {
    memory.erase(memory.begin(),memory.end());
//    for(unsigned int i =0;i<list.size();i++){
//        delete list[i].ptr;
//    }
    list.erase(list.begin(),list.end());
    if(!freed){
        std::free(BigMemory);
        freed=true;
    }
}
void MemoryManager::setAllocator(std::function<int(int, void *)> allocator) {
    this->allocator = allocator;
    setallocator++;
}
unsigned int MemoryManager::getMemoryLimit() {
    return this->wordSize * this->sizeInWords;
}
unsigned int MemoryManager::getWordSize() {
    return this->wordSize;
}
void * MemoryManager::getMemoryStart() {
    return &BigMemory[0];
}
void MemoryManager::initialize(size_t sizeInWords) {
    shutdown();
    this->sizeInWords = sizeInWords;
    unsigned int total = sizeInWords * this->wordSize;
    for (unsigned int i = 0;i<sizeInWords;i++){
        this->memory.push_back(('0'));
    }
    list.push_back(Block(0,total,false));
    BigMemory = (uint64_t*) malloc(total);
    freed = false;
    setallocator=0;
}
void * MemoryManager::allocate(size_t sizeInBytes) {

    int word = sizeInBytes / wordSize;


    int Index = allocator(word, getList());
    std::free(arr);

    if(Index==-1){
 //       std::free(BigMemory);
        return nullptr;
    }
    for(unsigned int i =0;i<word;i++){
        memory[i + Index] = ('1');
    }
    for(unsigned int i = 0;i<list.size();i++){
        if(Index == list[i].offset){
            unsigned int tempLength = list[i].length;
            unsigned int tempOffset = list[i].offset;
            /** not sure **/
            if(tempLength - sizeInBytes >0){
                list.insert(list.begin()+i+1,Block(tempOffset + word,tempLength - sizeInBytes,false));
            }
            /** need to know the correct block to free **/
            list[i].ptr = &BigMemory[Index];

            list[i].length = sizeInBytes;
            list[i].used = true;

            int maxcheck=0;
            for(int i =0;i<list.size();i++){
                if(list[i].used){
                    maxcheck+=list[i].length;
                }
            }
            if(maxcheck>=this->wordSize*this->sizeInWords){
                cout<<"max"<<endl;
                std::free(BigMemory);
            }
            return list[i].ptr;
        }
    }
}
void MemoryManager::free(void *address) {
//    Block* ptr = (Block*) address;

    uint64_t * ptr = (uint64_t*) address;
    unsigned int length = 0;
    unsigned int offset = 0;
    int location = 0;
    for(unsigned int i =0; i<list.size();i++){
        if(list[i].ptr == ptr){
            list[i].used = false;
            offset = list[i].offset;
            length = list[i].length;
            location = i;
            //delete the ptr pointing to Bigmemory
            ptr = nullptr;
            delete ptr;
            break;
        }
    }
    /** clear portion of the memory vector **/
    int word = length / wordSize;
    for(unsigned int i =0;i<word;i++){
        memory[i + offset] = ('0');
    }
    /** merge adjacent holes **/
    if(location > 0){
        bool merge = mergeLeft(offset,length,location);
        if(merge){
            location--;
        }
    }
    if(location < list.size()){
        mergeRight(offset,length,location);
    }
    if(list.size()==1 && !list[0].used){std::free(BigMemory);}
}

void * MemoryManager::getList() {

    int numOfHoles = 0;
    for(unsigned int i =0;i<list.size();i++){
        if(list[i].used==false){
            numOfHoles++;
        }
    }
    int size = numOfHoles * 2 + 1;
    arr = (uint16_t*) malloc(size);
    if (numOfHoles == 0){
        return nullptr;
    }
//    unsigned int *arr = new unsigned int [size];

    arr[0] = numOfHoles;
    int j =1;
    for(unsigned int i = 0;i<list.size();i++){
        if(list[i].used == false){
            arr[j] = list[i].offset;
            arr[j+1] = list[i].length / wordSize;
            j+=2;
        }
    }
    return arr;
}
void * MemoryManager::getBitmap() {
    int size = ceil((float)memory.size()/8);
    uint8_t* bitmap = new uint8_t [size];
    /** first two byte: the size of bitmap **/
    int listSize = list.size();
    stringstream ss;
    ss<<setfill('0')<<setw(sizeof(size))<<hex<<size;
    string res = ss.str();
    string highByteStr = res.substr(0,2);
    string lowByteStr = res.substr(2,4);
    uint8_t low = (uint8_t)stoi(lowByteStr, 0,16);
    uint8_t high = (uint8_t)stoi(highByteStr, 0,16);
    bitmap[0] = low;
    bitmap[1] = high;

    /** the rest of bitmap **/
    reverse(memory.begin(),memory.end());
    vector<char> litEndian = memory;
    reverse(memory.begin(),memory.end());

    int j =2;
    int byteCount = 0;
    string byte="";

    for(int i =litEndian.size()-1;i>=0;i--){
        byteCount++;
        byte.insert(0,string(1,litEndian[i]));
        if(byteCount==8 && litEndian.size()>=8){
            bitmap[j] = (uint8_t)stoi(byte, 0,2);
            byteCount=0;
            byte="";
            j++;
        }
        else if (i==0){
            bitmap[j]=(uint8_t)(stoi(byte, 0,2));
            break;
        }
    }
    return bitmap;
}
int MemoryManager::dumpMemoryMap(char *filename) {

    uint16_t* arr2 = static_cast<uint16_t*>(getList());



    int fd=-1;
    if((fd = open(filename,O_CREAT | O_RDWR | O_TRUNC, 0644))==-1){
        return -1;
    }
    int size = arr2[0] * 2 + 1;

    std::string vectorString = "";

    vectorString += "[" + std::to_string(arr2[1]) + ", " + std::to_string(arr2[2]);
    for (int i = 3; i < size; i +=2)
    {
        vectorString += "] - [" + std::to_string(arr2[i]) + ", " + std::to_string(arr2[i + 1]);
    }
    vectorString += "]";
    char buf[vectorString.length()];

    strcpy(buf,vectorString.c_str());
    write(fd, buf, strlen(buf));
    delete []arr2;
    close(fd);

    return 0;
}
