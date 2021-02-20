---
layout: default
title: Lists
nav_order: 1
parent: Collections
---

# Lists
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Lists

Lists in Dictu allow you to store mutliple values of any type. Lists automatically resize for you as data is added and removed. Lists are similar to arrays in languages like Python, JavaScript, and Ruby.

```cs
var list = [1, 2.3, "Mango", false, nil];
```

### Indexing

Lists are 0-indexed, meaning that 0 is the first item in a list, and 1 is the second item. You can access an item at a specific index using square brackets.

```cs
var list = [1, 2, 3];
print(list[0]); // 1
```

You can use the same syntax to update a value in a list, except you must provide a value.

```cs
var list = [1, 5, 3];
list[1] = 2; // [1, 2, 3]
```

### Slicing

If you want to return only part of a list, you slice it! To slice a list, use square brackets with the range you want to slice. The starting index is inclusive, but the ending index is exclusive. You can also use negative numbers to get characters from the end of a list. Only one index is required.

```cs
[1, 2, 3, 4, 5][1:]; // [2, 3, 4, 5]
[1, 2, 3, 4, 5][:4]; // [1, 2, 3, 4]
[1, 2, 3, 4, 5][1:4]; // [2, 3, 4]
[1, 2, 3, 4, 5][2:4]; // [3, 4]
```

### Adding to lists
#### list.push(value)

To add append a new value to a list, use the `.push()` method.

```cs
var myList = [];
myList.push(10); // [10]
myList.push(11); // [10, 11]
```

#### list.insert(value, number)

To insert a value into a list at a given index without replacing the value use .insert().

```cs
var x = [10, 12];

// Can insert to a list. This will not replace the value.
myList.insert(11, 1); // [10, 11, 12]
```

#### + Operator

Using the + operator on lists will join two lists and return a new list.
It must be used on two values which are both lists.

```cs
var x = [10];

x + [11, 12];
print(x); // [10, 11, 12]
```

#### list.extend(list)

Similar to the + operator however this mutates the list the method is called on rather than returning a new list.

**Note:** values are not copied to the new list, they are just referenced. This means if the value is mutable
it will mutate in the extended list as well.

```cs
var x = [];
x.extend([1, 2, 3]);
print(x); // [1, 2, 3]
x.extend([1, 2, 3]);
print(x); // [1, 2, 3, 1, 2, 3]
```

### list.toString()

Converts a list to a string.

```cs
["1", 11].toString();        // ['1', 11]
["1", [11, "1"]].toString(); // ['1', [11, '1']]
```
### list.len()

Returns the length of the given list.

```cs
[1, 2, 3].len(); // 3
```

### list.toBool()

Converts a list to a boolean. A list is a "truthy" value when it has a length greater than 0.

```cs
[].toBool(); // false
[1].toBool(); // true
[[]].toBool(); // true
```

### list.contains(value)

To check if a value contains within a list we use `.contains()`

```cs
var myList = [1, 2, 3];
myList.contains(2); // true
myList.contains(10); // false
```

### list.join(string: delimiter -> optional)

To convert a list of elements to a string use `.join()` to concatenate elements together by a given delimiter.
If a delimiter is not supplied `", "` is the default. Attempting to join an empty list will return an empty string.

```cs
var myList = [1, 2, 3];
print(myList.join()); // "1, 2, 3"
print(myList.join("")); // "123"
print(myList.join("-")); // "1-2-3"

print([].join("delimiter")); // ""
```

### list.remove(value)

To remove a value from a list use `.remove()`. If the value does not exist within
the list a runtime error occurs. Use together with [list.contains()](#listcontainsvalue).

Note: If a list contains multiple values which are the same, a call to `.remove()` will only
remove the first occurrence, not all.

```cs
var myList = [1, 2, 3];
myList.remove(3);
print(myList); // [1, 2]
myList.remove(1);
print(myList); // [2]
```

### list.pop(number: index -> optional)

To remove a value from a list, with an optional index, use `.pop()`

```cs
var myList = [1, 2, 3];
// If no index is given, pops from the end
var someNumber = myList.pop(); // 3
print(myList); // [1, 2]
myList.pop(0); // 1
print(myList); // [2]
```

### Copying lists
#### list.copy()

When you are working with a mutable datatype taking a reference of a list when creating a new variable will modify the original list.
```cs
var list1 = [1, 2];
var list2 = list1;
list2[0] = 10;
print(list1); // [10, 2]
```
To get around this we can make copies of the list. Dictu offers the ability to both shallow and deep copy a list.
```cs
var list1 = [1, 2];
var list2 = list1.copy(); // shallow copy
list2[0] = 10;
print(list1); // [1, 2]
print(list2); // [10, 2]
```

#### list.deepCopy()
To get around this, we can deepCopy the list.
```cs
var list1 = [[1, 2]];
var list2 = list1.deepCopy();
list2[0][0] = 10;
print(list1); // [[1, 2]]
print(list2); // [[10, 2]]
```

### Sorting Lists
#### list.sort()

To sort numeric lists (that is lists that contain only numbers) you can use the method
sort.

```cs
var list1 = [1, -1, 4, 2, 10, 5, 3];

print(list1); // [1, -1, 4, 2, 10, 5, 3]
list1.sort();

print(list1); // [-1, 1, 2, 3, 4, 5, 10]
```

### list.forEach(func)

To run a function on every element in a list we use `.forEach`. The callback function
passed to `.forEach` expects one parameter which will be the current value.

```cs
const myList = [1, 2, 3, 4, 5];

myList.forEach(def (value) => {
    print("Val: {}".format(value));
});
```

### list.map(func)

Similar to `.foreach`, `.map` will run a function for each element within the list, however
the difference is that `.map` returns a new list of values generated from the callback function.
Map expects a single parameter which is a callback. The callback itself also expects one
parameter which is the current item in the list.

```cs
print([1, 2, 3, 4, 5].map(def (x) => x * 2)); // [2, 4, 6, 8, 10]
```

### list.filter(func)

To filter out values within a list we use `.filter()`. Filter expects a single parameter which is a callback
and if the callback returns a truthy value it will be appended to the list. The callback itself also expects one
parameter which is the current item in the list.

Note: `.filter()` returns a new list.

```cs
print([1, 2, 3, 4, 5].filter(def (x) => x > 2)); // [3, 4, 5]
```

### list.reduce(func, value: initial -> optional)

To reduce a list down to a single value we use `.reduce()`. Reduce expects at least one parameter which is a callback
that will be executed on each item of the list. The value of the callback function is returned and saved for the next
call. The callback itself also expects two parameters which is the accumulated value and the current value in the list.

```cs
print([1, 2, 3, 4, 5].reduce(def (accumulate, element) => accumulate + element)); // 15
```

By default the initial value for `.reduce()` is 0, however we can change this to be any value.

```cs
print(["Dictu ", "is", " great!"].reduce(def (accumulate, element) => accumulate + element, "")); // 'Dictu is great!'
```

### list.find(func)

To find a single item within a list we use `.find()`. Find will search through each item in the list and as soon as the
callback returns a truthy value, the item that satisfied the callback is returned, if none of the items satisfy the callback
function then `nil` is returned.

Note: The first item to satisfy the callback is returned.

```cs
print([1, 2, 3].find(def (item) => item == 2)); // 2
```