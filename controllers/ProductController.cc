// ============================================================
// MODULE: ProductController
// PURPOSE: Implementation of product HTTP request handlers.
// WHY: Defines how product API endpoints respond to requests.
// INPUT: HTTP requests with product data.
// OUTPUT: JSON responses with product information.
// ARCHITECTURE: Controller
// ============================================================

#include "ProductController.h"
#include "../services/ProductService.h"
#include "../services/AuthService.h"

using namespace drogon;

// Auth helpers (fix: product writes were public; now seller/admin only).
static std::string sri_bearer(const HttpRequestPtr &req)
{
    std::string h = req->getHeader("Authorization");
    if (h.empty() || h.find("Bearer ") != 0) return "";
    return h.substr(7);
}

// Returns "" if authorized, else error message. Sets HTTP code via out param.
static std::string sri_requireSeller(const HttpRequestPtr &req, int &code)
{
    std::string token = sri_bearer(req);
    if (token.empty())
    {
        code = 401;
        return "Authorization header with Bearer token required";
    }
    std::string role = AuthService::getRoleFromToken(token);
    if (role != "seller" && role != "admin")
    {
        code = 403;
        return "Seller or admin only";
    }
    code = 200;
    return "";
}

static bool sri_parseDouble(const std::string &s, double &out)
{
    if (s.empty()) return false;
    try
    {
        size_t pos = 0;
        out = std::stod(s, &pos);
        return pos == s.size();
    }
    catch (...)
    {
        return false;
    }
}

static bool sri_parseInt(const std::string &s, int &out)
{
    if (s.empty()) return false;
    try
    {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    }
    catch (...)
    {
        return false;
    }
}

// Registers all product routes with the Drogon server.
// Each route maps a URL + HTTP method to a function.
void ProductController::initRoutes()
{
    // GET /api/v1/products -> returns all products
    app().registerHandler("/api/v1/products",
        &ProductController::getProducts,
        {Get});

    // POST /api/v1/products -> creates a new product
    app().registerHandler("/api/v1/products",
        &ProductController::createProduct,
        {Post});

    // GET /api/v1/products/search -> search/filter.
    // Registered BEFORE /products/{id} so "search" is never captured as an id.
    app().registerHandler("/api/v1/products/search",
        &ProductController::searchProducts,
        {Get});

    // GET /api/v1/products/{id} -> returns one product
    app().registerHandler("/api/v1/products/{id}",
        &ProductController::getProductById,
        {Get});

    // PUT /api/v1/products/{id} -> updates a product
    app().registerHandler("/api/v1/products/{id}",
        &ProductController::updateProduct,
        {Put});

    // DELETE /api/v1/products/{id} -> deletes a product
    app().registerHandler("/api/v1/products/{id}",
        &ProductController::deleteProduct,
        {Delete});
}

// Fetches products with pagination: ?limit=&offset= (defaults 100/0, max 500).
void ProductController::getProducts(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    int limit = 100, offset = 0;
    std::string limS = req->getParameter("limit");
    std::string offS = req->getParameter("offset");
    if (!limS.empty() && (!sri_parseInt(limS, limit) || limit <= 0 || limit > 500))
    {
        Json::Value error;
        error["error"] = "limit must be an integer 1-500";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!offS.empty() && (!sri_parseInt(offS, offset) || offset < 0))
    {
        Json::Value error;
        error["error"] = "offset must be an integer >= 0";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    auto products = ProductService::getAllProducts(limit, offset);
    auto resp = HttpResponse::newHttpJsonResponse(products);
    callback(resp);
}

// Reads the JSON body, requires seller/admin, validates, creates product.
void ProductController::createProduct(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    int code = 200;
    std::string authErr = sri_requireSeller(req, code);
    if (!authErr.empty())
    {
        Json::Value error;
        error["error"] = authErr;
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(code == 401 ? k401Unauthorized : k403Forbidden);
        callback(resp);
        return;
    }
    auto json = req->getJsonObject();
    if (!json || !json->isMember("name") || !json->isMember("price"))
    {
        Json::Value error;
        error["error"] = "Name and price are required";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    auto result = ProductService::createProduct(*json);
    if (result.isMember("error"))
    {
        auto resp = HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(k201Created);
    callback(resp);
}

// Finds one product by its ID. Returns 404 if not found.
void ProductController::getProductById(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string productId)
{
    auto product = ProductService::getProductById(productId);
    if (product.isNull())
    {
        Json::Value error;
        error["error"] = "Product not found";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
    else
    {
        auto resp = HttpResponse::newHttpJsonResponse(product);
        callback(resp);
    }
}

// Updates an existing product (seller/admin only, merge semantics).
// Returns 404 if the product doesn't exist.
void ProductController::updateProduct(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string productId)
{
    int code = 200;
    std::string authErr = sri_requireSeller(req, code);
    if (!authErr.empty())
    {
        Json::Value error;
        error["error"] = authErr;
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(code == 401 ? k401Unauthorized : k403Forbidden);
        callback(resp);
        return;
    }
    auto json = req->getJsonObject();
    if (!json)
    {
        Json::Value error;
        error["error"] = "Invalid request body";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    auto result = ProductService::updateProduct(productId, *json);
    if (result.isNull())
    {
        Json::Value error;
        error["error"] = "Product not found";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
    else if (result.isMember("error"))
    {
        auto resp = HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
    }
    else
    {
        auto resp = HttpResponse::newHttpJsonResponse(result);
        callback(resp);
    }
}

// Deletes a product by its ID (seller/admin only). Returns 404 if not found.
void ProductController::deleteProduct(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string productId)
{
    int code = 200;
    std::string authErr = sri_requireSeller(req, code);
    if (!authErr.empty())
    {
        Json::Value error;
        error["error"] = authErr;
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(code == 401 ? k401Unauthorized : k403Forbidden);
        callback(resp);
        return;
    }
    bool deleted = ProductService::deleteProduct(productId);
    if (deleted)
    {
        Json::Value json;
        json["message"] = "Product deleted successfully";
        auto resp = HttpResponse::newHttpJsonResponse(json);
        callback(resp);
    }
    else
    {
        Json::Value error;
        error["error"] = "Product not found";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}

// Searches products: ?q=&minPrice=&maxPrice=&limit=&offset=.
// Invalid numbers return 400 instead of throwing (previous std::stod crash fix).
void ProductController::searchProducts(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    std::string q = req->getParameter("q");
    std::string minS = req->getParameter("minPrice");
    std::string maxS = req->getParameter("maxPrice");
    std::string limS = req->getParameter("limit");
    std::string offS = req->getParameter("offset");
    double minPrice = -1, maxPrice = -1;
    int limit = 100, offset = 0;
    if (!minS.empty() && !sri_parseDouble(minS, minPrice))
    {
        Json::Value error;
        error["error"] = "minPrice must be a number";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!maxS.empty() && !sri_parseDouble(maxS, maxPrice))
    {
        Json::Value error;
        error["error"] = "maxPrice must be a number";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!limS.empty() && (!sri_parseInt(limS, limit) || limit <= 0 || limit > 500))
    {
        Json::Value error;
        error["error"] = "limit must be an integer 1-500";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!offS.empty() && (!sri_parseInt(offS, offset) || offset < 0))
    {
        Json::Value error;
        error["error"] = "offset must be an integer >= 0";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    auto result = ProductService::searchProducts(q, minPrice, maxPrice, limit, offset);
    auto resp = HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}
