//
// Created by QSilver on 2022/3/14.
//

#ifndef OS_P2_BLOCK_H
#define OS_P2_BLOCK_H
class Block{
public:
    unsigned int offset;
    unsigned int length;
    bool used;
    uint64_t * ptr;
    Block(unsigned int offset, unsigned int length, bool used){
        this->offset = offset;
        this->length = length;
        this->used = used;
    }
};
#endif //OS_P2_BLOCK_H
