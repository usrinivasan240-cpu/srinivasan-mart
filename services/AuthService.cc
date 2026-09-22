// ============================================================
// MODULE: AuthService
// PURPOSE: Implementation of user authentication logic.
// WHY: Handles user registration, login, and token validation.
// INPUT: User data from controllers.
// OUTPUT: JSON responses with user info or authentication tokens.
// ARCHITECTURE: Service
// ============================================================

#include "AuthService.h"
#include <cctype>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <map>

#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>
#define SRI_HAVE_OPENSSL_SHA 1
#endif

// Generates a unique ID (UUID) for new users.
std::string AuthService::generateUUID()
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

// Current timestamp as "YYYY-MM-DD HH:MM:SS" for created_at/updated_at.
std::string AuthService::currentTimestamp()
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

#if defined(SRI_HAVE_OPENSSL_SHA)
static std::string sri_sha256Hex(const std::string &input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(input.data()), input.size(), hash);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : hash) oss << std::setw(2) << static_cast<int>(c);
    return oss.str();
}
#endif

static std::string sri_randomSaltHex()
{
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_int_distribution<> dis(0, 255);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; i++) oss << std::setw(2) << dis(gen);
    return oss.str();
}

// Salted hash. Stored as "salt$hex". Falls back to salted std::hash chain
// if OpenSSL headers are unavailable (still non-reversible, per-user salt).
std::string AuthService::hashPassword(const std::string &password)
{
    std::string salt = sri_randomSaltHex();
#if defined(SRI_HAVE_OPENSSL_SHA)
    // 10k-round SHA-256(salt + password + round) stretching.
    std::string digest = sri_sha256Hex(salt + password);
    for (int i = 1; i < 10000; i++) digest = sri_sha256Hex(digest + salt + password);
    return salt + "$" + digest;
#else
    std::hash<std::string> hasher;
    size_t h = hasher(salt + password);
    for (int i = 1; i < 10000; i++) h ^= hasher(std::to_string(h) + salt + password + std::to_string(i));
    std::ostringstream oss;
    oss << std::hex << h << hasher(salt + std::to_string(h));
    return salt + "$" + oss.str();
#endif
}

static bool sri_legacyXorEquals(const std::string &stored, const std::string &plain)
{
    std::string hashed = plain;
    for (auto &c : hashed) c = c ^ 0x5A;
    return stored == hashed;
}

bool AuthService::verifyPassword(const std::string &stored, const std::string &plain)
{
    auto pos = stored.find('$');
    if (pos == std::string::npos)
    {
        // Legacy XOR accounts seeded before the fix.
        return sri_legacyXorEquals(stored, plain);
    }
    std::string salt = stored.substr(0, pos);
    std::string expected = stored.substr(pos + 1);
#if defined(SRI_HAVE_OPENSSL_SHA)
    std::string digest = sri_sha256Hex(salt + plain);
    for (int i = 1; i < 10000; i++) digest = sri_sha256Hex(digest + salt + plain);
    return digest == expected;
#else
    std::hash<std::string> hasher;
    size_t h = hasher(salt + plain);
    for (int i = 1; i < 10000; i++) h ^= hasher(std::to_string(h) + salt + plain + std::to_string(i));
    std::ostringstream oss;
    oss << std::hex << h << hasher(salt + std::to_string(h));
    return oss.str() == expected;
#endif
}

// Generates a random authentication token.
std::string AuthService::generateToken()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; i++)
    {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

// Returns the in-memory user list.
// On first call, creates a default admin user (admin/admin123).
std::vector<User> &AuthService::getUserStorage()
{
    static std::vector<User> users;
    if (users.empty())
    {
        std::string now = currentTimestamp();
        // Create default admin user
        User admin;
        admin.id = generateUUID();
        admin.username = "admin";
        admin.email = "admin@srimart.com";
        admin.password = hashPassword("admin123");
        admin.role = "admin";
        admin.created_at = now;
        admin.updated_at = now;
        users.push_back(admin);

        // Create default customer user (buyer)
        User customer;
        customer.id = generateUUID();
        customer.username = "customer";
        customer.email = "customer@srimart.com";
        customer.password = hashPassword("customer123");
        customer.role = "customer";
        customer.created_at = now;
        customer.updated_at = now;
        users.push_back(customer);

        // Create default seller user
        User seller;
        seller.id = generateUUID();
        seller.username = "seller";
        seller.email = "seller@srimart.com";
        seller.password = hashPassword("seller123");
        seller.role = "seller";
        seller.created_at = now;
        seller.updated_at = now;
        users.push_back(seller);
    }
    return users;
}

// Returns the in-memory token storage (token -> user_id).
std::map<std::string, std::string> &AuthService::getTokenStorage()
{
    static std::map<std::string, std::string> tokens;
    return tokens;
}

std::map<std::string, std::chrono::steady_clock::time_point> &AuthService::getTokenExpiry()
{
    static std::map<std::string, std::chrono::steady_clock::time_point> expiry;
    return expiry;
}

// Erases all expired tokens from both maps. Caller must hold getMutex().
void AuthService::purgeExpiredTokens()
{
    auto now = std::chrono::steady_clock::now();
    auto &tokens = getTokenStorage();
    auto &expiry = getTokenExpiry();
    for (auto it = expiry.begin(); it != expiry.end();)
    {
        if (now >= it->second)
        {
            tokens.erase(it->first);
            it = expiry.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ---- login rate limiting (file-local): 5 fails / 10 min -> 5 min lock ----
struct SriLoginAttempt
{
    int fails = 0;
    std::chrono::steady_clock::time_point windowStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lockUntil{};
    bool locked = false;
};

static std::map<std::string, SriLoginAttempt> &sri_loginAttempts()
{
    static std::map<std::string, SriLoginAttempt> attempts;
    return attempts;
}

// ---- input validation (file-local, no <regex> dependency) ----
static bool sri_validUsername(const std::string &u)
{
    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            return false;
    }
    return true;
}

static bool sri_validEmail(const std::string &e)
{
    if (e.size() < 5 || e.size() > 254) return false;
    if (e.find(' ') != std::string::npos) return false;
    auto at = e.find('@');
    if (at == std::string::npos || at == 0 || at == e.size() - 1) return false;
    if (e.find('@', at + 1) != std::string::npos) return false;
    auto dot = e.find('.', at);
    if (dot == std::string::npos || dot == e.size() - 1) return false;
    return true;
}

// Returns a mutex to keep authentication operations safe.
std::mutex &AuthService::getMutex()
{
    static std::mutex mtx;
    return mtx;
}

// Registers a new user with username, email, and password.
// Returns the created user (without password) or an error.
Json::Value AuthService::registerUser(const Json::Value &userData)
{
    std::lock_guard<std::mutex> lock(getMutex());

    // Validate required fields
    if (!userData.isMember("username") || !userData.isMember("email") || !userData.isMember("password"))
    {
        Json::Value error;
        error["error"] = "Username, email, and password are required";
        return error;
    }

    std::string username = userData["username"].asString();
    std::string email = userData["email"].asString();
    std::string password = userData["password"].asString();

    if (!sri_validUsername(username))
    {
        Json::Value error;
        error["error"] = "Username must be 3-32 chars (letters, digits, _ , -)";
        return error;
    }
    if (!sri_validEmail(email))
    {
        Json::Value error;
        error["error"] = "Invalid email address";
        return error;
    }
    if (password.size() < 4 || password.size() > 128)
    {
        Json::Value error;
        error["error"] = "Password must be 4-128 characters";
        return error;
    }

    // Check if username already exists
    for (auto &user : getUserStorage())
    {
        if (user.username == username)
        {
            Json::Value error;
            error["error"] = "Username already exists";
            return error;
        }
        if (user.email == email)
        {
            Json::Value error;
            error["error"] = "Email already registered";
            return error;
        }
    }

    // Create new user. Only customer/seller allowed via self-register;
    // admin accounts must be seeded, never self-granted (privilege-escalation fix).
    std::string requestedRole = userData.get("role", "customer").asString();
    std::string role = "customer";
    if (requestedRole == "seller") role = "seller";
    // "admin" or anything else falls back to customer.

    // Create new user
    User user;
    user.id = generateUUID();
    user.username = username;
    user.email = email;
    user.password = hashPassword(password);
    user.role = role;
    std::string now = currentTimestamp();
    user.created_at = now;
    user.updated_at = now;

    getUserStorage().push_back(user);

    // Return user without password
    return user.toJson();
}

// Logs in a user with username and password.
// Returns a 24h token on success, or an error.
// Rate limit: 5 failures per 10-min window locks the name for 5 min (429).
Json::Value AuthService::loginUser(const std::string &username, const std::string &password)
{
    using clock = std::chrono::steady_clock;
    std::lock_guard<std::mutex> lock(getMutex());

    auto now = clock::now();
    auto &att = sri_loginAttempts()[username];
    if (att.locked)
    {
        if (now < att.lockUntil)
        {
            auto waitSecs = std::chrono::duration_cast<std::chrono::seconds>(
                att.lockUntil - now).count();
            Json::Value error;
            error["error"] = "Too many login attempts, try again later";
            error["locked"] = true;
            error["retry_after_seconds"] = static_cast<int>(waitSecs);
            return error;
        }
        att = SriLoginAttempt(); // lock expired, reset
    }
    else if (now - att.windowStart > std::chrono::minutes(10))
    {
        att.fails = 0;
        att.windowStart = now;
    }

    for (auto &user : getUserStorage())
    {
        if (user.username == username && verifyPassword(user.password, password))
        {
            sri_loginAttempts().erase(username);
            // Generate token with 24h expiry
            std::string token = generateToken();
            getTokenStorage()[token] = user.id;
            getTokenExpiry()[token] = now + std::chrono::hours(kTokenTtlHours);

            Json::Value result;
            result["token"] = token;
            result["user"] = user.toJson();
            result["expires_in_hours"] = kTokenTtlHours;
            return result;
        }
    }

    att.fails++;
    if (att.fails >= 5)
    {
        att.locked = true;
        att.lockUntil = now + std::chrono::minutes(5);
        Json::Value error;
        error["error"] = "Too many login attempts, try again in 5 minutes";
        error["locked"] = true;
        error["retry_after_seconds"] = 300;
        return error;
    }
    Json::Value error;
    error["error"] = "Invalid username or password";
    return error;
}

// Validates an authentication token.
// Returns the user if token is valid, or an error.
Json::Value AuthService::validateToken(const std::string &token)
{
    std::lock_guard<std::mutex> lock(getMutex());
    purgeExpiredTokens();

    auto &tokens = getTokenStorage();
    if (tokens.find(token) != tokens.end())
    {
        std::string userId = tokens[token];
        for (auto &user : getUserStorage())
        {
            if (user.id == userId)
            {
                return user.toJson();
            }
        }
    }

    Json::Value error;
    error["error"] = "Invalid or expired token";
    return error;
}

// Returns all users (without passwords).
Json::Value AuthService::getAllUsers()
{
    std::lock_guard<std::mutex> lock(getMutex());
    Json::Value result(Json::arrayValue);
    for (auto &user : getUserStorage())
    {
        result.append(user.toJson());
    }
    return result;
}

// Finds a user by ID.
Json::Value AuthService::getUserById(const std::string &id)
{
    std::lock_guard<std::mutex> lock(getMutex());
    for (auto &user : getUserStorage())
    {
        if (user.id == id)
        {
            return user.toJson();
        }
    }
    Json::Value error;
    error["error"] = "User not found";
    return error;
}

// Logs out a token by erasing it from token + expiry storage.
bool AuthService::logoutToken(const std::string &token)
{
    std::lock_guard<std::mutex> lock(getMutex());
    auto &tokens = getTokenStorage();
    auto it = tokens.find(token);
    if (it != tokens.end())
    {
        tokens.erase(it);
        getTokenExpiry().erase(token);
        return true;
    }
    return false;
}

// Returns user_id for a token, or empty string if invalid/expired.
std::string AuthService::getUserIdFromToken(const std::string &token)
{
    std::lock_guard<std::mutex> lock(getMutex());
    purgeExpiredTokens();
    auto &tokens = getTokenStorage();
    auto it = tokens.find(token);
    if (it != tokens.end())
    {
        return it->second;
    }
    return "";
}

// Returns role for a token, or "" if invalid/expired. Used for seller/admin gates.
std::string AuthService::getRoleFromToken(const std::string &token)
{
    std::lock_guard<std::mutex> lock(getMutex());
    purgeExpiredTokens();
    auto &tokens = getTokenStorage();
    auto it = tokens.find(token);
    if (it == tokens.end()) return "";
    std::string userId = it->second;
    for (auto &user : getUserStorage())
    {
        if (user.id == userId) return user.role;
    }
    return "";
}
