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
#include <type_traits>

// TODO Add description
// Stable. LSD variation

namespace lab {
	
	template<typename T>
	struct DefaultKeyAccessor {
		T operator()(T& obj) {
			return obj;
		}
	};
	
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE == 8, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value >> (3*digitIdx)) & 0x7;
	}
	
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE == 64, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value >> (6*digitIdx)) & 0x3F;
	}
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE == 1024, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value >> (10*digitIdx)) & 0x3FF;
	}
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE == 16384, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value >> (14*digitIdx)) & 0x3FFF;
	}
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE == 65536, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value >> (16*digitIdx)) & 0xFFF;
	}
	
	template<typename ValueType, typename BaseType, BaseType BASE>
	typename std::enable_if<BASE != 8 && BASE != 64 && BASE != 1024 && BASE != 16384 && BASE != 65536, BaseType>::type
	radix_get_digit(ValueType value, unsigned long int exponent, int digitIdx) {
		return (value / exponent) % BASE;
	}
	
	// TODO Define Iterator category
	template<
		typename RandomIt,
		typename KeyAccessor=DefaultKeyAccessor<typename std::iterator_traits<RandomIt>::value_type>,
		unsigned int BASE = 10
	>
	void radix_sort(RandomIt first, RandomIt last, KeyAccessor accessor) {
		using ValueType = typename std::iterator_traits<RandomIt>::value_type;
		using DiffType = typename std::iterator_traits<RandomIt>::difference_type;
		using KeyType = typename std::result_of<KeyAccessor(ValueType&)>::type;
		
		static_assert(std::is_integral<KeyType>::value, "Key type must be an integral type");
		
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
		using BaseType = unsigned int;
		using BucketArrType = unsigned long int;

//		static const BaseType vBASE = BASE;
		unsigned long int exponent = 1;
		int digitIdx = 0;
		std::vector<ValueType> tempVec(length);
		
		// Sort body
		while (maxElem / exponent > 0) {
			BucketArrType bucketArr[BASE] = { 0 };
			
			// Making histogram
			for (RandomIt iter = first; iter < last; ++iter) {
//				BaseType digit = (accessor(*iter) / exponent) % BASE;
				BaseType digit = radix_get_digit<KeyType, BaseType, BASE>(accessor(*iter), exponent, digitIdx);
				++bucketArr[digit];
			}
			
			// Making cumulative histogram
			for (int i = 1; i < BASE; ++i)
				bucketArr[i] += bucketArr[i - 1];
			
			// Moving elements to tempVec positions using histogram
			for (RandomIt iter = first + (length-1); iter >= first; --iter) {
//				BaseType digit = (accessor(*iter) / exponent) % BASE;
				BaseType digit = radix_get_digit<KeyType, BaseType, BASE>(accessor(*iter), exponent, digitIdx);
				BucketArrType place = --bucketArr[digit];
				
				tempVec[place] = *iter;
			}
			
			// Moving back base-sorted values from tempVec
			for (RandomIt iter = first, tempIter = tempVec.begin(); iter  <last; ++iter, ++tempIter) {
				*iter = *tempIter;
			}
			
			exponent *= BASE;
			++digitIdx;
		}
	}
	
	template<
		typename RandomIt,
		typename KeyAccessor=DefaultKeyAccessor<typename std::iterator_traits<RandomIt>::value_type>,
		unsigned int BASE = 10
	>
	void radix_sort(RandomIt first, RandomIt last) {
		KeyAccessor accessor;
		radix_sort<RandomIt, KeyAccessor, BASE>(first, last, accessor);
	}
	
} // namespace lab

#endif // AlgoAndData_sort_radix_sort_h
