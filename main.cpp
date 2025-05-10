#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

#include "./LRU/LruCache.h"
#include "./LRU/KLruCache.h"

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// 辅助函数：打印结果
void printResults(const std::string& testName, int capacity,
                 const std::vector<int>& get_operations,
                 const std::vector<int>& hits) {
    std::cout << "Cache Capacity: " << capacity << std::endl;
    std::cout << "LRU - Hit Rate: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[0] / get_operations[0]) << "%" << std::endl;

}

void testHotDataAccess() {
    std::cout << "\n=== Senrio 1: Hot point data access test===" << std::endl;

    const int CAPACITY = 50;  // 增加缓存容量
    const int OPERATIONS = 500000;  // 增加操作次数
    const int HOT_KEYS = 20;   // 增加热点数据的数量
    const int COLD_KEYS = 5000;

    LruCache<int, std::string> lru(CAPACITY);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> hits(1, 0);
    std::vector<int> get_operations(1, 0);

    // 先进行一系列put操作
    for (int op = 0; op < OPERATIONS; ++op) {
        int key;
        if (op % 100 < 70) {  // 70%热点数据
            key = gen() % HOT_KEYS;
        } else {  // 30%冷数据
            key = HOT_KEYS + (gen() % COLD_KEYS);
        }
        std::string value = "value" + std::to_string(key);
        lru.put(key, value);
    }

    // 然后进行随机get操作
    for (int get_op = 0; get_op < OPERATIONS; ++get_op) {
        int key;
        if (get_op % 100 < 70) {  // 70%概率访问热点
            key = gen() % HOT_KEYS;
        } else {  // 30%概率访问冷数据
            key = HOT_KEYS + (gen() % COLD_KEYS);
        }

        std::string result;
        get_operations[0]++;
        if (lru.get(key, result)) {
            hits[0]++;
        }
    }

    printResults("Hot point data access test", CAPACITY, get_operations, hits);
}


int main() {
    testHotDataAccess();
    return 0;
}