# Sri Mart — How to Run (CMD Instructions)

This file explains exactly how to run the Sri Mart project from the Windows Command Prompt.

---

## Prerequisites (Already Installed)

These are already installed on your system:
- **MSYS2 UCRT64** — C++ compiler (MinGW)
- **CMake** — Build system
- **vcpkg** — Package manager
- **Drogon** — HTTP framework
- **PostgreSQL** — Database (portable, on port 5433)

---

## Step 1: Start PostgreSQL Database

Open CMD and run:

```cmd
:: Start PostgreSQL server
C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\pg_ctl.exe -D C:\pgsql_data -l C:\pgsql.log start
```

**Verify it's running:**
```cmd
C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\psql.exe -U postgres -h localhost -p 5433 -c "\l"
```

You should see the `sri_mart` database in the list.

---

## Step 2: Set PATH for MSYS2

```cmd
set PATH=C:\Users\Srinivasan\MSYS2\ucrt64\bin;%PATH%
```

---

## Step 3: Build the Project (First Time Only)

```cmd
:: Navigate to project root
cd "C:\Users\Srinivasan\Downloads\sri projects\sri mart"

:: Configure CMake
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic

:: Build the project
cmake --build build
```

---

## Step 4: Run the Server

```cmd
:: Navigate to build directory
cd "C:\Users\Srinivasan\Downloads\sri projects\sri mart\build"

:: Run the server
sri_mart.exe
```

**You should see:**
```
Connected to PostgreSQL database: sri_mart
Products table created/verified in PostgreSQL.
Sri Mart API running on http://0.0.0.0:8080
Login page: http://localhost:8080/
```

---

## Step 5: Open in Browser

Open your web browser and go to:
```
http://localhost:8080
```

You'll see the Sri Mart login page.

---

## Quick Start (All-in-One)

Copy and paste these commands in CMD:

```cmd
:: Start PostgreSQL
C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\pg_ctl.exe -D C:\pgsql_data -l C:\pgsql.log start

:: Set PATH
set PATH=C:\Users\Srinivasan\MSYS2\ucrt64\bin;%PATH%

:: Run the server
cd "C:\Users\Srinivasan\Downloads\sri projects\sri mart\build"
sri_mart.exe
```

Then open: http://localhost:8080

---

## Default Login Credentials

| Role | Username | Password |
|------|----------|----------|
| Admin | admin | admin123 |
| Customer | customer | customer123 |

---

## All API Endpoints

### Health & Test
| Method | URL | Purpose |
|--------|-----|---------|
| GET | `http://localhost:8080/` | Login page (HTML) |
| GET | `http://localhost:8080/api/v1/health` | Check if server is alive |
| GET | `http://localhost:8080/api/v1/hello` | Simple test endpoint |

### Authentication
| Method | URL | Purpose |
|--------|-----|---------|
| POST | `http://localhost:8080/api/v1/auth/register` | Create new account |
| POST | `http://localhost:8080/api/v1/auth/login` | Login and get token |
| GET | `http://localhost:8080/api/v1/auth/validate` | Check if token is valid |
| GET | `http://localhost:8080/api/v1/auth/users` | List all users |

### Products (PostgreSQL)
| Method | URL | Purpose |
|--------|-----|---------|
| GET | `http://localhost:8080/api/v1/products` | List all products |
| POST | `http://localhost:8080/api/v1/products` | Create a product |
| GET | `http://localhost:8080/api/v1/products/{id}` | Get one product |
| PUT | `http://localhost:8080/api/v1/products/{id}` | Update a product |
| DELETE | `http://localhost:8080/api/v1/products/{id}` | Delete a product |

---

## Test with cURL (CMD)

```cmd
:: Health check
curl http://localhost:8080/api/v1/health

:: Register
curl -X POST http://localhost:8080/api/v1/auth/register -H "Content-Type: application/json" -d "{\"username\":\"test\",\"email\":\"test@test.com\",\"password\":\"test123\"}"

:: Login
curl -X POST http://localhost:8080/api/v1/auth/login -H "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"admin123\"}"

:: Get products
curl http://localhost:8080/api/v1/products

:: Create product
curl -X POST http://localhost:8080/api/v1/products -H "Content-Type: application/json" -d "{\"name\":\"Phone\",\"price\":699.99,\"stock\":25}"
```

---

## Stop the Server

Press `Ctrl + C` in the CMD window where the server is running.

---

## Stop PostgreSQL

```cmd
C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\pg_ctl.exe -D C:\pgsql_data stop
```

---

## Troubleshooting

### "Port 8080 already in use"
Another program is using port 8080. Close it or change the port in `config/drogon.json`.

### "PostgreSQL connection refused"
PostgreSQL is not running. Start it with Step 1.

### "libgcc_s_seh-1.dll not found"
Set the PATH correctly with Step 2.

### Products not saving
Make sure PostgreSQL is running on port 5433 and the `sri_mart` database exists.
