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

### Indexing

Strings can be indexed just like you can with lists, to return single characters, or to loop over strings.

```py
var x = "Dictu";

x[0]; // D

for (var i = 0; i < len(x); ++i) {
    print(x[i]);
}
// D
// i
// c
// t
// u
```

### Slicing

Slicing is technique used when we wish to return a substring. It has a syntax of <string>[start:end] where start and end are optional
as long as one is provided.

E.g `"Dictu"[1:]` or `"Dictu"[:5]` are both valid. The start index is inclusive but the end index is exclusive.

```py
"Dictu"[1:]; // ictu
"Dictu"[:4]; // Dict
"Dictu"[1:4]; // ict
"Dictu"[2:4]; // ct
```

### string.lower()

To make all characters within a string lowercase, use the `.lower()` method.

```py
"DICTU".lower(); // dictu
"DiCtU".lower(); // dictu
```

### string.upper()

To make all characters within a string uppercase, use the `.upper()` method.

```py
"dictu".upper(); // DICTU
"DiCtU".upper(); // DICTU
```

### string.startsWith(string)

To check if a string starts with a certain substring, use the `.startsWith()` method.

```py
"dictu".startsWith("d"); // true
"dictu".startsWith("u"); // false
```

### string.endsWith(string)

To check if a string ends with a certain substring, use the `.endsWith()` method.

```py
"dictu".endsWith("d"); // false
"dictu".startsWith("u"); // true
```

### string.split(delimiter)

To split a string up into a list by a certain character or string, use the `.split()` method.

```py
"Dictu is great".split(" "); // ['Dictu', 'is', 'great']
```

### string.replace(old, new)

To replace a substring within a string, use the `.replace()` method.

```py
"Dictu is okay".replace("okay", "great"); // "Dictu is great"
```

### string.contains(string)

To check if a string contains another string, use the `.contains()` method.

```py
"Dictu is great".contains("Dictu"); // true
```

### string.find(string, skip: optional)

To find the index of a given string, use the `.find()` method.

Skip is an optional parameter which can be passed to skip the first `n` amount of appearances of the given substring.

```py
"Hello, how are you?".find("how"); // 7
"hello something hello".find("hello", 2); // 16 -- Skipped the first occurance of the word "hello"
```

### string.leftStrip()

To strip all whitespace at the beginning of a string, use the `.leftStrip()` method.

```py
"   hello".leftStrip(); // "hello"
```

### string.rightStrip()

To strip all whitespace at the end of a string, use the `.rightStrip()` method.

```py
"hello   ".rightStrip(); // "hello"
```

### string.strip()

To strip whitespace at the beginning and end of a string, use the `.strip()` method.

```py
"    hello    ".strip(); // "hello"
```

### string.format(...args...)

To format a string with any type of value `.format()` can be used. This will convert any type
to a string and swap placeholders `{}` for values.

```py
"Hello {}".format("Jason"); // "Hello Jason"

"{} {} {} {}".format(10, "hi", [10, 20], {"test": 10}) // '10 hi [10, 20] {"test": 10}'
```

### string.count(string)

To count the amount of occurrences of a given substring within another string.

```py
"Dictu is great!".count("Dictu"); // 1

"Sooooooooooome characters".count("o"); // 11
```