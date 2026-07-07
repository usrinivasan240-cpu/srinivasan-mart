# Source Code Comment Template

Use this at the top of every new source module:

```cpp
// ============================================================
// MODULE: <Name>
// PURPOSE: <Explain what this file does in simple English.>
// WHY: <Explain why Sri Mart needs this file.>
// INPUT: <What information enters this module?>
// OUTPUT: <What information leaves this module?>
// ARCHITECTURE: <controller/service/model/config/entry point>
// ============================================================
```

## Example

```cpp
// ============================================================
// MODULE: ProductController
// PURPOSE: Handles HTTP requests related to products.
// WHY: Keeps product API request handling separate from business logic.
// INPUT: HTTP requests such as GET /api/v1/products.
// OUTPUT: JSON responses containing product information.
// ARCHITECTURE: Controller
// ============================================================
```
