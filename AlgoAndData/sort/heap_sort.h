//
//  heap_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 08/04/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_heap_sort_h
#define AlgoAndData_sort_heap_sort_h

#include "../data/heap.h"

// TODO Add description

namespace lab {
	
	// TODO Define Iterator category
	template<typename RandomIt, typename Compare>
	void heap_sort(RandomIt first, RandomIt last, Compare comp) {
		using ValueType = typename std::iterator_traits<RandomIt>::value_type;
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		
		if (!(first < last))
			return;
		
		DiffType length = last - first;
		if (length < 2)
			return;
		
		heap_make(first, last, comp);
		
		for (DiffType sortedHeadIdx = length - 1; sortedHeadIdx > 0; --sortedHeadIdx) {
			RandomIt sortedHeadIt = first + sortedHeadIdx;
			std::iter_swap(first, sortedHeadIt);
			heap_sift_down(first, first, sortedHeadIt, comp);
		}
	}
	
} // namespace lab

#endif // AlgoAndData_sort_heap_sort_h
