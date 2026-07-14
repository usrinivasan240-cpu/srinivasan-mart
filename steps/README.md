# Sri Mart — Step-by-Step Guides

This folder contains detailed explanations of how every part of Sri Mart works.

## Files

| File | What It Explains |
|------|------------------|
| `01-registration-flow.md` | How account creation works, every single line |
| `02-login-flow.md` | How login works, every single line |
| `03-complete-workflow.md` | How everything connects together |

## How to Use

1. Start with `01-registration-flow.md` — understand how users create accounts
2. Then read `02-login-flow.md` — understand how users login
3. Finally read `03-complete-workflow.md` — understand the complete system

## Quick Start

Double-click `start.bat` or `dev.bat` in the project folder to start everything.

### Or use CMD:
```cmd
:: Start PostgreSQL
"C:\Users\Srinivasan\Downloads\sri projects\sri mart\postgresql\pgsql\bin\pg_ctl.exe" -D C:\pgsql_data -l C:\pgsql.log start

:: Set PATH and run
set PATH=C:\Users\Srinivasan\MSYS2\ucrt64\bin;%PATH%
cd "C:\Users\Srinivasan\Downloads\sri projects\sri mart\build"
sri_mart.exe
```

## Default Accounts

| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | admin |
| customer | customer123 | customer |

## Quick Test Commands

```bash
# Register
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@test.com","password":"test123"}'

# Login
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'

# Get Products (from PostgreSQL database)
curl http://localhost:8080/api/v1/products

# Create Product (saved to PostgreSQL)
curl -X POST http://localhost:8080/api/v1/products \
  -H "Content-Type: application/json" \
  -d '{"name":"Phone","price":699.99,"stock":25}'
```

## Database

- **PostgreSQL** runs on port `5433`
- **Database name**: `sri_mart`
- **Products table** stores all product data permanently
- **Users/Tokens** are stored in-memory (will be moved to PostgreSQL later)
