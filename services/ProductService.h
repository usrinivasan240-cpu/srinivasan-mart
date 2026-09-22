// ============================================================
// MODULE: ProductService
// PURPOSE: Contains business logic for product operations.
// WHY: Controllers should call this service for all product data operations.
// INPUT: Product data from controllers.
// OUTPUT: JSON responses with product information from PostgreSQL.
// ARCHITECTURE: Service
// ============================================================

#pragma once

#include <drogon/drogon.h>
#include <json/json.h>
#include <libpq-fe.h>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

// This class handles all product operations (CRUD) using PostgreSQL database directly via libpq.
class ProductService
{
public:
    // Connects to PostgreSQL and creates the products table.
    // Connection reads PGHOST/PGPORT/PGDATABASE/PGUSER/PGPASSWORD env vars.
    static bool initializeDatabase();

    // The PostgreSQL database connection (public so OrderService can run
    // checkout stock transactions on the same handle).
    static PGconn *getConnection();

    // Returns products as a JSON array with pagination (defaults: 100/0).
    static Json::Value getAllProducts(int limit = 100, int offset = 0);

    // Returns one product by its ID, or empty JSON if not found.
    static Json::Value getProductById(const std::string &id);

    // Creates a new product in the database and returns it (or error JSON).
    static Json::Value createProduct(const Json::Value &productData);

    // Updates an existing product (merge semantics) and returns it.
    static Json::Value updateProduct(const std::string &id, const Json::Value &productData);

    // Deletes a product by ID. Returns true if deleted, false if not found.
    static bool deleteProduct(const std::string &id);

    // Case-insensitive SQL search by name substring + price range + pagination.
    static Json::Value searchProducts(const std::string &query, double minPrice, double maxPrice,
                                      int limit = 100, int offset = 0);

    // Atomically decrements stock if sufficient. Returns true on success,
    // false with err message on insufficient stock / missing product / DB error.
    // Caller must hold the checkout transaction (BEGIN/COMMIT) for atomicity
    // across multiple lines; this helper works both inside and outside one.
    static bool decrementStock(const std::string &id, int qty, std::string &err);

    // Returns total product count (for admin/seller stats, avoids LIMIT cap).
    static int countProducts();

    // Recursive mutex serializing all libpq use on the shared connection.
    // Recursive so helpers (update->get, checkout->get/decrement) can nest.
    static std::recursive_mutex &dbMutex();

    // All-or-nothing stock reservation for checkout: verifies each
    // (productId, qty), decrements stock atomically, sums price*qty into
    // total. Returns false with err set on any failure (nothing decremented).
    static bool checkoutItems(const std::vector<std::pair<std::string, int>> &items,
                              double &total, std::string &err);

private:

    // Generates a unique ID (UUID) for new products.
    static std::string generateUUID();

    // Frees a PGresult after use.
    static void freeResult(PGresult *res);
};
