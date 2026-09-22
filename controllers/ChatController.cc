// ============================================================
// MODULE: ChatController
// PURPOSE: Implementation of chatbot stub.
// WHY: Keyword replies for demo; replace with LLM call later.
// ARCHITECTURE: Controller
// ============================================================

#include "ChatController.h"
#include <cctype>

using namespace drogon;

void ChatController::initRoutes()
{
    app().registerHandler("/api/v1/chat", &ChatController::chat, {Post});
}

void ChatController::chat(const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();
    std::string msg = (json && json->isMember("message")) ? (*json)["message"].asString() : "";
    if (msg.empty())
    {
        Json::Value e; e["error"] = "message is required";
        auto resp = HttpResponse::newHttpJsonResponse(e);
        resp->setStatusCode(k400BadRequest); callback(resp); return;
    }
    std::string lower = msg;
    for (auto &c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    std::string reply = "Sri Mart bot: I can help with products, orders, and auth. LLM integration planned.";
    if (lower.find("hour") != std::string::npos) reply = "Sri Mart bot: Open 9am-9pm.";
    else if (lower.find("return") != std::string::npos) reply = "Sri Mart bot: Returns within 7 days with bill.";
    else if (lower.find("offer") != std::string::npos || lower.find("deal") != std::string::npos || lower.find("discount") != std::string::npos) reply = "Sri Mart bot: Check /api/v1/products/search for deals.";
    Json::Value ok;
    ok["reply"] = reply;
    ok["echo"] = msg;
    auto resp = HttpResponse::newHttpJsonResponse(ok);
    callback(resp);
}
