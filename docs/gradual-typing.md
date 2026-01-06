# Aether Gradual Typing Guide

Aether supports **gradual typing** - a flexible approach where types are optional and type checking can be adjusted based on your needs.

## Philosophy

- **Types are optional** - Write code without types, add them later for clarity
- **No type errors block compilation** - Type mismatches generate warnings, not errors
- **Runtime safety** - Type checking moves to runtime when static checking isn't possible
- **Progressive enhancement** - Add types incrementally as your codebase evolves

## Type Modes

### 1. Dynamic Mode (Default)
```aether
x = 42;              // No type
y = "hello";
z = x + y;           // Warning: mixing int + string, but compiles
```

### 2. Gradual Mode (Recommended)
```aether
x: int = 42;         // Explicit type
y = "hello";         // Inferred type
z = x + y;           // Warning: type mismatch
```

### 3. Strict Mode (Optional)
```aether
// Enable strict mode (future feature)
#pragma strict_types

x: int = 42;
y: string = "hello";
z = x + y;           // ERROR: Won't compile
```

## Examples

### Fully Dynamic
```aether
// No types at all - works like Python/JavaScript
func calculate(a, b) {
    return a + b;
}

main() {
    print(calculate(5, 10));      // 15
    print(calculate("a", "b"));   // "ab"
    print(calculate([1], [2]));   // [1, 2] (array concat)
}
```

### Gradually Typed
```aether
// Mix typed and untyped code
func process_data(items: array<int>): int {
    total = 0;  // Type inferred
    for item in items {
        total = total + item;
    }
    return total;
}

main() {
    data = [1, 2, 3, 4, 5];  // Type inferred
    result = process_data(data);
    print(result);
}
```

### Runtime Type Checking
```aether
func safe_add(a, b) {
    if (typeof(a) == "int" && typeof(b) == "int") {
        return a + b;
    } else if (typeof(a) == "string" && typeof(b) == "string") {
        return a + b;
    } else {
        print("Type error: Cannot add " + typeof(a) + " and " + typeof(b));
        return 0;
    }
}

main() {
    print(safe_add(5, 10));        // 15
    print(safe_add("a", "b"));     // "ab"  
    print(safe_add(5, "hello"));   // Error message
}
```

## Runtime Type Functions

### typeof(value)
Returns the runtime type of a value as a string.
```aether
x = 42;
print(typeof(x));  // "int"

y = "hello";
print(typeof(y));  // "string"

z = [1, 2, 3];
print(typeof(z));  // "array"
```

### is_type(value, type_name)
Checks if a value matches a specific type.
```aether
x = 42;
if (is_type(x, "int")) {
    print("x is an integer");
}

func process(data) {
    if (is_type(data, "array")) {
        // Handle array
    } else if (is_type(data, "string")) {
        // Handle string
    }
}
```

### convert_type(value, target_type)
Attempts to convert a value to another type.
```aether
x = "42";
y = convert_type(x, "int");  // 42 (int)

a = 42;
b = convert_type(a, "string");  // "42" (string)
```

## Dynamic Property Access

Access struct fields dynamically:
```aether
struct Player {
    name,
    score,
    health
}

main() {
    p = Player{ name: "Alice", score: 100, health: 50 };
    
    // Static access
    print(p.name);
    
    // Dynamic access
    field = "score";
    print(p[field]);  // 100
    
    // Set dynamically
    p[field] = 200;
    print(p.score);  // 200
}
```

## Best Practices

### When to Use Types
1. **Public APIs** - Add types to function signatures
2. **Complex data structures** - Type struct fields
3. **Performance-critical code** - Types enable optimizations
4. **Team collaboration** - Types serve as documentation

### When to Skip Types
1. **Prototypes** - Move fast without type annotations
2. **Scripts** - Short programs don't need full typing
3. **Exploratory code** - Figure out the design first
4. **Dynamic algorithms** - When types change frequently

### Migration Strategy
```aether
// 1. Start dynamic
func process(data) {
    return data.length;
}

// 2. Add input types
func process(data: array) {
    return data.length;
}

// 3. Add output types
func process(data: array): int {
    return data.length;
}

// 4. Add full annotations (optional)
func process(data: array<string>): int {
    count: int = data.length;
    return count;
}
```

## Compiler Flags

```bash
# Default: Dynamic mode (warnings only)
aetherc program.ae

# Strict mode: Type errors block compilation (future)
aetherc --strict program.ae

# Disable type warnings
aetherc --no-type-warnings program.ae

# Type checking level (future)
aetherc --type-level=strict|gradual|dynamic program.ae
```

## Summary

- **Default behavior**: All types optional, warnings for mismatches
- **Runtime safety**: Use `typeof()`, `is_type()` for runtime checks
- **Progressive enhancement**: Add types incrementally
- **No breaking changes**: Code without types always compiles
- **Future-proof**: Can enable strict mode per-file when needed

Aether gives you the flexibility to choose your typing style!
