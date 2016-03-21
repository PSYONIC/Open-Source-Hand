/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CircularBuffer.h"
#include "Arduino.h"

/* 
 *  This class implements a generic circular buffer used for the sampling window
 */

template<class T>
CircularBuffer<T>::CircularBuffer() : startIndex(0), currentIndex(0), currentSize(0), maxSize(WINDOWLENGTH) {
    for (int i = 0; i < maxSize; ++i) {
        data[i] = T();
    }
}

template<class T>
CircularBuffer<T>::CircularBuffer(int size) : startIndex(0), currentIndex(0), currentSize(0), maxSize(size) {
    for (int i = 0; i < maxSize; ++i) {
        data[i] = T();
    }
}

/* Adds an object T to the buffer */
template<class T>
void CircularBuffer<T>::add(const T value) {
    if (currentSize < maxSize) {
        data[(startIndex + currentIndex) % maxSize] = value;
        ++currentIndex;
        ++currentSize;
        currentIndex %= maxSize;
    }
}

/* Access instance at index i */
template<class T>
T CircularBuffer<T>::operator[](const int i) {
    if (i < currentSize) {
        return data[(startIndex + i) % maxSize];
    }
    return -1;
}

/* Unroll the circular buffer into an array */
template<class T>
void CircularBuffer<T>::copyDataToArray(T* dst) {
    int firstBlockSize = maxSize - startIndex;
    int secondBlockSize = maxSize - firstBlockSize;

    if (firstBlockSize > 0) {
        memcpy(dst, data + startIndex, firstBlockSize * sizeof(T));
        memcpy(dst + firstBlockSize, 0, secondBlockSize * sizeof(T));
    } else {
        memcpy(dst, data, maxSize * sizeof(T));
    }
}

/* Remove amount values from the circular buffer */
template<class T>
void CircularBuffer<T>::removeValues(int amount) {
    if (amount > currentSize) {
        startIndex = (startIndex + currentSize) % maxSize;
        currentSize = 0;
    } else {
        startIndex = (startIndex + amount) % maxSize;
        currentSize -= amount;
    }
    
}

template class CircularBuffer<float>;
template class CircularBuffer<int>;
