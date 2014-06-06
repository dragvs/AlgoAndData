//
//  intro_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 23/05/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_intro_sort_h
#define AlgoAndData_sort_intro_sort_h

#include "quick_sort.h"
#include "insertion_sort.h"
#include <cmath>

// TODO Add description

namespace lab {
	
	template<typename RandomIt, typename Size, typename Compare>
    void introsort_loop(RandomIt first, RandomIt last, Size depth_limit, Compare comp) {
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
		
		static const int THRESHOLD = 16;
		while (length > THRESHOLD) {
//		while (last > first) {
			if (depth_limit == 0) {
				heap_sort(first, last, comp);
				return;
			}
			--depth_limit;
			
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
			introsort_loop(pivotRange.second, last, depth_limit, comp);
			
			last = pivotRange.first;
			length = last - first;
		}
    }
	
	template<typename RandomIt, typename Compare>
	void intro_sort(RandomIt first, RandomIt last, Compare comp) {
		if (!(first < last))
			return;
		
		introsort_loop(first, last, (int)(log2(last - first) * 2), comp);
		insertion_sort(first, last, comp);
	}
	
	template<typename RandomIt>
	void intro_sort(RandomIt first, RandomIt last) {
		intro_sort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_intro_sort_h
