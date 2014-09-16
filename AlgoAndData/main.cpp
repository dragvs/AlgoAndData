//
//  main.cpp
//  AlgoAndData
//
//  Created by Vladimir Shishov on 03/01/14.
//  Copyright (c) 2014 Vladimir Shishov. All rights reserved.
//


#include "sort/sort.h"
#include "data/hash_map.h"
#include "data/twothree_tree.h"

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
#include <thread>
#include <future>
#include <string>
#include <utility>


struct Data {
	int value;
	int index;
    
    Data() {}
    explicit Data(int value) : value(value), index(0) {}
    Data(int value, int index) : value(value), index(index) {}
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

bool operator==(const Data& left, const Data& right)
{
	return left.value == right.value && left.index == right.index;
}
bool operator<(const Data& left, const Data& right)
{
    return left.value < right.value;
}

//
// Helpers
//

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
bool isSorted(ForwardIt sortedFirst, ForwardIt sortedLast, ForwardIt originalFirst, ForwardIt originalLast, Compare comp) {
//	return std::is_sorted(first, last, comp);
	
	using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
	std::vector<ValueType> tempSortedVec(originalFirst, originalLast);
	std::sort(tempSortedVec.begin(), tempSortedVec.end(), comp);

	auto mismatchPair = std::mismatch(sortedFirst, sortedLast, tempSortedVec.begin());
	return mismatchPair.first == sortedLast && mismatchPair.second == tempSortedVec.end();
}

template<typename ForwardIt, typename Compare>
bool isStableSorted(ForwardIt sortedFirst, ForwardIt sortedLast, ForwardIt originalFirst, ForwardIt originalLast, Compare comp) {
	using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
	std::vector<ValueType> tempSortedVec(originalFirst, originalLast);
	std::stable_sort(tempSortedVec.begin(), tempSortedVec.end(), comp);
	
	auto mismatchPair = std::mismatch(sortedFirst, sortedLast, tempSortedVec.begin());
	return mismatchPair.first == sortedLast && mismatchPair.second == tempSortedVec.end();
}

//
// Generators
//

std::function<int()> createIntUniformGenerator(int maxValue) {
    std::random_device rd; // for seed
	std::minstd_rand engine(rd());
	std::uniform_int_distribution<int> uniform_dist(0, maxValue);
    
    return [uniform_dist, engine]() mutable { return uniform_dist(engine); };
}

int generateRandomInt(int maxValue) {
    auto generator = createIntUniformGenerator(maxValue);
    return generator();
}

std::vector<int> generateRandomInput(int inputSize, int maxValue) {
	std::vector<int> inputVec;
	inputVec.reserve(inputSize);
	
    auto generator = createIntUniformGenerator(maxValue);
    
	for (int i = 0; i < inputSize; ++i) {
		inputVec.push_back(generator());
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

template<typename Iter>
void runBenchmark(GeneratorFunc generator, Iter begFuncContainer, Iter endFuncContainer) {
	using DataType = int;
	using DataVec = std::vector<DataType>;
	using Compare = std::less<DataType>;
	
	std::vector<int> inputVecSizes { 10, 127, 1005, 10123, 15000, 101137, 400000, 700000, 1000013 };
	
	for (int inputSize : inputVecSizes) {
		DataVec inputVec = generator(inputSize);
		
		int i = 0;
		std::cout << inputSize << "\t";
		
		for (Iter funcIter = begFuncContainer; funcIter < endFuncContainer; ++funcIter) {
			// TODO Remove
			if ((i == 0 || i == 1) && inputSize > 50000) {
				++i;
				std::cout << "" << "\t";
				continue;
			}
			
			auto duration =
			runWithTimerAndMedian<DataType, Compare>(inputVec, 10, *funcIter);
			
//			std::cout << "Duration: " << duration.count() << "ms" << std::endl;
			std::cout << duration.second.count() << "\t";
			std::cout << std::flush;
			
			++i;
		}
		
		std::cout << std::endl;
	}
}

void runBenchmark(GeneratorFunc generator) {
	using DataType = int;
	using DataVec = std::vector<DataType>;
	using Compare = std::less<DataType>;
	using DataSortFunc = SortFunc<DataVec::iterator, Compare>;
	
	DataSortFunc std_sort = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) { std::sort(begin, end, comp); };
//	DataSortFunc radix_sort10 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) { lab::radix_sort(begin, end); };
//	DataSortFunc radix_sort8 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
//		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 8>(begin, end);
//	};
	DataSortFunc radix_sort1024 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 1024>(begin, end);
	};
	
	std::array<DataSortFunc, 10> sortAlgoArr {{
		lab::selection_sort<DataVec::iterator, Compare>,
		lab::insertion_sort<DataVec::iterator, Compare>,
		lab::shell_sort<DataVec::iterator, Compare>,
		lab::quick_sort<DataVec::iterator, Compare>,
		lab::merge_sort<DataVec::iterator, Compare>,
		lab::heap_sort<DataVec::iterator, Compare>,
		std_sort,
//		radix_sort10,
//		radix_sort8,
		radix_sort1024,
		lab::intro_sort<DataVec::iterator, Compare>,
		lab::timsort<DataVec::iterator, Compare>
	}};
	
	runBenchmark(generator, std::begin(sortAlgoArr), std::end(sortAlgoArr));
}

void runRadixSortBenchmark() {
	using DataType = int;
	using DataVec = std::vector<DataType>;
	using Compare = std::less<DataType>;
	using DataSortFunc = SortFunc<DataVec::iterator, Compare>;

	DataSortFunc radix_sort8 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 8>(begin, end);
	};
	DataSortFunc radix_sort10 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) { lab::radix_sort(begin, end); };
	DataSortFunc radix_sort64 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 64>(begin, end);
	};
	DataSortFunc radix_sort100 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 100>(begin, end);
	};
	DataSortFunc radix_sort1000 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 1000>(begin, end);
	};
	DataSortFunc radix_sort1024 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 1024>(begin, end);
	};
	DataSortFunc radix_sort10000 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 10000>(begin, end);
	};
	DataSortFunc radix_sort16384 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
		lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 16384>(begin, end);
	};
	
	std::array<DataSortFunc, 8> sortAlgoArr {{
		radix_sort8,
		radix_sort10,
		radix_sort64,
		radix_sort100,
		radix_sort1000,
		radix_sort1024,
		radix_sort10000,
		radix_sort16384
	}};
	
	auto generator1 = [](int inputSize) { return generateRandomInput(inputSize, inputSize); };
	auto generator2 = [](int inputSize) { return generateRandomInput(inputSize, (int)(3 + 0.00097f*(inputSize - 10))); };
	
	std::cout << "--- Random simple ---" << std::endl;
	runBenchmark(generator1, std::begin(sortAlgoArr), std::end(sortAlgoArr));
	
    std::this_thread::sleep_for( std::chrono::milliseconds{ 30000 } );
	
	std::cout << "--- Random few unique ---" << std::endl;
	runBenchmark(generator2, std::begin(sortAlgoArr), std::end(sortAlgoArr));
}

void runStabilityCheck() {
	using DataVec = std::vector<Data>;
	using DataList = std::list<Data>;
	using Iterator = DataVec::iterator;

	std::initializer_list<Data> values { {5,0}, {1,1}, {1,2}, {10,0}, {100,1}, {7,0}, {3,0}, {12,0}, {13,0}, {14,0}, {100,2}  };
//	std::initializer_list<Data> values { {2,1}, {2,2}, {1,1} };
	
	DataVec inputData(values);
	DataVec testInputData(inputData);
//	DataList testInputData(values);
	DataComparison comparison;
	

	auto sortAlgo = [](Iterator begin, Iterator end, decltype(comparison) comp) {
//		lab::selection_sort(begin, end, comp);
//		lab::insertion_sort(begin, end, comp);
//		lab::shell_sort(begin, end, comp);
//		lab::quick_sort(begin, end, comp);
//		lab::merge_sort(begin, end, comp);
//		lab::heap_sort(begin, end, comp);
//		std::sort(begin, end, comp);
		lab::radix_sort<Iterator, DataKeyAccessor>(begin, end);
	};
	
	sortAlgo(testInputData.begin(), testInputData.end(), comparison);
	
	if (!isStableSorted(testInputData.begin(), testInputData.end(), inputData.begin(), inputData.end(), comparison)) {
		std::cout << "#SORT ERROR" << std::endl;
		print(testInputData.begin(), testInputData.end());
	} else {
		std::cout << "#SORT OK" << std::endl;
	}
}

void runSortCorrectnessCheck() {
	using DataType = int;
	using DataVec = std::vector<DataType>;
	using Compare = std::less<DataType>;
	using DataSortFunc = SortFunc<DataVec::iterator, Compare>;
	
	while (true) {
//		DataVec inputVec = generateRandomInput(10, 5);
//		DataVec inputVec = generateRandomInput(15113, 100);
//		DataVec inputVec = generateRandomInput(1000113, 100);
		DataVec inputVec = generateRandomInput(1000013, 1000013);
//		DataVec inputVec = generatePartiallySorted(1000013, 1000013, 0.9f, std::less<int>{});
//		DataVec inputVec = generateSorted(1000013, 1000013, std::greater<int>{});
//		DataVec inputVec = { 3, 4, 3, 5, 1, 2, 4, 2, 0, 3 };
		
		DataSortFunc radix_sort8 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 8>(begin, end);
		};
		DataSortFunc radix_sort10 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort(begin, end);
		};
		DataSortFunc radix_sort64 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 64>(begin, end);
		};
		DataSortFunc radix_sort100 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 100>(begin, end);
		};
		DataSortFunc radix_sort1024 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 1024>(begin, end);
		};
		DataSortFunc radix_sort16384 = [](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
			lab::radix_sort<DataVec::iterator, lab::DefaultKeyAccessor<DataVec::value_type>, 16384>(begin, end);
		};
		
		DataVec testInputVec(inputVec);
		auto sortAlgo = [&](DataVec::iterator begin, DataVec::iterator end, Compare comp) {
//			lab::selection_sort(begin, end, comp);
//			lab::insertion_sort(begin, end, comp);
//			lab::shell_sort(begin, end, comp);
//			lab::quick_sort(begin, end, comp);
//			lab::merge_sort(begin, end, comp);
//			lab::heap_sort(begin, end, comp);
//			std::sort(begin, end, comp);
//			lab::intro_sort(begin, end, comp);
			lab::timsort(begin, end, comp);
			
//			radix_sort8(begin, end, comp);
//			radix_sort10(begin, end, comp);
//			radix_sort64(begin, end, comp);
//			radix_sort100(begin, end, comp);
//			radix_sort1000(begin, end, comp);
//			radix_sort1024(begin, end, comp);
//			radix_sort16384(begin, end, comp);
		};
		
		Compare comparison;
		auto duration =
			runWithTimer([&]() { sortAlgo(testInputVec.begin(), testInputVec.end(), comparison); });
		std::cout << "Duration: " << duration.count() << "ms" << std::endl;
		
		if (!isSorted(testInputVec.begin(), testInputVec.end(), inputVec.begin(), inputVec.end(), comparison)) {
			print(inputVec.begin(), inputVec.end());
			std::cout << "#SORT ERROR" << std::endl;
			print(testInputVec.begin(), testInputVec.end());
		} else {
			std::cout << "#SORT OK" << std::endl;
		}
	}
}

void testHashMap() {
    using DataMap = lab::hash_map<std::string, Data>;
    DataMap testMap;
    Data d1 { 1, 1 };
    Data d2 { 2, 2 };
    Data d3 { 3, 3 };
    testMap["1"] = d1;
    testMap["2"] = d2;
    testMap["3"] = d3;
    
    assert(testMap["1"] == d1);
    assert(testMap["2"] == d2);
    assert(testMap["3"] == d3);
    
    for (int i = 4; i < 104; ++i) {
        testMap[std::to_string(i)] = Data { i, i };
    }
    
    // Iteration and size test
    auto it = testMap.begin();
    auto endIt = testMap.end();
    int count = 0;
    
    while (it != endIt) {
        auto& pair = *it;
        
        assert(pair.second.value == pair.second.index);
        assert(pair.first == std::to_string(pair.second.index));
        
        ++count;
        ++it;
    }
    
    assert(count == testMap.size());
    
    // Insert test
    Data d150 { 150, 150 };
    auto testPair1 = std::make_pair("1", d1);
    auto testPair2 = std::make_pair("150", d150);
    std::pair<DataMap::iterator, bool> res1 = testMap.insert(testPair1);
    std::pair<DataMap::iterator, bool> res2 = testMap.insert(testPair2);
    
    assert(res1.second == false);
    assert(res1.first->first == "1");
    assert(res1.first->second == d1);
    assert(res2.second == true);
    assert(res2.first->first == "150");
    assert(res2.first->second == d150);
    
    // Erasure
    
    for (int i = 0; i < 104; i+=2) {
        testMap.erase(std::to_string(i));
    }
    for (int i = 50; i >= 0; --i) {
        testMap[std::to_string(i)] = Data { i, i };
    }
    auto cIt = testMap.cbegin();
    auto cEndIt = testMap.cend();
    count = 0;
    
    while (cIt != cEndIt) {
        auto& pair = *cIt;
        
        assert(pair.second.value == pair.second.index);
        assert(pair.first == std::to_string(pair.second.index));
        
        ++count;
        ++cIt;
    }
    
    assert(count == testMap.size());
    
    assert(testMap.find("33") != testMap.end());
    assert(testMap.count("44") == 1);
    
    testMap.erase(++testMap.cbegin());
}

//

template<typename T, typename K, typename U>
void assertSortedUniques(const lab::twothree_tree<T, K, U>& tree, int count) {
    std::vector<T> elemsVec { tree.cbegin(), tree.cend() };
    
    assert(elemsVec.size() == count);
    assert(std::is_sorted(elemsVec.cbegin(), elemsVec.cend()));
    assert(std::adjacent_find(elemsVec.cbegin(), elemsVec.cend()) == elemsVec.cend());
}

template<typename T>
void assertNodeValueEqual(const std::pair<T, bool> nodeValue, T value) {
    assert(nodeValue == std::make_pair(value, true));
}

template<typename T>
void assertNodeValueEmpty(const std::pair<T, bool> nodeValue) {
    assert(nodeValue.second == false);
}

void testTwoThreeTree() {
    using DataTree = lab::twothree_tree<int>;
    
    // Insertion tests
    // # oneElement
    {
        DataTree testTree;
        testTree.insert(10);
        assert(testTree.value(0) == std::make_pair(10, true));
        assert(testTree.value(1).second == false);
        assert(testTree.child(0).exists() == false);
        assert(testTree.child(1).exists() == false);
    }
    
    // # twoElements
    {
        // 10 15
        DataTree testTree;
        testTree.insert(10);
        testTree.insert(15);
        assert(testTree.value(0) == std::make_pair(10, true));
        assert(testTree.value(1) == std::make_pair(15, true));
        assert(testTree.child(0).exists() == false);
        assert(testTree.child(1).exists() == false);
        assertSortedUniques(testTree, 2);
    }
    
    // # twoLevelsSimple
    {
        //       10
        // 4  8     15  20
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 8 });
        assert(testTree.value(0) == std::make_pair(10, true));
        assert(testTree.value(1).second == false);
        assert(testTree.child(0).value(0) == std::make_pair(4, true));
        assert(testTree.child(0).value(1) == std::make_pair(8, true));
        assert(testTree.child(1).value(0) == std::make_pair(15, true));
        assert(testTree.child(1).value(1) == std::make_pair(20, true));
        assert(testTree.child(2).exists() == false);
        assertSortedUniques(testTree, 5);
    }

    // # twoLevelsFilled
    {
        //       10 20
        //    /    |    \
        // 4  7  15 17   23 25
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 7, 23, 17, 25 });
        assert(testTree.value(0) == std::make_pair(10, true));
        assert(testTree.value(1) == std::make_pair(20, true));
        assert(testTree.child(0).value(0) == std::make_pair(4, true));
        assert(testTree.child(0).value(1) == std::make_pair(7, true));
        assert(testTree.child(1).value(0) == std::make_pair(15, true));
        assert(testTree.child(1).value(1) == std::make_pair(17, true));
        assert(testTree.child(2).value(0) == std::make_pair(23, true));
        assert(testTree.child(2).value(1) == std::make_pair(25, true));
        assertSortedUniques(testTree, 8);
    }
    
    // # threeLevelsSingle
    {
        //       4
        //    2      6
        // 1    3  5    7
        DataTree testTree;
        testTree.insert({ 4, 3, 6, 7, 5, 2, 1 });
        assert(testTree.value(0) == std::make_pair(4, true));
        assert(testTree.child(0).value(0) == std::make_pair(2, true));
        assert(testTree.child(0).child(0).value(0) == std::make_pair(1, true));
        assert(testTree.child(0).child(1).value(0) == std::make_pair(3, true));
        assert(testTree.child(1).value(0) == std::make_pair(6, true));
        assert(testTree.child(1).child(0).value(0) == std::make_pair(5, true));
        assert(testTree.child(1).child(1).value(0) == std::make_pair(7, true));
        assertSortedUniques(testTree, 7);
    }
    
    // # twoLevelsFilledInsertLeft
    {
        //            10
        //        /        \
        //      4            20
        //   /    \       /      \
        //  2      7    15 17    23 25
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 7, 23, 17, 25 });
        testTree.insert(2);
        assert(testTree.value(0) == std::make_pair(10, true));
        assert(testTree.value(1).second == false);
        assert(testTree.child(0).value(0) == std::make_pair(4, true));
        assert(testTree.child(0).value(1).second == false);
        assert(testTree.child(0).child(0).value(0) == std::make_pair(2, true));
        assert(testTree.child(0).child(1).value(0) == std::make_pair(7, true));
        
        assert(testTree.child(1).value(0) == std::make_pair(20, true));
        assert(testTree.child(1).value(1).second == false);
        assert(testTree.child(1).child(0).value(0) == std::make_pair(15, true));
        assert(testTree.child(1).child(0).value(1) == std::make_pair(17, true));
        assert(testTree.child(1).child(1).value(0) == std::make_pair(23, true));
        assert(testTree.child(1).child(1).value(1) == std::make_pair(25, true));
        assertSortedUniques(testTree, 9);
    }
    
    // # twoLevelsFilledInsertMiddle
    {
        //           16
        //        /      \
        //      10          20
        //    /    \      /    \
        // 4  7     15   17   23 25
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 7, 23, 17, 25 });
        testTree.insert(16);
        assertNodeValueEqual(testTree.value(0), 16);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(0).value(0), 10);
        assertNodeValueEqual(testTree.child(0).child(0).value(0), 4);
        assertNodeValueEqual(testTree.child(0).child(0).value(1), 7);
        assertNodeValueEqual(testTree.child(0).child(1).value(0), 15);
        
        assertNodeValueEqual(testTree.child(1).value(0), 20);
        assertNodeValueEqual(testTree.child(1).child(0).value(0), 17);
        assertNodeValueEqual(testTree.child(1).child(1).value(0), 23);
        assertNodeValueEqual(testTree.child(1).child(1).value(1), 25);
        assertSortedUniques(testTree, 9);
    }
    
    // # twoLevelsFilledInsertRight
    {
        //             20
        //          /      \
        //       10          25
        //    /     \      /    \
        // 4  7   15 17   23     27
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 7, 23, 17, 25 });
        testTree.insert(27);
        assertNodeValueEqual(testTree.value(0), 20);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(0).value(0), 10);
        assertNodeValueEqual(testTree.child(0).child(0).value(0), 4);
        assertNodeValueEqual(testTree.child(0).child(0).value(1), 7);
        assertNodeValueEqual(testTree.child(0).child(1).value(0), 15);
        assertNodeValueEqual(testTree.child(0).child(1).value(1), 17);
        
        assertNodeValueEqual(testTree.child(1).value(0), 25);
        assertNodeValueEqual(testTree.child(1).child(0).value(0), 23);
        assertNodeValueEqual(testTree.child(1).child(1).value(0), 27);
        assertSortedUniques(testTree, 9);
    }
    
    // # eraseTest1 - removing last value from tree
    {
        DataTree testTree;
        testTree.insert(16);
        testTree.erase(16);
        assert(testTree.empty());
    }
    
    // # eraseTest2 - removing last but one value from tree
    {
        DataTree testTree;
        testTree.insert({ 16, 17 });
        testTree.erase(16);
        assertNodeValueEqual(testTree.value(0), 17);
        assertNodeValueEmpty(testTree.value(1));
        assertSortedUniques(testTree, 1);
    }
    
    // # eraseTest3 - case 1, simple
    {
        //    5
        //  3   7
        DataTree testTree;
        testTree.insert({ 3, 5, 7 });
        testTree.erase(7);
        assertNodeValueEqual(testTree.value(0), 3);
        assertNodeValueEqual(testTree.value(1), 5);
        assertSortedUniques(testTree, 2);
    }
    
    // # eraseTest4 - case 1b+1a
    {
        //       4
        //    2      6
        // 1    3  5    7
        
        //        4 6
        //      /  |  \
        //   1 2   5    7
        DataTree testTree;
        testTree.insert({ 4, 3, 6, 7, 5, 2, 1 });
        testTree.erase(3);
        assertNodeValueEqual(testTree.value(0), 4);
        assertNodeValueEqual(testTree.value(1), 6);
        assertNodeValueEqual(testTree.child(0).value(0), 1);
        assertNodeValueEqual(testTree.child(0).value(1), 2);
        assertNodeValueEqual(testTree.child(1).value(0), 5);
        assertNodeValueEqual(testTree.child(2).value(0), 7);
        assertSortedUniques(testTree, 6);
    }
    
    // # eraseTest5 - case 2, simple
    {
        //     5
        //  3    7 9
        DataTree testTree;
        testTree.insert({ 3, 5, 7, 9 });
        testTree.erase(3);
        assertNodeValueEqual(testTree.value(0), 7);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(0).value(0), 5);
        assertNodeValueEqual(testTree.child(1).value(0), 9);
        assertSortedUniques(testTree, 3);
    }
    
    // # eraseTest6 - case 2a
    {
        //         4
        //    2        6,9
        // 1    3   5   7  11
        
        //         6
        //     4         9
        // 1,2   5     7   11
        DataTree testTree;
        testTree.insert({ 4, 3, 6, 7, 5, 2, 1, 9, 11 });
        testTree.erase(3);
        assertNodeValueEqual(testTree.value(0), 6);
        assertNodeValueEqual(testTree.child(0).value(0), 4);
        assertNodeValueEqual(testTree.child(0).child(0).value(0), 1);
        assertNodeValueEqual(testTree.child(0).child(0).value(1), 2);
        assertNodeValueEqual(testTree.child(0).child(1).value(0), 5);
        assertNodeValueEqual(testTree.child(1).value(0), 9);
        assertNodeValueEqual(testTree.child(1).child(0).value(0), 7);
        assertNodeValueEqual(testTree.child(1).child(1).value(0), 11);
        assertSortedUniques(testTree, 8);
    }

    // # eraseTest7 - case 3, erase from left
    {
        //              16,24
        //    8           21          29
        // 1     12    20    22     28   30
        DataTree testTree;
        testTree.insert({ 16, 12, 24, 28, 20, 8, 1 });
        testTree.insert({ 21, 29, 22, 30 });
        //               24
        //       16,21          29
        //   1,8   20  22     28   30
        testTree.erase(12);
        assertNodeValueEqual(testTree.value(0), 24);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(0).value(0), 16);
        assertNodeValueEqual(testTree.child(0).value(1), 21);
        assertNodeValueEqual(testTree.child(0).child(0).value(0), 1);
        assertNodeValueEqual(testTree.child(0).child(0).value(1), 8);
        assertSortedUniques(testTree, 10);
    }
    
    // # eraseTest8 - case 3, erase from middle
    {
        //              16,24
        //    8           21          29
        // 1     12    20    22     28   30
        DataTree testTree;
        testTree.insert({ 16, 12, 24, 28, 20, 8, 1 });
        testTree.insert({ 21, 29, 22, 30 });
        //                24
        //     8,16              29
        // 1    12  20,21     28    30
        testTree.erase(22);
        assertNodeValueEqual(testTree.value(0), 24);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(0).value(0), 8);
        assertNodeValueEqual(testTree.child(0).value(1), 16);
        assertNodeValueEqual(testTree.child(0).child(2).value(0), 20);
        assertNodeValueEqual(testTree.child(0).child(2).value(1), 21);
        assertSortedUniques(testTree, 10);
    }
    
    // # eraseTest9 - case 3, erase from right
    {
        //              16,24
        //    8           21          29
        // 1     12    20    22     28   30
        DataTree testTree;
        testTree.insert({ 16, 12, 24, 28, 20, 8, 1 });
        testTree.insert({ 21, 29, 22, 30 });
        //           16
        //    8               21,24
        // 1     12       20   22   29,30
        testTree.erase(28);
        assertNodeValueEqual(testTree.value(0), 16);
        assertNodeValueEmpty(testTree.value(1));
        assertNodeValueEqual(testTree.child(1).value(0), 21);
        assertNodeValueEqual(testTree.child(1).value(1), 24);
        assertNodeValueEqual(testTree.child(1).child(0).value(0), 20);
        assertNodeValueEqual(testTree.child(1).child(1).value(0), 22);
        assertNodeValueEqual(testTree.child(1).child(2).value(0), 29);
        assertNodeValueEqual(testTree.child(1).child(2).value(1), 30);
        assertSortedUniques(testTree, 10);
    }
    
    
    // ______X______
    
    // # eraseTest3 - ??
    {
        //              16
        //         /         \
        //      10            20 24
        //    /    \       /    |     \
        // 4  7     15   17   22 23   25 27
        
        //              17
        //         /         \
        //      10            20 24
        //    /    \       /    |     \
        // 4  7     10    _   22 23   25 27
        DataTree testTree;
        testTree.insert({ 10, 15, 4, 20, 7, 23, 17, 25, 16, 24, 22, 27 });
        testTree.erase(16);
        assertSortedUniques(testTree, 11);
    }
    
    // #3 Random insertions and erasures
    while (true) {
        DataTree testTree;
        std::vector<int> randomIntVec = generateRandomInput(30, 30); // 1000000
        int uniquesInserted = 0;
        
        for (auto& value : randomIntVec) {
            auto result = testTree.insert(value);
            
            if (result.second)
                ++uniquesInserted;
        }
        assertSortedUniques(testTree, uniquesInserted);
        
        int valueToErase = randomIntVec.at(generateRandomInt(static_cast<int>(randomIntVec.size()-1)));
        testTree.erase(valueToErase);
        assertSortedUniques(testTree, uniquesInserted-1);
    }
}

int main2(int argc, const char * argv[])
{
//    testHashMap();
    testTwoThreeTree();
    return 0;
    
//	runRadixSortBenchmark();
	
//	runBenchmark([](int inputSize) { return generateRandomInput(inputSize, inputSize); });
//	runBenchmark([](int inputSize) { return generateRandomInput(inputSize, (int)(3 + 0.00097f*(inputSize - 10))); });
//	runBenchmark([](int inputSize) { return generatePartiallySorted(inputSize, inputSize, 0.9f, std::less<int>{}); });
//	runBenchmark([](int inputSize) { return generateSorted(inputSize, inputSize, std::greater<int>{}); });
//	return 0;

//	runSortCorrectnessCheck();
//	runStabilityCheck();
//	return 0;
}
