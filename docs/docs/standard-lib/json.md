---
layout: default
title: JSON
nav_order: 12
parent: Standard Library
---

# JSON
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## JSON

To make use of the JSON module an import is required.

```js
import JSON;
```

### JSON.parse(String) -> Result\<Value>

Parses a JSON string and turns it into a valid Dictu datatype.
Returns a Result type and unwraps to <any> datatype.

```cs
JSON.parse('true'); // true
JSON.parse('{"test": 10}'); // {"test": 10}
JSON.parse('[1, 2, 3]'); // [1, 2, 3]
JSON.parse('null'); // nil
```

### JSON.stringify(value, String: indent -> Optional) -> Result\<String>

Stringify converts a Dictu value into a valid JSON string.
Returns a Result type and unwraps to string.

```cs
JSON.stringify(true); // 'true'
JSON.stringify({"test": 10}); // '{"test": 10}'
JSON.stringify([1, 2, 3]); // '[1, 2, 3]'
JSON.stringify(nil); // 'null'
```

Stringify also takes an optional `indent` parameter, which is a number of the amount of spaces to indent by, while
also making the response multiline.
```cs
JSON.stringify([1, 2, 3], 4);

// Output
'[
    1,
    2,
    3
]'
```
