# Srinivasan Mart — Full Workflows with Folder/File Code & Database

> Single file docs for every click: login, logout, add to cart, edit, billing, etc.
> `file:line` lets you open that file and jump to the line.
> Based on `master` at Sep 21, 2026 — PostgreSQL on `localhost:5433` for products, in-memory for auth/cart/orders/reviews (Postgres migration planned).

## 1. Folder / File Map (where code lives)

```
srinivasan-mart/
├── sri.cpp                  # 101 lines — entry point, listeners, route wiring, DB init, app.run()
├── CMakeLists.txt           # 50 lines — add_executable sri_mart, links Drogon::Drogon + pq
├── config/
│   ├── drogon.json          # 14 lines — listeners 0.0.0.0:8080 + log
│   └── login.html           # 174 lines — login/register UI, fetch() + localStorage token
├── controllers/             # HTTP layer: validates request, calls service, returns JSON
│   ├── AuthController.h/.cc      # 41/184 lines — register/login/validate/users/logout
│   ├── ProductController.h/.cc   # 48/174 lines — CRUD + search
│   ├── CartController.h/.cc      # 26/135 lines — cart stub (Bearer auth)
│   ├── OrderController.h/.cc     # 28/127 lines — checkout/billing + status
│   ├── ReviewController.h/.cc    # 19/52 lines — reviews per product
│   ├── AdminController.h/.cc     # 23/71 lines — seller/admin stats
│   └── ChatController.h/.cc      # 19/37 lines — /chat stub
├── services/                # Business logic layer
│   ├── AuthService.h/.cc         # 62/268 lines — users + tokens in-memory, hash, UUID
│   ├── ProductService.h/.cc      # 50/332 lines — Postgres via libpq (products table)
│   ├── CartService.h/.cc         # 39/101 lines — per-user cart in-memory
│   ├── OrderService.h/.cc        # 38/122 lines — checkout stub, clears cart
│   └── ReviewService.h/.cc       # 28/68 lines — in-memory reviews, rating 1-5
└── models/                  # Data structs + toJson/fromJson
    ├── User.h               # 64 lines — id, username, email, password (hashed), role, timestamps; toJson() excludes password
    ├── Product.h            # 66 lines — id, name, description, price, stock, timestamps
    ├── CartItem.h           # 33 lines — userId, productId, quantity
    ├── Order.h              # 38 lines — id, userId, total, status, itemCount
    └── Review.h             # 35 lines — id, productId, userId, rating, comment
```

Build: `CMakeLists.txt:18-32` lists all `sri.cpp` + `controllers/*.cc` + `services/*.cc` → `sri_mart` → `target_link_libraries Drogon::Drogon pq`.

## 2. Database — Connected How

### 2.1 Config
`config/drogon.json:1-14`
```json
{
  "listeners": [{ "address": "0.0.0.0", "port": 8080 }],
  "log": { "output": "stdout", "log_level": "info" }
}
```
`sri.cpp:25` `app().addListener("0.0.0.0",8080)` mirrors it.

### 2.2 Postgres connection (Products)
`services/ProductService.cc:15-41` — global `PGconn *g_conn` + `getConnection()`
```cpp
// services/ProductService.cc:16,20-41
static PGconn *g_conn = nullptr;
PGconn *ProductService::getConnection() {
  if (g_conn==nullptr || PQstatus(g_conn)!=CONNECTION_OK) {
    if (g_conn) PQfinish(g_conn);
    g_conn = PQconnectdb("host=localhost port=5433 dbname=sri_mart user=postgres password=postgres"); // 29
    // LOG_ERROR if != CONNECTION_OK else LOG_INFO Connected to sri_mart
  }
  return g_conn;
}
```
`services/ProductService.cc:43-50` `freeResult()` → `PQclear` to avoid leaks.

### 2.3 Table creation
`sri.cpp:93` `ProductService::initializeDatabase()` called before `app().run()`
`services/ProductService.cc:76-106`
```cpp
// 84-93 CREATE TABLE IF NOT EXISTS products (id VARCHAR(36) PK, name VARCHAR(255), description TEXT, price DECIMAL(10,2), stock INT, created_at TIMESTAMP, updated_at TIMESTAMP)
PGresult *res = PQexec(conn, query); // 95
// LOG_INFO Products table created/verified
```

### 2.4 Product SQL patterns
- List: `services/ProductService.cc:118-120` `SELECT id,name,description,price,stock,created_at::text,updated_at::text FROM products` → loop `PQntuples`, `PQgetvalue` → JSON (132-143)
- Get one: `160-163` `SELECT ... WHERE id=$1` via `PQexecParams`
- Create: `216-219` `INSERT INTO products (id,name,description,price,stock) VALUES ($1..$5::decimal/integer)` → `return getProductById(id)` (231)
- Update: `245-247` check exists, `264-268` `UPDATE products SET name=$1,description=$2,price=$3::decimal,stock=$4::integer,updated_at=CURRENT_TIMESTAMP WHERE id=$5` → re-read
- Delete: `291-293` `DELETE FROM products WHERE id=$1` → `PQcmdTuples(res)[0]!='0'` (302)
- Search: `309-331` stub — `getAllProducts()` then in-RAM filter by `q` substring + `minPrice/maxPrice` (planned: `WHERE ILIKE`).

### 2.5 In-memory stores (no Postgres yet)
- Auth: `services/AuthService.cc:68-96` `vector<User> users` + `100-103` `map<string,string> tokens` (token→userId), `106-110` mutex. Seeded admin/customer.
- Cart: `services/CartService.cc:5-14` `map<userId, vector<CartItem>> carts` + mutex
- Orders: `services/OrderService.cc:5-14` `vector<Order> orders`
- Reviews: `services/ReviewService.cc:5-14` `vector<Review>`

All follow same rule: `route → controller → service → model/DB → JSON`.

## 3. App Boot
`sri.cpp:22-101`
- `25` listen 8080
- `28-50` `GET /health` UP, `GET /hello` welcome
- `54-77` `GET /` serves `config/login.html` else JSON fallback
- `80-90` `ProductController::initRoutes()`, `AuthController::initRoutes()`, `CartController::initRoutes()`, `OrderController::initRoutes()`, `ReviewController::initRoutes()`, `AdminController::initRoutes()`, `ChatController::initRoutes()`
- `93` `initializeDatabase()` (see 2.3)
- `96-98` `app().run()` blocks

## 4. Auth Workflows

### 4.1 Register — "Register button click"
UI: `config/login.html:56-70` inputs `regUsername, regEmail, regPassword` + `69: <button onclick="register()">`
JS: `config/login.html:136-163`
```js
// 136-151
async function register(){
  // read 137-139, empty check 141-144
  const response = await fetch('http://localhost:8080/api/v1/auth/register', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({username,email,password}) // 150
  });
  // 154-156 success → showMessage + showForm('login')
}
```
HTTP: `POST /api/v1/auth/register` JSON `{username,email,password}`
Route: `controllers/AuthController.cc:19-21` in `initRoutes():16-42`
Controller: `controllers/AuthController.cc:46-74` `registerUser()` — `50:getJsonObject()`, `51:require 3 fields else 400`, `61:AuthService::registerUser(*json)`, `62-73: error→400 else 201`
Service: `services/AuthService.cc:114-161` — `116:lock_guard(mutex)`, `126-128` parse, `131-145` duplicate username/email check via `getUserStorage():68-96`, `148-157` new `User{id=generateUUID():17-37, password=hashPassword():41-49 XOR 0x5A, role=customer}`, `157:push_back`, `160:toJson()`
Model: `models/User.h:18-25` fields, `31-41:toJson()` excludes password; `43-49:toFullJson()` internal.

### 4.2 Login — "Login button click"
UI: `config/login.html:43-53` + `52: <button onclick="login()">`
JS: `105-134`
```js
// 105-119
async function login(){
  const response = await fetch(API_BASE+'/api/v1/auth/login', {
    method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({username,password})
  });
  const data = await response.json(); // 120
  if(response.ok){ // 122
    localStorage.setItem('token', data.token); // 126
    localStorage.setItem('user', JSON.stringify(data.user)); // 127
    tokenBox.textContent='Token: '+data.token; // 124
  }
}
```
HTTP: `POST /api/v1/auth/login` `{username,password}`
Route: `controllers/AuthController.cc:24-26`
Controller: `79-109` `loginUser()` — `84:getJson`, `94-95:asString`, `97:AuthService::loginUser`, `98-108: error→401 else 200 {token,user}`
Service: `165-189` — `169:hashPassword`, `171-184:loop users, match → 176:generateToken():52-64 (32 hex) → 177:tokens[token]=user.id → 179-182:{token,user.toJson()}` else error.

### 4.3 Validate — "on page load / protected call"
JS pattern: `GET /api/v1/auth/validate` + `Authorization: Bearer <token>`
Controller: `113-141` `validateToken()` — `117:getHeader("Authorization")`, `118:must start Bearer else 401`, `128:substr(7)`, `129:AuthService::validateToken`
Service: `193-213` — `197:tokens.find`, `200:userId=tokens[token]`, loop users → `toJson()` else `Invalid or expired token`.

### 4.4 Users list
`GET /api/v1/auth/users` → `controllers/AuthController.cc:34-36,144-151` → `services/AuthService.cc:216-225:getAllUsers()` loop `toJson()` array.

### 4.5 Logout — "Logout button click"
JS (add to any page):
```js
await fetch('http://localhost:8080/api/v1/auth/logout', {
  method:'POST', headers:{'Authorization':'Bearer '+localStorage.getItem('token')}
});
localStorage.removeItem('token'); localStorage.removeItem('user');
```
Route: `controllers/AuthController.cc:38-41` `POST /api/v1/auth/logout`
Controller: `154-184` `logoutUser()` — `158:getHeader`, `168:substr(7)`, `169:AuthService::logoutToken(token)` → `200:Logged out` else 401
Service: `services/AuthService.cc:243-267` — `logoutToken()` erases `tokens[token]`, `getUserIdFromToken()` returns `it->second` or "".

## 5. Product Workflows (Database-backed)

Routes: `controllers/ProductController.cc:17-43`
- `20:GET /api/v1/products → getProducts`
- `25:POST /api/v1/products → createProduct`
- `30:GET /api/v1/products/{id} → getProductById`
- `35:PUT /api/v1/products/{id} → updateProduct` ← **Edit**
- `40:DELETE /api/v1/products/{id} → deleteProduct`
- `44:GET /api/v1/products/search → searchProducts`

### 5.1 Browse / List
Controller `46-53:getProducts()` → `50:ProductService::getAllProducts()` → JSON
Service `109-147` — `112:getConnection()`, `118:SELECT ... FROM products`, `129-143:rows→JSON` (price stod, stock stoi)

### 5.2 Create — "Add product"
Controller `57-75:createProduct()` — `61:getJson`, `62:require name+price else 400`, `71:createProduct(*json)`, `73:201`
Service `198-232` — `208:generateUUID`, `209-212:parse`, `216-219:INSERT ...`, `231:return getProductById(id)`
Model `models/Product.h:18-25` fields, `40-51:toJson()`

### 5.3 Get one
Controller `78-97:getProductById()` — `83:getProductById(productId)`, `84:Null→404`
Service `150-195` — `160-170:PQexecParams SELECT ... WHERE id=$1`, `179:ntuples>0?item:Null`

### 5.4 Edit — "Edit/Update product" (the `edit` you asked)
Frontend (example):
```js
await fetch('http://localhost:8080/api/v1/products/'+id, {
  method:'PUT', headers:{'Content-Type':'application/json'},
  body: JSON.stringify({name, description, price, stock})
});
```
Controller `101-130:updateProduct()` — `106:getJson else 400`, `116:ProductService::updateProduct(id,*json)`, `117:Null→404`
Service `235-279` — `244-253:SELECT id FROM products WHERE id=$1` check exists, `257-262:parse`, `264-268:UPDATE ... WHERE id=$5`, `278:return getProductById(id)` fresh row. Code at `controllers/ProductController.cc:101-130` + `services/ProductService.cc:235-279` shows comment lines mapping.

### 5.5 Delete
Controller `133-154:deleteProduct()` → `138:deleteProduct(id)` → `200:{message}` else 404
Service `282-305` — `291:DELETE WHERE id=$1`, `302:PQcmdTuples[0]!='0'`

### 5.6 Search / Filter — "filter by name/price"
`GET /api/v1/products/search?q=phone&minPrice=100&maxPrice=800`
Controller `156-174:searchProducts()` — `163:getParameter("q")`, `164-166:stod min/max`, `167:ProductService::searchProducts(q,min,max)`
Service `309-331` — `311:getAllProducts()` then RAM filter `317:find(query)`, `321/325:price range` → append.

## 6. Add to Cart Workflow

Routes: `controllers/CartController.cc:27-41`
- `GET /api/v1/cart` list
- `POST /api/v1/cart {productId,quantity}` add
- `DELETE /api/v1/cart/{productId}` remove

### 6.1 Add to Cart — "Add to cart button"
JS:
```js
await fetch('http://localhost:8080/api/v1/cart', {
  method:'POST',
  headers:{'Content-Type':'application/json','Authorization':'Bearer '+localStorage.getItem('token')},
  body: JSON.stringify({productId, quantity:1})
});
```
Controller `76-108:addToCart()` — `78-90:getBearer()+getUserIdFromToken else 401`, `92:require productId else 400`, `98:addItem(userId,productId,qty)` → `201` else 400
Service `services/CartService.cc:39-62` — `44:qty<=0 error`, `47:ProductService::getProductById(productId)` validates exists else `Product not found`, `54-62:merge quantity if same product else push CartItem{userId,productId,quantity}`
Model `models/CartItem.h:14-22` struct + `18:toJson()`
Storage: `services/CartService.cc:5-14` `map<userId,vector<CartItem>>` in-memory (planned `cart_items(user_id,product_id,qty)` Postgres)

### 6.2 View Cart / Remove
- List: `controllers/CartController.cc:48-66:getCart()` → `services/CartService.cc:23-37:getCart(userId)` array
- Remove: `110-134:removeFromCart()` → `services/CartService.cc:64-82:removeItem` loop erase → `Removed from cart` else 404

## 7. Billing / Checkout Workflow

Routes: `controllers/OrderController.cc:24-31`
- `POST /api/v1/orders/checkout` billing
- `GET /api/v1/orders` list my orders
- `GET /api/v1/orders/{id}` get one
- `PUT /api/v1/orders/{id}/status` update status

### 7.1 Checkout — "Pay / Billing"
JS:
```js
await fetch('http://localhost:8080/api/v1/orders/checkout', {
  method:'POST', headers:{'Authorization':'Bearer '+token}
});
```
Controller `33-53:checkout()` — `37:authUser (Bearer→userId) else 401`, `42:OrderService::checkout(userId)` → `201` else 400 (empty cart)
Service `services/OrderService.cc:28-51` — `30:getCart(userId)` if 0 → error `Cart is empty`, `35-40:sum price*qty` via `ProductService::getProductById(...).get("price")`, `43-50:create Order{id=UUID, userId, total, status=created, itemCount, created_at}`, `50:clearCart(userId)` (see `CartService::84-89`)
Model `models/Order.h:14-27` + `19:toJson()`
DB note: stub totals in RAM; planned Postgres transaction `BEGIN; INSERT orders; INSERT order_items; UPDATE products stock; COMMIT;` using same `PQexecParams` style as ProductService.

### 7.2 Order status
Controller `91-127:updateStatus()` — `99:getJson status`, `102:OrderService::updateStatus(userId,orderId,status)` → validates `created|paid|shipped` (`services/OrderService.cc:68-86`) else 400, `Null→404`.

## 8. Reviews Workflow
Routes: `controllers/ReviewController.cc:12-16` `GET/POST /api/v1/products/{id}/reviews`
- List: `18-24:listReviews()` → `services/ReviewService.cc:31-40:getByProduct(productId)` array
- Add: `26-46:addReview()` — `30:Bearer→userId else 401`, `38-39:rating/comment`, `40:ReviewService::addReview` → validates `1-5` else 400, `ProductService::getProductById` else 400, generates id `16-26` → `toJson()` with `201`
Model `models/Review.h:10-24` + Service `services/ReviewService.cc:50-68`

## 9. Seller / Admin + Chatbot

- Seller stats: `controllers/AdminController.cc:24-46:sellerStats()` — `29:validateToken` else 401, `37:ProductService::getAllProducts().size()` + role
- Admin stats: `48-71:adminStats()` — `60:role != admin →403`, returns `userCount` via `AuthService::getAllUsers()` + `productCount`
- Chat: `controllers/ChatController.cc:12-31` `POST /api/v1/chat {message}` → keyword replies (`hour→Open 9am-9pm`, `return→Returns 7 days`, `offer→search deals`) + echo, planned LLM hook.

## 10. Folder/File with Code Comments (quick reference)

Every file starts with `// MODULE: ... PURPOSE: ... WHY: ... INPUT: ... OUTPUT: ... ARCHITECTURE: ...`:
- `sri.cpp:1-8` — entry point; `sri.cpp:80-93` wiring
- `controllers/AuthController.h:1-8`, `controllers/AuthController.cc:1-8` — controller
- `services/AuthService.h:1-8`, `services/AuthService.cc:1-8` — service
- `models/User.h:1-8`, `models/Product.h:1-8` — model
- `services/ProductService.h:1-8` — `PGconn *getConnection():20-41`, `initializeDatabase():76-106`
- `CMakeLists.txt:1-8` — build

## 11. End-to-End Examples (curl)

```cmd
# health
curl http://localhost:8080/api/v1/health
# register
curl -X POST http://localhost:8080/api/v1/auth/register -H "Content-Type: application/json" -d "{\"username\":\"test\",\"email\":\"test@test.com\",\"password\":\"test123\"}"
# login
curl -X POST http://localhost:8080/api/v1/auth/login -H "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"admin123\"}"
# logout (Bearer)
curl -X POST http://localhost:8080/api/v1/auth/logout -H "Authorization: Bearer <token>"
# add product
curl -X POST http://localhost:8080/api/v1/products -H "Content-Type: application/json" -d "{\"name\":\"Phone\",\"price\":699.99,\"stock\":25}"
# edit product
curl -X PUT http://localhost:8080/api/v1/products/<id> -H "Content-Type: application/json" -d "{\"name\":\"Phone Pro\",\"price\":799.99}"
# search
curl "http://localhost:8080/api/v1/products/search?q=Phone&minPrice=100&maxPrice=800"
# add to cart
curl -X POST http://localhost:8080/api/v1/cart -H "Authorization: Bearer <token>" -H "Content-Type: application/json" -d "{\"productId\":\"<id>\",\"quantity\":2}"
# checkout
curl -X POST http://localhost:8080/api/v1/orders/checkout -H "Authorization: Bearer <token>"
```
