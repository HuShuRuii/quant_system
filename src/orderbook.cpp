
#include "orderbook.h"
#include "type_declare.h"
// we consider if there are order matched they are completed immediately and yield deal to the deal vector
void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (order.side == Orderside::Buy) {
        // first check if there is any matching sell order
        for (auto it=sell_orders_.begin(); it!=sell_orders_.end();) {
            if (it->price <= order.price) {
                // match found
                int deal_quantity = std::min(order.quantity, it->quantity);
                deals_.emplace_back(order.orderId, it->orderId, it->price, deal_quantity, std::chrono::system_clock::now());
                // update quantities
                Order updated_sell_order = *it;
                updated_sell_order.quantity -= deal_quantity;
                Order updated_buy_order = order;
                updated_buy_order.quantity -= deal_quantity;
                // remove the original order and insert another one if its not fully matched
                order_map_sell_.erase(it->orderId);
                sell_orders_.erase(it);
                if (updated_sell_order.quantity == 0){
                    auto insert_it = sell_orders_.insert(updated_sell_order);
                    order_map_sell_[updated_sell_order.orderId] = insert_it.first;
                    return ; // buy order is fully filled
                }
                // if buy order is fully filled, return
                if (updated_buy_order.quantity != 0) {
                    ++it; // continue to check next sell order
                } else {
                    return; // buy order is fully filled
                }
            } 
            else {
                break; // no more matches possible
            }
        }
        auto it=buy_orders_.insert(order);
        order_map_buy_[order.orderId]=it.first;
    } else {
        // first check if there is any matching buy order
        for (auto it=buy_orders_.begin(); it!=buy_orders_.end();) {
            if (it->price >= order.price) {
                // match found
                int deal_quantity = std::min(order.quantity, it->quantity);
                deals_.emplace_back(it->orderId, order.orderId, it->price, deal_quantity, std::chrono::system_clock::now());
                // update quantities
                Order updated_buy_order = *it;
                updated_buy_order.quantity -= deal_quantity;
                Order updated_sell_order = order;
                updated_sell_order.quantity -= deal_quantity;
                // remove the original order and insert another one if its not fully matched
                order_map_buy_.erase(it->orderId);
                buy_orders_.erase(it);
                if (updated_buy_order.quantity == 0){
                    auto insert_it = buy_orders_.insert(updated_buy_order);
                    order_map_buy_[updated_buy_order.orderId] = insert_it.first;
                    return ; // sell order is fully filled
                }
                // if sell order is fully filled, return
                if (updated_sell_order.quantity != 0) {
                    ++it; // continue to check next buy order
                } else {
                    return; // sell order is fully filled
                }
            } 
            else {
                break; // no more matches possible
            }
        }
        auto it=sell_orders_.insert(order);
        order_map_sell_[order.orderId]=it.first;
    }
}

bool OrderBook::removeOrder(int orderId) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it_buy = order_map_buy_.find(orderId);
    if (it_buy != order_map_buy_.end()) {
        buy_orders_.erase(it_buy->second);
        order_map_buy_.erase(it_buy);
        return;
    }
    auto it_sell = order_map_sell_.find(orderId);
    if (it_sell != order_map_sell_.end()) {
        sell_orders_.erase(it_sell->second);
        order_map_sell_.erase(it_sell);
        return;
    }
}

long long OrderBook::get_volume_at_price(double price, Orderside side) const {
    std::lock_guard<std::mutex> lock(mtx_);
    long long total_volume = 0;
    if (side == Orderside::Buy) {
        Order key;
        key.price = price;
        auto it = buy_orders_.lower_bound(key);
        while (it != buy_orders_.end() && it->price == price) {
            total_volume += it->quantity;
            ++it;
        }
    }
    else {
        Order key;
        key.price = price;
        auto it = sell_orders_.lower_bound(key);
        while (it != buy_orders_.end() && it->price == price) {
            total_volume += it->quantity;
            ++it;
        }
    }
    return total_volume;
}

std::vector<std::pair<float,long long>> OrderBook::get_bid(int depth=5) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::pair<float,long long>> snapshot;
    auto it = ob.buy_orders_.begin();
    while (it != ob.buy_orders_.end() && snapshot.size() < depth) {
        float price = it->price;
        long long volume = ob.get_volume_at_price(price, Orderside::Buy);
        snapshot.emplace_back(price, volume);
        ++it;
    }
    return snapshot;
}

std::vector<std::pair<float,long long>> OrderBook::get_ask(int depth=5) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::pair<float,long long>> snapshot;
    for (const auto& order : sell_orders_) {
        snapshot.emplace_back(order.price, order.quantity);
        if (snapshot.size() >= static_cast<size_t>(depth)) break;
    }
    return snapshot;
}
