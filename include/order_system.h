#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

class OrderManager {
 public:
  // 订单结构
  struct Order {
    int id;
    double price;
    int quantity;
    std::string status;  // "pending", "completed", or "canceled"
  };

  // 回调函数类型
  using Callback = std::function<void(const Order&)>;

  // 构造函数
  OrderManager(Callback callback) : orderCallback(callback) {}

  // 下单函数
  void placeOrder(int id, double price, int quantity) {
    Order order = {id, price, quantity, "pending"};
    orders[id] = order;
    orderCallback(order);  // 通知下单成功
  }

  // 撤单函数
  void cancelOrder(int id) {
    auto it = orders.find(id);
    if (it != orders.end()) {
      Order order = it->second;
      order.status = "canceled";
      orders.erase(it);      // 删除订单
      orderCallback(order);  // 通知撤单成功
    } else {
      std::cerr << "Order ID " << id << " not found." << std::endl;
    }
  }

 private:
  std::unordered_map<int, Order> orders;  // 存储订单
  Callback orderCallback;                 // 回调函数
};

// 示例回调函数
void orderStatusCallback(const OrderManager::Order& order) {
  std::cout << "Order ID: " << order.id << ", Status: " << order.status
            << std::endl;
}

