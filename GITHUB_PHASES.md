# Sri Mart — GitHub Contribution Plan

## Important
This file defines the contribution history to use for the project. It does not claim that GitHub commits have already been created remotely.

For the existing work from project start through 2026-08-13, preserve the work as separate logical phases rather than one giant commit.

## Historical Phase Plan

### Phase 00 — Foundation
Approximate project-start work:
- Created initial C++20/Drogon project.
- Configured CMake.
- Configured vcpkg.
- Configured MSYS2 UCRT64/MinGW.
- Verified Drogon server.
- Added health endpoint.
- Added hello endpoint.

Suggested commit:
`phase-00: establish C++20 Drogon API foundation`

### Phase 01 — Product API
- Added project directories.
- Added ProductController.
- Added `/api/v1/products`.
- Added test product response.
- Registered the product route.

Suggested commit:
`phase-01: add product controller and products endpoint`

### Phase 02 — Build/Debug Stabilization
- Fixed source/build configuration issues encountered during development.
- Verified direct compiler invocation.
- Verified build environment.
- Keep debugging changes separate from feature work.

Suggested commit:
`phase-02: stabilize MinGW CMake build workflow`

### Phase 03+ — Future Work
Use one commit per completed feature/module group.

## Future Commit Naming
```text
phase-03: add product management
phase-04: add user module
phase-05: add cart and order workflow
phase-06: add validation and error handling
phase-07: integrate modules
phase-08: finalize documentation and testing
```

## Contribution Rule
Do not create fake dates or fake contributions. If the historical work was made as one or several real commits, preserve those real commits. If it was not committed at the time, use clearly labeled reconstruction commits rather than pretending they were made earlier.

## Before Every Commit
```text
[ ] Code builds
[ ] Endpoint/module tested
[ ] Comments added
[ ] Documentation updated
[ ] No unrelated files included
[ ] Commit message describes the phase
```
