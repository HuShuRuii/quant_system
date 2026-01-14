#pragma once
#include<vector>
#include<string>
#include <future>
#include <unordered_map>

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

enum class OrderStatus {
    Pending,
    Submitted,
    Cancelled,
    Filled
};

struct Order {
    int orderId = 0;
    std::string symbol;
    int volume = 0;
    float price = 0.0f;
    OrderStatus status = OrderStatus::Pending;
};

class Position {
public:
    std::future<int> submitOrderAsync(const std::string& symbol, int volume, float price) {
        return std::async(std::launch::async, [this, symbol, volume, price]() {
            int id = ++orderCounter;
            Order order{id, symbol, volume, price, OrderStatus::Submitted};
            orders[id] = order;
            return id;
        });
    }

    std::future<bool> cancelOrderAsync(int orderId) {
        return std::async(std::launch::async, [this, orderId]() {
            auto it = orders.find(orderId);
            if (it != orders.end() && it->second.status == OrderStatus::Submitted) {
                it->second.status = OrderStatus::Cancelled;
                return true;
            }
            return false;
        });
    }

    OrderStatus queryOrderStatus(int orderId) {
        auto it = orders.find(orderId);
        if (it != orders.end()) {
            return it->second.status;
        }
        return OrderStatus::Pending;
    }

private:
    std::unordered_map<int, Order> orders;
    int orderCounter = 0;
};