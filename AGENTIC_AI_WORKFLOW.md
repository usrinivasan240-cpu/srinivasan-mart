# Sri Mart — Agentic AI Learning Workflow

The AI is used as a development assistant, but the user remains the project owner.

## AI Cycle For Every Module

```text
1. PLAN
   ↓
2. EXPLAIN
   ↓
3. GENERATE
   ↓
4. BUILD
   ↓
5. TEST
   ↓
6. DEBUG
   ↓
7. DOCUMENT
   ↓
8. COMMIT
```

## 1. PLAN
Ask the AI:
> Explain what we are about to build, which existing module it belongs to, what files will change, and why.

## 2. EXPLAIN
Before accepting code:
> Explain this design to me as a non-coder. Tell me what each file and function is responsible for.

## 3. GENERATE
AI generates only the required code using the locked stack.

## 4. BUILD
Run the project's existing CMake/MinGW build process.

## 5. TEST
Test the endpoint or module immediately.

## 6. DEBUG
Give the AI the exact compiler/runtime output. Do not paraphrase errors.

## 7. DOCUMENT
Update the relevant Markdown file and code comments.

## 8. COMMIT
Make one Git commit for the completed phase.

## Golden Rule
Never ask AI to build the entire project blindly in one step. Work module-by-module so the architecture stays understandable.
