// ============================================================
// MODULE: OrderService
// PURPOSE: Implementation of checkout stub.
// WHY: Sums cart into an order; real stock decrement planned in Postgres.
// INPUT: userId.
// OUTPUT: JSON order or error.
// ARCHITECTURE: Service
// ============================================================

#include "OrderService.h"
#include "CartService.h"
#include "ProductService.h"
#include <random>
#include <sstream>
#include <chrono>
#include <ctime>
#include <utility>
#include <vector>

std::vector<Order> &OrderService::getStorage()
{
    static std::vector<Order> orders;
    return orders;
}

std::mutex &OrderService::getMutex()
{
    static std::mutex mtx;
    return mtx;
}

std::string OrderService::generateUUID()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

static std::string sri_nowStamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmBuf);
    return std::string(buf);
}

Json::Value OrderService::checkout(const std::string &userId)
{
    auto cart = CartService::getCart(userId);
    if (cart.size() == 0)
    {
        Json::Value error;
        error["error"] = "Cart is empty";
        return error;
    }

    // All stock work happens inside ProductService::checkoutItems under one
    // lock + one Postgres transaction (all-or-nothing). Cart is cleared only
    // after it succeeds.
    std::vector<std::pair<std::string, int>> items;
    for (auto &line : cart)
    {
        items.emplace_back(line["productId"].asString(),
                           line.get("quantity", 1).asInt());
    }
    double total = 0.0;
    std::string err;
    if (!ProductService::checkoutItems(items, total, err))
    {
        Json::Value error;
        error["error"] = err;
        return error;
    }

    std::lock_guard<std::mutex> lock(getMutex());
    Order order;
    order.id = generateUUID();
    order.userId = userId;
    order.total = total;
    order.status = "created";
    order.itemCount = (int)items.size();
    order.created_at = sri_nowStamp();
    getStorage().push_back(order);

    // Clear cart only after successful COMMIT.
    CartService::clearCart(userId);
    return order.toJson();
}

Json::Value OrderService::getOrders(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(getMutex());
    Json::Value result(Json::arrayValue);
    for (auto &o : getStorage())
    {
        if (o.userId == userId)
        {
            result.append(o.toJson());
        }
    }
    return result;
}

Json::Value OrderService::getOrderById(const std::string &userId, const std::string &orderId)
{
    std::lock_guard<std::mutex> lock(getMutex());
    for (auto &o : getStorage())
    {
        if (o.id == orderId && o.userId == userId)
        {
            return o.toJson();
        }
    }
    return Json::Value();
}

Json::Value OrderService::updateStatus(const std::string &userId, const std::string &orderId, const std::string &status)
{
    if (status != "created" && status != "paid" && status != "shipped")
    {
        Json::Value error;
        error["error"] = "Status must be created, paid, or shipped";
        return error;
    }
    std::lock_guard<std::mutex> lock(getMutex());
    for (auto &o : getStorage())
    {
        if (o.id == orderId && o.userId == userId)
        {
            o.status = status;
            return o.toJson();
        }
    }
    return Json::Value();
}
