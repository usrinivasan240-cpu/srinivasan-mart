# Sri Mart — Tomorrow's Plan

## Priority 1 — Reproduce Build
1. Open the project root.
2. Use the existing `build-mingw` directory.
3. Build using the existing CMake + MinGW setup.
4. Record the exact compiler/build result.

## Priority 2 — Freeze the Foundation
Verify:
- `/api/v1/health`
- `/api/v1/hello`
- `/api/v1/products`

## Priority 3 — Understand Existing Code
For each current file, write down:
- Why it exists.
- What it receives.
- What it returns.
- Which module it calls.
- Which module calls it.

## Priority 4 — Continue One Module At A Time
For each new feature:
```text
model -> service -> controller -> route -> build -> test -> document -> commit
```

## Priority 5 — No Stack Changes
Do not replace:
- C++20
- Drogon
- CMake
- vcpkg
- MSYS2 UCRT64/MinGW

## End-of-Day Definition
A feature is considered complete only when:
- code exists;
- build passes;
- endpoint/function is tested;
- source comments explain its purpose;
- documentation is updated;
- Git phase commit is made.
