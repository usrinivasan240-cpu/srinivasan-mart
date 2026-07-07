# Sri Mart — Project Documentation

## Purpose
Sri Mart is being developed as a C++20 backend/API project. The documentation is intentionally written so a non-coder can understand what each part does.

## Fixed Technology Stack
- C++20
- Drogon
- PostgreSQL (Drogon has built-in ORM support)
- CMake
- vcpkg
- MSYS2 UCRT64 / MinGW
- JSON through the JSON support already provided by Drogon/JsonCpp
- Git/GitHub for source control

**Rule:** Do not introduce another technology stack. New libraries/frameworks/tools are not part of this plan.

## Current Project Structure
```text
sri mart/
├── sri.cpp
├── CMakeLists.txt
├── controllers/
│   └── ProductController.h
├── models/
├── services/
├── config/
└── build-mingw/
```

## Current Known API
- `GET /api/v1/health` → checks whether the API is running.
- `GET /api/v1/hello` → simple Sri Mart test response.
- `GET /api/v1/products` → current ProductController test endpoint.

## Current Build Target
Executable: `sri_mart`
Port: `8080`

## Documentation Rule
Every source module must begin with a plain-English comment explaining:
1. What this file is.
2. Why it exists.
3. What the main class/function does.
4. What another module can expect from it.

Example:
```cpp
// PURPOSE: This controller receives product-related HTTP requests.
// It converts the request into application work and sends a JSON response.
// A non-coder can think of this file as the "front desk" for product requests.
```
