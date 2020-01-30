---
layout: default
title: Getting Started
nav_order: 2
---

# Getting Started
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Hello World!

We saw on the last page how to open the REPL and execute code on there, now lets create a Dictu file
and execute that. For this step, i'm going to create an empty file via the command line, this
is to leave it "editor agnostic".

```bash
$ touch new_dictu.du
```
At this point, i'm going to assume you have opened `new_dictu.du` in whichever editor you use.

```js
//new_dictu.du
print("Hello World!");
```

And now lets execute our file.

```bash
$ ./dictu new_dictu.du
Hello World!
```

As you see, Hello World! is printed to the screen. You have just created your first Dictu
program, now onto bigger and better things!

## REPL

The REPL is a language shell, allowing you to run code on the fly via your terminal. This is very useful for debugging, or small little checks you may want to perform. When you enter the REPL, there is a magic variable `_` which stores the last returned value (nil excluded).

```
Dictu Version: {{ site.version }}
>>> var someNumber = 10;
>>> 5 + someNumber;
15
>>> _;
15
```

## Comments

Comments are an important part of any language, and here in Dictu, this is no exception. Comment syntax should be very familiar to you as it's the same in a variety of languages.

```js
// This is a single line comment

/*
this
is
a
multiline
comment
*/
```

## Argv

Passing arguments to the script via the CLI is very important, especially in a headless environment. Dictu allows you to access these arguments very easily with the magic `argv` variable. `argv` is a list of all arguments supplied to the script. This will be more apparent with the following examples. Note - the first element of the argv list is always the script name.

### CLI

`./dictu myFile.du`

```js
# myFile.du

print(argv); // ["myDile.du"]
```

`./dictu myFile.du arg1 arg2 arg3`

```js
# myFile.du

print(argv); // ["myDile.du", "arg1", "arg2", "arg3"]
```

{: .no_toc }