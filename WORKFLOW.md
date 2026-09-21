# Srinivasan Mart — Full Frontend ↔ Backend Workflow (Exact Code)

> How a click on the frontend travels to the backend, how data is passed, verified, stored, and how the next page/frontend changes. Every step shows `file:line` + exact code + explanation.
> Stack: `login.html` (frontend) → Drogon routes → `controllers/*` → `services/*` → `models/*` → PostgreSQL (products) or in-memory (auth/cart/orders).

## 0. Architecture — How Frontend and Backend Are Connected

```
[Browser: login.html] --fetch JSON--> [Drogon HTTP server: sri.cpp] --route--> [Controller] --calls--> [Service] --uses--> [Model/DB] --JSON--> [Browser]
```
- `sri.cpp:25` opens port 8080, `config/drogon.json:4-5` same port.
- `sri.cpp:80-90` wires every URL to a controller function.
- Frontend uses `fetch(API_BASE + '/api/v1/...')` with `Content-Type: application/json` and `Authorization: Bearer <token>`.
- Backend always returns `HttpResponse::newHttpJsonResponse(json)` with status 200/201/400/401/404.

---

## 1. LOGIN — Full Click-to-Verify-to-Next-Page Workflow

### Step 1: User types and clicks Login

**File `config/login.html:42-52`**
```html
<!-- Login Form -->
<div id="loginForm" class="form active">
  <div class="form-group">
    <label>Username</label>
    <input type="text" id="loginUsername" placeholder="Enter username">
  </div>
  <div class="form-group">
    <label>Password</label>
    <input type="password" id="loginPassword" placeholder="Enter password">
  </div>
  <button onclick="login()">Login</button>  <!-- CLICK TRIGGERS login() -->
</div>
```
- User enters `admin` / `admin123` and clicks **Login** → browser calls JS `login()`.

### Step 2: Frontend passes data to backend via `fetch`

**File `config/login.html:105-119` — exact code**
```js
async function login() {
  const username = document.getElementById('loginUsername').value; // 106
  const password = document.getElementById('loginPassword').value; // 107
  if (!username || !password) { // 109
    showMessage('Please fill in all fields', 'error'); return;
  }
  try {
    const response = await fetch(API_BASE + '/api/v1/auth/login', { // 115
      method: 'POST',
      headers: { 'Content-Type': 'application/json' }, // tells backend: JSON body
      body: JSON.stringify({ username, password }) // 118 — data passed as JSON string
    });
    const data = await response.json(); // 120 — backend reply
```
- `API_BASE = 'http://localhost:8080'` at `77`
- `JSON.stringify({username,password})` converts `{username:"admin", password:"admin123"}` to `'{"username":"admin","password":"admin123"}'` and sends in HTTP body.
- Headers `Content-Type: application/json` lets Drogon parse it via `getJsonObject()`.

### Step 3: Backend route receives request

**File `sri.cpp:83` + `controllers/AuthController.cc:24-26`**
```cpp
// sri.cpp:83
AuthController::initRoutes();
// AuthController.cc:24-26
app().registerHandler("/api/v1/auth/login", &AuthController::loginUser, {Post});
```
- Drogon matches `POST /api/v1/auth/login` → calls `AuthController::loginUser`.

### Step 4: Controller extracts and validates JSON

**File `controllers/AuthController.cc:79-109`**
```cpp
void AuthController::loginUser(const HttpRequestPtr &req, ...) { // 79
  auto json = req->getJsonObject(); // 83 — parses body JSON
  if (!json || !json->isMember("username") || !json->isMember("password")) { // 84
    // 400 Username and password are required
  }
  std::string username = (*json)["username"].asString(); // 94
  std::string password = (*json)["password"].asString(); // 95
  auto result = AuthService::loginUser(username, password); // 97 — call service
  if (result.isMember("error")) { // 98
    resp->setStatusCode(k401Unauthorized); // wrong credentials
  } else {
    auto resp = HttpResponse::newHttpJsonResponse(result); // 101 — {token, user}
    callback(resp);
  }
}
```
- Controller does **no DB logic**, only request handling.

### Step 5: Service verifies email/username + password (how verification works)

**File `services/AuthService.cc:165-189`**
```cpp
Json::Value AuthService::loginUser(const std::string &username, const std::string &password) { // 165
  std::lock_guard<std::mutex> lock(getMutex()); // 167 thread-safe
  std::string hashedPassword = hashPassword(password); // 169 — hash input
  for (auto &user : getUserStorage()) { // 171 — loop in-memory users
    if (user.username == username && user.password == hashedPassword) { // 173 — VERIFY
      std::string token = generateToken(); // 176 — 32 hex chars, 52-64
      getTokenStorage()[token] = user.id; // 177 — store token→userId
      Json::Value result;
      result["token"] = token; // 180
      result["user"] = user.toJson(); // 181 — excludes password, User.h:31-41
      return result;
    }
  }
  Json::Value error; error["error"] = "Invalid username or password"; return error; // 186
}
```
**How password is hashed — `services/AuthService.cc:41-49`**
```cpp
std::string AuthService::hashPassword(const std::string &password) {
  std::string hashed = password;
  for (auto &c : hashed) c = c ^ 0x5A; // XOR each char with 0x5A (demo only, planned bcrypt)
  return hashed;
}
```
**Where users are stored — `services/AuthService.cc:68-96`**
```cpp
std::vector<User> &AuthService::getUserStorage() {
  static std::vector<User> users;
  if (users.empty()) {
    User admin; admin.id = generateUUID(); admin.username="admin";
    admin.email="admin@srimart.com"; admin.password=hashPassword("admin123"); admin.role="admin";
    users.push_back(admin); // seeded on first call
  }
  return users;
}
```
- Verification is **username + hashed password equality** against in-memory `vector<User>`. On success, a random token is generated and mapped in `map<string,string> tokens` at `services/AuthService.cc:99-103`.

### Step 6: Backend sends JSON back, frontend changes page

**Backend returns** `{ "token": "a3f9...", "user": { "id": "...", "username":"admin", "email":"admin@srimart.com", "role":"admin" } }` with 200.

**Frontend `config/login.html:122-133` handles reply and does next-page/frontend change:**
```js
if (response.ok) { // 122
  showMessage('Login successful! Welcome ' + data.user.username, 'success'); // 123
  document.getElementById('tokenBox').textContent = 'Token: ' + data.token; // 124
  document.getElementById('tokenBox').style.display = 'block'; // 125
  localStorage.setItem('token', data.token); // 126 — SAVE token for next requests
  localStorage.setItem('user', JSON.stringify(data.user)); // 127 — SAVE user
  // NEXT PAGE LOGIC: In this app, login.html stays but shows token. For a multi-page app you would do:
  // window.location.href = '/products.html';  // or '/dashboard.html'
  // That new page would then do: fetch('/api/v1/products', {headers: {'Authorization':'Bearer '+localStorage.getItem('token')}})
} else {
  showMessage(data.error || 'Login failed', 'error'); // 129
}
```
- `localStorage` keeps login across page reloads. The “next page open” is **frontend-driven**: after storing token, JS can `window.location.href = '/dashboard.html'` or show hidden sections. In this demo the token box appears and you would manually navigate to product APIs. For a full app, the product page would read `localStorage.getItem('token')` and send it as `Authorization: Bearer <token>`.

---

## 2. LOGOUT — How Frontend Clears and Backend Invalidates

### Frontend click (add this button to any authenticated page)
```html
<button onclick="logout()">Logout</button>
<script>
async function logout(){
  const token = localStorage.getItem('token');
  await fetch('http://localhost:8080/api/v1/auth/logout', {
    method:'POST',
    headers:{'Authorization':'Bearer '+token} // pass token to backend
  });
  localStorage.removeItem('token'); // frontend change
  localStorage.removeItem('user');
  window.location.href = '/'; // back to login page — frontend navigation
  // or showForm('login');
}
</script>
```

### Backend `controllers/AuthController.cc:38-41,154-184`
```cpp
// 38-41 route
app().registerHandler("/api/v1/auth/logout", &AuthController::logoutUser, {Post});
// 154-184 handler
void AuthController::logoutUser(...) {
  std::string authHeader = req->getHeader("Authorization"); // 158
  if (authHeader.find("Bearer ") != 0) return 401;
  std::string token = authHeader.substr(7); // 168
  if (AuthService::logoutToken(token)) { // 169
    ok["message"]="Logged out successfully";
  }
}
```
**Service `services/AuthService.cc:243-267`**
```cpp
bool AuthService::logoutToken(const std::string &token){
  auto &tokens = getTokenStorage();
  auto it = tokens.find(token);
  if (it != tokens.end()){ tokens.erase(it); return true; }
  return false;
}
```
- Backend **erases token** from in-memory map, so `validateToken` will fail afterwards. Frontend **clears storage + redirects**.

---

## 3. REGISTER — Similar to Login but Creates User

**Frontend `config/login.html:136-163`**
```js
async function register(){
  const username=document.getElementById('regUsername').value; // 137
  const email=document.getElementById('regEmail').value;
  const password=document.getElementById('regPassword').value;
  const response = await fetch(API_BASE+'/api/v1/auth/register', { // 147
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({username,email,password}) // 150
  });
  if(response.ok){ showMessage('Registration successful! Please login.','success'); showForm('login'); } // 154-156 — frontend switches tab to login
}
```
**Controller `controllers/AuthController.cc:46-74`** validates 3 fields, calls `AuthService::registerUser`.
**Service `services/AuthService.cc:114-161`** checks duplicate username/email via loop `131-145`, creates `User{id=generateUUID():17-37, password=hashPassword(), role=customer}`, pushes to `users`, returns `user.toJson()` with 201.

---

## 4. CRUD OPERATIONS — Product Example (Database Connected)

All product routes at `controllers/ProductController.cc:17-44` wired in `sri.cpp:80`.

### CREATE — Add product
**Frontend**
```js
await fetch('http://localhost:8080/api/v1/products', {
  method:'POST', headers:{'Content-Type':'application/json', 'Authorization':'Bearer '+token},
  body: JSON.stringify({name:"Phone", price:699.99, description:"Smartphone", stock:25})
});
```
**Controller `controllers/ProductController.cc:57-75`**
```cpp
auto json = req->getJsonObject(); // 61
if (!json->isMember("name") || !json->isMember("price")) return 400; // 62
auto result = ProductService::createProduct(*json); // 71
resp->setStatusCode(k201Created);
```
**Service `services/ProductService.cc:198-232`**
```cpp
std::string id = generateUUID(); // 208
std::string name = productData["name"].asString(); // 209
const char *paramValues[5] = {id.c_str(), name.c_str(), description.c_str(), price.c_str(), stock.c_str()};
PGresult *res = PQexecParams(conn,
  "INSERT INTO products (id, name, description, price, stock) VALUES ($1,$2,$3,$4::decimal,$5::integer)", // 217
  5, nullptr, paramValues, nullptr,nullptr,0);
return getProductById(id); // 231 — re-read from DB
```
**DB `services/ProductService.cc:20-41,76-106`** — `getConnection()` connects to `host=localhost port=5433 dbname=sri_mart`, `initializeDatabase()` creates table `id VARCHAR(36) PK, name VARCHAR(255), price DECIMAL(10,2), stock INT`.

### READ — List and Get One
- **List**: `GET /api/v1/products` → `controllers/ProductController.cc:46-53:getProducts()` → `services/ProductService.cc:109-147:getAllProducts()` → `118:SELECT ... FROM products` → loop `PQgetvalue` → JSON array → frontend `await response.json()` and render list.
- **Get one**: `GET /api/v1/products/{id}` → `78-97:getProductById()` → `services/ProductService.cc:150-195:PQexecParams SELECT ... WHERE id=$1` → 404 if Null.

### UPDATE — Edit product (the `edit` you asked)
**Frontend (edit button on product row)**
```js
// user clicks Edit, fills form, clicks Save
await fetch('http://localhost:8080/api/v1/products/'+productId, {
  method:'PUT',
  headers:{'Content-Type':'application/json'},
  body: JSON.stringify({name:"Phone Pro", price:799.99, stock:20})
});
 // frontend then updates DOM: document.getElementById('name-'+id).textContent = "Phone Pro"
```
**Controller `controllers/ProductController.cc:101-130`**
```cpp
void ProductController::updateProduct(..., std::string productId){ // 101
  auto json = req->getJsonObject(); if(!json) return 400; // 106
  auto result = ProductService::updateProduct(productId, *json); // 116
  if(result.isNull()) return 404; else callback(json);
}
```
**Service `services/ProductService.cc:235-279`**
```cpp
// 244 check exists
PGresult *checkRes = PQexecParams(conn, "SELECT id FROM products WHERE id=$1",...);
if(PQntuples(checkRes)==0) return Null;
// 264 update
PGresult *res = PQexecParams(conn,
  "UPDATE products SET name=$1, description=$2, price=$3::decimal, stock=$4::integer, updated_at=CURRENT_TIMESTAMP WHERE id=$5", // 265
  5, nullptr, paramValues,...,0);
return getProductById(id); // 278 — return updated row
```
- Frontend sees updated JSON and **re-renders** the product card without full reload.

### DELETE
`DELETE /api/v1/products/{id}` → `controllers/ProductController.cc:133-154:deleteProduct()` → `services/ProductService.cc:282-305:DELETE FROM products WHERE id=$1` → `PQcmdTuples[0]!='0'` → frontend removes element: `element.remove()`.

---

## 5. ADD TO CART — Full Workflow

**Frontend button on product list**
```html
<button onclick="addToCart('product-uuid-123')">Add to Cart</button>
<script>
async function addToCart(productId){
  const token = localStorage.getItem('token');
  const resp = await fetch('http://localhost:8080/api/v1/cart', {
    method:'POST',
    headers:{'Content-Type':'application/json','Authorization':'Bearer '+token},
    body: JSON.stringify({productId, quantity:1}) // data passed
  });
  const data = await resp.json();
  if(resp.ok){
    document.getElementById('cartCount').textContent = parseInt(cartCount)+1; // frontend change: badge +1
    showMessage('Added to cart','success');
  }
}
</script>
```

**Route `controllers/CartController.cc:27-41`**
```cpp
app().registerHandler("/api/v1/cart", &CartController::addToCart, {Post});
```

**Controller `controllers/CartController.cc:76-108` exact**
```cpp
void CartController::addToCart(...) {
  std::string token = getBearer(req); // 78 — reads Authorization header
  std::string userId = AuthService::getUserIdFromToken(token); // 79 — verify token
  if(userId.empty()) return 401; // no login
  auto json = req->getJsonObject(); // 92
  int qty = json->get("quantity",1).asInt(); // 98
  auto result = CartService::addItem(userId, (*json)["productId"].asString(), qty); // 99
}
```

**Service `services/CartService.cc:39-62`**
```cpp
Json::Value CartService::addItem(userId, productId, quantity){
  if(quantity<=0) return error;
  auto product = ProductService::getProductById(productId); // 47 — verify product exists in DB
  if(product.isNull()) return {error:"Product not found"};
  auto &items = getStorage()[userId]; // in-memory map
  for(auto &item: items) if(item.productId==productId){ item.quantity+=quantity; return item.toJson(); }
  CartItem item; item.userId=userId; item.productId=productId; item.quantity=quantity;
  items.push_back(item); return item.toJson();
}
```
**Storage `services/CartService.cc:5-14`**
```cpp
std::map<std::string, std::vector<CartItem>> &getStorage(){ static ... carts; return carts; }
```
**Model `models/CartItem.h:14-22`** `userId, productId, quantity` + `toJson()`.

**View cart** `GET /api/v1/cart` → `controllers/CartController.cc:48-66` → `services/CartService.cc:23-37` array, frontend renders via `resp.json().forEach(item=> html+=...)`.
**Remove** `DELETE /api/v1/cart/{productId}` → `110-134` → frontend `remove()`.

---

## 6. CHECKOUT / BILLING — How Cart Becomes Order

**Frontend Checkout button**
```js
await fetch('http://localhost:8080/api/v1/orders/checkout', {
  method:'POST', headers:{'Authorization':'Bearer '+token}
});
// then
window.location.href = '/orders.html'; // open orders page
```

**Controller `controllers/OrderController.cc:33-53`**
```cpp
void OrderController::checkout(...){
  std::string userId = authUser(req); // Bearer → userId
  auto result = OrderService::checkout(userId); // 42
}
```
**Service `services/OrderService.cc:28-51`**
```cpp
Json::Value OrderService::checkout(const std::string &userId){
  auto cart = CartService::getCart(userId); if(cart.size()==0) return {error:"Cart is empty"};
  double total=0; for(auto &line: cart){
    auto product = ProductService::getProductById(line["productId"].asString()); // fetch price from DB
    total += product["price"].asDouble() * line["quantity"].asInt();
  }
  Order order; order.id=generateUUID(); order.userId=userId; order.total=total; order.status="created";
  getStorage().push_back(order);
  CartService::clearCart(userId); // 50 — empty cart after billing
  return order.toJson();
}
```
- Frontend then **navigates** to orders list: `GET /api/v1/orders` → `controllers/OrderController.cc:55-70` → `services/OrderService.cc:53-65` filtered by userId → render.

---

## 7. How Pages Change (Frontend Navigation)

- **After login**: `localStorage.setItem('token',...)` at `login.html:126`, then `window.location.href = '/dashboard'` or staying on same page and showing `tokenBox`. Future pages check `if(!localStorage.getItem('token')) window.location.href='/'` to force login.
- **After logout**: `localStorage.removeItem` + `window.location.href='/'` back to login.
- **After CRUD/cart/checkout**: No full reload needed — JS updates DOM (`innerHTML`, `textContent`, `element.remove()`) based on JSON response. Or `location.reload()` for simple demo.

## 8. Folder/File with Code Comments (Reference)

All files start with `// MODULE: ... PURPOSE: ... WHY: ...`:
- `sri.cpp:1-8` entry, `sri.cpp:93` DB init, `sri.cpp:96-98` run
- `controllers/AuthController.h:1-8` + `.cc:1-8` controller layer
- `services/AuthService.h:1-8` + `.cc:1-8` service, `68-103` stores
- `models/User.h:1-8` + `Product.h:1-8` models
- `services/ProductService.h:1-8` + `.cc:1-8` Postgres, `20-41` connection, `76-106` table
- `CMakeLists.txt:1-8` build

This file covers login, logout, CRUD (create/read/update-edit/delete), add-to-cart, billing — each with exact frontend `fetch` code, how data is passed as JSON, how backend verifies (hash + token + DB `PQexecParams`), how data is stored (Postgres vs in-memory), and how frontend changes (localStorage + DOM + navigation).
