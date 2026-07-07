# Sri Mart — Technology Stack Lock

This file is a HARD constraint.

## Allowed Stack
1. C++20 — application language.
2. Drogon — HTTP server/web framework.
3. PostgreSQL — relational database (Drogon has built-in ORM support).
4. CMake — build configuration.
5. vcpkg — C/C++ dependency management.
6. MSYS2 UCRT64 / MinGW — compiler/build environment.
7. Git/GitHub — source control.
8. JSON support already used by the C++/Drogon setup.

## Not Allowed
Do not add a different backend framework, different language, different package manager, or replacement build system.

## Build Facts
- Executable: `sri_mart`
- Local API port: `8080`
- CMake build directory: `build-mingw`
- Current vcpkg include path observed:
  `C:/dev/vcpkg/installed/x64-mingw-dynamic/include`

## Reason
The project is being built around the stack already configured and tested. The goal is to finish the project and understand the workflow rather than restart the stack.
