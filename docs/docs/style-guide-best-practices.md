---
layout: default
title: Style Guide Best Practices
nav_order: 9
---

# Style-Guide
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Introduction

These guidelines attempt to provide a framework for code styling and formatting with the end goal of having all Dictu code similar enough for maximal readability and familiarity.

## Formatting

Favor 4 spaces over tabs.

## Commentary

Dictu supports single and multiline comments in the form of `//` and `/** */` respectively. Declaration comments should use the former. For example:

```cs
// class User provides ...
class User {
    private password;
    
     // User class initializer. Creates a new user value, sets the given name fields,
     // and generates a new password.
    init(var firstName, var lastName) {
        // generate a new password.
        this.password = generatePassword();
    }
}
```

## Module Names

Depending on how a module is imported, either the module name becomes the accessor or the item imported is accessed directly. Modules should have short names and be simple nouns describing the contents. For example: `Sockets`, `HTTP`, `System`.

```cs
import System;
```

## Imports

Imports should be grouped together into 3 sections. The first section is for regular imports from the standard library. The second is for `from` imports from the standard library. And the third is for local file based imports. The 3 above sections should be seperated by 1 line and listed alphabetically.

```cs
import Env;
import HTTP;
import Sockets;

from Argparse import Parser;

from "standard_model/particles.du" import Boson;
from "standard_model/colliders.du" import Cern; 
```

Add 2 blank lines after imports are complete.

## Mixed Caps

The naming convention in Dictu is to use mixed caps, Camel Case and Pascal Case, to write multiword names. Variables, constants, and methods should favor the former, and classes and annotations the latter.

## Control Flow

Dictu provides a number of [control structures](https://dictu-lang.com/docs/control-flow/) allowing for robust functionality. These structures should be formatted as follows:

```cs
if () {
    // code
}
```

```cs
while () {
    // code
}
```

```cs
for (...) {
    // code
}
```

## Error Handling

The `Result` type is preferred for error handling and include values to be unwrapped. 2 common patterns have emerged to be the most common. These are preferred and can be referenced below:

```cs
const val = someFunc().match(
    def(result) => result,
    def(error) {
        print(error);
        System.exit(0);        
    }
);
```

```cs
const ret = someFunc();
if (not ret.success()) {
    // do something with the error
    ret.unwrapError();
}
```

## Project Structure

