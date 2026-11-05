---
name: code-complexity
description: Evaluates C++ code complexity using clang-tidy and the readability-function-cognitive-complexity check. Use when asked to analyze, refactor, or report on code complexity.
---

# Code Complexity Evaluation Skill

## Workflow

1. Identify the files or functions to analyze.
2. Run `clang-tidy` with the `readability-function-cognitive-complexity` check enabled.
3. Interpret the results and provide a summary of the complexity.

## Execution

Use the following command to run the analysis on specific files:

```bash
clang-tidy -checks='-*,readability-function-cognitive-complexity' <file_path> -- -I<project_include_path>
```

Replace `<project_include_path>` with the appropriate include directory (e.g., `src/`, `include/`).

## Interpreting Results

- Complexity scores identify functions that may need refactoring.
- High complexity scores (e.g., > 15) suggest that logic should be broken down into smaller, more testable functions.
