---
layout: default
title: JSON
nav_order: 14
---

# JSON
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### JSON.parse(string)

Parses a JSON string and turns it into a valid Dictu datatype.

```js
JSON.parse('true'); // true
JSON.parse('{"test": 10}'); // {"test": 10}
JSON.parse('[1, 2, 3]'); // [1, 2, 3]
JSON.parse('null'); // nil
```

### JSON.stringify(value)

Stringify converts a Dictu value into a valid JSON string.

```js
JSON.stringify(true); // 'true'
JSON.stringify({"test": 10}); // '{"test": 10}'
JSON.stringify([1, 2, 3]); // '[1, 2, 3]'
JSON.stringify(nil); // 'null'
```

Stringify also takes an optional `indent` parameter, which is a number of the amount of spaces to indent by.
```js
JSON.stringify([1, 2, 3], 4);

// Output
'[
    1,
    2,
    3
]'
```