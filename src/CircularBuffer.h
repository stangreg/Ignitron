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
        capacity_ = capacity;
        buffer_.clear();
        // buffer.resize(capacity);
    }

    void reset() { buffer_.clear(); }
    void add_element(int num) {
        buffer_.push_back(num);
        if (buffer_.size() > capacity_) {
            buffer_.pop_front();
        }
    };
    int averageValue() {
        if (buffer_.size() == 0) {
            return 0;
        }
        int count = buffer_.size();
        return std::accumulate(buffer_.begin(), buffer_.end(), 0) / count;
    }

    int size() {
        return buffer_.size();
    }

private:
    deque<int> buffer_;
    int capacity_;
};

#endif