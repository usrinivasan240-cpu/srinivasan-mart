# Sri Mart — Week-by-Week Project Schedule

This schedule is designed for an AI-assisted, learning-first build. The objective is not only to produce code but to understand the complete workflow.

## Week 0 — Existing Setup / Baseline
### Goal
Make the existing environment reproducible.

### Tasks
- Confirm C++20 compiler.
- Confirm MSYS2 UCRT64/MinGW.
- Confirm CMake.
- Confirm vcpkg.
- Confirm Drogon.
- Confirm `sri_mart` build target.
- Confirm port 8080.
- Confirm `/api/v1/health`.
- Confirm `/api/v1/hello`.
- Confirm `/api/v1/products`.

### Learning
Understand:
`source code -> CMake -> compiler -> executable -> Drogon server -> HTTP endpoint`.

### Git phase
`phase-00-foundation`

---

## Week 1 — Product Domain
### Goal
Build the product feature cleanly.

### Tasks
- Product model.
- Product service.
- Product controller.
- Product list endpoint.
- Product detail endpoint.
- Product validation.
- Consistent JSON response format.

### AI learning questions
- Why is the model separate from the controller?
- Why does the controller call a service?
- What does Drogon do?
- What does CMake do during the build?

### Git phase
`phase-01-products`

---

## Week 2 — Product Management
### Goal
Complete product operations.

### Tasks
- Add product flow.
- Update product flow.
- Delete product flow.
- Input validation.
- Error responses.
- Keep controller/service/model responsibilities separated.

### Git phase
`phase-02-product-management`

---

## Week 3 — User/Customer Domain
### Goal
Introduce the customer/user domain using the same existing architecture.

### Tasks
- User model.
- User service.
- User controller.
- Required API routes.
- Validation.
- Error handling.
- Explain every file with comments.

### Git phase
`phase-03-users`

---

## Week 4 — Cart / Order Domain
### Goal
Build the shopping workflow.

### Tasks
- Cart-related model/data structures.
- Cart service.
- Cart controller.
- Order model.
- Order service.
- Order controller.
- Connect product -> cart -> order workflow.

### Git phase
`phase-04-cart-orders`

---

## Week 5 — Business Rules & Reliability
### Goal
Make the API behave predictably.

### Tasks
- Validate invalid input.
- Handle missing records.
- Handle invalid IDs.
- Standardize successful responses.
- Standardize error responses.
- Test every endpoint after changes.

### Git phase
`phase-05-validation`

---

## Week 6 — Integration
### Goal
Connect the completed modules through the existing architecture.

### Tasks
- Verify route registration.
- Verify controller -> service calls.
- Verify service -> model/data flow.
- Remove duplicated logic.
- Keep comments accurate.
- Rebuild from a clean build directory when required.

### Git phase
`phase-06-integration`

---

## Week 7 — Final Testing & Documentation
### Goal
Make the project understandable and reproducible.

### Tasks
- Endpoint checklist.
- Build checklist.
- Run checklist.
- Error-case checklist.
- Architecture documentation.
- Module documentation.
- Update README.
- Record known limitations.

### Git phase
`phase-07-finalization`

---

## Week 8 — Presentation / Demonstration
### Goal
Explain the project without depending on the AI.

### Tasks
- Explain every module in simple English.
- Explain one complete request from HTTP route to response.
- Explain why each technology is used.
- Demonstrate build.
- Demonstrate API endpoints.
- Demonstrate Git history.

### Git phase
`phase-08-release`
