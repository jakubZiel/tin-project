//
// Created by skons on 24.01.2022.
//

#ifndef TIN_21Z_MESSAGEHISTORY_H
#define TIN_21Z_MESSAGEHISTORY_H

#include <queue>

template <typename T, int history_len, typename Container=std::deque<T>>
class MessageHistory : public std::queue<T, Container> {
public:
    void push(const T& value) {
        if (this->size() == history_len) {
            this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};



#endif //TIN_21Z_MESSAGEHISTORY_H
