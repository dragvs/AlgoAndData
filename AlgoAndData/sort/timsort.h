//
//  timsort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 06/06/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_timsort_h
#define AlgoAndData_sort_timsort_h

// TODO Add description

namespace lab {
	
	template<typename RandomIt, typename Size, typename Compare>
    void timsort(RandomIt first, RandomIt last, Size depth_limit, Compare comp) {
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
		
		
    }
	
	template<typename RandomIt, typename Compare>
	void intro_sort(RandomIt first, RandomIt last, Compare comp) {
		if (!(first < last))
			return;
		
		introsort_loop(first, last, (int)(log2(last - first) * 2), comp);
		insertion_sort(first, last, comp);
	}
	
	template<typename RandomIt>
	void timsort(RandomIt first, RandomIt last) {
		timsort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_timsort_h
