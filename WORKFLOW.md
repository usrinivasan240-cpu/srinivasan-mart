# Sri Mart — Complete Workflow (A to Z)

This file explains how Sri Mart works, from start to finish, in simple language anyone can understand.

---

## What is Sri Mart?

Sri Mart is an online store API (like the backend of Amazon or Flipkart).
It handles user accounts, products, and shopping operations.

Think of it like a **real shop**:
- **Users** = Customers who visit the shop
- **Products** = Items on the shelves
- **Auth** = The security guard checking IDs at the door
- **Controller** = The shop's front desk
- **Service** = The manager who decides what to do
- **Model** = The filing cabinet where data is stored

---

## Step 1: How the Server Starts

```
File: sri.cpp
```

1. The server starts and opens **port 8080** (like opening the shop door)
2. It registers all the URL routes (like putting up signs for different sections)
3. It waits for customers (HTTP requests) to come in

**What happens when you type `http://localhost:8080` in your browser:**
1. Your browser sends a request to the server
2. The server reads the `login.html` file
3. The server sends back the login page
4. You see the Sri Mart login form

---

## Step 2: User Registration (Creating an Account)

```
Flow: Browser → AuthController → AuthService → User storage
```

**Example: You want to create a new account**

1. You fill in the registration form:
   - Username: `john`
   - Email: `john@email.com`
   - Password: `secret123`

2. You click "Register"

3. The browser sends this data to: `POST /api/v1/auth/register`

4. **AuthController** receives the request and calls **AuthService**

5. **AuthService** does:
   - Checks if username already exists
   - Hashes the password (encrypts it for safety)
   - Generates a unique ID (UUID)
   - Saves the user to storage
   - Returns the user info (without password)

6. You see "Registration successful!" and can now login

**Default accounts:**
| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | admin |
| customer | customer123 | customer |

---

## Step 3: User Login (Getting a Token)

```
Flow: Browser → AuthController → AuthService → Token
```

**Example: You want to login**

1. You enter:
   - Username: `admin`
   - Password: `admin123`

2. You click "Login"

3. The browser sends this to: `POST /api/v1/auth/login`

4. **AuthController** receives the request and calls **AuthService**

5. **AuthService** does:
   - Finds the user by username
   - Checks if the password matches
   - Generates a **token** (a random string like a temporary key)
   - Saves the token → user mapping
   - Returns the token and user info

6. You see "Login successful!" and your token is saved in the browser

**What is a token?**
> A token is like a **visitor badge** at a conference.
> You show it at every request to prove who you are.
> Example: `ea4da133ee6f00cdead2691b91fd8257`

---

## Step 4: Validating a Token (Checking Identity)

```
Flow: Browser → AuthController → AuthService → User info
```

**Example: You want to check if you're still logged in**

1. The browser sends: `GET /api/v1/auth/validate`
   - Header: `Authorization: Bearer ea4da133ee6f00cdead2691b91fd8257`

2. **AuthController** extracts the token from the header

3. **AuthService** does:
   - Looks up the token in storage
   - Finds the user associated with it
   - Returns the user info

4. You see your user details (you're still logged in)

---

## Step 5: Viewing Products (Browsing the Shop)

```
Flow: Browser → ProductController → ProductService → Product storage → JSON response
```

**Example: You want to see all products**

1. The browser sends: `GET /api/v1/products`

2. **ProductController** receives the request and calls **ProductService**

3. **ProductService** does:
   - Gets all products from storage
   - Converts each product to JSON format
   - Returns the list

4. You see:
```json
[
  {
    "id": "550e8400-e29b-41d4-a716-446655440001",
    "name": "Sample Product",
    "price": 99.99,
    "stock": 100
  }
]
```

---

## Step 6: Creating a Product (Adding to Shelves)

```
Flow: Browser → ProductController → ProductService → Product storage
```

**Example: Admin wants to add a new product**

1. Admin sends: `POST /api/v1/products`
   ```json
   {
     "name": "Wireless Mouse",
     "description": "Ergonomic wireless mouse",
     "price": 29.99,
     "stock": 50
   }
   ```

2. **ProductController** validates the request:
   - Is "name" provided? ✓
   - Is "price" provided? ✓
   - If not → returns error (400 Bad Request)

3. **ProductService** does:
   - Generates a unique ID
   - Creates the product
   - Saves it to storage
   - Returns the created product

4. Admin sees the new product with its unique ID

---

## Step 7: Updating a Product (Changing Price/Stock)

```
Flow: Browser → ProductController → ProductService → Product storage
```

**Example: Admin wants to change the price**

1. Admin sends: `PUT /api/v1/products/{id}`
   ```json
   {
     "price": 24.99,
     "stock": 75
   }
   ```

2. **ProductController** finds the product by ID

3. **ProductService** does:
   - Checks if product exists
   - Updates only the fields provided
   - Returns the updated product

4. Admin sees the product with new price

---

## Step 8: Deleting a Product (Removing from Shelves)

```
Flow: Browser → ProductController → ProductService → Product storage
```

**Example: Admin wants to remove a product**

1. Admin sends: `DELETE /api/v1/products/{id}`

2. **ProductController** finds the product by ID

3. **ProductService** does:
   - Checks if product exists
   - Removes it from storage
   - Returns success message

4. Admin sees "Product deleted successfully"

---

## The Complete Request Flow

Here's what happens for **every single request**:

```
User (Browser/Postman/curl)
    │
    ▼
HTTP Request (GET/POST/PUT/DELETE)
    │
    ▼
sri.cpp (Server entry point)
    │
    ▼
Route matching (URL → Controller)
    │
    ▼
Controller (validates request, calls service)
    │
    ▼
Service (business logic, calls storage)
    │
    ▼
Storage (in-memory list, will be PostgreSQL later)
    │
    ▼
Response (JSON data)
    │
    ▼
User (sees the result)
```

---

## All API Endpoints

### Health & Test
| Method | URL | Purpose | Auth Required |
|--------|-----|---------|---------------|
| GET | `/api/v1/health` | Check if server is alive | No |
| GET | `/api/v1/hello` | Simple test endpoint | No |
| GET | `/` | Login page (HTML) | No |

### Authentication
| Method | URL | Purpose | Auth Required |
|--------|-----|---------|---------------|
| POST | `/api/v1/auth/register` | Create new account | No |
| POST | `/api/v1/auth/login` | Login and get token | No |
| GET | `/api/v1/auth/validate` | Check if token is valid | Yes (Bearer token) |
| GET | `/api/v1/auth/users` | List all users | No |

### Products
| Method | URL | Purpose | Auth Required |
|--------|-----|---------|---------------|
| GET | `/api/v1/products` | List all products | No |
| POST | `/api/v1/products` | Create a product | No |
| GET | `/api/v1/products/{id}` | Get one product | No |
| PUT | `/api/v1/products/{id}` | Update a product | No |
| DELETE | `/api/v1/products/{id}` | Delete a product | No |

---

## File Structure Explained

```
sri mart/
├── sri.cpp                     → Server startup and route registration
├── CMakeLists.txt              → Build instructions for the compiler
│
├── controllers/                → "Front desk" - receives HTTP requests
│   ├── ProductController.h     → Product route declarations
│   ├── ProductController.cc    → Product route implementations
│   ├── AuthController.h        → Auth route declarations
│   └── AuthController.cc       → Auth route implementations
│
├── services/                   → "Manager" - business logic
│   ├── ProductService.h        → Product operations declarations
│   ├── ProductService.cc       → Product operations implementations
│   ├── AuthService.h           → Auth operations declarations
│   └── AuthService.cc          → Auth operations implementations
│
├── models/                     → "Filing cabinet" - data structure
│   ├── Product.h               → What a product looks like
│   └── User.h                  → What a user looks like
│
├── config/                     → Settings and static files
│   ├── drogon.json             → Server and database settings
│   └── login.html              → Login/Register web page
│
└── build/                      → Compiled executable
    └── sri_mart.exe            → The running server
```

---

## How to Run

1. Start PostgreSQL (if available):
   ```
   C:\pgsql_data\bin\pg_ctl.exe -D C:\pgsql_data start
   ```

2. Start the server:
   ```
   cd "sri mart\build"
   sri_mart.exe
   ```

3. Open browser:
   ```
   http://localhost:8080
   ```

4. You'll see the login page

---

## Default Login Credentials

| Role | Username | Password |
|------|----------|----------|
| Admin | admin | admin123 |
| Customer | customer | customer123 |

---

## Current Status

| Feature | Status |
|---------|--------|
| Server running | ✅ Working |
| Health check | ✅ Working |
| Login page | ✅ Working |
| User registration | ✅ Working |
| User login | ✅ Working |
| Token validation | ✅ Working |
| Product list | ✅ Working |
| Product create | ✅ Working |
| Product update | ✅ Working |
| Product delete | ✅ Working |
| PostgreSQL connection | ✅ Working (port 5433) |

---

## Future Plans

1. ~~**Connect PostgreSQL**~~ — ✅ DONE
2. **Add validation** — Check all inputs are correct
3. **Add error handling** — Better error messages
4. **User module** — Profile management, password change
5. **Cart module** — Add products to shopping cart
6. **Order module** — Place orders, order history
7. **Integration** — Connect all modules together
8. **Testing** — Test every endpoint thoroughly
