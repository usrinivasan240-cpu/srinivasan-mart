# Sri Mart — Copy/Paste AI Prompts

## Prompt 1 — Planning
```text
You are helping me develop Sri Mart using ONLY the existing project technology stack:
C++20, Drogon, CMake, vcpkg, MSYS2 UCRT64/MinGW, JSON support already used by the project, and Git/GitHub.

Before writing code, explain:
1. What we are building.
2. Which existing module it belongs to.
3. Which files need to be created/changed.
4. What each file is responsible for.
5. How the request will flow through the architecture.

Do not introduce another technology stack.
```

## Prompt 2 — Non-Coder Explanation
```text
Explain this Sri Mart code to me as if I am a beginner who does not know C++.
For every file and function:
- explain its purpose;
- explain what comes in;
- explain what goes out;
- explain why it is separated from other files;
- give a simple real-world analogy.

Do not skip any function.
```

## Prompt 3 — Code Generation
```text
Generate only the code required for this one Sri Mart module.
Use only C++20 + Drogon + the existing CMake/vcpkg/MSYS2 setup.
Do not add another framework or library.
Every source file must start with a plain-English PURPOSE comment.
Keep HTTP handling in controllers and business logic in services.
```

## Prompt 4 — Debugging
```text
I will give you the exact compiler/runtime output.
Do not guess.
Identify the first real error, explain why it occurs, and give the smallest safe fix.
Do not replace the project's technology stack.
After the fix, give me the exact build/test command.
```

## Prompt 5 — Review
```text
Review this Sri Mart module.
Check:
- architecture;
- controller/service/model separation;
- comments;
- naming;
- validation;
- error handling;
- build compatibility with C++20/Drogon/CMake/vcpkg/MSYS2;
- unnecessary code.

Explain every issue in beginner-friendly language.
```
