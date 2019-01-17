// Copyright 2017, Steven Zhang.  All rights reserved.
//
// Author: Steven Zhang (stevenzscn@gmail.com)
//
// This is a public header file, it must only include public header files.

#ifndef _CACHE_H_
#define _CACHE_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <pthread.h>
#include <time.h>
#include <sstream>
#include <type_traits>
#include <iostream>

#include "timer_task.h"

namespace cache {

enum EN_RETURN_CODE {
    RC_OK = 0,
    RC_ERROR = -1,
    RC_KEY_NOT_EXIST = 1,
    RC_KEY_EMPTY = 2,
    RC_NOT_NUM = 3,
};

template <typename V>
class Cache {
public:
    struct Item {
        V value_;
        time_t deadtime_;

        Item() : deadtime_(-1) {
            std::cout << "Item init: " < value_ << std::endl;
        }

        Item(const V & value, const time_t deadtime) :
            value_(value), deadtime_(deadtime) {
            std::cout << "Item init: " << value_ << std::endl;
        }

        ~Item() {
            std::cout << "~Item destroy: " << value_ << std::endl;
        }
    };

    Cache();
    Cache(const Cache&) = delete;
    Cache & operator=(const Cache&) = delete;
    ~Cache();
public:
    int get(const std::string& key, V& value);
    int set(const std::string& key, const V& value, const time_t expire = -1);
    int ttl(const std::string& key, time_t& expire);
    int del(const std::string& key);
    int incr(const std::string& key, V& value, const time_t expire = -1);
    int incrby(const std::string& key, const V& inc, V& value, const time_t expire = -1);
    std::string print();

private:
    bool is_value_num(const V& value);
    static void clean_expire(Cache<V>* obj);

private:
    std::unordered_map<std::string, std::shared_ptr<Item>> cache_map_;
    pthread_rwlock_t lock_;
    TimerTask timer_;
};

template <typename V>
Cache<V>::Cache() {
    pthread_rwlock_init(&lock_, nullptr);
    timer_.start(1000, std::bind(clean_expire, this));
}

template <typename V>
Cache<V>::~Cache() {
    if (timer_.is_running())
        timer_.stop();

    pthread_rwlock_destroy(&lock_);
}

template <typename V>
int Cache<V>::get(const std::string& key, V& value) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    pthread_rwlock_rdlock(&lock_);
    auto itr = cache_map_.find(key);
    if (itr == cache_map_.end()) {
        ret = RC_KEY_NOT_EXIST;
    } else {
        value = itr->second->value_;
    }
    pthread_rwlock_unlock(&lock_);

    return ret;
}

template <typename V>
int Cache<V>::set(const std::string& key, const V& value, const time_t expire) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    time_t deadtime = expire;
    if (-1 != expire) {
        time_t now = time(nullptr);
        deadtime = now + expire;
    }

    std::shared_ptr<Item> item = std::make_shared<Item>(value, deadtime);

    pthread_rwlock_wrlock(&lock_);
    cache_map_[key] = item;
    pthread_rwlock_unlock(&lock_);

    return ret;
}

template <typename V>
int Cache<V>::ttl(const std::string& key, time_t& expire) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    pthread_rwlock_rdlock(&lock_);
    auto itr = cache_map_.find(key);
    if (itr == cache_map_.end()) {
        ret = RC_KEY_NOT_EXIST;
    } else {
        int deadtime = itr->second->deadtime_;
        if (-1 == deadtime) {
            expire = -1;
        } else {
            time_t now = time(nullptr);
            expire = deadtime - now;
        }
    }
    pthread_rwlock_unlock(&lock_);

    return ret;
}

template <typename V>
int Cache<V>::del(const std::string& key) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    pthread_rwlock_wrlock(&lock_);
    int opt = cache_map_.erase(key);
    pthread_rwlock_unlock(&lock_);

    if (0 == opt) {
        ret = RC_KEY_NOT_EXIST;
    }

    return ret;
}
/*
template <typename V>
bool Cache<V>::is_value_num(const V& value) {
    if (
        typeid(value) == typeid(char)
        || typeid(value) == typeid(unsigned char)
        || typeid(value) == typeid(signed char)
        || typeid(value) == typeid(short)
        || typeid(value) == typeid(unsigned short)
        || typeid(value) == typeid(int)
        || typeid(value) == typeid(unsigned)
        || typeid(value) == typeid(long)
        || typeid(value) == typeid(unsigned long)
        || typeid(value) == typeid(long long)
        || typeid(value) == typeid(unsigned long long)
        || typeid(value) == typeid(float)
        || typeid(value) == typeid(double)
        || typeid(value) == typeid(long double)
    ) {
        return true;
    }

    return false;
}
*/
template <typename V>
bool Cache<V>::is_value_num(const V& value) {
    if (
        std::is_same<decltype(value), const char&>::value
        || std::is_same<decltype(value), const unsigned char&>::value
        || std::is_same<decltype(value), const signed char&>::value
        || std::is_same<decltype(value), const short&>::value
        || std::is_same<decltype(value), const unsigned short&>::value
        || std::is_same<decltype(value), const int&>::value
        || std::is_same<decltype(value), const unsigned&>::value
        || std::is_same<decltype(value), const long&>::value
        || std::is_same<decltype(value), const unsigned long&>::value
        || std::is_same<decltype(value), const long long&>::value
        || std::is_same<decltype(value), const unsigned long long&>::value
        || std::is_same<decltype(value), const float&>::value
        || std::is_same<decltype(value), const double&>::value
        || std::is_same<decltype(value), const long double&>::value
    ) {
        return true;
    }

    return false;
}

template <typename V>
int Cache<V>::incr(const std::string& key, V& value, const time_t expire) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    if (!is_value_num(value)) {
        return RC_NOT_NUM;
    }

    time_t deadtime = expire;
    if (-1 != expire) {
        time_t now = time(nullptr);
        deadtime = now + expire;
    }

    pthread_rwlock_wrlock(&lock_);
    auto itr = cache_map_.find(key);
    if (itr == cache_map_.end()) {
        value = 1;
        std::shared_ptr<Item> item = std::make_shared<Item>(value, deadtime);
        cache_map_[key] = item;
    } else {
        value = ++itr->second->value_;
        itr->second->deadtime_ = deadtime;
    }
    pthread_rwlock_unlock(&lock_);

    return ret;
}

template <typename V>
int Cache<V>::incrby(const std::string& key, const V& inc, V& value, const time_t expire) {
    int ret = RC_OK;
    if (key.empty()) {
        return RC_KEY_EMPTY;
    }

    if (!is_value_num(value)) {
        return RC_NOT_NUM;
    }

    time_t deadtime = expire;
    if (-1 != expire) {
        time_t now = time(nullptr);
        deadtime = now + expire;
    }

    pthread_rwlock_wrlock(&lock_);
    auto itr = cache_map_.find(key);
    if (itr == cache_map_.end()) {
        value = inc;
        std::shared_ptr<Item> item = std::make_shared<Item>(value, deadtime);
        cache_map_[key] = item;
    } else {
        itr->second->value_ += inc;
        value = itr->second->value_;
        itr->second->deadtime_ = deadtime;
    }
    pthread_rwlock_unlock(&lock_);

    return ret;
}

template <typename V>
std::string Cache<V>::print() {
    std::stringstream ss;
    ss << "\n----------------------------------------\n";
    pthread_rwlock_rdlock(&lock_);
    ss << "[key]\t[value]\t[expire]\t(" << cache_map_.size() << " keys)\n";
    for (auto itr = cache_map_.begin(); itr != cache_map_.end(); itr++) {
        ss << itr->first << "\t"; 
        ss << itr->second->value_ << "\t";
        if (-1 == itr->second->deadtime_) {
            ss << -1 << "\n";
        } else {
            time_t now = time(nullptr);
            ss << itr->second->deadtime_ - now << "\n";
        }
    }
    pthread_rwlock_unlock(&lock_);
    ss << "----------------------------------------\n";
    return ss.str();
}

template <typename V>
void Cache<V>::clean_expire(Cache<V>* obj) {
    if (obj == nullptr) {
        return;
    }

    for (auto itr = obj->cache_map_.begin(); itr != obj->cache_map_.end(); ) {
        auto temp = itr++;
        time_t now = time(nullptr);
        if (temp->second->deadtime_ > 0 && temp->second->deadtime_ < now) {
            pthread_rwlock_wrlock(&obj->lock_);
            obj->cache_map_.erase(temp);
            pthread_rwlock_unlock(&obj->lock_);
        }
    }
}

}

#endif
