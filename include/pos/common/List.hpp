#ifndef POS_COMMON_LIST_HPP
#define POS_COMMON_LIST_HPP

#include <string>

#include "pos/common/PosError.hpp"

namespace pos {

// A small growing array. We were limited to <iostream> and <string>, so this
// stands in for std::vector. It has the calls the rest of the POS code needs:
// push_back, size, [], begin/end (so range-for works), erase, clear.
// T must be default constructible and copy assignable (Order, OrderLine,
// std::string and List itself all are).
template <typename T>
class List {
public:
    using size_type = std::string::size_type;

    List() : data_(nullptr), size_(0), capacity_(0) {}

    List(const List& other) : data_(nullptr), size_(0), capacity_(0) {
        copyFrom(other);
    }

    List(List&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    List& operator=(const List& other) {
        if (this != &other) {
            List copy(other);
            swapWith(copy);
        }
        return *this;
    }

    List& operator=(List&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    ~List() { delete[] data_; }

    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& operator[](size_type index) { return data_[index]; }
    const T& operator[](size_type index) const { return data_[index]; }

    // Same as [] but refuses an index that is out of range.
    T& at(size_type index) {
        checkIndex(index);
        return data_[index];
    }
    const T& at(size_type index) const {
        checkIndex(index);
        return data_[index];
    }

    T& back() { return at(size_ - 1); }
    const T& back() const { return at(size_ - 1); }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            T copy(value);  // value may live inside this list, so copy before growing
            grow();
            data_[size_] = copy;
        } else {
            data_[size_] = value;
        }
        ++size_;
    }

    // Removes the element at index and shifts the rest one place left.
    void erase(size_type index) {
        checkIndex(index);
        for (size_type i = index; i + 1 < size_; ++i) {
            data_[i] = data_[i + 1];
        }
        --size_;
    }

    void clear() { size_ = 0; }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    void checkIndex(size_type index) const {
        if (index >= size_) {
            throw PosError("List index " + std::to_string(index) +
                           " is out of range (size " + std::to_string(size_) + ")");
        }
    }

    void grow() {
        size_type newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
        T* fresh = new T[newCapacity];
        try {
            for (size_type i = 0; i < size_; ++i) fresh[i] = data_[i];
        } catch (...) {
            delete[] fresh;
            throw;
        }
        delete[] data_;
        data_ = fresh;
        capacity_ = newCapacity;
    }

    void copyFrom(const List& other) {
        if (other.size_ == 0) return;
        data_ = new T[other.size_];
        try {
            for (size_type i = 0; i < other.size_; ++i) data_[i] = other.data_[i];
        } catch (...) {
            delete[] data_;
            data_ = nullptr;
            throw;
        }
        size_ = other.size_;
        capacity_ = other.size_;
    }

    void swapWith(List& other) {
        T* d = data_;
        data_ = other.data_;
        other.data_ = d;
        size_type s = size_;
        size_ = other.size_;
        other.size_ = s;
        size_type c = capacity_;
        capacity_ = other.capacity_;
        other.capacity_ = c;
    }

    T* data_;
    size_type size_;
    size_type capacity_;
};

}  // namespace pos

#endif
