# To Learn

## Singleton pattern
Guarantees exactly one instance of a class: private constructor plus a static accessor.
```cpp
static OutputLog& instance() { static OutputLog log(...); return log; }
```
- Downside: it is global state in disguise (any code can reach it), which breaks explicit dependencies and makes testing harder.
- In this project: not used. One `OutputLog` is created in `main` and passed by reference.
