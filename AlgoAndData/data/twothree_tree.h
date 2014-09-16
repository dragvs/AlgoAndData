//
//  twothree_tree.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 01/08/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_data_twothree_tree_h
#define AlgoAndData_data_twothree_tree_h

#include <utility>
#include <memory>
#include <cassert>

namespace lab {
	
    //
    // 2-3 Tree
    //
    // Search: O(log N)
    // Insert: O(log N)
    // Delete: O(log N)
    // Space: O(n)
    //
    
    template<typename Value, typename Compare = std::less<Value>>
    class Node {
    private:
        using Node_type = Node<Value, Compare>;
        
    public:
        using value_type = Value;
    
        Node() : parent(nullptr), childsCount(0), valuesCount(0) {
            resetChilds();
        }
        
        Node(const value_type& value) : parent(nullptr), childsCount(0) {
            valuesArr[0] = value;
            valuesCount = 1;
            resetChilds();
        }
        
        /// Getters & other utility checks
        
        Node_type* getParent() const noexcept {
            return parent;
        }
        
        bool isConsistent() const {
            assert(valuesCount <= MaxValuesCount);
            return valuesCount < MaxValuesCount;
        }
        
        int getValuesCount() const noexcept {
            return valuesCount;
        }

        const value_type& getValue(int valueIdx) const {
            assert(valueIdx >= 0 && valueIdx < MaxValuesCount);
            return valuesArr[valueIdx];
        }
        value_type& getValue(int valueIdx) {
            assert(valueIdx >= 0 && valueIdx < MaxValuesCount);
            return valuesArr[valueIdx];
        }
        
        int getValueIdx(const value_type& value) const {
            if (valuesCount == 0)
                return -1;
            
            if (compare(value, valuesArr[0]) || compare(valuesArr[valuesCount-1], value))
                return -1;
            
            for (int i = 0; i < valuesCount; ++i) {
                if (isEquivalent(value, valuesArr[i]))
                    return i;
            }
            
            return -1;
        }
        
        bool containsValue(const value_type& value) const {
            return getValueIdx(value) != -1;
        }
        
        Node* getChild(int childIdx) const noexcept {
            assert(childIdx >= 0);
            
            if (childIdx > MaxValuesCount)
                return nullptr;
            
            return childsArr[childIdx];
        }
        
        int getChildIdx(Node* child) const noexcept {
            for (int i = 0; i <= MaxValuesCount; ++i) {
                if (childsArr[i] == child)
                    return i;
            }
            return -1;
        }
        
        Node_type* chooseChild(const value_type& value) const {
            arrSize_t idx = chooseChildIdx(value);
            
            if (idx != -1 && idx < childsCount)
                return childsArr[idx];
            
            return nullptr;
        }
        
        Node_type* getLeftmostChild() const {
            Node_type* node = const_cast<Node_type*>(this);
            Node_type* firstChild = node->getChild(0);
            
            while (firstChild != nullptr) {
                node = firstChild;
                firstChild = node->getChild(0);
            }
            
            return node;
        }
        
        int getChildrenCount() const {
            return childsCount;
        }
        
        bool isLeafNode() const {
            return childsCount == 0;
        }
        
        bool isTwoNode() const {
            return valuesCount == 1;
        }
        
        bool isThreeNode() const {
            return valuesCount == 2;
        }
        
        /// Modifiers
        
        // Move complex operation to the tree?
        // - splitIf4Node
        // - mergeWithChild
        
        //
        // Determines index of the value to insert in the node, shifts existing values to the right
        // Returns index of the inserted value in the node
        //
        int insertValue(const value_type& value) {
            assert(valuesCount < MaxValuesCount);
            arrSize_t newValueIdx = 0;
            
            if (valuesCount > 0) {
                newValueIdx = chooseChildIdx(value);
                assert(newValueIdx != -1);
                // Shifting values to insert the new value
                for (int i = valuesCount; i > newValueIdx; --i) {
                    valuesArr[i] = std::move(valuesArr[i-1]);
                }
            }
            
            valuesArr[newValueIdx] = value;
            ++valuesCount;
            return newValueIdx;
        }
        
        // remove+insert optimization
        value_type replaceValue(const value_type& newValue, int atIndex) {
            assert(atIndex >= 0 && atIndex < valuesCount);
            
            value_type lastValue = valuesArr[atIndex];
            valuesArr[atIndex] = newValue;
            return lastValue;
        }
        
        //
        // Removes values from 2-/3-nodes with shifting remaining values to the left
        // Returns removed value
        //
        value_type removeValueAtIdx(int valueIdx) {
            assert(valuesCount > 0 && valuesCount < MaxValuesCount); // from 2-/3-nodes only
            assert(valueIdx == 0 || valueIdx == 1);

            value_type removedValue = valuesArr[valueIdx];
            
            for (int i = valueIdx; i < valuesCount-1; ++i) {
                valuesArr[i] = std::move(valuesArr[i+1]);
            }
            --valuesCount;
            return removedValue;
        }
        
        //
        // Inserts a child at a specified index, shifts existing children to the right
        //
        void insertChild(Node_type* child, int atIndex) {
            assert(child != nullptr);
            assert(childsCount < MaxValuesCount+1);
            
            // Shifting childs to insert the new child
            for (int i = childsCount; i > atIndex; --i) {
                childsArr[i] = childsArr[i-1];
            }
            
            childsArr[atIndex] = child;
            child->parent = this;
            ++childsCount;
        }
        
        //
        // Removes a child from 2-/3-nodes with shifting remaining children to the left
        // Returns pointer to the removed child
        //
        void removeChild(Node_type* child, bool withShift = true) {
            assert(child != nullptr);
            
            int childIdx = getChildIdx(child);
            removeChild(childIdx, withShift);
        }
        Node_type* removeChild(int childIdx, bool withShift = true) {
            assert(childIdx >= 0 && childIdx < MaxValuesCount);
            
            // Should we shift remaining child pointers to the left?
            Node_type* childPtr = childsArr[childIdx];
            if (childPtr != nullptr) {
                childPtr->parent = nullptr;
                childsArr[childIdx] = nullptr;
                --childsCount;
            }
            if (withShift) {
                for (int i = childIdx; i < childsCount; ++i) {
                    childsArr[i] = childsArr[i+1];
                }
                childsArr[childsCount] = nullptr;
            }
            
            return childPtr;
        }
        
        // Comlpex operation, purpose: insertion fix
        void splitIf4Node(Node* newLeftChild, Node* newRightChild) {
            if (isConsistent()) // Only for 4-nodes
                return;
            
            // left child
            newLeftChild->insertValue(valuesArr[0]);
            if (childsCount >= 1)
                newLeftChild->insertChild(childsArr[0], 0);
            if (childsCount >= 2)
                newLeftChild->insertChild(childsArr[1], 1);
            
            // right child
            newRightChild->insertValue(valuesArr[2]);
            if (childsCount >= 3)
                newRightChild->insertChild(childsArr[2], 0);
            if (childsCount == 4)
                newRightChild->insertChild(childsArr[3], 1);
            
            valuesArr[0] = std::move(valuesArr[1]);
            valuesCount = 1;
            resetChilds();
            insertChild(newLeftChild, 0);
            insertChild(newRightChild, 1);
        }
        
        // Comlpex operation, purpose: insertion fix
        void mergeWithChild(Node* child) {
            assert(child->valuesCount == 1); // Only for 2-nodes
            assert(child->childsCount == 2); // with two childs

            int childIdx = chooseChildIdx(child->valuesArr[0]);
            assert(childsArr[childIdx] == child);
            
            insertValue(child->valuesArr[0]);
            setChildAt(child->childsArr[0], childIdx); // remove+insert optimization
            insertChild(child->childsArr[1], childIdx+1);

            child->reset();
        }
        
    private:
        using arrSize_t = int;
        static const arrSize_t MaxValuesCount = 3;
        
        Compare compare;
        value_type valuesArr[MaxValuesCount];
        arrSize_t valuesCount;
        
        Node_type* childsArr[MaxValuesCount+1];
        arrSize_t childsCount;
        
        Node_type* parent;
        
        bool isEquivalent(const value_type& v1, const value_type& v2) const {
            return !compare(v1, v2) && !compare(v2, v1);
        }
        
        arrSize_t chooseChildIdx(const value_type& value) const {
            for (int i = 0; i < valuesCount; ++i) {
                if (compare(value, valuesArr[i]))
                    return i;

                if (i > 0 && compare(valuesArr[i-1], value) && compare(value, valuesArr[i])) // redundant?
                    return i;
                
                if (i == valuesCount-1)
                    return i+1;
            }
            return -1;
        }
        
        void setChildAt(Node* child, int childIdx) noexcept {
            if (childsArr[childIdx] != nullptr)
                childsArr[childIdx]->parent = nullptr;
            
            childsArr[childIdx] = child;
            child->parent = this;
        }
        
        void reset() noexcept {
            valuesCount = 0;
            parent = nullptr;
            resetChilds();
        }
        
        void resetChilds() noexcept {
            childsArr[0] = childsArr[1] = childsArr[2] = childsArr[3] = nullptr;
            childsCount = 0;
        }
    };
    
    ///// Iterators
    /// Base class for node iterators.
    template<typename Value, typename node_type>
    struct Node_iterator_base;
    template<typename Value, typename node_type>
    bool operator!=(const Node_iterator_base<Value, node_type>& x,
                    const Node_iterator_base<Value, node_type>& y);
    template<typename Value, typename node_type>
    bool operator==(const Node_iterator_base<Value, node_type>& x,
                    const Node_iterator_base<Value, node_type>& y);
    
    template<typename Value, typename node_type>
    struct Node_iterator_base
    {
        Value& operator*() const {
            return current->getValue(valueIdx);
        }
        
        Value* operator->() const {
            return std::addressof(current->getValue(valueIdx));
        }
        
        int valueIdx;
        node_type* current;
        
    protected:
        Node_iterator_base(node_type* curNode, int valueIdx)
        : current(curNode), valueIdx(valueIdx) {}
        
        void incr() {
            if (current == nullptr)
                return;
            
            // Depth-first in-order (sorted) tree traversal
            while (true) {
                int nextChildIdx = valueIdx + 1;
                auto nextChild = current->getChild(nextChildIdx);
                
                if (nextChild) {
                    // Moving down through childs
                    current = nextChild->getLeftmostChild();
                    valueIdx = 0;
                    assert(current->getValuesCount() > 0);
                    break;
                } else {
                    // No child: use next value or search back at parent
                    ++valueIdx;
                    
                    if (valueIdx < current->getValuesCount()) {
                        // Using next value
                        break;
                    } else {
                        // Moving up (searching in parents)
                        auto parent = current->getParent();
                        
                        if (parent == nullptr) {
                            // Tree traverse finished
                            valueIdx = 0;
                            current = nullptr;
                            break;
                        }
                        
                        int idxInParent = parent->getChildIdx(current);
                        assert(idxInParent >= 0 && idxInParent <= 2);
                        valueIdx = idxInParent;
                        current = parent;
                        
                        if (idxInParent == parent->getValuesCount()) {
                            continue;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
        
    private:
        friend bool operator==<Value, node_type>(const Node_iterator_base& x,
                                                 const Node_iterator_base& y);
        friend bool operator!=<Value, node_type>(const Node_iterator_base& x,
                                                 const Node_iterator_base& y);
    };
    
    template<typename Value, typename node_type>
    inline bool
    operator==(const Node_iterator_base<Value, node_type>& x,
               const Node_iterator_base<Value, node_type>& y)
    { return x.current == y.current && x.valueIdx == y.valueIdx; }
    
    template<typename Value, typename node_type>
    inline bool
    operator!=(const Node_iterator_base<Value, node_type>& x,
               const Node_iterator_base<Value, node_type>& y)
    { return x.current != y.current || x.valueIdx != y.valueIdx; }
    
    /// Node iterators, used to iterate through all the 2-3 three.
    template<typename Value, typename node_type>
    struct Node_iterator : public Node_iterator_base<Value, node_type>
    {
    private:
        using base_type = Node_iterator_base<Value, node_type>;
        
    public:
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        
        using pointer = Value*;
        using reference = Value&;
        
        Node_iterator(node_type* curNode, int valueIdx) :
            base_type(curNode, valueIdx) {}
        
        Node_iterator&
        operator++() {
            this->incr();
            return *this;
        }
        
        Node_iterator
        operator++(int) {
            Node_iterator tmp(*this);
            this->incr();
            return tmp;
        }
    };
    
    template<typename Value, typename node_type>
    struct Node_const_iterator : public Node_iterator_base<Value, node_type>
    {
    private:
        using base_type = Node_iterator_base<Value, node_type>;
        
    public:
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        
        using pointer = const Value*;
        using reference = const Value&;
        
        Node_const_iterator(node_type* curNode, int valueIdx) :
            base_type(curNode, valueIdx) {}
        
        Node_const_iterator(const Node_iterator<Value, node_type>& other) :
            base_type(other.current, other.valueIdx)
        {}
        
        reference
        operator*() const {
            return base_type::operator*();
        }
        
        pointer
        operator->() const {
            return base_type::operator->();
        }
        
        Node_const_iterator&
        operator++() {
            this->incr();
            return *this;
        }
        
        Node_const_iterator
        operator++(int) {
            Node_const_iterator tmp(*this);
            this->incr();
            return tmp;
        }
    };
    ///// Iterators
    
    template<
        typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator< Key >
    >
    class twothree_tree {
    private:
        using Node_type = Node<Key, Compare>;
        using Node_allocator_type = typename Allocator::template rebind<Node_type>::other;
        
    public:
        using key_type = Key;
        using value_type = Key;
        using size_type = std::size_t;
        
        using iterator = Node_iterator<value_type, Node_type>;
        using const_iterator = Node_const_iterator<value_type, Node_type>;
        
        // .ctors
        
        twothree_tree() : rootNode(nullptr) {
            
        }
        ~twothree_tree() {
            clear();
        }
        
        // Lookup
        
        iterator find(const value_type& value) {
            Node_type* node = findNode(value);
            
            if (!node)
                return iterator{nullptr, 0};
            
            int valueIdx = node->getValueIdx(value);
            if (valueIdx == -1)
                return iterator{nullptr, 0};
            
            return iterator{node, valueIdx};
        }
        const_iterator find(const value_type& value) const {
            Node_type* node = findNode(value);
            
            if (!node)
                return const_iterator{nullptr, 0};
            
            int valueIdx = node->getValueIdx(value);
            if (valueIdx == -1)
                return const_iterator{nullptr, 0};
            
            return const_iterator{node, valueIdx};
        }
        
        // Modifiers
        
        std::pair<iterator,bool> insert(const value_type& value) {
            return insertImpl(value);
        }
        
        template< class InputIt >
        void insert(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                insertImpl(*first);
            }
        }
        
        void insert(std::initializer_list<value_type> ilist) {
            insert(ilist.begin(), ilist.end());
        }

        // TODO Return on Iterator following the last removed element
        void erase(const_iterator pos) {
            eraseImpl(pos);
        }

        // Returns: Number of elements removed.
        size_type erase(const value_type& value) {
            const_iterator pos = find(value);
            
            if (pos == end()) {
                // An element with provided value isn't found to erase
                return 0;
            }
            
            eraseImpl(pos);
            return 1;
        }
        
        void clear() noexcept {
            // Depth-first post-order tree traversal
            Node_type* curNode = rootNode;
            
            while (curNode) {
                // Checking node's children from left to right
                if (curNode->getChildrenCount() > 0) {
                    Node_type* child = nullptr;
                    
                    for (int i = 0; i < 4; ++i) {
                        if ((child = curNode->getChild(i)) != nullptr)
                            break;
                    }
                    
                    if (child != nullptr) {
                        curNode = child;
                        continue;
                    }
                }
                
                // The node is a leaf node, deleting it
                Node_type* parent = curNode->getParent();
                
                if (parent == nullptr) {
                    assert(curNode == rootNode); // Only root doesn't have a parent
                } else {
                    parent->removeChild(curNode, false);
                }
                deallocateNode(curNode);
                curNode = parent;
            }
            
            rootNode = nullptr;
        }
        
        // Capacity
        
        bool empty() const noexcept {
            return rootNode == nullptr;
        }
        //...
        
        // Iterators
        
        iterator begin() noexcept {
            return iterator(getFirstNode(), 0);
        }
        
        const_iterator begin() const noexcept {
            return const_iterator(getFirstNode(), 0);
        }
        
        iterator end() noexcept {
            return iterator(nullptr, 0);
        }
        
        const_iterator end() const noexcept {
            return const_iterator(nullptr, 0);
        }
        
        const_iterator cbegin() const noexcept {
            return const_iterator(getFirstNode(), 0);
        }
        
        const_iterator cend() const noexcept {
            return const_iterator(nullptr, 0);
        }
        
        //
        // Testing purposes methods
        //
        
        class node_proxy {
        public:
            explicit node_proxy(Node_type* node) : node(node) {}
            
            node_proxy child(int childIdx) const {
                if (!node)
                    return node_proxy{nullptr};
                return node_proxy { node->getChild(childIdx) };
            }
            
            std::pair<value_type, bool> value(int valueIdx) const {
                if (!node || node->getValuesCount() <= valueIdx)
                    return std::make_pair(value_type{}, false);
                
                return std::make_pair(node->getValue(valueIdx), true);
            }
            
            bool exists() const {
                return node != nullptr;
            }
            
        private:
            Node_type* node;
        };
        
        node_proxy child(int childIdx) const {
            node_proxy proxy { rootNode };
            return proxy.child(childIdx);
        }
        
        std::pair<value_type, bool> value(int valueIdx) const {
            node_proxy proxy { rootNode };
            return proxy.value(valueIdx);
        }
        
    private:
        Node_type* rootNode;
        Node_allocator_type nodeAllocator;
        
        template<typename... Args>
        Node_type* allocateNode(Args&&... args) {
            Node_type* node = nodeAllocator.allocate(1);
            
            try {
                nodeAllocator.construct(node, std::forward<Args>(args)...);
                return node;
            }
            catch(...) {
                nodeAllocator.deallocate(node, 1);
                throw;
            }
        }
        
        void deallocateNode(Node_type* node) {
            nodeAllocator.destroy(node);
            nodeAllocator.deallocate(node, 1);
        }
        
        Node_type* getFirstNode() const {
            if (rootNode == nullptr)
                return nullptr;
            
            return rootNode->getLeftmostChild();
        }
        
        // Returns node, containing search value, or insertion proposal node
        Node_type* findNode(const value_type& value) const {
            if (!rootNode)
                return nullptr;
            
            Node_type* curNode = rootNode;
            
            while (true) {
                if (curNode->containsValue(value))
                    break;
                
                Node_type* child = curNode->chooseChild(value);
                if (!child)
                    break;
                
                curNode = child;
            }
            
            return curNode;
        }
        
        std::pair<iterator,bool> insertImpl(const value_type& value) {
            // Returns a pair consisting of an iterator to the inserted element (or to the element that prevented the insertion) and a bool denoting whether the insertion took place.
            if (!rootNode) {
                rootNode = allocateNode(value);
                return std::make_pair(iterator{rootNode, 0}, true);
            } else {
                Node_type* node = findNode(value);
                
                // node is always non null here, at least we have root node
                int valueIdx = node->getValueIdx(value);
                if (valueIdx != -1)
                    return std::make_pair(iterator{node, valueIdx}, false);
                
                node->insertValue(value);
                node = fixNodeInsert(node, value);
                
                assert(node && node->containsValue(value));
                valueIdx = node->getValueIdx(value);
                return std::make_pair(iterator{node, valueIdx}, true);
            }
        }
        
        // After all fixes retuns node that contains new inserted value
        Node_type* fixNodeInsert(Node_type* node, const value_type& value) {
            if (node->isConsistent())
                return node;
            
            Node_type* valueNode = nullptr;
            
            do {
                // We have a 4-node here, split it and propagate higher
                // - Get the middle value of the 4-node, insert it into node's parent
                // - Split other 4-node childs and set them as a parent's new value childs
                // - Recursion: if parent is 4-node repeat from 1st step
                Node_type* newLeftChild = nullptr;
                Node_type* newRightChild = nullptr;
                try {
                    newLeftChild = allocateNode();
                    newRightChild = allocateNode();

                    node->splitIf4Node(newLeftChild, newRightChild);
                    assert(node->isConsistent()); // Check if splitted down to 2-node

                    if (!valueNode && newLeftChild->containsValue(value))
                        valueNode = newLeftChild;
                    if (!valueNode && newRightChild->containsValue(value))
                        valueNode = newRightChild;
                    
                    if (node->getParent()) {
                        Node_type* parent = node->getParent();
                        parent->mergeWithChild(node);
                        deallocateNode(node); // Can be reused
                        
                        node = parent;
                    }
                } catch(...) {
                    if (newLeftChild && !newLeftChild->getParent())
                        deallocateNode(newLeftChild);
                    if (newRightChild && !newRightChild->getParent())
                        deallocateNode(newRightChild);
                    throw;
                }
            } while (!node->isConsistent());
            
            if (!valueNode)
                valueNode = node;
            
            assert(valueNode != nullptr);
            assert(valueNode->containsValue(value));
            return valueNode;
        }
        
        int eraseImpl(const_iterator pos) {
            Node_type* node = pos.current;
            int valueIdx = pos.valueIdx;

            if (node == nullptr)
                return 0;
            
            // Downward phase, cases:
            // 1. Del from non-terminal 3-node: replace the value from its in-order predecessor
            // 2. Del from non-terminal 2-node: replace the value from its in-order predecessor
            // 3. Del from a terminal 3-node: convert it into a 2-node
            // 4. Del from a terminal 2-node: consider as a special hole-node with a single subtree
            if (node->isLeafNode()) {
                // Leaf node
                node->removeValueAtIdx(valueIdx);
            } else if (node->isTwoNode() || node->isThreeNode()) {
                // Internal 2-node or 3-node
                const_iterator nextPos = pos; // in-order successor
                ++nextPos;
                
                Node_type* leafNode = nextPos.current; // the next value always lays in a leaf node
                int leafValueIdx = nextPos.valueIdx;

                assert(leafNode != nullptr);
                assert(leafNode->isLeafNode());
                assert(leafNode->getValuesCount() > 0);

                node->replaceValue(leafNode->removeValueAtIdx(leafValueIdx), valueIdx);
                node = leafNode;
            } else {
                assert(false);
            }
            
            if (node == rootNode && node->getValuesCount() == 0) {
                // root node removed
                deallocateNode(rootNode);
                rootNode = nullptr;
            } else if (node->isLeafNode() && node->getValuesCount() == 1) {
                // former 3-node, the tree is in consistent state now
            } else {
                fixNodeRemove(node);
            }
            
            return 1;
        }
        
        // http://www.cs.princeton.edu/~dpw/courses/cos326-12/ass/2-3-trees.pdf
        void fixNodeRemove(Node_type* holeNode) {
            // Upward phase cases:
            // 1. The hole has a 2-node as a parent and a 2-node as a sibling (the hole is left or right child):
            //      Merge the parent with the sibling, rearrange children, propagate the hole node higher
            // 2. The hole has a 2-node as a parent and a 3-node as a sibling:
            //      Move the parent to the hole node's place (left or right), rearrange children
            // 3. 3-node as a parent, 2-node as a sibling: two subcases...
            // 4. 3-node as a parent, 3-node as a sibling: two subcases...
            while (holeNode) {
                Node_type* parent = holeNode->getParent();
                assert(parent != nullptr);
                int idxInParent = parent->getChildIdx(holeNode);
                assert(idxInParent != -1);
                
                if (parent->isTwoNode()) {
                    int siblingIdx = idxInParent == 0 ? 1 : 0;
                    Node_type* sibling = parent->getChild(siblingIdx);
                    assert(sibling != nullptr);
                    
                    if (sibling->isTwoNode()) { // Case 1
                        // removeValueFromParent atIndex:0 -> insertValue to:Sibling // Parent is now a hole node
                        // removeChildFromHole -> insertToSibling atIndex:0|2
                        // removeHoleChildFromParent -> deleteHoleNode
                        sibling->insertValue(parent->removeValueAtIdx(0));
                        
                        Node_type* holeChild = holeNode->removeChild(0);
                        if (holeChild != nullptr)
                            sibling->insertChild(holeChild, idxInParent == 0 ? 0 : 2);
                        
                        deallocateNode(parent->removeChild(idxInParent));
                        
                        if (parent == rootNode) { // eliminate hole at the root of the tree (tree height decrement)
                            assert(parent->getChildrenCount() == 1);
                            rootNode = parent->removeChild(0);
                            deallocateNode(parent);
                            holeNode = nullptr;
                        } else {
                            holeNode = parent;
                        }
                    } else if (sibling->isThreeNode()) { // Case 2
                        // removeValueFromParent atIndex:0 -> insertValue to:Hole
                        // removeValueFromSibling atIndex:(0|1) -> insertValue to:Parent
                        // removeChildFromSibling atIndex:(0|2) -> insertChild to:Hole atIndex:(1|0)
                        holeNode->insertValue(parent->removeValueAtIdx(0));
                        parent->insertValue(sibling->removeValueAtIdx(idxInParent));
                        
                        Node_type* siblingChild = sibling->removeChild(idxInParent == 0 ? 0 : 2);
                        if (siblingChild != nullptr)
                            holeNode->insertChild(siblingChild, siblingIdx);
                        
                        holeNode = nullptr;
                    } else {
                        assert(false);
                    }
                } else if (parent->isThreeNode()) {
                    int siblingIdx = 0;
                    if (idxInParent == 0 || idxInParent == 2) {
                        siblingIdx = 1;
                    } else if (idxInParent == 1) {
                        // Should we choose here either left or right sibling?
                    }
                    Node_type* sibling = parent->getChild(siblingIdx);
                    assert(sibling != nullptr);
                    
                    bool rotateLeft = idxInParent < siblingIdx;
                    bool inParentsLeftPart = true;
                    if (idxInParent == 2 || (idxInParent == 1 && siblingIdx == 2))
                        inParentsLeftPart = false;
                    
                    if (sibling->isTwoNode()) { // Case 3
                        // removeValue from:Parent atIndex:(0|1) -> insertValue to:Sibling
                        // removeChild from:Hole atIndex:0 -> insertChild to:Sibling atIndex:(0|2)
                        // removeChild from:Parent atIndex:HoleIdx -> deleteNode
                        sibling->insertValue(parent->removeValueAtIdx(inParentsLeftPart ? 0 : 1));
                        
                        Node_type* holeChild = holeNode->removeChild(0);
                        if (holeChild != nullptr)
                            sibling->insertChild(holeChild, rotateLeft ? 0 : 2);
                        
                        deallocateNode(parent->removeChild(idxInParent));
                    } else if (sibling->isThreeNode()) { // Case 4
                        // removeValue from:Sibling atIndex:(0|1) -> replaceValue to:Parent atIndex:(0|1) -> insertValue to:Hole
                        // removeChild from:Sibling atIndex:(0|2) -> insertChild to:Hole atIndex:(1|0)
                        holeNode->insertValue(
                          parent->replaceValue(
                           sibling->removeValueAtIdx(rotateLeft ? 0 : 1),
                           inParentsLeftPart ? 0 : 1));
                        
                        Node_type* siblingChild = sibling->removeChild(rotateLeft ? 0 : 2);
                        if (siblingChild != nullptr)
                            holeNode->insertChild(siblingChild, rotateLeft ? 1 : 0);
                    } else {
                        assert(false);
                    }
                    holeNode = nullptr;
                } else {
                    assert(false);
                }
            }
        }
    };

} // namespace lab

#endif // AlgoAndData_data_twothree_tree_h
