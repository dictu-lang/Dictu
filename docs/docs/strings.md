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

Strings in Dictu are an immutable data type. This means that once a string has been created, there is no way to modify a string directly. Instead, a new string is created. Strings are indicated with quotation marks. In Dictu, you can use `'single'` or `"double"` quotes. They are treated the same.

### Concatenation

Use a plus sign to join string together. This is called concatenation.

```cs
"This item costs " + "27" + " euros"; // "This item costs 27 euros"
```

### Indexing

You can access a specific letter in a string with the same syntax you would use to access an item in a list.

```cs
var string = "Dictu";

string[0]; // D
string[-1]; // u
string[100]; // String index out of bounds.

for (var i = 0; i < x.len(); ++i) {
    print(string[i]);
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

If you want to get only part of a string, you slice it! To slice a string, use square brackets with the range you want to slice. The starting index is inclusive, but the ending index is exclusive. You can also use negative numbers to get characters from the end of a string.

```cs
"Dictu"[1:]; // ictu
"Dictu"[:4]; // Dict
"Dictu"[1:4]; // ict
"Dictu"[2:4]; // ct
"Dictu"[100:101]; // '' (empty string)
"Dictu"[0:100]; // Dictu
```

### Escape codes

“Escape codes” are used to create things like new lines and tabs. Because backslashes are used in escape codes, you need to type two backshashes (`\\`) to use a single backslash.

| \n | \n is the escape code for a new line |
| \r | \r is the escape code for a carriage return |
| \t | \t is the escape code for a horizontal tab |
| \v | \v is the escape code for a vertical tab |
| \\\ | \\\ is the escape code for a backslash |
| \\" | \\" is the escape code for double quotes |
| \\' | \\' is the escape code for single quotes |

You should only need the escape codes for single and double quotes if you are using that type of quote within a string indicated with the same type. If that doesn't make sense, you'll quickly realize it won't work when you come across it.

#### Raw strings

Sometimes, you wil need to ignore escape codes. Simply prefix your string with an `r` and any escape codes will be ignored.

```cs
// Normal Strings
"Dictu\trocks!"; // 'Dictu  rocks!'
// Raw Strins
r"Dictu\trocks!"; // 'Dictu\trocks!'
r"Dictu\trocks!".len(); // 12
```

### string.len()

Returns the length of the given string.

```cs
"string".len(); // 6
```

### string.lower()

Returns a lowercase version of the given string.

```cs
"DICTU".lower(); // dictu
"DiCtU".lower(); // dictu
"dictu".lower(); // dictu
```

### string.upper()

Returns an uppercase version of the given string.

```cs
"dictu".upper(); // DICTU
"DiCtU".upper(); // DICTU
"DICTU".upper(); // DICTU
```

### string.toNumber()

Casts a string to a number. This method returns a Result type and will unwrap to a number.

```cs
"10.2".toNumber().unwrap(); // 10.2
"10".toNumber().unwrap(); // 10
"10px".toNumber(); // <Result Err>
```

### string.toBool()

Casts a string to a boolean value. Any string except an empty string is considered `true`.

```cs
"".toBool(); // false
"false".toBool(); // true
```

### string.startsWith(string)

Returns true if a string starts with a given string.

```cs
"Dictu".startsWith("D"); // true
"Dictu".startsWith("Dict"); // true
"Dictu".startsWith("Dictu"); // true
"Dictu".startsWith("q"); // false
"Dictu".startsWith("d"); // false
```

### string.endsWith(string)

Returns true if a string ends with a given string.

```cs
"Dictu".endsWith("u"); // true
"Dictu".endsWith("tu"); // true
"Dictu".endsWith("Dictu"); // true
"Dictu".endsWith("ut"); // false
"Dictu".endsWith("q"); // false
"Dictu".endsWith("U"); // false
```

### string.split(delimiter)

Returns a list of strings, split based on a given delimiter.
Returns a list of all characters in a string if an empty string is passed as delimiter.
```cs
"Dictu is awesome!".split(" "); // ['Dictu', 'is', 'awesome!']
"Dictu is awesome!".split(""); // ["D", "i", "c", "t", "u", " ", "i", "s", " ", "a", "w", "e", "s", "o", "m", "e", "!"]
```

### string.replace(string: old, string: new)

Replaces part (a substring) of a string with another string.

```cs
"Dictu is okay...".replace("okay...", "awesome!"); // "Dictu is awesome!"
```

### string.contains(string)

Returns true if a string contains another string.

```cs
"Dictu is awesome!".contains("Dictu"); // true
"Dictu is awesome!".contains("Dictu is awesome!"); // true
```

### string.find(string, number: skip -> optional)

To find the index of a given substring, use the `.find()` method. This method takes an optional second parameter which can be used to skip the first `n` number of appearances of the substring. This method returns `-1` if the substring could not be found. Otherwise, it returns the index of the string.

```cs
"Hello, how are you?".find("how"); // 7
"hello something hello".find("hello", 2); // 16 (Skipped first occurrence)
"House".find("Lost Keys"); // -1 (Not found)
```

### string.leftStrip()

Strips whitespace from the left side of a string and returns the result.

```cs
"     Dictu".leftStrip(); // "Dictu"
```

### string.rightStrip()

Strips whitespace from the right side of a string and returns the result.

```cs
"Dictu     ".rightStrip(); // "Dictu"
```

### string.strip()

Strips whitespace from both sides of a string and returns the result.

```cs
"     Dictu     ".strip(); // "Dictu"
```

### string.format(...value: args...)

This method will replace any instances of `{}` with the provided parameters. It also casts all arguments to strings.

```cs
"Hello {}".format("Jerome"); // "Hello Jerome"
"Hello".format("Jerome"); // Error: format() placeholders do not match arguments
"Hello {}".format(); // Error: format() placeholders do not match arguments
"String: {}, Number: {}, Boolean: {}, List: {}, Dictionary: {}".format("String", 123, true, ["String", 123, true], {"key": "value"}); // "String: String, Number: 123, Boolean: true, List: ["String", 123, true], Dictionary: {"key": "value"}"
```

### string.count(string)

Returns the number of occurrences of a given substring within another string.

```cs
"Dictu is great!".count("Dictu"); // 1
"This documentation".count("Good jokes"); // 0
"Sooooooooooome characters".count("o"); // 11
```

### string.title()

Returns a title cased version of string with first letter of each word capitalized.

```cs
"dictu language".title(); // Dictu Language
"this documentation".title(); // This Documentation
"once upon a time".title(); // Once Upon A Time
```