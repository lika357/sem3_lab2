#pragma once

#include <cstddef>

template <typename Key, typename Value>
class IDictionary {
   public:
    virtual ~IDictionary() = default;

    virtual size_t GetCount() const = 0;
    virtual size_t GetCapacity() const = 0;
    virtual bool ContainsKey(const Key& key) const = 0;
    virtual Value Get(const Key& key) const = 0;
    virtual Value* Find(const Key& key) = 0;
    virtual void Add(const Key& key, const Value& element) = 0;
    virtual void Remove(const Key& key) = 0;
    virtual void Clear() = 0;
};
