#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>

#include "i_dictionary.hpp"

template <typename Key, typename Value>
class Node {
   public:
    Key key;
    Value value;
    Node<Key, Value>* left;
    Node<Key, Value>* right;

    Node(const Key& k, const Value& val) : key(k), value(val), left(nullptr), right(nullptr) {}
};

template <typename Key, typename Value>
class TreeDictionary : public IDictionary<Key, Value> {
   private:
    using NodePtr = Node<Key, Value>*;

    NodePtr root;

    void ClearRec(NodePtr node) {
        if(node == nullptr) {
            return;
        }
        ClearRec(node->left);
        ClearRec(node->right);
        delete node;
    }

    NodePtr CloneRec(const NodePtr node) const {
        if(node == nullptr) {
            return nullptr;
        }
        NodePtr copy = new Node<Key, Value>(node->key, node->value);
        copy->left = CloneRec(node->left);
        copy->right = CloneRec(node->right);
        return copy;
    }

    NodePtr AddRec(NodePtr node, const Key& key, const Value& element) {
        if(node == nullptr) {
            return new Node<Key, Value>(key, element);
        }

        if(key < node->key) {
            node->left = AddRec(node->left, key, element);
        } else if(key > node->key) {
            node->right = AddRec(node->right, key, element);
        } else {
            node->value = element;
        }
        return node;
    }

    NodePtr GetMinNode(NodePtr node) const {
        while(node->left != nullptr) { node = node->left; }
        return node;
    }

    NodePtr RemoveRec(NodePtr node, const Key& key) {
        if(node == nullptr) {
            return nullptr;
        }
        if(key < node->key) {
            node->left = RemoveRec(node->left, key);
        } else if(key > node->key) {
            node->right = RemoveRec(node->right, key);
        } else {
            if(node->left == nullptr) {
                NodePtr temp = node->right;
                delete node;
                return temp;
            }
            if(node->right == nullptr) {
                NodePtr temp = node->left;
                delete node;
                return temp;
            }
            NodePtr replacement = GetMinNode(node->right);
            Key minKey = replacement->key;
            node->key = minKey;
            node->value = replacement->value;
            node->right = RemoveRec(node->right, minKey);
        }
        return node;
    }

    NodePtr FindNodeRec(NodePtr node, const Key& key) const {
        if(node == nullptr) {
            return nullptr;
        }
        if(key < node->key) {
            return FindNodeRec(node->left, key);
        }
        if(key > node->key) {
            return FindNodeRec(node->right, key);
        }
        return node;
    }

    size_t GetCountRec(const NodePtr node) const {
        if(node == nullptr) {
            return 0;
        }
        return GetCountRec(node->left) + GetCountRec(node->right) + 1;
    }

    template <typename NewValue>
    void MapRec(const NodePtr node, const std::function<NewValue(Value)>& func,
                TreeDictionary<Key, NewValue>& newTree) const {
        if(node == nullptr) {
            return;
        }
        newTree.Add(node->key, func(node->value));
        MapRec(node->left, func, newTree);
        MapRec(node->right, func, newTree);
    }

    void WhereRec(const NodePtr node, const std::function<bool(Value)>& predicat,
                  TreeDictionary<Key, Value>& newTree) const {
        if(node == nullptr) {
            return;
        }
        if(predicat(node->value)) {
            newTree.Add(node->key, node->value);
        }
        WhereRec(node->left, predicat, newTree);
        WhereRec(node->right, predicat, newTree);
    }

    template <typename NewValue>
    void ReduceRec(const NodePtr node, const std::function<NewValue(Value, NewValue)>& func,
                   NewValue& acc) const {
        if(node == nullptr) {
            return;
        }
        ReduceRec(node->left, func, acc);
        acc = func(node->value, acc);
        ReduceRec(node->right, func, acc);
    }

    void CopyRec(const NodePtr node, TreeDictionary<Key, Value>& newTree) const {
        if(node == nullptr) {
            return;
        }
        newTree.Add(node->key, node->value);
        CopyRec(node->left, newTree);
        CopyRec(node->right, newTree);
    }

    bool SameTreeRec(const NodePtr a, const NodePtr b) const {
        if(a == nullptr && b == nullptr) {
            return true;
        }
        if(a == nullptr || b == nullptr) {
            return false;
        }
        if(a->key < b->key || b->key < a->key) {
            return false;
        }
        if(a->value != b->value) {
            return false;
        }
        return SameTreeRec(a->left, b->left) && SameTreeRec(a->right, b->right);
    }

   public:
    TreeDictionary() : root(nullptr) {}
    ~TreeDictionary() { ClearRec(root); }

    TreeDictionary(const TreeDictionary& other) : root(CloneRec(other.root)) {}
    TreeDictionary(TreeDictionary&& other) noexcept : root(other.root) { other.root = nullptr; }
    TreeDictionary& operator=(const TreeDictionary&) = delete;
    TreeDictionary& operator=(TreeDictionary&&) = delete;

    void Add(const Key& key, const Value& element) override { root = AddRec(root, key, element); }

    void Remove(const Key& key) override {
        if(!ContainsKey(key)) {
            throw std::out_of_range("Key is not found");
        }
        root = RemoveRec(root, key);
    }

    bool ContainsKey(const Key& key) const override { return FindNodeRec(root, key) != nullptr; }

    size_t GetCount() const override { return GetCountRec(root); }

    size_t GetCapacity() const override { return GetCount(); }

    Value Get(const Key& key) const override {
        NodePtr node = FindNodeRec(root, key);
        if(node == nullptr) {
            throw std::out_of_range("Key is not found in the dictionary");
        }
        return node->value;
    }

    Value* Find(const Key& key) override {
        NodePtr node = FindNodeRec(root, key);
        if(node == nullptr) {
            return nullptr;
        }
        return &node->value;
    }

    void Clear() override {
        ClearRec(root);
        root = nullptr;
    }

    template <typename NewValue>
    TreeDictionary<Key, NewValue> Map(const std::function<NewValue(Value)>& func) const {
        TreeDictionary<Key, NewValue> newTree;
        MapRec(root, func, newTree);
        return newTree;
    }

    TreeDictionary<Key, Value> Where(const std::function<bool(Value)>& predicat) const {
        TreeDictionary<Key, Value> newTree;
        WhereRec(root, predicat, newTree);
        return newTree;
    }

    template <typename NewValue>
    NewValue Reduce(const std::function<NewValue(Value, NewValue)>& func, NewValue start) const {
        NewValue acc = start;
        ReduceRec(root, func, acc);
        return acc;
    }

    TreeDictionary<Key, Value> ExtractSubtree(const Key& startKey) const {
        NodePtr startNode = FindNodeRec(root, startKey);
        if(startNode == nullptr) {
            throw std::invalid_argument("Node is not found");
        }
        TreeDictionary<Key, Value> newTree;
        CopyRec(startNode, newTree);
        return newTree;
    }

    bool ContainsSubtree(const TreeDictionary<Key, Value>& subTree) const {
        if(subTree.root == nullptr) {
            return true;
        }
        const NodePtr node = FindNodeRec(root, subTree.root->key);
        if(node == nullptr) {
            return false;
        }
        return SameTreeRec(node, subTree.root);
    }

    const Node<Key, Value>* GetRoot() const { return root; }
};
