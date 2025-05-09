#pragma once
#include "KLruCache.h"
#include <cmath>
#include <cstring>
#include <vector>


template <typename Key, typename Value>
class HashKlruCache : public KLruCache<Key, Value> {
    public:
        HashKlruCache(int capacity, int splitNum) : capacity_(capacity), splitNum_(splitNum){
            int splitSize = std::ceil(capacity / splitNum);
            for (int i = 0; i < splitNum_; i++){
                splitCaches.push_back(new LruCache<Key, Value>(splitSize));
            }
        }

        void put(Key key){
            int index = Hash(key) % splitNum_;
            splitCaches[index]->put(key);

            return;
        }

        bool get(Key key, Value& value){
            int index = Hash(key) % splitNum_;
            return splitCaches[index]->get(key, value);
        }

        Value get(Key key){
            Value value;
            memset(&value, 0, sizeof(value));
            get(key, value);
            return value;
        }

    private:
        int splitNum_;
        int capacity_;
        std::vector<std::unique_ptr<LruCache<Key, Value>>> splitCaches;
    private:
        int Hash(Key key){
            std::hash<Key> hashFunction;
            return hashFunction(key);
        }
};