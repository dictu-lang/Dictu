---
layout: default
title: Path
nav_order: 15
---

# Path
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Path

To make use of the Path module an import is required.

```js
import Path;
```

### Constants

| Constant           | Description                          |
|--------------------|--------------------------------------|
| Path.delimiter     | System dependent path delimiter      |
| Path.dirSeparator  | System dependent directory separator |
| Path.errno         | Number of the last error (UNIX only) |

### Path.basename(string)

Returns the basename of string.

```js
Path.basename("/usr/bin"); // 'bin'
```

### Path.dirname(string)

Returns the directory name of string.

```js
Path.dirname("/usr/bin"); // '/usr'
```

### Path.extname(string)

Returns the extension portion of string, including the dot.

```js
Path.extname("/tmp/t.ext"); // '.ext'
Path.extname("/tmp/t");     // ''
```

### Path.isAbsolute(string)

Returns true if string is an absolute path or false otherwise.

```js
Path.isAbsolute("/usr"); // true
Path.isAbsolute("usr");  // false
```

### Path.strerror(number: error -> optional)
Get the string representation of an error.
An optional error status can be passed, otherwise the default is Path.errno.
It returns a string that describes the error.
**Note:** This is not available on windows systems.

```js
print(Path.strerror());
```

### Path.realpath(string)

Returns the canonicalized absolute pathname or nil on error and sets Path.errno accordingly.

**Note:** This is not available on windows systems.

```js
Path.realpath("/dir/../dir/../dir"); // '/dir'
```

### Path.exists(string)

Returns a boolean whether a file exists at a given path.

```js
Path.exists("some/path/to/a/file.du"); // true
```

### Path.isdir(string)

Checks whether a given path points to a directory or not. 

**Note:** This is not available on windows systems yet.

```js
Path.isdir("/usr/bin/"); //true
```

### Path.listdir(string)

Returns a list of strings containing the contents of the input path.

**Note:** This function does not guarantee any ordering of the returned list.

```js
Path.listdir("/"); // ["bin", "dev", "home", "lib", ...]
```
