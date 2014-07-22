//
//  hash_map.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 17/06/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_data_hash_map_h
#define AlgoAndData_data_hash_map_h

#include <iterator>
#include <utility>
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>
//#include <cstdio>

namespace lab {
	
    //
    // Open addressing (closed hashing) hash table, linear probing
    //
    
    template<typename Value>
    struct Bucket {
        bool is_busy;
        bool is_deleted;
        Value contents;
        
        void makeActive(const Value& value) {
            assert(!isActive());
            
            is_busy = true;
            is_deleted = false;
            contents = value;
        }
        
        bool isActive() const noexcept {
            return is_busy && !is_deleted;
        }
        
        void markAsDeleted() noexcept {
            is_deleted = true;
        }
    };
    
    ///// Iterators
    /// Base class for node iterators.
    template<typename Value, typename node_type>
    struct Bucket_iterator_base
    {
        using bucket_type = Bucket<Value>;
        node_type current;
        node_type end;
        
        Bucket_iterator_base(node_type curNode, node_type endNode)
            : current(curNode), end(endNode) {}
        
        void incr() {
            if (current == end)
                return;

            while (current != end) {
                ++current;
                
                if (current->is_deleted)
                    continue;
                if (current->is_busy)
                    break;
            }
        }
    };
    
    template<typename Value, typename node_type>
    inline bool
    operator==(const Bucket_iterator_base<Value, node_type>& x,
               const Bucket_iterator_base<Value, node_type>& y)
    { return x.current == y.current && x.end == y.end; }
    
    template<typename Value, typename node_type>
    inline bool
    operator!=(const Bucket_iterator_base<Value, node_type>& x,
               const Bucket_iterator_base<Value, node_type>& y)
    { return x.current != y.current || x.end != y.end; }
    
    /// Node iterators, used to iterate through all the hashtable.
    template<typename Value, typename node_type>
    struct Bucket_iterator : public Bucket_iterator_base<Value, node_type>
    {
    private:
        using base_type = Bucket_iterator_base<Value, node_type>;
        
    public:
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        
        using pointer = Value*;
        using reference = Value&;
        
        Bucket_iterator(node_type curNode, node_type endNode) : base_type(curNode, endNode) {}
        
        reference
        operator*() const
        {
            return reinterpret_cast<reference>(this->current->contents);
        }
        
        pointer
        operator->() const
        {
            return reinterpret_cast<pointer>(std::addressof(this->current->contents));
        }
        
        Bucket_iterator&
        operator++()
        {
            this->incr();
            return *this;
        }
        
        Bucket_iterator
        operator++(int)
        {
            Bucket_iterator tmp(*this);
            this->incr();
            return tmp;
        }
    };
    
    template<typename Value, typename node_type>
    struct Bucket_const_iterator : public Bucket_iterator_base<Value, node_type>
    {
    private:
        using base_type = Bucket_iterator_base<Value, node_type>;
        
    public:
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        
        using pointer = const Value*;
        using reference = const Value&;
        
        Bucket_const_iterator(node_type curNode, node_type endNode) : base_type(curNode, endNode) {}
        
        Bucket_const_iterator(const Bucket_iterator<Value, node_type>& other) :
            base_type(other.current, other.end) {}
        
        reference
        operator*() const
        {
            return reinterpret_cast<reference>(this->current->contents);
        }
        
        pointer
        operator->() const
        {
            return reinterpret_cast<pointer>(std::addressof(this->current->contents));
        }
        
        Bucket_const_iterator&
        operator++()
        {
            this->incr();
            return *this;
        }
        
        Bucket_const_iterator
        operator++(int)
        {
            Bucket_const_iterator tmp(*this);
            this->incr();
            return tmp;
        }
    };
    ///// Iterators
    
    
    template<
        typename Key,
        typename T,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<Key>,
        typename Allocator = std::allocator< std::pair<const Key, T> >
    >
    class hash_map {
    private:
        using internal_value_type = std::pair<Key, T>;
        using Bucket_type = Bucket<internal_value_type>;
        using Bucket_allocator_type = typename Allocator::template rebind<Bucket_type>::other;
        using BucketArray = std::vector<Bucket_type, Bucket_allocator_type>;
        
    public:
        using key_type = Key;
        using mapped_type = T;
        using value_type = std::pair<const Key, T>;
        using size_type = std::size_t;
        
        using iterator = Bucket_iterator<value_type, typename BucketArray::iterator>;
        using const_iterator = Bucket_const_iterator<value_type, typename BucketArray::const_iterator>;
        
        hash_map() : hash_map(10) {}
        
        explicit hash_map( size_type bucket_count,
                          const Hash& hash = Hash(),
                          const KeyEqual& equal = KeyEqual(),
                          const Allocator& alloc = Allocator() )
        {
            this->array_size = bucket_count;
            this->elementsCount = 0;
            this->hash = hash;
            this->keyEqual = keyEqual;
            
            bucket_array.resize(array_size);
        }
        
        // Lookup
        
        // TODO implement 'at'
        
        T& operator[](const key_type& key) {
            std::size_t index = getIndex(key);
            
            if (bucket_array[index].isActive()) {
                // Found existing entry
                return bucket_array[index].contents.second;
            }
            
            // Entry with provided key not found, creating one
            iterator iter = insertImpl(index, std::make_pair(key, T{}));
            return iter->second;
        }
        
        iterator find(const Key& key) {
            std::size_t index = getIndex(key);
            
            if (!bucket_array[index].isActive()) {
                // Element not found
                return end();
            }
            
            return iterator { bucket_array.begin() + index, bucket_array.end() };
        }
        
        const_iterator find(const Key& key) const {
            std::size_t index = getIndexLookup(key);
            
            if (!bucket_array[index].isActive()) {
                // Element not found
                return end();
            }
            
            return const_iterator { bucket_array.begin() + index, bucket_array.end() };
        }
        
        size_type count(const Key& key) const {
            std::size_t index = getIndexLookup(key);
            
            if (!bucket_array[index].isActive()) {
                // Element not found
                return 0;
            }
            return 1;
        }
        
        // Modifiers
        
        std::pair<iterator, bool> insert(const value_type& value) {
            std::size_t index = getIndex(value.first);
            
            if (bucket_array[index].isActive()) {
                // Insertion prevented by the existing element
                iterator iter { bucket_array.begin() + index, bucket_array.end() };
                return std::make_pair(iter, false);
            }
            
            iterator iter = insertImpl(index, value);
            return std::make_pair(iter, true);
        }
        
        iterator erase(const_iterator pos) {
            typename BucketArray::iterator bucketArrIter = bucket_array.begin();
            std::advance(bucketArrIter, std::distance(bucket_array.cbegin(), pos.current));
            
            iterator iter { bucketArrIter , bucket_array.end() };
            return eraseImpl(iter);
        }
        
        size_type erase(const key_type& key) {
            iterator iter = find(key);
            
            if (iter == end()) {
                // Element with provided key not found to erase
                return 0;
            }
            
            eraseImpl(iter);
            return 1;
        }
        
        void rehash(size_type newSize) {
            rehashImpl(newSize);
        }
        
        void clear() noexcept {
            array_size = 0;
            bucket_array.clear(); // still keeps memory busy
            elementsCount = 0;
        }
        
        // Capacity
        
        bool empty() const noexcept {
            return elementsCount > 0;
        }
        
        size_type size() const noexcept {
            return elementsCount;
        }
        
        size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max();
        }
        
        float max_load_factor() const {
            return 0.75f;
        }
        
        // Iterators
        
        iterator begin() noexcept {
            return iterator(getFirstBucket(), bucket_array.end());
        }
        
        const_iterator begin() const noexcept {
            return const_iterator(getFirstBucket(), bucket_array.end());
        }
        
        iterator end() noexcept {
            return iterator(bucket_array.end(), bucket_array.end());
        }
        
        const_iterator end() const noexcept {
            return const_iterator(bucket_array.end(), bucket_array.end());
        }
        
        const_iterator cbegin() const noexcept {
            return const_iterator(getFirstBucket(), bucket_array.end());
        }
        
        const_iterator cend() const noexcept {
            return const_iterator(bucket_array.end(), bucket_array.end());
        }
        
    private:
        
        std::size_t getHashCode(const Key& key) const {
            return hash(key);
        }
        
        std::size_t getBucketIndex(size_t hashCode) const {
            return hashCode % array_size;
        }
        std::size_t getBucketIndex(const key_type& key) const {
            return getBucketIndex(getHashCode(key));
        }
        
        //
        // Returns:
        //  - If node found:
        //      - Swap with first found is_deleted node
        //      - Return node index
        //  - If node didn't find:
        //      - Returns first found is_deleted or !is_busy node
        //
        //  - Insertion proposal:
        //      - !is_busy || (is_busy && is_deleted)
        //  - Found existing:
        //      - is_busy && !is_deleted
        std::size_t getIndex(const key_type& key) {
            std::size_t bucketIdx = getBucketIndex(key);
            size_t index = bucketIdx;
            bool circle_run = false;
            bool foundDeletedNode = false;
            size_t deletedNodeIndex = 0;
            
            while (bucket_array[index].is_busy && !circle_run) {
                if (!foundDeletedNode && bucket_array[index].is_deleted) {
                    foundDeletedNode = true;
                    deletedNodeIndex = index;
                }
                
                if (keyEqual(key, bucket_array[index].contents.first))
                    break;
                ++index;
                
                if (index == array_size) {
                    index = 0;
                }
                if (index == bucketIdx) {
                    circle_run = true;
                }
            }
            
            assert(index < array_size);
            
            if (circle_run) {
                // Didn't find nor required entry nor free node to insert (too much 'is_deleted' nodes)
                // Return first found 'is_deleted' as an insertion proposal
                assert(foundDeletedNode);
                return deletedNodeIndex;
            }
            
            if (bucket_array[index].is_busy) {
                // Found something...
                if (bucket_array[index].is_deleted) {
                    // Entry is deleted already
                    // Swap with first found 'is_deleted' and return as an insertion proposal
                } else {
                    // Found existing entry
                    // Swap with first found 'is_deleted' and return
                }
                
                if (foundDeletedNode && deletedNodeIndex != index) {
                    std::swap(bucket_array[deletedNodeIndex], bucket_array[index]);
                }
            } else {
                // Didn't find nor active required entry nor it's is_deleted node
                // Return first found 'is_deleted' node or this
                
                if (foundDeletedNode)
                    index = deletedNodeIndex;
            }
            
            return index;
        }
        
        //
        // getIndex version without swaps
        std::size_t getIndexLookup(const key_type& key) const {
            std::size_t bucketIdx = getBucketIndex(key);
            size_t index = bucketIdx;
            bool circle_run = false;
            bool foundDeletedNode = false;
            size_t deletedNodeIndex = 0;
            
            while (bucket_array[index].is_busy && !circle_run) {
                if (!foundDeletedNode && bucket_array[index].is_deleted) {
                    foundDeletedNode = true;
                    deletedNodeIndex = index;
                }
                
                if (keyEqual(key, bucket_array[index].contents.first))
                    break;
                ++index;
                
                if (index == array_size) {
                    index = 0;
                }
                if (index == bucketIdx) {
                    circle_run = true;
                }
            }
            
            assert(index < array_size);
            
            if (circle_run) {
                // Didn't find nor required entry nor free node to insert (too much 'is_deleted' nodes)
                // Return first found 'is_deleted' as an insertion proposal
                assert(foundDeletedNode);
                return deletedNodeIndex;
            }
            
            if (bucket_array[index].is_busy) {
                // Found something...
                if (bucket_array[index].is_deleted) {
                    // Entry is deleted already
                    // Return as an insertion proposal
                } else {
                    // Found existing entry
                }
            } else {
                // Didn't find nor active required entry nor it's is_deleted node
                // Return first found 'is_deleted' node or this
                
                if (foundDeletedNode)
                    index = deletedNodeIndex;
            }
            
            return index;
        }
        
        typename BucketArray::iterator
        getFirstBucket() noexcept
        {
            auto iter = bucket_array.begin();
            auto endIter = bucket_array.end();
            
            while (iter != endIter) {
                if (iter->isActive())
                    return iter;
                
                ++iter;
            }
            
            return endIter;
        }
        
        typename BucketArray::const_iterator
        getFirstBucket() const noexcept
        {
            auto iter = bucket_array.begin();
            auto endIter = bucket_array.end();
            
            while (iter != endIter) {
                if (iter->isActive())
                    return iter;
                
                ++iter;
            }
            
            return endIter;
        }
        
        std::pair<bool, size_type> isRehashNeeded(size_type elemsCount) const {
            if (elemsCount >= max_load_factor() * array_size) {
                return std::make_pair(true, array_size * 2.25);
            }
            
            return std::make_pair(false, 0);
        }
        
        void rehashImpl(size_type newSize) {
            if (elementsCount > max_load_factor() * newSize) {
                newSize = elementsCount / max_load_factor();
            }
            
            BucketArray oldBuckets { bucket_array };
            
            clear();
            array_size = newSize;
            bucket_array.resize(array_size);
            
            auto iter = oldBuckets.begin();
            auto endIter = oldBuckets.end();
            
            while (iter != endIter) {
                if (iter->isActive())
                    operator[](iter->contents.first) = iter->contents.second;
                
                ++iter;
            }
        }
        
        iterator insertImpl(size_type index, const value_type& value) {
            std::pair<bool, size_type> rehashNeededPair = isRehashNeeded(elementsCount+1);
            
            if (rehashNeededPair.first) {
                rehash(rehashNeededPair.second);
                index = getIndex(value.first);
            }
            
            bucket_array[index].makeActive(value);
            ++elementsCount;
            
            return iterator(bucket_array.begin() + index, bucket_array.end());
        }

        iterator eraseImpl(iterator pos) {
            iterator nextIter = pos;
            ++nextIter;
            
            assert(pos.current->is_busy);

            pos.current->markAsDeleted();
            --elementsCount;
            
            return nextIter;
        }

        BucketArray bucket_array;
        size_type array_size;
        size_type elementsCount;
        
        Hash hash;
        KeyEqual keyEqual;
    };
    
} // namespace lab

#endif // AlgoAndData_data_hash_map_h
