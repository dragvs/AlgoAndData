//
//  radix_sort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 15/04/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_radix_sort_h
#define AlgoAndData_sort_radix_sort_h

#include "../data/heap.h"
#include <vector>
#include <utility>

// TODO Add description
// Stable. LSD variation

namespace lab {
	
	template<typename T>
	struct DefaulAccessor {
		T operator()(T& obj) {
			return obj;
		}
	};
	
	// TODO Define Iterator category
	template<
		typename RandomIt,
		typename Accessor=DefaulAccessor<typename std::iterator_traits<RandomIt>::value_type>
	>
	void radix_sort(RandomIt first, RandomIt last, Accessor accessor) {
		using ValueType = typename std::iterator_traits<RandomIt>::value_type;
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		using KeyType = typename std::result_of<Accessor(ValueType&)>::type;
		
		// TODO KeyType must be an integral type
		if (!(first < last))
			return;
		
		DiffType length = last - first;
		if (length < 2)
			return;
		
		// Get max elem
		KeyType maxElem = accessor(*first);
		
		for (RandomIt iter = first+1; iter < last; ++iter) {
			KeyType elem = accessor(*iter);
			
			if (elem > maxElem)
				maxElem = elem;
		}
		
		// Preparations
		using BaseType = int;
		using BucketArrType = unsigned long int;

		static const BaseType BASE = 10;
		int exponent = 1;
		std::vector<ValueType> tempVec(length);
		
		// Sort body
		while (maxElem / exponent > 0) {
			BucketArrType bucketArr[BASE] = { 0 };
			
			// Making histogram
			for (RandomIt iter = first; iter < last; ++iter) {
				BaseType value = (accessor(*iter) / exponent) % BASE;
				++bucketArr[value];
			}
			
			// Making cumulative histogram
			for (int i = 1; i < BASE; ++i)
				bucketArr[i] += bucketArr[i - 1];
			
			// Moving elements to tempVec positions using histogram
			for (RandomIt iter = first + (length-1); iter >= first; --iter) {
				BaseType value = (accessor(*iter) / exponent) % BASE;
				BucketArrType place = --bucketArr[value];
				
				tempVec[place] = *iter;
			}
			
			// Moving back base-sorted values from tempVec
			for (RandomIt iter = first, tempIter = tempVec.begin(); iter  <last; ++iter, ++tempIter) {
				*iter = *tempIter;
			}
			
			exponent *= BASE;
		}
	}
	
	template<
	typename RandomIt,
	typename Accessor=DefaulAccessor<typename std::iterator_traits<RandomIt>::value_type>
	>
	void radix_sort(RandomIt first, RandomIt last) {
		Accessor accessor;
		radix_sort(first, last, accessor);
	}
	
} // namespace lab

#endif // AlgoAndData_sort_radix_sort_h
