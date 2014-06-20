//
//  timsort.h
//  AlgoAndData
//
//  Created by Vladimir Shishov on 06/06/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//

#ifndef AlgoAndData_sort_timsort_h
#define AlgoAndData_sort_timsort_h

#include "insertion_sort.h"
#include "merge_sort.h"
#include <iterator>
#include <algorithm>
#include <memory>
#include <cassert>

// TODO Add description

// Python listobject.c variation

namespace lab {
	
	namespace {
		template<typename Iter, typename Size>
		struct Slice {
			Iter base;
			Size len;
		};
		
		template<typename Iter, typename Size, typename Compare>
		class MergeState {
		public:
			MergeState(Size totalLength) : pendingCount(0) {
				// TODO Use RAII
				tempBufferPair = std::get_temporary_buffer<value_type>(totalLength);
			}
			
			~MergeState() {
				std::return_temporary_buffer(tempBufferPair.first);
			}
			
			void push_run(Iter base, Size len) {
				assert(pendingCount < MAX_MERGE_PENDING);
				
				pendingRuns[pendingCount].base = base;
				pendingRuns[pendingCount].len = len;
				++pendingCount;
			}
			
			void mergeRuns(int fromIdx, Compare comp) {
				Slice_t* const p = pendingRuns;
				
				merge(p[fromIdx].base, p[fromIdx+1].base,
					  p[fromIdx+1].base + p[fromIdx+1].len, tempBufferPair.first, comp);
				
				pendingRuns[fromIdx].len = p[fromIdx].len + p[fromIdx + 1].len;
				
				if (fromIdx == pendingCount - 3)
					pendingRuns[fromIdx + 1] = pendingRuns[fromIdx + 2];
				
				--pendingCount;
			}
			
			void checkCollapse(Compare comp) {
				Slice_t* const p = pendingRuns;
				
				while (pendingCount > 1) {
					int n = pendingCount - 2;
					
					if (n > 0 && p[n-1].len <= p[n].len + p[n+1].len) {
						if (p[n-1].len < p[n+1].len)
							--n;
						
						mergeRuns(n, comp);
					}
					else if (p[n].len <= p[n+1].len) {
//						merge(p[n].base, p[n+1].base, p[n+1].base + p[n+1].len, tempBufferPair.first, comp);
						
						mergeRuns(n, comp);
					}
					else
						break;
				}
			}
			
			void forceCollapse(Compare comp) {
				Slice_t* const p = pendingRuns;
				
				while (pendingCount > 1) {
					int n = pendingCount - 2;
					
					if (n > 0 && p[n-1].len < p[n+1].len)
						--n;

					mergeRuns(n, comp);
				}
			}
			
		private:
			using value_type = typename std::iterator_traits<Iter>::value_type;
			using Slice_t = Slice<Iter, Size>;
			
			// 85 is large enough, good for an array with 2**64 elements.
			static const int MAX_MERGE_PENDING = 85;
			
			/* A stack of n pending runs yet to be merged. Run #i starts at
			 * address base[i] and extends for len[i] elements. It's always
			 * true (so long as the indices are in bounds) that
			 *
			 * pending[i].base + pending[i].len == pending[i+1].base
			 *
			 * so we could cut the storage for this, but it's a minor amount,
			 * and keeping all the info explicit simplifies the code.
			 */
			int pendingCount;
			Slice_t pendingRuns[MAX_MERGE_PENDING];
			
			std::pair<value_type*, std::ptrdiff_t> tempBufferPair;
		};
		
		template<typename Size>
		Size compute_minrun(Size len) {
			Size r = 0;
			
			while (len >= 64) {
				r |= len & 1;
				len >>= 1;
			}
			return len + r;
		}
		
		template<typename RandomIt, typename Compare, typename Size = typename std::iterator_traits<RandomIt>::difference_type>
		Size count_run_len(RandomIt first, RandomIt last, Compare comp, bool *outDescending)
		{
			*outDescending = false;
			
			RandomIt iter = first;
			++iter;
			if (iter == last)
				return 1;
			
			Size runLen = 2;
			RandomIt prevIter = iter;
			
			if (comp(*iter, *first)) {
				*outDescending = true;
				
				for (++iter; iter < last; prevIter = iter, ++iter, ++runLen) {
					if (comp(*iter, *prevIter)) {
					} else
						break;
				}
			} else {
				for (++iter; iter < last; prevIter = iter, ++iter, ++runLen) {
					if (comp(*iter, *prevIter))
						break;
						
				}
			}
			
			return runLen;
		}
		
		template<typename RandomIt>
		void reverse_slice(RandomIt first, RandomIt last) {
			--last;
			
			while (first < last) {
				std::iter_swap(first, last);
				
				++first;
				--last;
			}
		}
	}
	
	template<typename RandomIt, typename Compare>
    void timsort(RandomIt first, RandomIt last, Compare comp) {
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
		
		DiffType remaining = length;
		DiffType minrun = compute_minrun(remaining);
		RandomIt curIter = first;
		
		MergeState<RandomIt, DiffType, Compare> mergeState { length };
		
		do {
			bool descending;
			
			DiffType runLen = count_run_len(curIter, curIter + remaining, comp, &descending);
			
			if (descending)
				reverse_slice(curIter, curIter + runLen);
			
			/* If short, extend to min(minrun, remaining). */
			if (runLen < minrun) {
				const DiffType force = remaining <= minrun ? remaining : minrun;
				
				insertion_sort(curIter, curIter + force, comp);
				runLen = force;
			}
			
			// Push run onto stack and merge optionally
			mergeState.push_run(curIter, runLen);
			mergeState.checkCollapse(comp);
			
			curIter += runLen;
			remaining -= runLen;
		} while (remaining > 0);
		
		mergeState.forceCollapse(comp);
    }
	
	template<typename RandomIt>
	void timsort(RandomIt first, RandomIt last) {
		timsort(first, last, std::less<typename RandomIt::value_type>());
	}
	
}

#endif // AlgoAndData_sort_timsort_h
