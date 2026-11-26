#pragma once

#include <cstddef>
#include <stdexcept>

template <typename T>
class ListSequence {
   private:
    struct Node {
        T value;
        Node* next;

        Node(const T& v) : value(v), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t length;

    void Clear() {
        while(head != nullptr) {
            Node* next = head->next;
            delete head;
            head = next;
        }
        tail = nullptr;
        length = 0;
    }

   public:
    class Iterator {
       private:
        Node* node;

       public:
        Iterator(Node* n) : node(n) {}

        T& operator*() const { return node->value; }
        Iterator& operator++() {
            node = node->next;
            return *this;
        }
        bool operator!=(const Iterator& other) const { return node != other.node; }
    };

    ListSequence() : head(nullptr), tail(nullptr), length(0) {}

    ListSequence(const ListSequence& other) : head(nullptr), tail(nullptr), length(0) {
        for(Node* node = other.head; node != nullptr; node = node->next) { Append(node->value); }
    }

    ListSequence& operator=(const ListSequence& other) {
        if(this != &other) {
            Clear();
            for(Node* node = other.head; node != nullptr; node = node->next) { Append(node->value); }
        }
        return *this;
    }

    ~ListSequence() { Clear(); }

    void Append(const T& item) {
        Node* node = new Node(item);
        if(tail == nullptr) {
            head = node;
        } else {
            tail->next = node;
        }
        tail = node;
        length++;
    }

    void PopFront() {
        if(head == nullptr) {
            throw std::out_of_range("Sequence is empty");
        }
        Node* next = head->next;
        delete head;
        head = next;
        if(head == nullptr) {
            tail = nullptr;
        }
        length--;
    }

    T& GetLast() {
        if(tail == nullptr) {
            throw std::out_of_range("Sequence is empty");
        }
        return tail->value;
    }

    T& operator[](size_t index) {
        if(index >= length) {
            throw std::out_of_range("Index is out of range");
        }
        Node* node = head;
        for(size_t i = 0; i < index; i++) { node = node->next; }
        return node->value;
    }

    size_t GetLength() const { return length; }

    Iterator begin() const { return Iterator(head); }
    Iterator end() const { return Iterator(nullptr); }
};
