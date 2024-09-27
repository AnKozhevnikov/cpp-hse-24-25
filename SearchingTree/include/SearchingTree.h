#pragma once

#include <iostream>
#include <algorithm>
#include <utility>
#include <stdio.h>
#include <random>
#include <stdexcept>
#include <memory>
#include <stack>

// Class of searching tree. which can do such basic things, as std::map
// It is implemented using treap, which is a binary search tree with random priorities
// Algorithm taken from https://neerc.ifmo.ru/wiki/index.php?title=Декартово_дерево
template <class K, class V>
class SearchingTree
{
private:
    // Node of treap, which contains key, value, priority, pointers to left and right children and parent
    //  key - key of the node
    //  value - value of the node
    //  priority - random priority of the node
    //  left - pointer to the left child, null if there is no left child
    //  right - pointer to the right child, null if there is no right child
    class Node
    {
    public:
        K key;
        V value;

        Node() = default;

    private:
        int priority;
        std::shared_ptr<Node> left, right;

        friend class SearchingTree;
    };

    // root - pointer to the root of the treap
    // leftest - pointer to the leftest node of the treap, used for begin()
    // rightest - pointer to the rightest node of the treap, used for prev() for Iterator::end()
    // reserved - pointer to the reserved node, which is used for end(), if insert() is called, reserved node becomes the new node, new reserved is made
    std::shared_ptr<Node> root, leftest, rightest, reserved;
    std::mt19937 engine;

    // split the treap into two treaps: first contains all nodes with keys less than k, second contains all nodes with keys greater than k
    std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> split(std::shared_ptr<Node> v, int k);

    // merge two treaps into one, keys of the left one are obligated to be less than keys of the right one
    std::shared_ptr<Node> merge(std::shared_ptr<Node> l, std::shared_ptr<Node> r);

    // update leftest and rightest pointers
    void updateBounds();

public:
    // Iterator for SearchingTree
    class Iterator
    {
    public:
        Iterator(const std::shared_ptr<Node> &n, const SearchingTree *st);

        Iterator &operator=(const Iterator &other);

        Iterator(const Iterator &other);

        bool operator==(const Iterator &it) const;

        bool operator!=(const Iterator &it) const;

        Iterator &operator++();

        Iterator operator++(int);

        Iterator &operator--();

        Iterator operator--(int);

        std::pair<const K &, V &> operator*();

        std::shared_ptr<std::pair<const K &, V &>> operator->();

    private:
        // v - pointer to the current node
        // p - shared pointer to the pair of key and value of the current node(made exactly shared pointer for the case of copying)
        // myTree - pointer to the SearchingTree, which contains the current node(made to know about reserved node)
        // parents - stack of parents, needed for going up in tree
        std::weak_ptr<Node> v;
        std::shared_ptr<std::pair<const K &, V &>> p;
        // std::shared_ptr<const SearchingTree> myTree;
        const SearchingTree *myTree;
        std::stack<std::weak_ptr<Node>> parents;

        // update pair of key and value, used after changing the current node, made outer to avoid code duplication
        void updatePair();

        friend class SearchingTree;
    };

    SearchingTree();

    void recursiveCopy(const std::shared_ptr<Node> &v, std::shared_ptr<Node> &v2);

    SearchingTree(const SearchingTree &st);

    SearchingTree &operator=(SearchingTree &&st);

    SearchingTree &operator=(const SearchingTree &st);

    SearchingTree(SearchingTree &&st);

    // insert key-value pair into the treap, if the key is already in the treap, do nothing
    void insert(const K &key, const V &value);

    // erase key-value pair from the treap, if the key is not in the treap, do nothing
    void erase(const K &key);

    // find key-value pair in the treap, return iterator, if the key is not in the treap, return end()
    SearchingTree::Iterator find(const K &key) const;

    // find the iterator to the first element with key not less than the given key, if there is no such element, return end()
    SearchingTree::Iterator lower_bound(const K &key) const;

    // find the iterator to the first element with key greater than the given key, if there is no such element, return end()
    SearchingTree::Iterator upper_bound(const K &key) const;

    // return the iterator to the first element of the treap, end() if the treap is empty
    SearchingTree::Iterator begin() const;

    // return the iterator, which is the next to the last element of the treap, end() if the treap is empty
    SearchingTree::Iterator end() const;

    // return vector of pairs of keys and values, which keys are in the range [l, r)
    std::vector<std::pair<const K&, V&>> range(K l, K r) const;
};

template <class K, class V>
std::pair<std::shared_ptr<typename SearchingTree<K, V>::Node>, std::shared_ptr<typename SearchingTree<K, V>::Node>>
SearchingTree<K, V>::split(std::shared_ptr<typename SearchingTree<K, V>::Node> v, int k)
{
    if (v == nullptr)
    {
        return std::make_pair(nullptr, nullptr);
    }
    else if (k > v->key)
    {
        std::pair<std::shared_ptr<typename SearchingTree<K, V>::Node>, std::shared_ptr<typename SearchingTree<K, V>::Node>> p = split(v->right, k);
        v->right = p.first;
        return std::make_pair(v, p.second);
    }
    else
    {
        std::pair<std::shared_ptr<typename SearchingTree<K, V>::Node>, std::shared_ptr<typename SearchingTree<K, V>::Node>> p = split(v->left, k);
        v->left = p.second;
        return std::make_pair(p.first, v);
    }
}

template <class K, class V>
std::shared_ptr<typename SearchingTree<K, V>::Node> SearchingTree<K, V>::merge(std::shared_ptr<typename SearchingTree<K, V>::Node> l, std::shared_ptr<typename SearchingTree<K, V>::Node> r)
{
    if (r == nullptr)
        return l;
    else if (l == nullptr)
        return r;
    else if (l->priority > r->priority)
    {
        l->right = merge(l->right, r);
        return l;
    }
    else
    {
        r->left = merge(l, r->left);
        return r;
    }
}

template <class K, class V>
void SearchingTree<K, V>::updateBounds()
{
    if (root == nullptr)
    {
        rightest = nullptr;
        leftest = nullptr;
        return;
    }

    std::shared_ptr<Node> v = root;
    while (v->left != nullptr)
    {
        v = v->left;
    }
    leftest = v;
    v = root;
    while (v->right != nullptr)
    {
        v = v->right;
    }
    rightest = v;
}

template <class K, class V>
SearchingTree<K, V>::Iterator::Iterator(const std::shared_ptr<Node> &n, const SearchingTree *st) : v(n), myTree(st)
{
    p = std::make_shared<std::pair<const K &, V &>>(std::cref(n->key), std::ref(n->value));

    if (n == myTree->reserved)
        return;
    std::shared_ptr<Node> cur = myTree->root;
    while (cur != n)
    {
        parents.push(cur);
        if (n->key < cur->key)
            cur = cur->left;
        else
            cur = cur->right;
    }
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator& SearchingTree<K, V>::Iterator::operator=(const Iterator &other)
{
    if (this != &other)
    {
        v = other.v;
        myTree = other.myTree;
        parents = other.parents;
        updatePair();
    }
    return *this;
}

template <class K, class V>
SearchingTree<K, V>::Iterator::Iterator(const Iterator &other)
{
    v = other.v;
    myTree = other.myTree;
    parents = other.parents;
    updatePair();
}

template <class K, class V>
bool SearchingTree<K, V>::Iterator::operator==(const Iterator &it) const
{
    return v.lock() == it.v.lock();
}

template <class K, class V>
bool SearchingTree<K, V>::Iterator::operator!=(const Iterator &it) const
{
    return !(*this == it);
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator& SearchingTree<K, V>::Iterator::operator++()
{
    std::shared_ptr<Node> cur = v.lock();

    if (cur == myTree->reserved)
        throw std::range_error("Iterator out of bounds");
    if (cur->right == nullptr)
    {
        while (true)
        {
            std::shared_ptr<Node> prev = cur;
            cur = parents.top().lock();
            parents.pop();
            if (cur == nullptr)
            {
                cur = myTree->reserved;
                break;
            }
            if (cur->left == prev)
                break;
        }
    }
    else
    {
        parents.push(cur);
        cur = cur->right;
        while (cur->left != nullptr)
            cur = cur->left;
    }
    v = cur;
    updatePair();
    return *this;
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::Iterator::operator++(int)
{
    Iterator temp = *this;
    ++(*this);
    return temp;
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator& SearchingTree<K, V>::Iterator::operator--()
{
    std::weak_ptr<Node> cur = v;

    if (cur == myTree->reserved)
        return Iterator(rightest, myTree);
    if (cur->left == nullptr)
    {
        while (true)
        {
            std::weak_ptr<Node> prev = cur;
            cur = parents.top().lock();
            parents.pop();
            if (cur == nullptr)
                throw std::range_error("Iterator out of bounds");
            if (cur->right == prev)
                break;
        }
    }
    else
    {
        parents.push(cur);
        cur = cur->left;
        while (cur->right != nullptr)
            cur = cur->right;
    }
    v = cur;
    updatePair();
    return *this;
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::Iterator::operator--(int)
{
    Iterator temp = *this;
    --(*this);
    return temp;
}

template <class K, class V>
std::pair<const K &, V &> SearchingTree<K, V>::Iterator::operator*()
{
    return *p;
}

template <class K, class V>
std::shared_ptr<std::pair<const K &, V &>> SearchingTree<K, V>::Iterator::operator->()
{
    return p;
}

template <class K, class V>
void SearchingTree<K, V>::Iterator::updatePair()
{
    std::shared_ptr<Node> cur = v.lock();
    p = std::make_shared<std::pair<const K &, V &>>(std::cref(cur->key), std::ref(cur->value));
}

template <class K, class V>
SearchingTree<K, V>::SearchingTree()
{
    root = nullptr;
    leftest = nullptr;
    rightest = nullptr;
    reserved = std::make_shared<Node>();
    engine = std::mt19937(std::random_device()());
}

template <class K, class V>
void SearchingTree<K, V>::recursiveCopy(const std::shared_ptr<Node> &v, std::shared_ptr<Node> &v2)
{
    if (v == nullptr)
        return;
    v2 = std::make_shared<Node>();
    v2->key = v->key;
    v2->value = v->value;
    v2->priority = v->priority;
    recursiveCopy(v->left, v2->left);
    recursiveCopy(v->right, v2->right);
    if (v2->left != nullptr)
        v2->left->parent = v2;
    if (v2->right != nullptr)
        v2->right->parent = v2;
}

template <class K, class V>
SearchingTree<K, V>::SearchingTree(const SearchingTree &st)
{
    recursiveCopy(st.root, root);
    recursiveCopy(st.reserved, reserved);
    updateBounds();
}

template <class K, class V>
SearchingTree<K, V>& SearchingTree<K, V>::operator=(SearchingTree &&st)
{
    if (this != &st)
    {
        root = st.root;
        leftest = st.leftest;
        rightest = st.rightest;
        reserved = st.reserved;
        engine = st.engine;
    }
    return *this;
}

template <class K, class V>
SearchingTree<K, V>& SearchingTree<K, V>::operator=(const SearchingTree &st)
{
    if (this != &st)
    {
        recursiveCopy(st.root, root);
        updateBounds();
        recursiveCopy(st.reserved, reserved);
    }
    return *this;
}

template <class K, class V>
SearchingTree<K, V>::SearchingTree(SearchingTree &&st)
{
    root = st.root;
    leftest = st.leftest;
    rightest = st.rightest;
    reserved = st.reserved;
    engine = st.engine;
}

template <class K, class V>
void SearchingTree<K, V>::insert(const K &key, const V &value)
{
    if (find(key) != SearchingTree::end())
        return;

    std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> p = split(root, key);
    std::shared_ptr<Node> newNode = reserved;
    newNode->key = key;
    newNode->value = value;
    newNode->priority = engine();

    reserved = std::make_shared<Node>();

    p.first = merge(p.first, newNode);
    p.first = merge(p.first, p.second);
    root = p.first;
    updateBounds();
}

template <class K, class V>
void SearchingTree<K, V>::erase(const K &key)
{
    if (find(key) == SearchingTree::end())
        return;

    std::shared_ptr<Node> v = root;
    while (v->key != key)
    {
        if (key < v->key)
            v = v->left;
        else
            v = v->right;
    }

    std::shared_ptr<Node> merged = merge(v->left, v->right);

    updateBounds();
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::find(const K &key) const
{
    if (root == nullptr)
        return Iterator(reserved, this);
    std::shared_ptr<Node> cur = root;
    while (cur->key != key)
    {
        if (key < cur->key)
            cur = cur->left;
        else
            cur = cur->right;
        if (cur == nullptr)
            return Iterator(reserved, this);
    }
    return Iterator(cur, this);
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::lower_bound(const K &key) const
{
    if (root == nullptr)
        return Iterator(SearchingTree::reserved, this);
    std::shared_ptr<Node> cur = root;
    while (cur->key != key)
    {
        if (key < cur->key)
        {
            if (cur->left != nullptr)
                cur = cur->left;
            else
                return Iterator(cur, this);
        }
        else
        {
            if (cur->right != nullptr)
                cur = cur->right;
            else
            {
                Iterator it = Iterator(cur, this);
                ++it;
                return it;
            }
        }
    }
    return Iterator(cur, this);
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::upper_bound(const K &key) const
{
    Iterator it = lower_bound(key);
    if (it == SearchingTree::reserved || it->first != key)
        return it;
    else
        return it->next;
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::begin() const
{
    if (root != nullptr)
        return Iterator(leftest, this);
    return end();
}

template <class K, class V>
typename SearchingTree<K, V>::Iterator SearchingTree<K, V>::end() const
{
    return Iterator(SearchingTree::reserved, this);
}

template <class K, class V>
std::vector<std::pair<const K&, V&>> SearchingTree<K, V>::range(K l, K r) const
{
    std::vector<std::pair<const K&, V&>> ret;
    Iterator it = lower_bound(l);
    while (it != end() && it->first < r)
    {
        ret.push_back(*it);
        ++it;
    }
    ret.shrink_to_fit();
    return ret;
}