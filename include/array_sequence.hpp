#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename T>
class ArraySequence {
   private:
    T* data;
    size_t length;
    size_t capacity;

    void Grow() {
        size_t newCapacity = capacity == 0 ? 4 : capacity * 2;
        T* newData = new T[newCapacity];
        for(size_t i = 0; i < length; i++) { newData[i] = std::move(data[i]); }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

    void CheckIndex(size_t index) const {
        if(index >= length) {
            throw std::out_of_range("Index is out of range");
        }
    }

   public:
    ArraySequence() : data(nullptr), length(0), capacity(0) {}

    ArraySequence(const ArraySequence& other) : data(nullptr), length(0), capacity(0) {
        for(size_t i = 0; i < other.length; i++) { Append(other.data[i]); }
    }

    ArraySequence(ArraySequence&& other) noexcept
        : data(other.data), length(other.length), capacity(other.capacity) {
        other.data = nullptr;
        other.length = 0;
        other.capacity = 0;
    }

    ArraySequence& operator=(const ArraySequence& other) {
        if(this != &other) {
            ArraySequence copy(other);
            std::swap(data, copy.data);
            std::swap(length, copy.length);
            std::swap(capacity, copy.capacity);
        }
        return *this;
    }

    ArraySequence& operator=(ArraySequence&& other) noexcept {
        if(this != &other) {
            delete[] data;
            data = other.data;
            length = other.length;
            capacity = other.capacity;
            other.data = nullptr;
            other.length = 0;
            other.capacity = 0;
        }
        return *this;
    }

    ~ArraySequence() { delete[] data; }

    void Append(const T& item) {
        if(length == capacity) {
            Grow();
        }
        data[length] = item;
        length++;
    }

    T& Get(size_t index) {
        CheckIndex(index);
        return data[index];
    }

    const T& Get(size_t index) const {
        CheckIndex(index);
        return data[index];
    }

    T& operator[](size_t index) { return Get(index); }
    const T& operator[](size_t index) const { return Get(index); }

    T& GetLast() { return Get(length - 1); }

    size_t GetLength() const { return length; }

    void RemoveAt(size_t index) {
        CheckIndex(index);
        for(size_t i = index + 1; i < length; i++) { data[i - 1] = std::move(data[i]); }
        length--;
    }

    void Clear() { length = 0; }

    T* begin() { return data; }
    T* end() { return data + length; }
    const T* begin() const { return data; }
    const T* end() const { return data + length; }
};
