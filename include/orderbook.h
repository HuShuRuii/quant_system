#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include<chrono>
#include <mutex>
#include <unordered_map>
class OrderBook {
public:
  /// @brief
	OrderBook();
  	~OrderBook();
  	void addOrder(const Order& order);
  	bool removeOrder(int orderId);
	long long at(double price) const; // get total volume at this price level (both sides)
	long long operator[](double price) const; // get total volume at this price level (both sides)
	std::vector<std::pair<double, long long >> print_bid(int depth) const;
	std::vector<std::pair<double, long long >> print_ask(int depth) const;
	 // sell means the ask side
private:
	std::set<Order> bid_orders_;
	std::vector<Deal> deals_;
	std::set<Order> ask_orders_;
	std::map<double, long long> bid_levels_; // price -> total volume
	std::map<double, long long> ask_levels_; // price -> total volume
	std::unordered_map<int,std::set<Order>::iterator> order_map_bid_;
	std::unordered_map<int,std::set<Order>::iterator> order_map_ask_;
	mutable std::mutex mtx_;
};



struct Deal {
  int buyOrderId;
  int sellOrderId;
  double price;
  int quantity;
  std::chrono::system_clock::time_point timestamp;

  Deal(int buyId, int sellId, double p, int qty,std::chrono::system_clock::time_point t)
	  : buyOrderId(buyId), sellOrderId(sellId), price(p), quantity(qty),
		timestamp(t) {}
};
struct Order {
  int orderId;
  double price;
  int quantity;
  OrderType type;
  Orderside side;
  std::chrono::system_clock::time_point timestamp;

  bool operator>(const Order& other) const {
      // the first executed order has the highest priority
      if (side == Orderside::Buy) {
          // for buy orders, higher price has higher priority
          if (price != other.price) {
              return price > other.price;
          }
      } else {
          // for sell orders, lower price has higher priority
          if (price != other.price) {
              return price < other.price;
          }
      }
      return timestamp < other.timestamp; // earlier order has higher priority
  }

  Order(int id, double p, int qty, OrderType t, Orderside s,
		std::chrono::system_clock::time_point ts)
	  : orderId(id), price(p), quantity(qty), type(t), side(s), timestamp(ts) {}
  Order(): orderId(0), price(0.0), quantity(0), type(OrderType::Market), side(Orderside::Buy),
			timestamp(std::chrono::system_clock::time_point::max()) {}
};

enum class OrderStatus {
    Pending,
    Submitted,
    Cancelled,
    Filled
};

enum class OrderType {
    Market,
    Limit
};

enum class Orderside {
    Buy,
    Sell
};