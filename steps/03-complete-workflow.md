# STEP 3: Complete Project Workflow — Everything Connected

This file shows how all parts of Sri Mart work together, from server startup to every feature.

---

## Part 1: Server Startup

### What happens when you run `sri_mart.exe`:

**Prerequisites:**
1. Start PostgreSQL: `pg_ctl.exe -D C:\pgsql_data start`
2. Set PATH: `set PATH=C:\Users\Srinivasan\MSYS2\ucrt64\bin;%PATH%`
3. Or just double-click `start.bat`

```cpp
// sri.cpp - The entry point
int main()
{
    // 1. Start listening on port 8080
    drogon::app().addListener("0.0.0.0", 8080);

    // 2. Register health check endpoint
    drogon::app().registerHandler("/api/v1/health", ...);

    // 3. Register hello endpoint
    drogon::app().registerHandler("/api/v1/hello", ...);

    // 4. Register login page
    drogon::app().registerHandler("/", ...);

    // 5. Register product routes
    ProductController::initRoutes();

    // 6. Register auth routes
    AuthController::initRoutes();

    // 7. Start the server
    drogon::app().run();
}
```

**What each line does:**
- `addListener("0.0.0.0", 8080)` — Opens port 8080 and waits for connections
- `registerHandler("/api/v1/health", ...)` — When someone visits /api/v1/health, send back {"status":"UP"}
- `ProductController::initRoutes()` — Sets up all product URLs (GET, POST, PUT, DELETE)
- `AuthController::initRoutes()` — Sets up all auth URLs (login, register, validate)
- `drogon::app().run()` — Start the server and keep it running forever

---

## Part 2: The Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     YOUR BROWSER                         │
│  (You type URLs and click buttons)                       │
└─────────────────────┬───────────────────────────────────┘
                      │
                      │ HTTP Request
                      │ (GET, POST, PUT, DELETE)
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                   SRI MART SERVER                         │
│  (sri.cpp - The entry point)                             │
│                                                          │
│  1. Receives the request                                 │
│  2. Finds the matching URL                               │
│  3. Sends it to the right Controller                     │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                   CONTROLLERS                             │
│  (The "front desk" - validates and routes)               │
│                                                          │
│  AuthController  │  ProductController                    │
│  - registerUser  │  - getProducts                        │
│  - loginUser     │  - createProduct                      │
│  - validateToken │  - getProductById                     │
│                  │  - updateProduct                       │
│                  │  - deleteProduct                       │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                   SERVICES                                │
│  (The "manager" - business logic)                        │
│                                                          │
│  AuthService     │  ProductService                       │
│  - hashPassword  │  - generateUUID                       │
│  - generateToken │  - getAllProducts                      │
│  - registerUser  │  - getProductById                     │
│  - loginUser     │  - createProduct                      │
│  - validateToken │  - updateProduct                      │
│                  │  - deleteProduct                       │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                   STORAGE                                  │
│                                                          │
│  ┌─────────────────────────────────────────────────────┐ │
│  │  PostgreSQL (port 5433)                             │ │
│  │  Database: sri_mart                                 │ │
│  │  Tables:                                            │ │
│  │  - products (id, name, description, price, stock)   │ │
│  └─────────────────────────────────────────────────────┘ │
│                                                          │
│  ┌─────────────────────────────────────────────────────┐ │
│  │  In-Memory Storage (AuthService)                    │ │
│  │  - Users (will be moved to PostgreSQL later)        │ │
│  │  - Tokens (session management)                      │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

---

## Part 3: Complete Request/Response Cycle

### Example: Creating a Product

```
STEP 1: Browser sends POST request
─────────────────────────────────────
POST /api/v1/products HTTP/1.1
Content-Type: application/json

{"name":"Laptop","price":999.99,"stock":10}

         │
         ▼

STEP 2: sri.cpp receives the request
─────────────────────────────────────
- Request comes in on port 8080
- URL matches "/api/v1/products"
- Method is POST
- Route found: ProductController::createProduct

         │
         ▼

STEP 3: ProductController::createProduct() runs
─────────────────────────────────────
- Extracts JSON from request
- Checks: Is "name" provided? ✓
- Checks: Is "price" provided? ✓
- Calls: ProductService::createProduct(*json)

         │
         ▼

STEP 4: ProductService::createProduct() runs
─────────────────────────────────────
- Locks the storage (thread safety)
- Generates a UUID: "a1b2c3d4-e5f6-7890-abcd-ef1234567890"
- Creates Product object:
    id = "a1b2c3d4-e5f6-7890-abcd-ef1234567890"
    name = "Laptop"
    price = 999.99
    stock = 10
- Adds to storage vector
- Returns product as JSON

         │
         ▼

STEP 5: Response goes back to browser
─────────────────────────────────────
HTTP/1.1 201 Created
Content-Type: application/json

{
    "id": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
    "name": "Laptop",
    "price": 999.99,
    "stock": 10
}

         │
         ▼

STEP 6: Browser shows the result
─────────────────────────────────────
You see the new product with its ID
```

---

## Part 4: All Files and Their Roles

### Entry Point
| File | Role | What It Does |
|------|------|--------------|
| `sri.cpp` | Server startup | Starts server, registers all routes |

### Controllers (Front Desk)
| File | Role | What It Does |
|------|------|--------------|
| `AuthController.h` | Auth declarations | Declares login/register functions |
| `AuthController.cc` | Auth implementations | Handles login, register, validate requests |
| `ProductController.h` | Product declarations | Declares product CRUD functions |
| `ProductController.cc` | Product implementations | Handles product GET/POST/PUT/DELETE |

### Services (Manager)
| File | Role | What It Does |
|------|------|--------------|
| `AuthService.h` | Auth business logic declarations | Declares auth operations |
| `AuthService.cc` | Auth business logic implementations | Hashes passwords, generates tokens, validates |
| `ProductService.h` | Product business logic declarations | Declares product operations |
| `ProductService.cc` | Product business logic implementations | CRUD operations on products |

### Models (Filing Cabinet)
| File | Role | What It Does |
|------|------|--------------|
| `User.h` | User data structure | Defines what a user looks like |
| `Product.h` | Product data structure | Defines what a product looks like |

### Config (Settings)
| File | Role | What It Does |
|------|------|--------------|
| `drogon.json` | Server settings | Port, database, logging config |
| `login.html` | Login page | HTML/CSS/JS for login/register form |

---

## Part 5: Data Flow for Each Feature

### Feature 1: Registration
```
Browser → POST /api/v1/auth/register → AuthController → AuthService → User Storage
         ← 201 Created + User JSON ←─────────────────────────────────────────────
```

### Feature 2: Login
```
Browser → POST /api/v1/auth/login → AuthController → AuthService → User Storage + Token Storage
         ← 200 OK + Token + User JSON ←──────────────────────────────────────────────────────
```

### Feature 3: Token Validation
```
Browser → GET /api/v1/auth/validate + Bearer token → AuthController → AuthService → Token Storage
         ← 200 OK + User JSON ←───────────────────────────────────────────────────────────────
```

### Feature 4: List Products
```
Browser → GET /api/v1/products → ProductController → ProductService → Product Storage
         ← 200 OK + Products JSON ←───────────────────────────────────────────────
```

### Feature 5: Create Product
```
Browser → POST /api/v1/products → ProductController → ProductService → Product Storage
         ← 201 Created + Product JSON ←─────────────────────────────────────────────
```

### Feature 6: Update Product
```
Browser → PUT /api/v1/products/{id} → ProductController → ProductService → Product Storage
         ← 200 OK + Updated Product JSON ←──────────────────────────────────────────────
```

### Feature 7: Delete Product
```
Browser → DELETE /api/v1/products/{id} → ProductController → ProductService → Product Storage
         ← 200 OK + "Product deleted successfully" ←────────────────────────────────────────
```

---

## Part 6: The Code Flow for Registration (Detailed)

### Step-by-step with exact code:

```
1. BROWSER: User clicks "Register"
   │
   │ Code: <button onclick="register()">Register</button>
   │
   ▼

2. BROWSER: JavaScript collectForm() function runs
   │
   │ Code: const username = document.getElementById('regUsername').value;
   │       const email = document.getElementById('regEmail').value;
   │       const password = document.getElementById('regPassword').value;
   │
   ▼

3. BROWSER: JavaScript sends HTTP request
   │
   │ Code: const response = await fetch('/api/v1/auth/register', {
   │           method: 'POST',
   │           headers: { 'Content-Type': 'application/json' },
   │           body: JSON.stringify({ username, email, password })
   │       });
   │
   ▼

4. SERVER: sri.cpp receives request
   │
   │ Code: drogon::app().run();  // Server is running, accepts connection
   │
   ▼

5. SERVER: Route matching finds AuthController::registerUser
   │
   │ Code: app().registerHandler("/api/v1/auth/register",
   │           &AuthController::registerUser, {Post});
   │
   ▼

6. SERVER: AuthController::registerUser() runs
   │
   │ Code: auto json = req->getJsonObject();
   │       // Check: if (!json || !json->isMember("username") || ...)
   │       auto result = AuthService::registerUser(*json);
   │
   ▼

7. SERVER: AuthService::registerUser() runs
   │
   │ Code: std::lock_guard<std::mutex> lock(getMutex());
   │       // Check: for (auto &user : getUserStorage())
   │       //         if (user.username == username) return error;
   │       User user;
   │       user.id = generateUUID();
   │       user.username = username;
   │       user.email = email;
   │       user.password = hashPassword(password);
   │       getUserStorage().push_back(user);
   │       return user.toJson();
   │
   ▼

8. SERVER: Response sent back to browser
   │
   │ Code: auto resp = HttpResponse::newHttpJsonResponse(result);
   │       resp->setStatusCode(k201Created);
   │       callback(resp);
   │
   ▼

9. BROWSER: JavaScript receives response
   │
   │ Code: const data = await response.json();
   │       if (response.ok) {
   │           showMessage('Registration successful!');
   │           showForm('login');
   │       }
   │
   ▼

10. BROWSER: User sees "Registration successful!"
```

---

## Part 7: The Code Flow for Login (Detailed)

```
1. BROWSER: User clicks "Login"
   │
   │ Code: <button onclick="login()">Login</button>
   │
   ▼

2. BROWSER: JavaScript collectForm() function runs
   │
   │ Code: const username = document.getElementById('loginUsername').value;
   │       const password = document.getElementById('loginPassword').value;
   │
   ▼

3. BROWSER: JavaScript sends HTTP request
   │
   │ Code: const response = await fetch('/api/v1/auth/login', {
   │           method: 'POST',
   │           headers: { 'Content-Type': 'application/json' },
   │           body: JSON.stringify({ username, password })
   │       });
   │
   ▼

4. SERVER: sri.cpp receives request
   │
   ▼

5. SERVER: Route matching finds AuthController::loginUser
   │
   │ Code: app().registerHandler("/api/v1/auth/login",
   │           &AuthController::loginUser, {Post});
   │
   ▼

6. SERVER: AuthController::loginUser() runs
   │
   │ Code: auto json = req->getJsonObject();
   │       std::string username = (*json)["username"].asString();
   │       std::string password = (*json)["password"].asString();
   │       auto result = AuthService::loginUser(username, password);
   │
   ▼

7. SERVER: AuthService::loginUser() runs
   │
   │ Code: std::string hashedPassword = hashPassword(password);
   │       for (auto &user : getUserStorage()) {
   │           if (user.username == username && user.password == hashedPassword) {
   │               std::string token = generateToken();
   │               getTokenStorage()[token] = user.id;
   │               Json::Value result;
   │               result["token"] = token;
   │               result["user"] = user.toJson();
   │               return result;
   │           }
   │       }
   │       return error;  // "Invalid username or password"
   │
   ▼

8. SERVER: Response sent back to browser
   │
   │ Code: auto resp = HttpResponse::newHttpJsonResponse(result);
   │       callback(resp);
   │
   ▼

9. BROWSER: JavaScript receives response
   │
   │ Code: const data = await response.json();
   │       if (response.ok) {
   │           showMessage('Login successful! Welcome ' + data.user.username);
   │           localStorage.setItem('token', data.token);
   │           localStorage.setItem('user', JSON.stringify(data.user));
   │       }
   │
   ▼

10. BROWSER: User sees "Login successful!" and token is saved
```

---

## Part 8: How Thread Safety Works

When multiple users access the server at the same time, we need to prevent data corruption.

```cpp
// This is how we keep data safe:
std::lock_guard<std::mutex> lock(getMutex());

// "lock" means: "I'm using the storage, nobody else can touch it"
// When the function ends, "lock_guard" automatically unlocks

// Without lock:
// User A reads: stock = 10
// User B reads: stock = 10
// User A sets: stock = 9
// User B sets: stock = 9
// Result: stock = 9 (WRONG! Should be 8)

// With lock:
// User A locks, reads stock = 10, sets stock = 9, unlocks
// User B locks, reads stock = 9, sets stock = 8, unlocks
// Result: stock = 8 (CORRECT!)
```

---

## Part 9: How Password Hashing Works

```
Registration:
─────────────
You enter: "admin123"
Hashed:    hashPassword("admin123") = "1!2#3$4%5&"
Stored:    user.password = "1!2#3$4%5&"

Login:
──────
You enter: "admin123"
Hashed:    hashPassword("admin123") = "1!2#3$4%5&"
Compare:   "1!2#3$4%5&" == "1!2#3$4%5&" ✓ MATCH!

Wrong password:
───────────────
You enter: "wrongpassword"
Hashed:    hashPassword("wrongpassword") = "x@y#z$w%v&"
Compare:   "x@y#z$w%v&" != "1!2#3$4%5&" ✗ NO MATCH!
```

---

## Part 10: Complete File Reference

### Source Files
```
sri.cpp                    → Server entry point, route registration
CMakeLists.txt             → Build instructions
start.bat                  → Quick start (PostgreSQL + server)
dev.bat                    → Dev mode (rebuild + run)
stop.bat                   → Stop PostgreSQL
```

### Controllers
```
controllers/AuthController.h      → Auth route declarations
controllers/AuthController.cc     → Auth route implementations
controllers/ProductController.h   → Product route declarations
controllers/ProductController.cc  → Product route implementations
```

### Services
```
services/AuthService.h      → Auth business logic declarations
services/AuthService.cc     → Auth business logic implementations
services/ProductService.h   → Product business logic declarations
services/ProductService.cc  → Product business logic implementations
```

### Models
```
models/User.h      → User data structure
models/Product.h   → Product data structure
```

### Config
```
config/drogon.json  → Server settings
config/login.html   → Login/Register web page
```

### Database
```
PostgreSQL (port 5433) → sri_mart database → products table
Location: C:\pgsql_data
Binaries: postgresql\pgsql\bin\
```

### Verify PostgreSQL
```cmd
:: Check if PostgreSQL is running
"C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\psql.exe" -U postgres -h localhost -p 5433 -d sri_mart -c "\dt"

:: View all products
"C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\psql.exe" -U postgres -h localhost -p 5433 -d sri_mart -c "SELECT * FROM products;"
```

### Documentation
```
README.md              → Project overview
ARCHITECTURE.md        → Architecture explanation
WORKFLOW.md            → Complete workflow
steps/01-registration-flow.md  → Registration step-by-step
steps/02-login-flow.md         → Login step-by-step
steps/03-complete-workflow.md  → This file
```

---

## Part 11: How to Test Everything

### Test Registration:
```cmd
curl -X POST http://localhost:8080/api/v1/auth/register -H "Content-Type: application/json" -d "{\"username\":\"test\",\"email\":\"test@test.com\",\"password\":\"test123\"}"
```

### Test Login:
```cmd
curl -X POST http://localhost:8080/api/v1/auth/login -H "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"admin123\"}"
```

### Test Validate Token:
```cmd
curl -X GET http://localhost:8080/api/v1/auth/validate -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

### Test Get Products:
```cmd
curl -X GET http://localhost:8080/api/v1/products
```

### Test Create Product:
```cmd
curl -X POST http://localhost:8080/api/v1/products -H "Content-Type: application/json" -d "{\"name\":\"Phone\",\"price\":699.99,\"stock\":25}"
```

---

## Part 12: What Each Status Code Means

| Code | Name | When Used |
|------|------|-----------|
| 200 | OK | Request succeeded |
| 201 | Created | New resource created |
| 400 | Bad Request | Invalid input (missing fields) |
| 401 | Unauthorized | Wrong credentials or no token |
| 404 | Not Found | Resource doesn't exist |
| 405 | Method Not Allowed | Wrong HTTP method for the URL |
