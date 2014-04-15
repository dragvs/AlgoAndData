//
//  quick_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 25/03/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_quick_sort_h
#define AlgoAndData_sort_quick_sort_h

#include <functional>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

//
// CPU on average: n log n
// CPU worst-case: n^2
// Memory: O(1) or O(log N) for recursion
//
// TODO WRITE
//

namespace lab {
	
	// Only RandomAccessIterator
	template<typename RandomIt, typename Compare>
	std::pair<RandomIt, RandomIt> quick_sort_partition(RandomIt first, RandomIt last, RandomIt pivotIter, Compare& comp) {
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		DiffType length = last - first;
		
		RandomIt tailIter = first + (length-1);
		
		if (pivotIter != tailIter) {
			std::iter_swap(pivotIter, tailIter); // moving pivot elem to the end
			pivotIter = tailIter;
		}
		
		RandomIt begIter = first;
		RandomIt curIter = first;
		
		while (curIter != tailIter) {
			if (comp(*curIter, *pivotIter)) {
				if (curIter != begIter)
					std::iter_swap(curIter, begIter);
				++begIter;
				++curIter;
			}
			else if (comp(*pivotIter, *curIter)) {
				--tailIter;
				while (tailIter != curIter && comp(*pivotIter, *tailIter)) --tailIter;
				
				if (tailIter != curIter)
					std::iter_swap(curIter, tailIter);
			} else {
				++curIter;
			}
		}
		
		std::iter_swap(tailIter, pivotIter);
		++tailIter; // after last repeating pivot
		
		return std::make_pair(begIter, tailIter);
	}
	
	// Only RandomAccessIterator
	template<typename RandomIt, typename Compare>
	void quick_sort(RandomIt first, RandomIt last, Compare comp) {
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		
		if (!(first < last))
			return;
		
		DiffType length = last - first;
		if (length < 2)
			return;
		if (length == 2) {
			RandomIt lastElem = first+1;
			if (comp(*lastElem, *first)) {
				std::iter_swap(first, lastElem);
			}
			return;
		}
		
		// Using median of three to get pivot
		RandomIt lowIter = first;
		RandomIt highIter = first + (length-1);
		RandomIt pivotIter = first + length/2; // middle
		
		if (comp(*pivotIter, *lowIter))
			std::iter_swap(pivotIter, lowIter);
		if (comp(*highIter, *lowIter))
			std::iter_swap(highIter, lowIter);
		if (comp(*highIter, *pivotIter))
			std::iter_swap(highIter, pivotIter);
		
		std::pair<RandomIt, RandomIt> pivotRange = quick_sort_partition(first, last, pivotIter, comp);
		quick_sort(first, pivotRange.first, comp);
		quick_sort(pivotRange.second, last, comp);
	}
	
	template<typename RandomIt>
	void quick_sort(RandomIt first, RandomIt last) {
		quick_sort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_quick_sort_h
