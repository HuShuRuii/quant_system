#pragma once
#include<vector>
#include<string>
#include <future>
#include <unordered_map>
#include<mutex>
struct Tick {
    std::string timetag = "";
    float lastPrice = 0.0f;
    float open = 0.0f;
    float low = 0.0f;
    float amount = 0.0f;
    int volume = 0;
    int pvolume = 0;
    int openInt = 0;
    std::string stockStatus = "";
    float lastClose = 0.0f;
    float lastSettlementPrice = 0.0f;
    float settlementPrice = 0.0f;
    std::vector<float> askPrice = {};
    std::vector<float> bidPrice = {};
    std::vector<int> askVol = {};
    std::vector<int> bidVol = {};
};

struct Bar {
    long long time = 0;
    float open = 0.0f;
    float high = 0.0f;
    float low = 0.0f;
    float close = 0.0f;
    float volume = 0.0f;
    float amount = 0.0f;
    float settlementPrice = 0.0f;
    int openInterest = 0;
    float preClose = 0.0f;
    int suspendFlag = 0;
};
// 需求是什么

template<typename T, size_t N>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), size_(0) {}

    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        buffer_[head_] = value;
        head_ = (head_ + 1) % N;
        if (size_ < N) ++size_;
    }

    size_t size() const { 
        std::lock_guard<std::mutex> lock(mtx_);
        return size_; }
    bool full() const { 
        std::lock_guard<std::mutex> lock(mtx_);
        return size_ == N; }

    const T& at(size_t i) const {
        std::lock_guard<std::mutex> lock(mtx_);
        return buffer_[(head_ + N - size_ + i) % N];
    }

private:
    T buffer_[N];
    size_t head_;
    size_t size_;
    mutable std::mutex mtx_;
};
