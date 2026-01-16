#include <algorithm>
#include "orderbook.h"
#include "type_declare.h"
// we consider if there are order matched they are completed immediately and yield deal to the deal vector
void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (order.side == Orderside::Bid) {
        int remain_qty = order.quantity;
        auto it = ask_orders_.begin();
        while (it != ask_orders_.end() && it->price <= order.price && remain_qty > 0) {
            int deal_qty = std::min(remain_qty, it->quantity);
            deals_.emplace_back(order.orderId, it->orderId, it->price, deal_qty, std::chrono::system_clock::now());
            remain_qty -= deal_qty;

            // 更新 ask_levels
            ask_levels_[it->price] -= deal_qty;
            if (ask_levels_[it->price] <= 0) ask_levels_.erase(it->price);

            // 更新撮合订单
            if (it->quantity > deal_qty) {
                Order updated_ask = *it;
                updated_ask.quantity -= deal_qty;
                order_map_ask_.erase(it->orderId);
                it = ask_orders_.erase(it);
                auto insert_it = ask_orders_.insert(updated_ask);
                order_map_ask_[updated_ask.orderId] = insert_it.first;
            } else {
                order_map_ask_.erase(it->orderId);
                it = ask_orders_.erase(it);
            }
        }
        if (remain_qty > 0) {
            Order new_bid = order;
            new_bid.quantity = remain_qty;
            auto insert_it = bid_orders_.insert(new_bid);
            order_map_bid_[new_bid.orderId] = insert_it.first;
            bid_levels_[new_bid.price] += remain_qty;
        }
    }
    else {
        int remain_qty = order.quantity;
        auto it = bid_orders_.begin();
        while (it != bid_orders_.end() && it->price >= order.price && remain_qty > 0) {
            int deal_qty = std::min(remain_qty, it->quantity);
            deals_.emplace_back(it->orderId, order.orderId, it->price, deal_qty, std::chrono::system_clock::now());
            remain_qty -= deal_qty;

            // 更新 bid_levels
            bid_levels_[it->price] -= deal_qty;
            if (bid_levels_[it->price] <= 0) bid_levels_.erase(it->price);

            // 更新撮合订单
            if (it->quantity > deal_qty) {
                Order updated_bid = *it;
                updated_bid.quantity -= deal_qty;
                order_map_bid_.erase(it->orderId);
                it = bid_orders_.erase(it);
                auto insert_it = bid_orders_.insert(updated_bid);
                order_map_bid_[updated_bid.orderId] = insert_it.first;
            } else {
                order_map_bid_.erase(it->orderId);
                it = bid_orders_.erase(it);
            }
        }
        if (remain_qty > 0) {
            Order new_ask = order;
            new_ask.quantity = remain_qty;
            auto insert_it = ask_orders_.insert(new_ask);
            order_map_ask_[new_ask.orderId] = insert_it.first;
            ask_levels_[new_ask.price] += remain_qty;
        }
    }
}

double OrderBook::getSpread() const{
    std::lock_guard<std::mutex> lock(mtx_);
    if (bid_orders_.empty() || ask_orders_.empty()){
        return 0.0;
    }
    double best_bid = bid_orders_.begin()->price;
    double best_ask = ask_orders_.begin()->price;
    return best_ask - best_bid;
}; // get current spread

long long OrderBook::at(double price) const {
    std::lock_guard<std::mutex> lock(mtx_);
    if (bid_levels_.find(price) != bid_levels_.end()) {
        return bid_levels_.at(price);
    }
    else if (ask_levels_.find(price) != ask_levels_.end()) {
        return ask_levels_.at(price);
    }
    else {
        return 0;
    }
}

bool OrderBook::cancelOrder(int orderId) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it_bid = order_map_bid_.find(orderId);
    if (it_bid != order_map_bid_.end()) {
        bid_levels_[ it_bid->second->price ] -= it_bid->second->quantity;
        bid_orders_.erase(it_bid->second);
        order_map_bid_.erase(it_bid);
        return;
    }
    auto it_ask = order_map_ask_.find(orderId);
    if (it_ask != order_map_ask_.end()) {
        ask_levels_[ it_ask->second->price ] -= it_ask->second->quantity;
        ask_orders_.erase(it_ask->second);
        order_map_ask_.erase(it_ask);
        return;
    }
}

std::vector<std::pair<double,long long>> OrderBook::printBid(int depth=5) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::pair<double,long long>> snapshot;
    for (auto it = bid_levels_.begin(); it != bid_levels_.end() && depth > 0; ++it, --depth) {
        snapshot.emplace_back(it->first, it->second);
    }
    return snapshot;
}
std::vector<std::pair<double,long long>> OrderBook::printAsk(int depth=5) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::pair<double,long long>> snapshot;
    for (auto it = ask_levels_.begin(); it != ask_levels_.end() && depth > 0; ++it, --depth) {
        snapshot.emplace_back(it->first, it->second);
    }
    return snapshot;
}

