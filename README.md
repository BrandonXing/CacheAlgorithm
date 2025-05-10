# CacheAlgorithm

## 项目概述
本项目实现了一套缓存系统，系作者学习项目。本项目包含多种缓存淘汰策略的实现。所有实现均为线程安全，采用C++模板编程，支持任意键值类型。

## 缓存实现列表

### 1. LRU缓存 (LruCache)
- **文件**: `LruCache.h`
- **策略**: 最近最少使用(Least Recently Used)
- **特点**:
    - 双向链表+哈希表实现O(1)操作
    - 线程安全(使用std::mutex)
    - 支持基本操作: put/get/remove


### 2. K-LRU缓存 (KLruCache)
- **文件**: `KLruCache.h`
- **继承自**: `LruCache`
- **策略**: 基于访问频率的LRU改进
- **特点**:
    - 记录元素历史访问次数
    - 仅当访问次数≥K时才加入主缓存
    - 使用独立的历史记录LRU缓存
    - 参数可配置(K值、历史缓存容量)

### 3. 哈希分片K-LRU缓存 (HashKlruCache)
- **文件**: `HashKlruCache.h`
- **继承自**: `KLruCache`
- **策略**: 分片哈希+K-LRU
- **特点**:
    - 将缓存分片到多个LRU实例
    - 减少锁竞争，提高并发性能


### 4. LFU缓存 (LfuCache)
- **文件**: `LfuCache.h`
- **策略**: 最不经常使用(Least Frequently Used)
- **特点**:
    - 频率分层双向链表结构
    - 独立管理不同访问频率的节点
    - 自动维护最小频率(minFreq)


### 5. 老化LFU缓存 (AgedLfuCache)
- **文件**: `AgedLfuCache.h`
- **继承自**: `LfuCache`
- **策略**: 带老化机制的LFU
- **特点**:
    - 解决传统LFU的"缓存污染"问题
    - 引入访问频率平均值概念
    - 当平均频率过高时自动老化(降频)


## 使用示例
```cpp
// 创建容量为100的LRU缓存
LruCache<int, std::string> lru(100);
// 添加元素
cache.put(42, "value_string");

// 获取元素
std::string value;
if(cache.get(42, value)) {
    // 使用value...
}