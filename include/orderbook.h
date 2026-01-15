#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
class OrderBook {
 public:
  /// @brief
  OrderBook();
  ~OrderBook();
  void addOrder(int orderId, double price, int quantity);
  void removeOrder(int orderId);
  void printOrderBook() const;

 private:
  struct Order {
    int orderId;
    double price;
    int quantity;
  };
  struct Deal {
    int buyOrderId;
    int sellOrderId;
    double price;
    int quantity;
    std::time_t timestamp;
  };
  std::set<Order> sell_orders_;
  std::set<Order> buy_orders_;
  std::set<Deal> deals_;
};