//
//  insertion_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_shell_sort_h
#define AlgoAndData_sort_shell_sort_h

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
	void shell_sort(RandomIt first, RandomIt last, Compare comp) {
		static const int gapsArr[16] = { 460316, 204585, 90927, 40412, 17961, 7983, 3458,
										 1577, 701, 301, 132, 57, 23, 10, 4, 1 };
		
		for (int gap : gapsArr) {
			RandomIt gapFirstIter = first+gap;
			
			for (RandomIt iIter = gapFirstIter; iIter < last; ++iIter) {
				RandomIt jIter = iIter;
				
				while (jIter >= gapFirstIter) {
					RandomIt prevIter = jIter - gap;
					
					if (comp(*jIter, *prevIter)) {
						std::iter_swap(jIter, prevIter);
						jIter -= gap;
					} else {
						break;
					}
				}
			}
		}
	}
	
	template<typename RandomIt>
	void shell_sort(RandomIt first, RandomIt last) {
		shell_sort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_shell_sort_h
