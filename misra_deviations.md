# MISRA-C Deviations Log

This document records **intentional deviations** from MISRA-C:2012 rules
used in this project.

Deviations are allowed under MISRA **only when**:
- The rule is understood
- The risk is assessed
- The deviation is documented and justified

This file provides traceability for reviewers, instructors, and auditors.

---

## How to Use This File

For **each MISRA violation** that cannot be removed:
1. Add a new entry below
2. Reference the MISRA rule number
3. Explain *why* the rule is violated
4. Explain *why it is safe* in this context
5. Describe any mitigation

---

## Deviation Entry Template

```
### Deviation ID: DEV-XXX

- **MISRA Rule**:
- **Rule Type**: Required / Advisory
- **File**:
- **Line(s)**:
- **Description**:
- **Justification**:
- **Risk Assessment**:
- **Mitigation**:
- **Approval**:
```

---

## When and Why MISRA Deviations Are Documented

MISRA-C deviations are documented **after the application code reaches a stable state** and all reasonable static analysis warnings have been addressed.

During development, the primary focus is on **functional correctness** and **system behavior**, especially in embedded systems where low-level hardware access, bootloader logic, and cryptographic operations are involved. Enforcing strict MISRA compliance too early may unnecessarily slow development without improving safety.

Once the code:

- Compiles cleanly with strict GCC warnings enabled  
- Has been analyzed using **Cppcheck with the MISRA addon**  
- Has had all *reasonable and fixable* MISRA warnings resolved  

the remaining MISRA violations are reviewed individually.

If a violation:

- Cannot be removed without reducing clarity, performance, or correctness, or  
- Is required due to hardware constraints (e.g., memory-mapped I/O, pointer–address casting),  

it is **intentionally accepted** and documented in the `misra_deviations.md` file.

Each documented deviation includes:

- The violated MISRA rule  
- A technical justification  
- A brief risk assessment  
- Any mitigation measures applied  

This approach follows MISRA’s own philosophy:

> *MISRA allows deviations provided they are understood, justified, and documented.*

By documenting deviations at this stage, the project ensures:

- Transparency  
- Maintainability  
- Traceability for reviewers and instructors  
- An industry-aligned embedded software workflow  

**In summary:**  
MISRA deviations are documented **after static analysis and cleanup**, when the remaining violations are intentional, justified, and controlled.

It demonstrates **professional embedded software discipline** even when
full MISRA compliance is not required.

```

---

## Logged Deviations

### Deviation ID: DEV-001

- **MISRA Rule**: MISRA-C:2012 Rule 11.4  
- **Rule Type**: Required  
- **File**: Core/Src/BL_Functions.c  
- **Line(s)**: 314  
- **Description**:  
  Conversion between integer address and pointer type.

- **Justification**:  
  This code accesses a fixed flash memory address defined by the STM32
  memory map. Such pointer casting is unavoidable in low-level embedded
  bootloader code.

- **Risk Assessment**:  
  The address is constant, aligned, and validated against the device
  reference manual. No dynamic or user-controlled input is involved.

- **Mitigation**:  
  - Address defined as a macro with clear documentation  
  - Access limited to a single translation unit  
  - Reviewed against STM32F7 memory layout

- **Approval**:  
  Project Supervisor / Code Reviewer

---

### Deviation ID: DEV-002

- **MISRA Rule**: MISRA-C:2012 Rule 15.5  
- **Rule Type**: Advisory  
- **File**: Core/Src/BL_Functions.c  
- **Line(s)**: Multiple  
- **Description**:  
  Multiple return statements used within functions.

- **Justification**:  
  Early returns improve readability and error handling clarity in
  cryptographic and bootloader logic.

- **Risk Assessment**:  
  Control flow remains simple and deterministic. All exit paths are
  explicitly handled.

- **Mitigation**:  
  - Functions kept short  
  - All resources released before return  
  - No dynamic memory used

- **Approval**:  
  Project Supervisor / Code Reviewer

---

### Deviation ID: DEV-003

- **MISRA Rule**: MISRA-C:2012 Rule 7.2  
- **Rule Type**: Required  
- **File**: Core/Src/BL_Functions.c  
- **Line(s)**: Various  
- **Description**:  
  Use of hexadecimal integer literals without explicit suffix.

- **Justification**:  
  Constants represent hardware-defined register values and flash
  addresses documented in the reference manual.

- **Risk Assessment**:  
  Literal values are within 32-bit unsigned range.

- **Mitigation**:  
  - Constants centralized in macros  
  - Values verified against STM32 reference manual

- **Approval**:  
  Project Supervisor / Code Reviewer

---

## Review Status

| Deviation ID | Reviewed | Date | Reviewer |
|-------------|---------|------|---------|
| DEV-001 | ⬜ | | |
| DEV-002 | ⬜ | | |
| DEV-003 | ⬜ | | |

---
