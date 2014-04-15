//
//  merge_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_merge_sort_h
#define AlgoAndData_sort_merge_sort_h

#include <functional>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

//
// CPU on average: n log n
// CPU worst-case: n log n
// Memory: O(n) (auxiliary for merge step)
//
// Stable. Efficient on slow access memory and linked lists. Parallelizes well (D&C nature).
// Compared to Quicksort and Heapsort.
//

namespace lab {
	
	template<typename RandomIt, typename OutputIt, typename Compare>
	void merge(RandomIt first, RandomIt middle, RandomIt last, OutputIt output, Compare comp);

	// Only RandomAccessIterator
	template<typename RandomIt, typename Compare>
	void merge_sort(RandomIt first, RandomIt last, Compare comp) {
		if (!(first < last))
			return;
		
		using ValueType = typename std::iterator_traits<RandomIt>::value_type;
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		DiffType length = last - first;
		
		if (length < 2)
			return;
		
		auto bufferPair = std::get_temporary_buffer<ValueType>(length);
		
		if (bufferPair.second < length) {
			std::return_temporary_buffer(bufferPair.first);
			throw std::exception();
		}
		
		for (int subset_width = 1; subset_width < length; subset_width *= 2) {
			for (int i = 0; i < length; i += 2*subset_width) {
				DiffType middleOffset = std::min<DiffType>(i+subset_width, length);
				DiffType lastOffset = std::min<DiffType>(i+subset_width*2, length);
				
				merge(first+i, first+middleOffset, first+lastOffset, bufferPair.first, comp);
			}
		}
		
		std::return_temporary_buffer(bufferPair.first);
	}
	
	template<typename RandomIt>
	void merge_sort(RandomIt first, RandomIt last) {
		merge_sort(first, last, std::less<typename RandomIt::value_type>());
	}
	
	template<typename RandomIt, typename OutputIt, typename Compare>
	void merge(RandomIt first, RandomIt middle, RandomIt last, OutputIt outputFirst, Compare comp) {
		RandomIt leftIt = first;
		RandomIt rightIt = middle;
		OutputIt outputIt = outputFirst;
		
		while (leftIt < middle || rightIt < last) {
			if (leftIt == middle) {
				*outputIt = std::move(*rightIt);
				++rightIt;
			}
			else if (rightIt == last) {
				*outputIt = std::move(*leftIt);
				++leftIt;
			}
			else {
				if (!comp(*rightIt, *leftIt)) {
					*outputIt = std::move(*leftIt);
					++leftIt;
				} else {
					*outputIt = std::move(*rightIt);
					++rightIt;
				}
			}
			
			++outputIt;
		}

		leftIt = first;
		outputIt = outputFirst;

		while (leftIt != last) {
			*leftIt = std::move(*outputIt);
			++outputIt;
			++leftIt;
		}
	}
	
}

#endif // AlgoAndData_sort_merge_sort_h
