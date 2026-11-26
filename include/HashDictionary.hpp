#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>//shrinkAt  сделать проценты добл

#include "array_sequence.hpp"
#include "hash_functions.hpp"
#include "i_dictionary.hpp"

template <typename Key, typename Value>
class HashDictionary : public IDictionary<Key, Value> {
   private:
    struct Entry {
        Key key;
        Value value;
    };

    using Bucket = ArraySequence<Entry>;
    using HashFunc = std::function<size_t(const Key&)>;

    ArraySequence<Bucket> buckets;
    HashFunc hashFunc;
    size_t count;
    size_t shrinkAt;
    size_t factor;

    size_t IndexOf(const Key& key, size_t capacity) const {
        return static_cast<unsigned int>(hashFunc(key)) % capacity;
    }

    Entry* FindEntry(const Key& key) {
        Bucket& bucket = buckets[IndexOf(key, buckets.GetLength())];
        for(size_t i = 0; i < bucket.GetLength(); i++) {
            if(bucket[i].key == key) {
                return &bucket[i];
            }
        }
        return nullptr;
    }

    void Resize(size_t newCapacity) {
        ArraySequence<Bucket> newBuckets;
        for(size_t i = 0; i < newCapacity; i++) { newBuckets.Append(Bucket()); }
        for(size_t i = 0; i < buckets.GetLength(); i++) {
            for(size_t j = 0; j < buckets[i].GetLength(); j++) {
                Entry& entry = buckets[i][j];
                newBuckets[IndexOf(entry.key, newCapacity)].Append(entry);
            }
        }
        buckets = std::move(newBuckets);
    }

   public:
    HashDictionary(HashFunc func = nullptr, size_t startCapacity = 8, size_t shrinkAt = 4, size_t factor = 2)
        : hashFunc(func), count(0), shrinkAt(shrinkAt), factor(factor) {
        if(startCapacity == 0 || factor < 2 || shrinkAt < factor) {
            throw std::invalid_argument("Wrong hash table parameters");
        }
        if(!hashFunc) {
            hashFunc = [](const Key& key) { return GetHashCode(key); };
        }
        Resize(startCapacity);
    }

    size_t GetCount() const override { return count; }

    size_t GetCapacity() const override { return buckets.GetLength(); }

    bool ContainsKey(const Key& key) const override {
        return const_cast<HashDictionary*>(this)->FindEntry(key) != nullptr;
    }

    Value Get(const Key& key) const override {
        Entry* entry = const_cast<HashDictionary*>(this)->FindEntry(key);
        if(entry == nullptr) {
            throw std::out_of_range("Key is not found in the dictionary");
        }
        return entry->value;
    }

    Value* Find(const Key& key) override {
        Entry* entry = FindEntry(key);
        if(entry == nullptr) {
            return nullptr;
        }
        return &entry->value;
    }

    void Add(const Key& key, const Value& element) override {
        Entry* entry = FindEntry(key);
        if(entry != nullptr) {
            entry->value = element;
            return;
        }
        if(count == buckets.GetLength()) {
            Resize(buckets.GetLength() * factor);
        }
        buckets[IndexOf(key, buckets.GetLength())].Append(Entry{key, element});
        count++;
    }

    void Remove(const Key& key) override {
        Bucket& bucket = buckets[IndexOf(key, buckets.GetLength())];
        for(size_t i = 0; i < bucket.GetLength(); i++) {
            if(bucket[i].key == key) {
                bucket.RemoveAt(i);
                count--;
                size_t capacity = buckets.GetLength();
                if(count <= capacity / shrinkAt && capacity / factor >= 1) {
                    Resize(capacity / factor);
                }
                return;
            }
        }
        throw std::out_of_range("Key is not found");
    }

    void Clear() override {
        for(size_t i = 0; i < buckets.GetLength(); i++) { buckets[i].Clear(); }
        count = 0;
    }

    size_t GetLongestChain() const {
        size_t longest = 0;
        for(size_t i = 0; i < buckets.GetLength(); i++) {
            if(buckets[i].GetLength() > longest) {
                longest = buckets[i].GetLength();
            }
        }
        return longest;
    }
};
