//
// Created by mortacious on 11/2/21.
//

#pragma once
#include <list>
#include <unordered_map>
#include <assert.h>

using namespace std;

/**
 * Function returning one for every item
 */
template<typename KEY_T, class VAL_T>
struct ConstantOne {
    size_t operator()(const KEY_T& k , const VAL_T& v) const {
        return 1;
    }
};

template <class KEY_T, class VAL_T, typename FUNC = ConstantOne<KEY_T, VAL_T>>
class LRUCache{
private:
    list< pair<KEY_T,VAL_T> > item_list_;
    unordered_map<KEY_T, decltype(item_list_.begin()) > item_map_;
    FUNC func_;
    size_t cache_size_;
    size_t current_cache_size_;
private:
    void clean(void){
        while(current_cache_size_ > cache_size_){
            auto last_it = item_list_.end(); last_it --;
            cache_size_ -= func_(last_it->first, last_it->second);
            item_map_.erase(last_it->first);
            item_list_.pop_back();
        }
    };
public:
    LRUCache(size_t cache_size_): func_(), cache_size_(cache_size_), current_cache_size_(0) {
        ;
    };

    void put(const KEY_T &key, const VAL_T &val){
        auto it = item_map_.find(key);
        if(it != item_map_.end()){
            item_list_.erase(it->second);
            item_map_.erase(it);
        }
        item_list_.push_front(make_pair(key, val));
        item_map_.insert(make_pair(key, item_list_.begin()));
        cache_size_ += func_(key, val);
        clean();
    };

    void cache_size(size_t cs) {
        cache_size_ = cs;
    }

    size_t cache_size() const {
        return cache_size_;
    }

    bool exist(const KEY_T &key) const {
        return (item_map_.count(key) > 0);
    };

    VAL_T get(const KEY_T &key){
        auto it = item_map_.find(key);
        item_list_.splice(item_list_.begin(), item_list_, it->second);
        return it->second->second;
    };

    void erase(const KEY_T& key) {
        auto it = item_map_.find(key);
        item_list_.erase(it->second);
        item_map_.erase(it->first);
    }

};