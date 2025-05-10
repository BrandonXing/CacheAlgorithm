//
// Created by Lenovo on 2025-05-09.
//由于LFU的淘汰策略是淘汰访问次数最小的数据块，但是新插入的数据块的访问次数为1，
//这样就会产生缓存污染，使得新数据块被淘汰。所以在LFU算法之上，引入访问次数平均
//值概念，当平均值大于最大平均值限制时，将所有节点的访问次数减去最大平均值限制的
//一半或一个固定值。
//
#pragma once
#include "LfuCache.h"

template<typename Key, typename Value>
class AgedLfuCache: public LfuCache<Key, Value> {
    private:
        using Node = LfuListNode<Key, Value>;
        using NodePtr = std::shared_ptr<Node>;
        using NodeMap = std::unordered_map<Key, NodePtr>;
        int maxAverage_;
        int curAverage_;
        int curTotal_;

        void updateMinFreq() {
            this->minFreq_ = INT8_MAX;
            for (const auto& pair : this->freqMap_) {
                if (pair.second && !pair.second->isEmpty()) {
                    this->minFreq_ = std::min(this->minFreq_, pair.first);
                }
            }
            if (this->minFreq_ == INT8_MAX) {
                this->minFreq_ = 1;
            }
            return;
        }

        void handleOverMaxAverage() {
            if (this->valueMap_.empty()) {
                return;
            }
            for (auto it = this->valueMap_.begin(); it != this->valueMap_.end(); it++) {
                if (!it->second) // 检查node是否为空
                    continue;
                NodePtr node = it->second;

                // 减小频率
                this->removeNodeFreqlist(node);
                node->freq_ -= maxAverage_ / 2;

                // 添加到新的频率列表
                this->addNodeFreqlist(node);
            }
            this->updateMinFreq();
            return;
        }

        void increaseFreq() {
            curTotal_++;
            if (this->valueMap_.empty()) {
                curAverage_ = 0;
            }else {
                curAverage_ = (curAverage_ + 1) / this->valueMap_.size();
            }
            if (curAverage_ > maxAverage_) {
                handleOverMaxAverage();
            }
        }

        void decreaseFreq(int n) {
            curTotal_ -= n;
            if (this->valueMap_.empty()) {
                curAverage_ = 0;
            }else {
                curAverage_ = curTotal_ / this->valueMap_.size();
            }
            return;
        }


    public:
        explicit AgedLfuCache(int capacity, int maxAverage): LfuCache<Key, Value>(capacity) {
            this->maxAverage_ = maxAverage;
            this->curAverage_ = 0;
            this->curTotal_ = 0;
            this->minFreq_ = INT8_MAX;
        };


        ~AgedLfuCache() = default;

        void put(Key key, Value value) {
            if (this->capacity_ == 0) { return; }
            std::lock_guard<std::mutex> lock(this->mutex_);
            auto it = this->valueMap_.find(key);
            if (it != this->valueMap_.end()) {
                // 缓存中找到了，重置value
                it->second->value_ = value;
                // 找到后需要 1.把Node从当前FreqList删除 2.添加到freq_+1的FreqList里 3.freq_+1
                removeNodeFreqlist(it->second);
                updateNodeFreq(it->second);
                addNodeFreqlist(it->second);
                return;
            }else {
                // 缓存中没找到
                if (this->valueMap_.size() == this->capacity_ ) {
                    // 满了则删除最不常用节点
                    NodePtr node = this->freqMap_[this->minFreq_]->getFirstNode();
                    removeNodeFreqlist(node);
                    this->valueMap_.erase(node->key);
                    decreaseFreq(node->freq_);
                }
                // 创建新节点并添加
                NodePtr node = std::make_shared<Node>(key, value);
                this->valueMap_[key] = node;
                addNodeFreqlist(node);
                increaseFreq();
                this->minFreq_ = std::min(this->minFreq_, 1);
            }
            return;
        }

        bool get(Key key, Value &value) {
            std::lock_guard<std::mutex> lock(this->mutex_);
            auto it = this->valueMap_.find(key);
            if (it != this->valueMap_.end()) {
                removeNodeFreqlist(it->second);
                updateNodeFreq(it->second);
                addNodeFreqlist(it->second);
                increaseFreq();
                return true;
            }
            return false;
        }



};
