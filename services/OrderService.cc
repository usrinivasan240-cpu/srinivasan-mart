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
#include <vector>
#include <libpq-fe.h>

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

    // Transactional checkout fix: previously totals were computed with no
    // stock change. Now BEGIN, verify + decrement each line atomically,
    // ROLLBACK on any failure, COMMIT on success.
    PGconn *conn = ProductService::getConnection();
    if (conn == nullptr)
    {
        Json::Value error;
        error["error"] = "Database connection failed";
        return error;
    }
    auto execOk = [&](const char *sql) -> bool {
        PGresult *r = PQexec(conn, sql);
        bool ok = (PQresultStatus(r) == PGRES_COMMAND_OK);
        PQclear(r);
        return ok;
    };
    if (!execOk("BEGIN"))
    {
        Json::Value error;
        error["error"] = "Checkout transaction failed to start";
        return error;
    }

    double total = 0.0;
    struct Line { std::string pid; int qty; double price; };
    std::vector<Line> lines;
    for (auto &line : cart)
    {
        std::string pid = line["productId"].asString();
        int qty = line.get("quantity", 1).asInt();
        auto product = ProductService::getProductById(pid);
        if (product.isNull())
        {
            execOk("ROLLBACK");
            Json::Value error;
            error["error"] = "Product not found: " + pid;
            return error;
        }
        double price = product.get("price", 0.0).asDouble();
        std::string err;
        if (!ProductService::decrementStock(pid, qty, err))
        {
            execOk("ROLLBACK");
            Json::Value error;
            error["error"] = err;
            return error;
        }
        total += price * qty;
        lines.push_back({pid, qty, price});
    }
    if (!execOk("COMMIT"))
    {
        execOk("ROLLBACK");
        Json::Value error;
        error["error"] = "Checkout commit failed";
        return error;
    }

    std::lock_guard<std::mutex> lock(getMutex());
    Order order;
    order.id = generateUUID();
    order.userId = userId;
    order.total = total;
    order.status = "created";
    order.itemCount = (int)lines.size();
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
