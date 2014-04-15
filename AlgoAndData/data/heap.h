//
//  heap.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 08/04/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_data_heap_h
#define AlgoAndData_data_heap_h

#include <iterator>
#include <cstdio>

namespace lab {
	
	// TODO Define Iterator required category
	template<typename RandomIt, typename Compare>
	void heap_sift_down(RandomIt first, RandomIt point, RandomIt last, Compare comp) {
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		DiffType length = last - first;
		
		if (point < first || point >= last)
			return; // Wrong 'point'
		
		DiffType parentIdx = point - first;
		RandomIt parentIt = point;
		
		while (true) {
			DiffType leftChildIdx = 2 * parentIdx + 1;
			
			if (leftChildIdx >= length) {
				return; // 'parentIt' is not a parent at all
			}
			
			RandomIt leftChild = first + leftChildIdx;
			RandomIt elemToSwapWith = parentIt;
			DiffType elemToSwapWithIdx = parentIdx;
			
			// Compare leftChild and parentIt
			if (comp(*elemToSwapWith, *leftChild)) {
				elemToSwapWith = leftChild;
				elemToSwapWithIdx = leftChildIdx;
			}
			
			DiffType rightChildIdx = leftChildIdx + 1;
			if (rightChildIdx < length) {
				// Compare leftChild, rightChild and parentIt
				RandomIt rightChild = first + rightChildIdx;
				
				if (comp(*elemToSwapWith, *rightChild)) {
					elemToSwapWith = rightChild;
					elemToSwapWithIdx = rightChildIdx;
				}
			}
			
			if (elemToSwapWith == parentIt) {
				return;
			} else {
				std::iter_swap(parentIt, elemToSwapWith);
				parentIt = elemToSwapWith;
				parentIdx = elemToSwapWithIdx;
			}
		}
	}
	
	// TODO Define Iterator required category
	template<typename RandomIt, typename Compare>
	void heap_make(RandomIt first, RandomIt last, Compare comp) {
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		
		if (!(first < last))
			return;
		
		DiffType length = last - first;
		if (length < 2)
			return;
		
		DiffType lastParentIdx = length / 2;
		RandomIt parentIt = first + lastParentIdx;
		
		while (true) {
			heap_sift_down(first, parentIt, last, comp);
			
			if (parentIt == first)
				break;
			
			--parentIt;
		}
	}
		
} // namespace lab

#endif // AlgoAndData_data_heap_h
