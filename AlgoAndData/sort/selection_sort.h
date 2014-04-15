//
//  selection_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_selection_sort_h
#define AlgoAndData_sort_selection_sort_h

#include <functional>
#include <algorithm>

//
// CPU on average: n^2
// CPU worst-case: n^2
// Memory: O(1) (in-place)
//
// Can be stable. Fast on small input sizes. Performs bad on sorted input.
//

namespace lab {

	// It can be used with ForwardIterator
	template<typename RandomIt, typename Compare>
	void selection_sort(RandomIt first, RandomIt last, Compare comp) {
		for (RandomIt iIter = first; iIter != last; ++iIter) {
			RandomIt nextElemIter = iIter;
			
			RandomIt jIter = iIter;
			++jIter;
			
			for (; jIter != last; ++jIter) {
				if (comp(*jIter, *nextElemIter)) {
					nextElemIter = jIter;
				}
			}
			
			if (iIter != nextElemIter)
				std::iter_swap(iIter, nextElemIter);
		}
	}
	
	template<typename RandomIt>
	void selection_sort(RandomIt first, RandomIt last) {
		selection_sort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_selection_sort_h
