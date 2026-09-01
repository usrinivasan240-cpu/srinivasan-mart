// ============================================================
// MODULE: sri.cpp
// PURPOSE: Application entry point for Sri Mart.
// WHY: Starts the Drogon server, registers routes, listens on port 8080.
// INPUT: None (program start).
// OUTPUT: Running HTTP server with registered API endpoints.
// ARCHITECTURE: Entry point
// ============================================================

#include <drogon/drogon.h>
#include "controllers/ProductController.h"
#include "controllers/AuthController.h"
#include "controllers/CartController.h"
#include "controllers/OrderController.h"
#include "controllers/ReviewController.h"
#include "controllers/AdminController.h"
#include "controllers/ChatController.h"
#include "services/ProductService.h"
#include <fstream>
#include <sstream>

int main()
{
    // Start the server and listen for HTTP requests on port 8080.
    drogon::app().addListener("0.0.0.0", 8080);

    // Health check endpoint: tells us if the server is alive.
    drogon::app().registerHandler(
        "/api/v1/health",
        [](const drogon::HttpRequestPtr &req,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            Json::Value json;
            json["status"] = "UP";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        },
        {drogon::Get});

    // Hello endpoint: simple test to prove the API is working.
    drogon::app().registerHandler(
        "/api/v1/hello",
        [](const drogon::HttpRequestPtr &req,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            Json::Value json;
            json["message"] = "Welcome to Sri Mart API";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        },
        {drogon::Get});

    // Login page: serves the HTML login/register form.
    drogon::app().registerHandler(
        "/",
        [](const drogon::HttpRequestPtr &req,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            std::ifstream file("config/login.html");
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setBody(buffer.str());
                resp->setContentTypeCode(drogon::CT_TEXT_HTML);
                callback(resp);
            }
            else
            {
                Json::Value json;
                json["message"] = "Sri Mart API is running. Visit /api/v1/health for status.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
                callback(resp);
            }
        },
        {drogon::Get});

    // Register all product-related routes (GET, POST, PUT, DELETE, search).
    ProductController::initRoutes();

    // Register all authentication routes (login, register, validate, logout).
    AuthController::initRoutes();

    // Register Week 2-11 stubs: cart, orders, reviews, seller/admin, chatbot.
    CartController::initRoutes();
    OrderController::initRoutes();
    ReviewController::initRoutes();
    AdminController::initRoutes();
    ChatController::initRoutes();

    // Initialize the PostgreSQL database table for products.
    ProductService::initializeDatabase();

    // Keep the server running and handle requests.
    std::cout << "Sri Mart API running on http://0.0.0.0:8080" << std::endl;
    std::cout << "Login page: http://localhost:8080/" << std::endl;
    drogon::app().run();

    return 0;
}
