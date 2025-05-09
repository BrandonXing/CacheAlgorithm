#pragma once
#include <unordered_map>
#include <memory>
#include "../LRU/LruCache.h"

template<typename Key, typename Value> class LfuCache;

template<typename Key, typename Value>
class LfuListNode: public ListNode<Key, Value> {
    public:
        LfuListNode(Key key, Value value): ListNode<Key, Value>(key, value), freq_(0) {};
        ~LfuListNode() {};
    private:
        int freq_;
   
};


template<typename Key, typename Value>
class LfuList {
    private:
        int freq_;
        using NodePtr = std::shared_ptr<LfuListNode<Key, Value>>;
        NodePtr head_;
        NodePtr tail_;

    public:
        explicit LfuList(int const n): freq_(n) {
            head_ = std::make_shared<LfuListNode<Key, Value>>();
            tail_ = std::make_shared<LfuListNode<Key, Value>>();
            head_->next = tail_;
            tail_->prev = head_;
        };

        bool isEmpty() {
            return head_ == tail_;
        }

        void addNode(NodePtr node) {
            if (!node || !tail_ || !head_) {
                return;
            }
            // 尾插
            node->next = tail_;
            node->prev = tail_->prev;
            tail_->prev->next = node;
            tail_->prev = node;
        }

        void removeNode(NodePtr node) {
            if (!node || !tail_ || !head_) {
                return;
            }
            // 检查移除的不是头尾
            if (!node->next || !node->prev) {
                return;
            }
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->prev = nullptr;
            node->next = nullptr;
        }

        NodePtr getFirstNode() {return head_->next;}


};


template<typename Key, typename Value>
class LfuCache {
    private:
        using Node = LfuListNode<Key, Value>;
        using NodePtr = std::shared_ptr<Node>;
        using NodeMap = std::unordered_map<Key, NodePtr>;
        int capacity_;
        int minFreq_;
        std::mutex mutex_; // 互斥锁
        NodeMap valueMap_;  //Key到Node的map
        std::unordered_map<int, std::shared_ptr<LfuList<Key, Value>>> freqMap_; //频率到链表的map

    private:
        void removeNodeFreqlist(NodePtr node) {
            if (node == nullptr) {
                return;
            }
            int freq = node->freq_;
            freqMap_[freq]->removeNode(node);
        }

        void addNodeFreqlist(NodePtr node) {
            if (node == nullptr) {
                return;
            }
            int freq = node->freq_;
            if (freqMap_.find(freq) == freqMap_.end()) {
                // 没有对应的List则创建
                freqMap_[freq] = std::make_shared<LfuList<Key, Value>>(node->freq_);
            }
            freqMap_[freq]->addNode(node);
            return;
        }

        void updateNodeFreq(NodePtr node) {
            if (node == nullptr) {
                return;
            }
            if (node->freq_ - 1 == minFreq_ && freqMap_[node->freq_ - 1]->isEmpty()) {
                // 如果当前node的访问频次如果等于minFreq+1，并且其前驱链表为空，则说明
                // freqMap_[node->freq_-1]链表因node的迁移已经空了，需要更新最小访问频次
                minFreq_++;
            }
            return;
        }

    public:
        explicit LfuCache(int const capacity): capacity_(capacity) {};

        ~LfuCache() {};

        void put(Key key, Value value) {
            if (capacity_ == 0) { return; }
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = valueMap_.find(key);
            if (it != valueMap_.end()) {
                // 缓存中找到了，重置value
                it->second->value_ = value;
                // 找到后需要 1.把Node从当前FreqList删除 2.添加到freq_+1的FreqList里 3.freq_+1
                removeNodeFreqlist(it->second);
                ++it->second->freq_;
                addNodeFreqList(it->second);

                updateNodeFreq(it->second);
                return;
            }else {
                // 缓存中没找到
                if (valueMap_.size() == capacity_ ) {
                    // 满了则删除最不常用节点
                    NodePtr node = freqMap_[minFreq_]->getFirstNode();
                    removeNodeFreqlist(node);
                    valueMap_.erase(node->key);
                    //decreaseFreq();
                }
                // 创建新节点并添加
                NodePtr node = std::make_shared<Node>(key, value);
                valueMap_[key] = node;
                addNodeFreqlist(node);
                // increaseFreq();
                minFreq_ = std::min(minFreq_, 1);
            }
            return;
        }

        bool get(Key key, Value &value) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = valueMap_.find(key);
            if (it != valueMap_.end()) {
                removeNodeFreqlist(it->second);
                ++it->second->freq_;
                addNodeFreqlist(it->second);
                updateNodeFreq(it->second);
                // increaseFreq();
                return true;
            }
            return false;
        }

        Value get(Key key) {
            Value value = {};
            get(key, value);
            return value;
        }

        void purge() {
            valueMap_.clear();
            freqMap_.clear();
            return;
        }
};
