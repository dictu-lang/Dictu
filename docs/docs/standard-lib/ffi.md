---
layout: default
title: FFI
nav_order: 7
parent: Standard Library
---
# FFI
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## FFI
To make use of the FFI module an import is required.

```js
import FFI;
```

### Creating a module
Creating an FFI module requires building a dynamically shared library(.dll/.dylib/.so).

To build a module there's only a single header needed [dictu_ffi_include.h](https://github.com/dictu-lang/Dictu/blob/develop/src/include/dictu_ffi_include.h).
The header contains definitions, function declarations and function pointers(set at runtime automatically) for anything needed to interact with the DictuVM.
Further it contains the following function declaration:
```c
int dictu_ffi_init(DictuVM *vm, Table *method_table);
```
You will need to write a function definition (implementation) for it.
The functions return value determines success or failure, `0` is success and anything > `0` is failure,
Dictu's VM will throw a runtime error with the given code if the function returns a non 0 value.

Upon loading the module (shared library) in Dictu, the VM will load and invoke this function.
**NOTE: do not use vm functions before the init function is called since functions won't be available yet!**

Within the function you can then define properties and functions which will then be available from dictu.
The functions must follow this signature, the types are defined in the `dictu_ffi_include.h` header:
```c
Value ffi_function(DictuVM *vm, int argCount, Value *args);
```

[Full example in C using cmake]()

#### Functions
Given the following c code:
```c
#include <dictu_ffi_include.h>
//...
Value dictu_ffi_function(DictuVM *vm, int argCount, Value *args) {
    return OBJ_VAL(copyString(vm, "Hello From Dictu FFI module!", 28));
}
//...
int dictu_ffi_init(DictuVM *vm, Table *method_table) {
    // ...
  defineNative(vm, method_table, "getString", dictu_ffi_function);
   // ...
  return 0;
}
```
In dictu you can do
```cs
import FFI;

const mod = FFI.load("/path/to/library{}".format(FFI.suffix));
const str = mod.getString(); // "Hello From Dictu FFI module!"
```

#### Properties
Given the following c code:
```c
//...
int dictu_ffi_init(DictuVM *vm, Table *method_table) {
    // ...
    defineNativeProperty(
    vm, method_table, "foo",
        OBJ_VAL(copyString(vm, "Dictu!", 6)));
   // ...
}
```
In dictu you can do
```cs
import FFI;

const mod = FFI.load("/path/to/library{}".format(FFI.suffix));
const str = mod.foo; // Dictu!
```

Here is a complete example from the [ffi-example]():
```c
#include <dictu_ffi_include.h>

Value dictu_ffi_test(DictuVM *vm, int argCount, Value *args) {
    if(argCount != 2 || !IS_NUMBER(args[0]) || !IS_NUMBER(args[1])){
        return NIL_VAL;
    }
    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    return NUMBER_VAL(a+b);
}
Value dictu_ffi_test_str(DictuVM *vm, int argCount, Value *args) {
    return OBJ_VAL(copyString(vm, "Hello From Dictu FFI module!", 28));
}

int dictu_ffi_init(DictuVM *vm, Table *method_table) {
  defineNative(vm, method_table, "dictuFFITestAdd", dictu_ffi_test);
  defineNative(vm, method_table, "dictuFFITestStr", dictu_ffi_test_str);
  defineNativeProperty(
    vm, method_table, "test",
        OBJ_VAL(copyString(vm, "Dictu!", 6)));
  return 0;
}
```

### Constants

| Constant        | Description                                                                                       |
| --------------- | ------------------------------------------------------------------------------------------------- |
| FFI.suffix      | The system specific shared library suffix with the period(`.`): .dll/.so/.dylib.                  |

```cs
import FFI;
const mod = FFI.load("/path/to/library{}".format(FFI.suffix));
print(mod.test); // Dictu!
print(mod.dictuFFITestStr()); // "Hello From Dictu FFI module!"
print(mod.dictuFFITestAdd(22, 22)); // 44
print(mod.dictuFFITestAdd(22)); // nil
```

### FFI.load(String) -> FFIInstance
Load a module. The shared library **MUST** include the `dictu_ffi_include.h` header and have `dictu_ffi_init` defined otherwise it might lead to UB.
```cs
const mod = FFI.load("/path/to/shared-object");
print(mod); // <FFIInstance>
// mod will contain all defined functions and properties by the module.
```
