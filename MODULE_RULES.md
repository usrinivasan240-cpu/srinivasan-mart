# Sri Mart — Module Documentation & Comment Rules

Every source file created or changed must have a clear purpose comment.

## Required Header Template

```cpp
// MODULE: <ModuleName>
// PURPOSE: <What this module does in simple English>
// WHY: <Why Sri Mart needs this module>
// INPUT: <What it receives>
// OUTPUT: <What it returns/changes>
// NOTE: Keep business logic in services and HTTP handling in controllers.
```

## Controller Comment

```cpp
// PURPOSE: This controller is the HTTP entry point for <feature>.
// It receives API requests, calls the service layer, and returns a response.
```

## Service Comment

```cpp
// PURPOSE: This service contains the business rules for <feature>.
// Controllers should call this service instead of implementing business rules themselves.
```

## Model Comment

```cpp
// PURPOSE: This model represents <data> used by Sri Mart.
// It keeps the structure of the data clear and reusable.
```

## sri.cpp Comment

```cpp
// PURPOSE: Application entry point for Sri Mart.
// It starts Drogon, registers API routes, and starts listening on port 8080.
```

## Non-Coder Rule
A person who does not know C++ should be able to read the first few comment lines and understand why the file exists.

## No Mystery Code Rule
Before adding a new function:
- state its purpose in a comment;
- use a descriptive function name;
- keep one responsibility per function where practical.
