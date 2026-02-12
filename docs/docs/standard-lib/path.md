---
layout: default
title: Path
nav_order: 17
parent: Standard Library
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

| Constant          | Description                          |
| ----------------- | ------------------------------------ |
| Path.delimiter    | System dependent path delimiter      |
| Path.dirSeparator | System dependent directory separator |

### Path.basename(String) -> String

Returns the basename of the path.

```cs
Path.basename("/usr/bin"); // 'bin'
```

### Path.dirname(String) -> String

Returns the directory name of the path.

```cs
Path.dirname("/usr/bin"); // '/usr'
```

### Path.extname(String) -> String

Returns the extension portion of the path, including the dot.

```cs
Path.extname("/tmp/t.ext"); // '.ext'
Path.extname("/tmp/t");     // ''
```

### Path.isAbsolute(String) -> Boolean

Returns true if path is absolute, false otherwise.

```cs
Path.isAbsolute("/usr"); // true
Path.isAbsolute("usr");  // false
```

### Path.realpath(String) -> Result\<String>

Returns A result type and unwraps the canonicalized absolute pathname as a string.

**Note:** This is not available on windows systems.

```cs
Path.realpath("/dir/../dir/../dir"); // '/dir'
```

### Path.exists(String) -> Boolean

Returns a boolean whether a file exists at a given path.

```cs
Path.exists("some/path/to/a/file.du"); // true
```

### Path.isDir(String) -> Boolean

Checks whether a given path points to a directory or not. 

```cs
Path.isDir("/usr/bin/"); //true
```

### Path.isSymbolicLink(String) -> Boolean

Checks whether a given path is a symbolic link. 

**Note:** This is not available on windows systems.

```cs
Path.isSymbolicLink("/usr/bin/"); //false
```

### Path.listDir(String) -> List

Returns a list of strings containing the contents of the input path.

**Note:** This function does not guarantee any ordering of the returned list.

```js
Path.listDir("/"); // ["bin", "dev", "home", "lib", ...]
```

### Path.join(Iterable) -> String

Returns the provided string arguments joined using the directory separator.

**Note:** A trailing directory separator is ignored from each argument

```js
Path.join('/tmp', 'abcd', 'efg') == '/tmp/abcd/efg';
Path.join(['/tmp', 'abcd', 'efg']) == '/tmp/abcd/efg';
Path.join('/tmp/', 'abcd/', 'efg/') == '/tmp/abcd/efg';
```
