#pragma once

#include <deque>
#include <mutex>

template <typename T, typename Container = std::deque<T>>
class SafeQueue {
public:
    using container_type = Container;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;

public:
    SafeQueue() {}

    reference front() {
        return c_.front();
    }

    const_reference front() const {
        return c_.front();
    }

    reference back() {
        return c_.back();
    }

    const_reference back() const {
        return c_.back();
    }

    bool empty() const {
        return c_.empty();
    }

    size_type size() const {
        return c_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mutex_);
        c_.clear();
    }

    void push(const value_type& value) {
        std::lock_guard<std::mutex> lk(mutex_);
        return c_.push_back(value);
    }

    void push(value_type&& value) {
        std::lock_guard<std::mutex> lk(mutex_);
        return c_.push_back(value);
    };

    template <class... Args>
    void emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex_);
        return c_.emplace_back(args...);
    }

    void pop() {
        std::lock_guard<std::mutex> lk(mutex_);
        return c_.pop_front();
    }

private:
    Container c_;
    std::mutex mutex_;
};
