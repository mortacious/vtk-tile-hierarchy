//
// Created by mortacious on 11/2/21.
//

#pragma once
#include <list>
#include <unordered_map>
#include <assert.h>
#include <stdexcept>
#include <iostream>
#include <mutex>


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
    //std::mutex mutex_;
    FUNC func_;
    size_t cache_size_;
    size_t current_cache_size_;
private:
    void clean(void){
        //std::cout << "Attempting cleanup " << current_cache_size_ << " max " << cache_size_ << std::endl;
        //bool first = true;
        while(current_cache_size_ > cache_size_){
            //if(first) {
            //    std::cout << "Cleaning cache" << std::endl;
            //    first = false;
            //}
            auto last_it = item_list_.end(); last_it --;
            current_cache_size_ -= func_(last_it->first, last_it->second);
            item_map_.erase(last_it->first);
            item_list_.pop_back();
        }
    };
public:
    LRUCache(size_t cache_size_): func_(), cache_size_(cache_size_), current_cache_size_(0) {
    };

    void put(const KEY_T &key, const VAL_T &val){
        //std::lock_guard<std::mutex> lock{mutex_};
        auto it = item_map_.find(key);
        if(it != item_map_.end()){
            current_cache_size_ -= func_(it->first, it->second->second);
            item_list_.erase(it->second);
            item_map_.erase(it);
        }
        item_list_.push_front(make_pair(key, val));
        item_map_.insert(make_pair(key, item_list_.begin()));
        current_cache_size_ += func_(key, val);
        clean();
    };

    void max_cache_size(size_t cs) {
        cache_size_ = cs;
    }

    size_t max_cache_size() const {
        return cache_size_;
    }

    size_t cache_size() const {
        return current_cache_size_;
    }

    size_t num_entries() const {
        return item_map_.size();
    }

    bool exist(const KEY_T &key) const {
        return (item_map_.count(key) > 0);
    };

    VAL_T get(const KEY_T &key){
        //std::lock_guard<std::mutex> lock{mutex_};
        auto it = item_map_.find(key);
        if(it == item_map_.end()) {
            throw std::runtime_error("Something went wrong!");
        }
        //std::cout << it->first->Name << " " << it->second->second.second << std::endl;

        item_list_.splice(item_list_.begin(), item_list_, it->second);
        return it->second->second;
    };

    void erase(const KEY_T& key) {
        //std::lock_guard<std::mutex> lock{mutex_};
        auto it = item_map_.find(key);
        item_list_.erase(it->second);
        item_map_.erase(it->first);
        current_cache_size_ -= func_(it->first, it->second->second);

    }

};