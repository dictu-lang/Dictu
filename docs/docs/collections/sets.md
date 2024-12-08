---
layout: default
title: Sets
nav_order: 3
parent: Collections
---

# Sets
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Sets

Sets are an unordered collection of unique hashable values. Set values must be of type string, number, boolean or nil.

```cs
var mySet = set("test", 10);
print(mySet); // {10, "test"}
```

### set.toString() -> String

Converts a given set to a string.

```cs
var set_a = set();

set_a.add("one");
set_a.add("two");

var set_b = set();
set_b.add(1);
set_b.add(2);

set_a.toString(); // '{"two", "one"}');
set_b.toString(); // '{2, 1}'
```

### set.toBool() -> Boolean

Converts a set to a boolean. A set is a "truthy" value when it has a length greater than 0.

```cs
var x = set();

x.toBool(); // false
x.add("test");
x.toBool(); // true
```

### set.len() -> Number

Returns the length of the given set.

```cs
var mySet = set();
mySet.add("Dictu!");
mySet.len(); // 1
```

### set.add(Value)

Adding to sets is just a case of passing a value to .add()

```cs
var mySet = set();
mySet.add("Dictu!");
```

### set.values() -> List

Returns a list of all of the values in the set.

```cs
var mySet = set("foo", "bar", 123);
const values = mySet.values(); // ["foo", "bar", 123]
```

### set.contains(Value) -> Boolean

To check if a set contains a value use `.contains()`

```cs
var mySet = set();
mySet.add("Dictu!");
print(mySet.contains("Dictu!")); // true
print(mySet.contains("Other!")); // false
```

### set.containsAll(Value) -> Boolean

To check if a set contains all elements in a given list use `.containsAll()`

```cs
var mySet = set("one",1,2,3);;
print(mySet.containsAll(["one",1])); // true
print(mySet.containsAll([1,2,3])); // true
print(mySet.containsAll(["one",1,2,3,"x"])); // false
```

### set.remove(Value)

To remove a value from a set use `.remove()`.

**Note**: If you try to remove a value that does not exist a runtime error is raised, use together with `.contains()`.

```cs
var mySet = set();
mySet.add("Dictu!");
mySet.remove("Dictu!");
```
