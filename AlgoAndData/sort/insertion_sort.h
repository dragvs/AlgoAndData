//
//  insertion_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_insertion_sort_h
#define AlgoAndData_sort_insertion_sort_h

#include <functional>
#include <algorithm>

//
// CPU on average: n^2
// CPU worst-case: n^2
// Memory: O(1) (in-place)
//
// Stable sort. Fast on small input sizes. Efficient (linear time) on sorted input. Online sort.
//

namespace lab {

	// It can be used with BidirectionalIterator
	template<typename RandomIt, typename Compare>
	void insertion_sort(RandomIt first, RandomIt last, Compare comp) {
		RandomIt iIter = first;
		++iIter;
		
		using ValueType = typename std::iterator_traits<RandomIt>::value_type;
		
		for (; iIter != last; ++iIter) {
			ValueType temp = *iIter;
			RandomIt jIter = iIter;
			
			while (jIter != first) {
				RandomIt prevIter = jIter;
				--prevIter;
				
				if (comp(temp, *prevIter)) {
					*jIter = *prevIter;
					--jIter;
				} else {
					break;
				}
			}
			
			*jIter = temp;
		}
	}
	
	template<typename RandomIt>
	void insertion_sort(RandomIt first, RandomIt last) {
		insertion_sort(first, last, std::less<typename RandomIt::value_type>());
	}
}

#endif // AlgoAndData_sort_insertion_sort_h
