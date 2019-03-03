---
layout: default
title: Home
nav_order: 1
description: "Just the Docs is a responsive Jekyll theme with built-in search that is easily customizable and hosted on GitHub Pages."
permalink: /
---

# Dictu - {{ site.version }}

### What is Dictu?

Dictu is a very small, and simple dynamically typed programming language originally stemming from
the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html). Since then Dictu
builds upon the concepts within the book, adding more features to the language.

_Dictu stands for simplistic in Latin. This is the aim of the language, to be as simplistic
as humanely possible._

### About the project

Dictu is a small language created to learn more about how programming languages are created
and learn how features used everyday within other languages are implemented to gain a better
understanding.

{: .fs-6 .fw-300 }

---

## Installing Dictu

```bash
$ git clone https://github.com/Jason2605/Dictu.git
$ cd Dictu
$ make dictu
```

Lets ensure everything went as planned by opening the Dictu REPL.

```bash
$ ./dictu
```
You should then be able to run dictu code in here!
```
Dictu Version: {{ site.version }}
>>> print("Hello World!");
```
When you're done playing around with the REPL, use CTRL + C to close it.

### Contributing

I want to make the Dictu environment as friendly, and as welcoming as possible so [pull requests](https://github.com/Jason2605/Dictu/pulls)
are massively encouraged, be that the tiniest typo error in some documentation, or a new feature
within the language itself, both will be massively appreciated! However pull requests don't have to be
the only contribution to the growth of Dictu, you could, [open issues](https://github.com/Jason2605/Dictu/issues), star the repo, or even tell people that Dictu is great!

### License

Dictu is licensed under the [MIT license](https://github.com/Jason2605/Dictu/blob/master/LICENSE).
