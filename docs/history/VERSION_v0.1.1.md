# VERSION v0.1.1 — exact ExecuteFunction overload resolver

## A. Identity / Lineage
```text
Version: v0.1.1
Date: 2026-08-16
Based On: v0.1.0
Reason Created: live v0.1.0 test proved bridge Probe PASS but AutoFight ON failed before Lua invoke because ExecuteFunction signature resolver rejected the live metadata
Last Known-Good AutoFight: UNKNOWN
Regression From: v0.1.0 adapter mismatch
Supersedes: v0.1.0 for further runtime testing
```

## B. User Runtime Evidence Triggering This Version
```text
Bridge probe PASS; chưa gửi AutoFight action
ON FAIL: ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp
```

Interpretation:
- external attach/shared-memory/hook callback path is confirmed working for Probe;
- ON reached the in-process Lua adapter;
- no AutoFight mutable action was sent because signature resolution failed first;
- this test does not yet implicate the MainThread/thread-context risk.

## C. Root Cause / Weakness Found
v0.1.0 used `il2cpp_class_get_method_from_name(class, "ExecuteFunction", argc)` and then checked the returned parameter types.

That API only selects by method name + parameter count. When multiple overloads share the same argc, it can return a different overload than the desired one. Therefore v0.1.0 could reject the live client even if a compatible overload exists elsewhere in the overload set.

Exact live overloads are not guessed in v0.1.1.

## D. Changes Made
- Added `il2cpp_class_get_methods`.
- Added `il2cpp_method_get_name`.
- Added `FindMethodExact(...)` to enumerate every method and match:
  - name;
  - parameter count;
  - exact IL2CPP parameter type names.
- Preserved only the two intended invocation shapes:
  - `(System.String,System.String)`;
  - `(System.String,System.String,System.Object[])`.
- Added fail-closed diagnostic `AppendExecuteFunctionSignatures(...)`.
- If neither known shape exists, the tool now prints the actual live overload list instead of trying arbitrary payloads.
- Bumped test protocol to `0x00020100` so v0.1.0 EXE/DLL cannot silently mix with v0.1.1.
- Updated output/version identity to v0.1.1.

## E. Safety / Scope
Still forbidden:
- mouse/input emulation;
- `UIButton.HandleClickEvent` replay;
- broad combat reconstruction;
- guessed ExecuteFunction arguments;
- marking runtime success from a returned call alone.

## F. Expected v0.1.1 Runtime Outcomes

### Outcome 1 — compatible overload exists
Expected log starts with one of:
```text
ExecuteFunction exact 2-param matched; ...
ExecuteFunction exact 3-param matched; ...
```
Then user must verify whether AutoFight actually turns ON/OFF.

### Outcome 2 — client uses another signature
Expected failure now contains:
```text
ExecuteFunction overloads: static(...)->... | inst(...)->...
```
That full line becomes the exact metadata evidence for v0.1.2. Do not guess before reading it.

### Outcome 3 — matched method invokes but crashes/disconnects/no-ops
Only then move diagnosis to BUG-001 / MainThread execution context or script lifetime.

## G. Runtime Status
```text
v0.1.0 Probe: PASS
v0.1.0 AutoFight ON: FAIL before Lua invoke
v0.1.1 AutoFight ON/OFF: RUNTIME UNTESTED
Known-Good AutoFight: UNKNOWN
```
