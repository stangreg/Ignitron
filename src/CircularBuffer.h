/*
 * CircularBuffer.h
 *
 *  Created on: 22.09.2024
 *      Author: steffen
 */

#ifndef CIRCULARBUFFER_H_
#define CCIRCULARBUFFER_H_

#include <deque>
#include <numeric>

using namespace std;

class CircularBuffer {

public:
    CircularBuffer(int capacity) {
        this->capacity = capacity;
        buffer.clear();
        // buffer.resize(capacity);
    }

    void reset() { buffer.clear(); }
    void add_element(int num) {
        buffer.push_back(num);
        if (buffer.size() > capacity) {
            buffer.pop_front();
        }
    };
    int averageValue() {
        if (buffer.size() == 0) {
            return 0;
        }
        int count = buffer.size();
        return std::accumulate(buffer.begin(), buffer.end(), 0) / count;
    }

    int size() {
        return buffer.size();
    }

private:
    deque<int> buffer;
    int capacity;
};

#endif