#pragma once
#include "LruCache.h"


template <typename Key, typename Value>
class KLruCache : public LruCache<Key, Value> {
    public:
        KLruCache(int capacity, int historyCapacity, int k) : LruCache<Key, Value>(capacity), k_(k),
            historyList_(std::make_unique<KLruCache<Key, int>> (historyCapacity)) {}

        Value get(Key key){
            int historyCount = historyList_->get(key);
            historyList_->put(key, historyCount + 1);
            return LruCache<Key, Value>::get(key);
        }

        void put(Key key, Value value){
            if (LruCache<Key, Value>::get(key) != "") {
                LruCache<Key, Value>::put(key, value);
            }

            int historyCount = historyList_->get(key);
            historyList_->put(key, historyCount + 1);

            if (historyCount >= k_){
                //移除历史访问记录，并将其加入缓存队列
                historyList_->remove(key);
                LruCache<Key, Value>::put(key, value);
            }
        }

    private:
        int k_; //超参数，访问k次后加入缓存队列
        std::unique_ptr<KLruCache<Key, int>> historyList_; //记录访问次数的列表
};