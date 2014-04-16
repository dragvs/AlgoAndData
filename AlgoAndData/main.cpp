//
//  main.cpp
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//


#include "sort/sort.h"

#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <algorithm>
#include <iterator>
#include <functional>
#include <initializer_list>
#include <random>
#include <chrono>
#include <future>


struct Data {
	int value;
	int index;
};

struct DataKeyAccessor {
	int operator()(const Data& data) { return data.value; }
};
struct DataComparison {
	bool operator()(const Data& left, const Data& right) {
		return left.value < right.value;
	}
};

std::ostream& operator<<( std::ostream& os, const Data& data) {
	std::cout << data.value;
	
	if (data.index != 0)
		std::cout << "(" << data.index << ")";
	
	return os;
}

template<typename ForwardIt>
void print(ForwardIt first, ForwardIt last) {
	bool isFirst = true;
	
	for (ForwardIt it = first; it < last; ++it) {
		if (!isFirst)
			std::cout << ", ";
		
		std::cout << *it;
		
		if (isFirst)
			isFirst = false;
	}
	std::cout << std::endl;
}

template<typename ForwardIt, typename Compare>
bool isSorted(ForwardIt first, ForwardIt last, Compare comp) {
	// TODO Use std::mismatch
	
	return std::is_sorted(first, last, comp);
}

//
// Generators
//

std::vector<int> generateRandomInput(int inputSize, int maxValue) {
	std::vector<int> inputVec;
	inputVec.reserve(inputSize);
	
	std::random_device rd; // for seed
	std::minstd_rand engine(rd());
	std::uniform_int_distribution<int> uniform_dist(0, maxValue);
	
	for (int i = 0; i < inputSize; ++i) {
		inputVec.push_back(uniform_dist(engine));
	}
	
	return inputVec;
}

template<typename Compare>
std::vector<int> generatePartiallySorted(int inputSize, int maxValue, float sortedPart, Compare compare) {
	std::vector<int> inputVec = generateRandomInput(inputSize, maxValue);
	std::partial_sort(inputVec.begin(), inputVec.begin() + inputSize*sortedPart, inputVec.end(), compare);
	return inputVec;
}

template<typename Compare>
std::vector<int> generateSorted(int inputSize, int maxValue, Compare compare) {
	std::vector<int> inputVec = generateRandomInput(inputSize, maxValue);
	std::sort(inputVec.begin(), inputVec.end(), compare);
	return inputVec;
}

//
// Runners
//

// Obsolete
template<typename RandomIt, typename Compare>
void runSort(RandomIt first, RandomIt last, Compare comparison) {
//	lab::insertion_sort(first, last, comparison);
//	lab::selection_sort(first, last, comparison);
//	lab::merge_sort(first, last, comparison);
	lab::quick_sort(first, last, comparison);
	
	if (!isSorted(first, last, comparison)) {
		std::cout << "#SORT ERROR" << std::endl;
		print(first, last);
	} else {
		std::cout << "#SORT OK" << std::endl;
	}
}

using time_duration = std::chrono::duration<double, std::milli>;

template<typename RandomIt, typename Compare>
using SortFunc = std::function<void (RandomIt, RandomIt, Compare)>;

time_duration runWithTimer(std::function<void ()> action) {
	using clock = std::chrono::high_resolution_clock;
	using time_point = std::chrono::time_point<clock>;
	
	time_point startTime = clock::now();
	action();
	time_point endTime = clock::now();
	
	time_duration duration = endTime - startTime;
	return duration;
}

std::pair<bool, time_duration> runWithTimerAndTimout(int timeoutMillis, std::function<void ()> action) {
	std::function<time_duration ()> timerFunc = [&]() { return runWithTimer(action); };
	
	auto future = std::async(timerFunc);
	
	std::future_status status = future.wait_for(std::chrono::milliseconds(timeoutMillis));
	
	if (status == std::future_status::deferred) {
		std::cout << "DEFERRED!" << std::endl; // The function to calculate the result has not been started yet
	}
	else if (status == std::future_status::timeout) {
		return std::make_pair<bool, time_duration>(false, std::chrono::seconds(9999));
	}
	else if (status == std::future_status::ready) {
		return std::make_pair<bool, time_duration>(true, future.get());
	}
	
	abort();
}

template<typename DataType, typename Compare, typename Container = std::vector<DataType>>
std::pair<bool, time_duration> runWithTimerAndMedian(const Container testData, int runTimes,
									SortFunc<typename Container::iterator, Compare> action)
{
	std::vector<time_duration> durationsVec;

	// TODO Add sort correctness check
	const int TimeoutMillis = 3000;
	
	for (int i = 0; i < runTimes; ++i) {
		std::vector<DataType> runTestData(testData);
		Compare comparison;
		
		auto runAction = [&]() { action(runTestData.begin(), runTestData.end(), comparison); };
		
		auto runTimePair = runWithTimerAndTimout(TimeoutMillis, runAction);
		
		if (runTimePair.first)
			durationsVec.push_back(runTimePair.second);
	}
	
	if (durationsVec.size() < runTimes*2/3) {
		// not enough run data
		return std::make_pair<bool, time_duration>(false, std::chrono::seconds(9999));
	}
	
	std::sort(durationsVec.begin(), durationsVec.end());
	return std::make_pair<bool, time_duration>(true, std::move(durationsVec[durationsVec.size()/2]));
}

using GeneratorFunc = std::function<std::vector<int> (int inputSize)>;

void runBenchmark(GeneratorFunc generator) {
	using DataType = int;
	using DataVec = std::vector<DataType>;
	using Compare = std::less<DataType>;
	using DataSortFunc = SortFunc<DataVec::iterator, Compare>;
	
	std::vector<int> inputVecSizes { 10, 127, 1005, 10123, 15000, 101137, 400000, 700000, 1000013 };
	
	DataSortFunc std_sort = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) { std::sort(begin, end, comp); };
	DataSortFunc radix_sort = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) { lab::radix_sort(begin, end); };
	
	std::array<DataSortFunc, 8> sortAlgoArr {{
		lab::selection_sort<DataVec::iterator, Compare>,
		lab::insertion_sort<DataVec::iterator, Compare>,
		lab::shell_sort<DataVec::iterator, Compare>,
		lab::quick_sort<DataVec::iterator, Compare>,
		lab::merge_sort<DataVec::iterator, Compare>,
		lab::heap_sort<DataVec::iterator, Compare>,
		std_sort,
		radix_sort
	}};
	
	for (int inputSize : inputVecSizes) {
		DataVec inputVec = generator(inputSize);

		int i = 0;
		std::cout << inputSize << "\t";
		
		for (auto& sortAlgoFunc : sortAlgoArr) {
			// TODO Remove
			if ((i == 0 || i == 1) && inputSize > 50000) {
				++i;
				std::cout << "" << "\t";
				continue;
			}
			
			auto duration =
				runWithTimerAndMedian<DataType, Compare>(inputVec, 10, sortAlgoFunc);
			
//			std::cout << "Duration: " << duration.count() << "ms" << std::endl;
			std::cout << duration.second.count() << "\t";
			std::cout << std::flush;
			
			++i;
		}
		
		std::cout << std::endl;
	}
}

int main(int argc, const char * argv[])
{
	runBenchmark([](int inputSize) { return generateRandomInput(inputSize, inputSize); });
//	runBenchmark([](int inputSize) { return generateRandomInput(inputSize, (int)(3 + 0.00097f*(inputSize - 10))); });
//	runBenchmark([](int inputSize) { return generatePartiallySorted(inputSize, inputSize, 0.9f, std::less<int>{}); });
//	runBenchmark([](int inputSize) { return generateSorted(inputSize, inputSize, std::greater<int>{}); });
	return 0;
	
	//
//	using DataVec = std::vector<Data>;
//	using DataList = std::list<Data>;
//	
//	std::initializer_list<Data> values { {5,0}, {1,1}, {1,2}, {10,0}, {100,1}, {7,0}, {3,0}, {12,0}, {13,0}, {14,0}, {100,2}  };
////	std::initializer_list<Data> values { {2,1}, {2,2}, {1,1} }; // stability check
//	DataVec testContainer(values);
////	DataList testContainer(values);
//	lab::radix_sort<DataVec::iterator, DataKeyAccessor>(testContainer.begin(), testContainer.end());
//
//	DataComparison comparison;
//	runSort(testContainer.begin(), testContainer.end(), comparison);
	
	using IntVec = std::vector<int>;
	std::less<int> comparison;
	
	while (true) {
//		IntVec inputVec = generateRandomInput(10, 5);
//		IntVec inputVec = generateRandomInput(15113, 100);
//		IntVec inputVec = generateRandomInput(1000113, 100);
		IntVec inputVec = generateRandomInput(1000013, 1000013);
//		IntVec inputVec = generatePartiallySorted(100000, 100000, 0.9f, std::less<int>{});
//		IntVec inputVec = generateSorted(1000013, 1000013, std::greater<int>{});
//		IntVec inputVec = { 3, 4, 3, 5, 1, 2, 4, 2, 0, 3 };
		
		IntVec testInputVec(inputVec);
		auto sortAlgo = [](IntVec::iterator begin, IntVec::iterator end, decltype(comparison) comp) {
//			lab::selection_sort(begin, end, comp);
//			lab::insertion_sort(begin, end, comp);
//			lab::shell_sort(begin, end, comp);
//			lab::quick_sort(begin, end, comp);
//			lab::merge_sort(begin, end, comp);
//			lab::heap_sort(begin, end, comp);
//			std::sort(begin, end, comp);
			lab::radix_sort(begin, end);
		};
		
		auto duration =
			runWithTimer([&]() { sortAlgo(testInputVec.begin(), testInputVec.end(), comparison); });
		std::cout << "Duration: " << duration.count() << "ms" << std::endl;
		
//		auto duration =
//			runWithTimerAndMedian<int, decltype(comparison)>(inputVec, 10, sortAlgo);
//		std::cout << "Duration: " << duration.second.count() << "ms" << std::endl;
		
		if (!isSorted(testInputVec.begin(), testInputVec.end(), comparison)) {
			print(inputVec.begin(), inputVec.end());
			std::cout << "#SORT ERROR" << std::endl;
			print(testInputVec.begin(), testInputVec.end());
		} else {
			std::cout << "#SORT OK" << std::endl;
		}
	}
	
    return 0;
}
