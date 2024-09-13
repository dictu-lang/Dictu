---
layout: default
title: Control flow
nav_order: 7
---

# Control Flow
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## If statement

```cs
if (false or true) {
    var variable = "Hello";
    print(variable);  // Print is a native function so parenthesis are required
}

// Single line if statement body does not require braces
if (true)
    print("Hello");
    
if (false)
    print("Not going to print!");
else
    print("Will print!");
    
    
var x = 5;
if (x == 6) {
    print("Not going to print!");
} else if (x == 5) {
    print("Will print!");
} else {
    print("Not going to print!");
}
```

## Switch Statement

A switch statement can be seen as a more maintainable if/else if chain. It evaluates an expression and then enters a 
case block depending upon if the expression matches the expression of the case block.

The value supplied to a case block can be of any type and follows the same rules of equality as the `==` operator.

```cs
switch (10) {
    case 1: {
        // 10 doesn't equal 1, so this is skipped.
    }

    case 10: {
        // Match! This block of code is executed!
    }
}
```

### Default

Sometimes there may be a need for a bit of code to be executed if none of the case blocks match the expression of the switch statement. This is where
the default block comes into play.

```
switch ("string") {
    case "nope": {

    }

    case "nope again!": {

    }

    default: {
        // This will get executed!
    }
}
```

### Fall 

```cs
switch (1) {
    case 1: {
        // This block of code is executed!
    }

    fall
    case 10: {
        //  This block of code is executed!
    }
}
```

```cs
switch (1) {
    case 1: {
        // This block of code is executed!
    }

    fall
    default: {
        //  This block of code is executed!
    }
}
```

```cs
switch (1) {
    case 1: {
        // This block of code is executed!
    }

    fall case 10: {
        //  This block of code is executed!
    }
}
```

```cs
switch (1) {
    case 1: {
        // This block of code is executed!
    }

    fall
    case 10: {
        //  This block of code is executed!
    }
    fall // error fall always followed by case or default statement

}
```

```cs
switch (1) {
    case 1: {
        // This block of code is executed!
    }

    fall default: {
        //  This block of code is executed!
    }
    
    fall // error fall always followed by case or default statement
}
```



### Multi-Value Case

Sometimes we need to execute a block of code based on a set of values. This is easily done by just supplying a comma separated list
of values to the case block.

```cs
switch (10) {
    case 1, 3, 5, 7, 9: {
        // 10 doesn't equal any of the supplied values, this is skipped.
    }

    case 0, 2, 4, 6, 8, 10: {
        // Match! This block of code is executed!
    }
}
```

## Loops
### While loop

```cs
// While loop
var i = 0;
while (i < 10) {
    print(i);
    i += 1;
}
```

Dictu also implements syntactic sugar for a while true loop.

```cs
// While 'true' loop
// If no expression is made, true is implicitly assumed
while {
    print("Hello");
    sleep(1);
}
```

### For loop

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    print(i);
}
```

### Continue statement

Continue allows execution of a loop to restart prematurely.

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    if (i % 2 == 0)
        continue; // Skip all even numbers

    print(i); // Only odd numbers will be printed
}
```

### Break statement

Break allows execution of a loop to stop prematurely.

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    if (i > 5)
        break; // Exit the loop here

    print(i); // Only numbers less than or equal to 5 will be printed
}
```
