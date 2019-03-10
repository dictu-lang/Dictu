---
layout: default
title: Strings
nav_order: 4
---

# Strings
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Strings
Strings in Dictu are an immutable data type, this means, once a string has been created there is no way to modify a string directly, and instead a new string is created. Strings are created by having some sort of text between quotes, `"hello"` or `'hello'` is fine in Dictu.

### Concatenation

To join strings together use the `+` operator.

```py
"hello " + "there!"; // "hello there!"
```

### .lower()

To make all characters within a string lowercase, use the `.lower()` method.

```py
"DICTU".lower(); // dictu
"DiCtU".lower(); // dictu
```

### .upper()

To make all characters within a string uppercase, use the `.upper()` method.

```py
"dictu".lower(); // DICTU
"DiCtU".lower(); // DICTU
```

### .startsWith(string)

To check if a string starts with a certain substring, use the `.startsWith()` method.

```py
"dictu".startsWith("d"); // true
"dictu".startsWith("u"); // false
```

### .endsWith(string)

To check if a string ends with a certain substring, use the `.endsWith()` method.

```py
"dictu".endsWith("d"); // false
"dictu".startsWith("u"); // true
```

### .split(delimiter)

To split a string up into a list by a certain character or string, use the `.split()` method.

```py
"Dictu is great".split(" "); // ['Dictu', 'is', 'great']
```

### .replace(old, new)

To replace a substring within a string, use the `.replace()` method.

```py
"Dictu is okay".replace("okay", "great"); // "Dictu is great"
```

### .contains(string)

To check if a string contains another string, use the `.contains()` method.

```py
"Dictu is great".contains("Dictu"); // true
```

### .find(string, skip: optional)

To find the index of a given string, use the `.find()` method.

Skip is an optional parameter which can be passed to skip the first `n` amount of appearances of the given substring.

```py
"Hello, how are you?".find("how"); // 7
"hello something hello".find("hello", 2); // 16 -- Skipped the first occurance of the word "hello"
```

### .leftStrip(string)

To strip all whitespace at the beginning of a string, use the `.leftStrip()` method.

```py
"   hello".leftStrip(); // "hello"
```

### .rightStrip(string)

To strip all whitespace at the end of a string, use the `.rightStrip()` method.

```py
"hello   ".rightStrip(); // "hello"
```

### .strip(string)

To strip whitespace at the beginning and end of a string, use the `.strip()` method.

```py
"    hello    ".strip(); // "hello"
```