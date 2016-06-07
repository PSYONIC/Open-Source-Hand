#ifndef CB_H
#define CB_H

#include "Config.h"
#define WINDOWLENGTH 200

template<class T>
class CircularBuffer {
public:
    CircularBuffer();
    CircularBuffer(int size);
        
    void add(const T value);
    
    T operator[](const int i);
    
    void removeValues(int amount);

    void copyDataToArray(T* dst);

    bool isFull() {
        return (currentSize == maxSize);
    }

    int size() {
        return currentSize;
    }

    int getMaxSize() {
        return maxSize;
    }
    
private:
    int startIndex;
    int currentIndex;
    int currentSize;
    int maxSize;
    T data[WINDOWLENGTH];
};

#endif
