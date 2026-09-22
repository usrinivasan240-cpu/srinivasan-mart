// ============================================================
// MODULE: ProductService
// PURPOSE: Implementation of product business logic using PostgreSQL via libpq.
// WHY: Handles all database operations for products.
// INPUT: Product data from controllers.
// OUTPUT: JSON responses with product information.
// ARCHITECTURE: Service
// ============================================================

#include "ProductService.h"
#include <cmath>
#include <random>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <mutex>

// The PostgreSQL connection (shared across all operations).
// libpq connections are NOT thread-safe for concurrent use, so every
// public method locks dbMutex() first (recursive: helpers may nest).
static PGconn *g_conn = nullptr;

std::recursive_mutex &ProductService::dbMutex()
{
    static std::recursive_mutex mtx;
    return mtx;
}

// Builds a libpq conninfo string from env vars with safe defaults.
// PGHOST (localhost), PGPORT (5433), PGDATABASE (sri_mart),
// PGUSER (postgres), PGPASSWORD (postgres).
static std::string sri_conninfo()
{
    const char *host = std::getenv("PGHOST");
    const char *port = std::getenv("PGPORT");
    const char *dbname = std::getenv("PGDATABASE");
    const char *user = std::getenv("PGUSER");
    const char *password = std::getenv("PGPASSWORD");
    std::ostringstream oss;
    oss << "host=" << (host && *host ? host : "localhost")
        << " port=" << (port && *port ? port : "5433")
        << " dbname=" << (dbname && *dbname ? dbname : "sri_mart")
        << " user=" << (user && *user ? user : "postgres")
        << " password=" << (password ? password : "postgres")
        << " connect_timeout=5";
    return oss.str();
}

// Returns the PostgreSQL connection.
// Creates a new connection if one doesn't exist.
PGconn *ProductService::getConnection()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    if (g_conn == nullptr || PQstatus(g_conn) != CONNECTION_OK)
    {
        if (g_conn != nullptr)
        {
            PQfinish(g_conn);
        }
        std::string conninfo = sri_conninfo();
        g_conn = PQconnectdb(conninfo.c_str());
        if (PQstatus(g_conn) != CONNECTION_OK)
        {
            LOG_ERROR << "PostgreSQL connection failed: " << PQerrorMessage(g_conn);
            g_conn = nullptr;
        }
        else
        {
            LOG_INFO << "Connected to PostgreSQL database: sri_mart";
        }
    }
    return g_conn;
}

// Money is always rounded to the cent before JSON leaves the service.
static double sri_roundMoney(double v)
{
    return std::round(v * 100.0) / 100.0;
}

// Frees a PGresult after use (prevents memory leaks).
void ProductService::freeResult(PGresult *res)
{
    if (res != nullptr)
    {
        PQclear(res);
    }
}

// Generates a unique ID (UUID) for new products.
std::string ProductService::generateUUID()
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

// Creates the products table in PostgreSQL if it doesn't exist.
bool ProductService::initializeDatabase()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        return false;
    }

    const char *query =
        "CREATE TABLE IF NOT EXISTS products ("
        "id VARCHAR(36) PRIMARY KEY,"
        "name VARCHAR(255) NOT NULL,"
        "description TEXT DEFAULT '',"
        "price DECIMAL(10,2) NOT NULL,"
        "stock INTEGER DEFAULT 0,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        LOG_ERROR << "Create table failed: " << PQerrorMessage(conn);
        freeResult(res);
        return false;
    }

    LOG_INFO << "Products table created/verified in PostgreSQL.";
    freeResult(res);
    return true;
}

// Returns products from PostgreSQL with LIMIT/OFFSET pagination.
Json::Value ProductService::getAllProducts(int limit, int offset)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    if (limit <= 0) limit = 100;
    if (limit > 500) limit = 500;
    if (offset < 0) offset = 0;
    Json::Value result(Json::arrayValue);
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        return result;
    }

    std::string lim = std::to_string(limit);
    std::string off = std::to_string(offset);
    const char *params[2] = {lim.c_str(), off.c_str()};
    PGresult *res = PQexecParams(conn,
        "SELECT id, name, description, price, stock, "
        "created_at::text, updated_at::text FROM products "
        "ORDER BY created_at DESC LIMIT $1::integer OFFSET $2::integer",
        2, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "Get products failed: " << PQerrorMessage(conn);
        freeResult(res);
        return result;
    }

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < rows; i++)
    {
        Json::Value item;
        item["id"] = PQgetvalue(res, i, 0);
        item["name"] = PQgetvalue(res, i, 1);
        item["description"] = PQgetvalue(res, i, 2);
        try { item["price"] = sri_roundMoney(std::stod(PQgetvalue(res, i, 3))); }
        catch (...) { item["price"] = 0.0; }
        try { item["stock"] = std::stoi(PQgetvalue(res, i, 4)); }
        catch (...) { item["stock"] = 0; }
        item["created_at"] = PQgetvalue(res, i, 5);
        item["updated_at"] = PQgetvalue(res, i, 6);
        result.append(item);
    }

    freeResult(res);
    return result;
}

// Finds one product by its ID in the PostgreSQL database.
Json::Value ProductService::getProductById(const std::string &id)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        return Json::Value();
    }

    const char *paramValues[1] = {id.c_str()};

    PGresult *res = PQexecParams(conn,
        "SELECT id, name, description, price, stock, "
        "created_at::text, updated_at::text "
        "FROM products WHERE id = $1",
        1,       // number of parameters
        nullptr, // param types (let PostgreSQL infer)
        paramValues, // param values
        nullptr, // param lengths (text doesn't need this)
        nullptr, // param formats (all text)
        0        // result format (text)
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "Get product failed: " << PQerrorMessage(conn);
        freeResult(res);
        return Json::Value();
    }

    if (PQntuples(res) > 0)
    {
        Json::Value item;
        item["id"] = PQgetvalue(res, 0, 0);
        item["name"] = PQgetvalue(res, 0, 1);
        item["description"] = PQgetvalue(res, 0, 2);
        try { item["price"] = sri_roundMoney(std::stod(PQgetvalue(res, 0, 3))); }
        catch (...) { item["price"] = 0.0; }
        try { item["stock"] = std::stoi(PQgetvalue(res, 0, 4)); }
        catch (...) { item["stock"] = 0; }
        item["created_at"] = PQgetvalue(res, 0, 5);
        item["updated_at"] = PQgetvalue(res, 0, 6);
        freeResult(res);
        return item;
    }

    freeResult(res);
    return Json::Value();
}

// Creates a new product in the PostgreSQL database.
Json::Value ProductService::createProduct(const Json::Value &productData)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        Json::Value error;
        error["error"] = "Database connection failed";
        return error;
    }

    std::string name = productData.get("name", "").asString();
    if (name.empty() || name.size() > 255)
    {
        Json::Value error;
        error["error"] = "Name is required (max 255 chars)";
        return error;
    }
    double priceVal = sri_roundMoney(productData.get("price", -1.0).asDouble());
    int stockVal = productData.get("stock", 0).asInt();
    if (priceVal < 0 || priceVal > 100000000)
    {
        Json::Value error;
        error["error"] = "Price must be >= 0";
        return error;
    }
    if (stockVal < 0 || stockVal > 1000000)
    {
        Json::Value error;
        error["error"] = "Stock must be >= 0";
        return error;
    }

    std::string description = productData.get("description", "").asString();
    if (description.size() > 5000)
    {
        Json::Value error;
        error["error"] = "Description too long (max 5000 chars)";
        return error;
    }

    std::string id = generateUUID();
    std::string price = std::to_string(priceVal);
    std::string stock = std::to_string(stockVal);

    const char *paramValues[5] = {id.c_str(), name.c_str(), description.c_str(), price.c_str(), stock.c_str()};

    PGresult *res = PQexecParams(conn,
        "INSERT INTO products (id, name, description, price, stock) "
        "VALUES ($1, $2, $3, $4::decimal, $5::integer)",
        5, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        LOG_ERROR << "Create product failed: " << PQerrorMessage(conn);
        freeResult(res);
        Json::Value error;
        error["error"] = "Failed to create product";
        return error;
    }

    freeResult(res);
    return getProductById(id);
}

// Updates an existing product in the PostgreSQL database.
Json::Value ProductService::updateProduct(const std::string &id, const Json::Value &productData)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        return Json::Value();
    }

    // Merge semantics fix: previously missing fields overwrote with ""/0.
    // Fetch existing first, keep old values when fields are absent.
    Json::Value existing = getProductById(id);
    if (existing.isNull())
    {
        return Json::Value();
    }
    std::string name = productData.isMember("name") && !productData["name"].asString().empty()
        ? productData["name"].asString() : existing.get("name", "").asString();
    std::string description = productData.isMember("description")
        ? productData["description"].asString() : existing.get("description", "").asString();
    double priceVal = sri_roundMoney(productData.isMember("price")
        ? productData["price"].asDouble() : existing.get("price", 0.0).asDouble());
    int stockVal = productData.isMember("stock")
        ? productData["stock"].asInt() : existing.get("stock", 0).asInt();
    if (name.empty() || name.size() > 255)
    {
        Json::Value error;
        error["error"] = "Name is required (max 255 chars)";
        return error;
    }
    if (priceVal < 0 || stockVal < 0)
    {
        Json::Value error;
        error["error"] = "Price and stock must be >= 0";
        return error;
    }
    if (description.size() > 5000)
    {
        Json::Value error;
        error["error"] = "Description too long (max 5000 chars)";
        return error;
    }
    std::string price = std::to_string(priceVal);
    std::string stock = std::to_string(stockVal);

    const char *paramValues[5] = {name.c_str(), description.c_str(), price.c_str(), stock.c_str(), id.c_str()};

    PGresult *res = PQexecParams(conn,
        "UPDATE products SET name = $1, description = $2, "
        "price = $3::decimal, stock = $4::integer, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = $5",
        5, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        LOG_ERROR << "Update product failed: " << PQerrorMessage(conn);
        freeResult(res);
        return Json::Value();
    }

    freeResult(res);
    return getProductById(id);
}

// Deletes a product from the PostgreSQL database.
bool ProductService::deleteProduct(const std::string &id)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr)
    {
        return false;
    }

    const char *paramValues[1] = {id.c_str()};
    PGresult *res = PQexecParams(conn,
        "DELETE FROM products WHERE id = $1",
        1, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        LOG_ERROR << "Delete product failed: " << PQerrorMessage(conn);
        freeResult(res);
        return false;
    }

    bool deleted = (PQcmdTuples(res)[0] != '0');
    freeResult(res);
    return deleted;
}

// Case-insensitive SQL search: ILIKE + price range + pagination.
// minPrice/maxPrice < 0 means "no bound".
Json::Value ProductService::searchProducts(const std::string &query, double minPrice, double maxPrice,
                                           int limit, int offset)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    if (limit <= 0) limit = 100;
    if (limit > 500) limit = 500;
    if (offset < 0) offset = 0;
    Json::Value result(Json::arrayValue);
    PGconn *conn = getConnection();
    if (conn == nullptr) return result;

    std::string q = query.size() > 200 ? query.substr(0, 200) : query;
    std::string minS = std::to_string(minPrice);
    std::string maxS = std::to_string(maxPrice);
    std::string limS = std::to_string(limit);
    std::string offS = std::to_string(offset);
    const char *params[5] = {q.c_str(), minS.c_str(), maxS.c_str(), limS.c_str(), offS.c_str()};
    PGresult *res = PQexecParams(conn,
        "SELECT id, name, description, price, stock, "
        "created_at::text, updated_at::text FROM products "
        "WHERE ($1 = '' OR name ILIKE '%' || $1 || '%') "
        "AND ($2::double precision < 0 OR price >= $2::decimal) "
        "AND ($3::double precision < 0 OR price <= $3::decimal) "
        "ORDER BY created_at DESC LIMIT $4::integer OFFSET $5::integer",
        5, nullptr, params, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "Search products failed: " << PQerrorMessage(conn);
        freeResult(res);
        return result;
    }
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++)
    {
        Json::Value item;
        item["id"] = PQgetvalue(res, i, 0);
        item["name"] = PQgetvalue(res, i, 1);
        item["description"] = PQgetvalue(res, i, 2);
        try { item["price"] = sri_roundMoney(std::stod(PQgetvalue(res, i, 3))); }
        catch (...) { item["price"] = 0.0; }
        try { item["stock"] = std::stoi(PQgetvalue(res, i, 4)); }
        catch (...) { item["stock"] = 0; }
        item["created_at"] = PQgetvalue(res, i, 5);
        item["updated_at"] = PQgetvalue(res, i, 6);
        result.append(item);
    }
    freeResult(res);
    return result;
}

// Returns SELECT COUNT(*) for stats endpoints.
int ProductService::countProducts()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    PGconn *conn = getConnection();
    if (conn == nullptr) return 0;
    PGresult *res = PQexec(conn, "SELECT COUNT(*) FROM products");
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        freeResult(res);
        return 0;
    }
    int count = 0;
    try { count = std::stoi(PQgetvalue(res, 0, 0)); } catch (...) { count = 0; }
    freeResult(res);
    return count;
}

// Atomically decrements stock when sufficient. Safe to call inside a
// caller-managed transaction (checkout) or standalone.
bool ProductService::decrementStock(const std::string &id, int qty, std::string &err)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    if (qty <= 0 || qty > 99) { err = "Quantity must be 1-99"; return false; }
    PGconn *conn = getConnection();
    if (conn == nullptr) { err = "Database connection failed"; return false; }
    // NOTE: qtyS must outlive PQexecParams (previous code bound a temporary).
    std::string qtyS = std::to_string(qty);
    const char *params[2] = {id.c_str(), qtyS.c_str()};
    // Single-statement atomic guard: only decrements when stock >= qty.
    PGresult *res = PQexecParams(conn,
        "UPDATE products SET stock = stock - $2::integer, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = $1 AND stock >= $2::integer",
        2, nullptr, params, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        err = std::string("Stock update failed: ") + PQerrorMessage(conn);
        freeResult(res);
        return false;
    }
    std::string tuples = PQcmdTuples(res);
    freeResult(res);
    if (tuples.empty() || tuples == "0")
    {
        // Distinguish missing product vs insufficient stock.
        Json::Value p = getProductById(id);
        err = p.isNull() ? "Product not found" : "Insufficient stock";
        return false;
    }
    return true;
}

// All-or-nothing checkout reservation under a single lock + transaction.
// Nested getProductById/decrementStock calls re-lock safely (recursive).
bool ProductService::checkoutItems(const std::vector<std::pair<std::string, int>> &items,
                                  double &total, std::string &err)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex());
    total = 0.0;
    if (items.empty()) { err = "Cart is empty"; return false; }
    if (items.size() > 100) { err = "Too many cart lines (max 100)"; return false; }
    PGconn *conn = getConnection();
    if (conn == nullptr) { err = "Database connection failed"; return false; }
    auto execOk = [&](const char *sql) -> bool {
        PGresult *r = PQexec(conn, sql);
        bool ok = (r != nullptr && PQresultStatus(r) == PGRES_COMMAND_OK);
        if (r) PQclear(r);
        return ok;
    };
    if (!execOk("BEGIN")) { err = "Checkout transaction failed to start"; return false; }
    for (auto &it : items)
    {
        const std::string &pid = it.first;
        int qty = it.second;
        if (qty <= 0 || qty > 99) { execOk("ROLLBACK"); err = "Invalid quantity"; return false; }
        Json::Value product = getProductById(pid);
        if (product.isNull()) { execOk("ROLLBACK"); err = "Product not found: " + pid; return false; }
        double price = sri_roundMoney(product.get("price", 0.0).asDouble());
        std::string derr;
        if (!decrementStock(pid, qty, derr)) { execOk("ROLLBACK"); err = derr; return false; }
        total += price * qty;
    }
    if (!execOk("COMMIT")) { execOk("ROLLBACK"); err = "Checkout commit failed"; return false; }
    total = sri_roundMoney(total);
    return true;
}
