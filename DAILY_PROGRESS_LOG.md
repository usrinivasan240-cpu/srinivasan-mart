# Sri Mart — Daily Progress Log

## 2026-08-12
### Environment / Foundation
- Cloned and configured vcpkg.
- Bootstrapped vcpkg.
- Verified vcpkg executable.
- Configured MSYS2 UCRT64/MinGW compiler.
- Installed the required Drogon package for the MinGW environment.
- Created/configured the CMake project.
- Started the Drogon API.
- Verified the local API server.

### API
- Health endpoint created/tested.
- Hello endpoint created/tested.
- Product endpoint added/tested.

### Architecture
- `controllers/`
- `models/`
- `services/`
- `config/`

### Git phase
`phase-00` / `phase-01` foundation and product API work.

---

## 2026-08-13
### Product Module
- `ProductController.h` added/used.
- `/api/v1/products` registered.
- Product test response added.

### Build Debugging
- CMake + MinGW build was investigated.
- Direct `c++.exe` compilation of `sri.cpp` completed without compiler output/errors.
- CMake build still requires a reproducible clean build state.

### Documentation Goal
From this point onward, every module should have:
- purpose comments;
- separate documentation;
- a test step;
- a Git phase/commit.

---

## Rule For Future Entries
Each daily entry must answer:
1. What was built?
2. What was tested?
3. What was learned?
4. What problem occurred?
5. How was it fixed?
6. Which Git phase contains the work?
