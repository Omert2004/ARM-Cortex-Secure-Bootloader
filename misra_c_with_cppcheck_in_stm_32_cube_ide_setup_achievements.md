# MISRA-C With Cppcheck in STM32CubeIDE – Setup & Achievements

This document explains how MISRA-C–oriented static analysis was implemented
for an STM32 project using **Cppcheck** and **GCC warning flags** inside
**STM32CubeIDE**, while analyzing **only application code**.

The goal is practical MISRA discipline using free tools.

---

## Objective

- Analyze only application code (`Core/Src`, `Core/Inc`)
- Exclude STM32 HAL / Drivers from analysis
- Detect MISRA-C–style violations and unsafe constructs
- Use Cppcheck for static analysis
- Use GCC strict warnings for compile-time safety

---

## Project Scope

Analyzed directories:
```
Core/
├── Inc/
└── Src/
```

Excluded:
```
Drivers/
Middlewares/
CMSIS/
```

---

## Toolchain Used

| Tool | Purpose |
|-----|--------|
| STM32CubeIDE | Build system and GCC compiler |
| GCC | Compile-time warnings |
| Cppcheck | Static analysis |
| Cppcheck MISRA addon | MISRA-C:2012 rule checking |

---

## Cppcheck Configuration

Run from the project root directory:

```
cppcheck --enable=all --std=c99 --addon=misra \
  --suppress=missingIncludeSystem \
  --suppress=*:Drivers/* \
  -I Core/Inc \
  Core/Src
```

### Explanation

- `--enable=all` enables all diagnostics
- `--std=c99` matches STM32 C standard
- `--addon=misra` enables MISRA-C:2012 rules
- `--suppress=missingIncludeSystem` avoids system-header noise
- `--suppress=*:Drivers/*` ignores HAL and driver code
- `-I Core/Inc` includes only application headers
- `Core/Src` limits analysis to application sources

---

## Handling Missing HAL Headers

Warnings such as:
```
Include file: "stm32f7xx_hal.h" not found
```
are expected and acceptable, since HAL code is intentionally excluded.
The focus is on application logic rather than vendor libraries.

---

## GCC Compiler Warnings in STM32CubeIDE

To align with MISRA expectations, strict GCC warnings were enabled.

### Enabled Flags

```
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wsign-conversion
```

---

## How GCC Flags Were Added

1. Open STM32CubeIDE
2. Right-click project → Properties
3. Navigate to:
   C/C++ Build → Settings → MCU GCC Compiler → Miscellaneous
4. Add the following to **Other flags**:

```
-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
```

5. Apply and rebuild the project

---

## Purpose of Each Flag

- **-Wall**: common safety warnings
- **-Wextra**: stricter diagnostics (unused parameters, comparisons)
- **-Wpedantic**: ISO C standard enforcement
- **-Wconversion**: warns about implicit narrowing conversions
- **-Wsign-conversion**: warns about signed/unsigned conversions

These flags help catch issues expected to be controlled under MISRA.

---

## Recommended Workflow

1. Compile with GCC warnings enabled
2. Resolve all compiler warnings
3. Run Cppcheck with MISRA addon
4. Review violations
5. Refactor or document justified deviations

---

## Achievements

- Application-only static analysis
- HAL code cleanly excluded
- MISRA-C:2012 rules applied
- Strong compile-time diagnostics enabled
- Reproducible setup for team members
- Industry-aligned embedded workflow

---

## Notes on Compliance

This setup does not certify MISRA compliance.
It provides a strong, practical approximation suitable for:

- Academic projects
- Internship and portfolio work
- Early-stage embedded development

---

## Conclusion

Combining Cppcheck with strict GCC warnings inside STM32CubeIDE
creates a powerful and free MISRA-oriented workflow for STM32 projects.
