---
layout: default
title: Path
nav_order: 13
---

# Path
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### Constants

| Constant           | Description                          |
|--------------------|--------------------------------------|
| Path.delimiter     | System dependent path delimiter      |
| Path.dir_separator | System dependent directory separator |

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

### Path.realpath(string)

Returns the canonicalized absolute pathname or nil on error.

**Note:** This is not available on windows systems.

```js
Path.realpath("/dir/../dir/../dir"); // '/dir'
```
