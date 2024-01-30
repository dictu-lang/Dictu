---
layout: default
title: Buffer
nav_order: 4
parent: Standard Library
---

# Buffer
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Buffer
To make use of the Buffer module an import is required.

```js
import Buffer;
```

The maximum byte size for a buffer is `2147483647 - 1`

### Buffer.new() -> Result\<Buffer>

Returns a Result with a new buffer with the given size in bytes.

```cs
const buffer = Buffer.new(1024).unwrap();
print(buffer);
// <Buffer>
```

### Buffer.fromString(String) -> Result\<Buffer>

Returns a Result with a new buffer created from the given string.

```cs
const buffer = Buffer.fromString("Dictu!").unwrap();
print(buffer);
// <Buffer>
```

### Buffer.resize(Number) -> Result\<Number>

Resizes the buffer to the given size. The argument needs to be greater than 0 or the function will return an error.

```cs
const buffer = Buffer.new(8).unwrap();
print(buffer.resize(16).unwrap());
// 16
```

### Buffer.get(Number) -> Result\<Number>

Returns the value of byte at the given index.
```cs
const buffer = Buffer.new("Dictu!").unwrap();
print(buffer.get(0).unwrap());

// 68 (ASCII value of 'D')
```

### Buffer.set(Number, Number) -> Result\<Number>

Sets the given index of the buffer to the second argument.
Returns a Number result with the new value.
```cs
const buffer = Buffer.fromString("dictu!").unwrap();
buffer.set(0, 68).unwrap();
print(buffer.string()); // "Dictu!"
```

### Buffer.subarray(Number: start -> Optional, Number: end -> Optional) -> Result\<Buffer>

Returns a new Buffer with the optional given start and end parameters.
* `start`: The start index within the buffer, default `0`
* `end`: The end index within the buffer(non inclusive), default: `buffer.len()`
```cs
const buffer = Buffer.fromString("Dictu!").unwrap();
const sub = buffer.subarray(0, buffer.len()-1).unwrap();
print(sub.string()); // "Dictu"
```

### Buffer.string() -> String

Returns a string representation of the buffer.
```cs
const buffer = Buffer.fromString("Dictu!").unwrap();
const str = buffer.string();
print(str); // "Dictu!"
```

### Buffer.len() -> Number

Returns the byte length of the buffer.
```cs
const buffer = Buffer.new(9).unwrap();
const len = buffer.len();
print(len); // 9
```

### Buffer.values() -> List

Returns a list with the integer values of the buffer.
```cs
const buffer = Buffer.fromString("Dictu!").unwrap();
const v = buffer.values();
print(v); // [68, 105, 99, 116, 117, 33]
```


### Buffer.writeString(Number, String) -> Result\<Nil>

Sets a string into buffer given the starting index. If the string doesn't fit in the buffer an error is returned.
```cs
const buffer = Buffer.new(6).unwrap();
buffer.writeString(0, "Dictu!");
```

### Buffer.readString(Number: start -> Optional, Number: end -> Optional) -> Result\<String>

Returns a String with the optional given start and end parameters, this works very similar to subarray.
* `start`: The start index within the buffer, default `0`
* `end`: The end index within the buffer(non inclusive), default: `buffer.len()`
```cs
const buffer = Buffer.new(6).unwrap();
buffer.writeString(0, "Dictu!");
const sub = buffer.string(0, buffer.len()-1).unwrap();
print(sub) // "Dictu"
```

### Buffer.readUInt64LE(Number) -> Result\<Number>

Returns the u64(unsigned 8 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.
**Note**: The maximum value supported is: `9007199254740992`, if a read of a larger value is attempted a error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeUInt64LE(0, 12000);
print(buffer.readUInt64LE(0).unwrap()) // 12000
```

### Buffer.readUInt32LE(Number) -> Result\<Number>

Returns the u32(unsigned 4 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeUInt32LE(0, 1337);
print(buffer.readUInt32LE(0).unwrap()) // 1337
```

### Buffer.readUInt16LE(Number) -> Result\<Number>

Returns the u16(unsigned 2 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeUInt16LE(0, 1337);
print(buffer.readUInt16LE(0).unwrap()) // 1337
```

### Buffer.readInt64LE(Number) -> Result\<Number>

Returns the i64(signed 8 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeInt64LE(0, -12000);
print(buffer.readInt64LE(0).unwrap()) // -12000
```

### Buffer.readInt32LE(Number) -> Result\<Number>

Returns the i32(signed 4 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeInt32LE(0, -1337);
print(buffer.readInt32LE(0).unwrap()) // -1337
```

### Buffer.readInt16LE(Number) -> Result\<Number>

Returns the i16(signed 2 byte integer in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeInt16LE(0, -1337);
print(buffer.readInt16LE(0).unwrap()) // -1337
```

### Buffer.readInt8(Number) -> Result\<Number>

Returns the i8(signed 1 byte integer) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.
**Note**: For the unsigned equivalent use get().

```cs
const buffer = Buffer.new(1).unwrap();
buffer.writeUInt8(0, -12);
print(buffer.readInt8(0).unwrap()) // -12
```

### Buffer.writeUInt64LE(Number, Number) -> Result\<Number>

Writes a u64(unsigned 8 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeUInt64LE(0, 12000);
```

### Buffer.writeUInt32LE(Number, Number) -> Result\<Number>

Writes a u32(unsigned 4 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeUInt32LE(0, 1337);
```

### Buffer.writeUInt16LE(Number, Number) -> Result\<Number>

Writes a u16(unsigned 2 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeUInt16LE(0, 1337);
```

### Buffer.writeInt64LE(Number, Number) -> Result\<Number>

Writes a i64(signed 8 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeInt64LE(0, 12000);
```

### Buffer.writeInt32LE(Number, Number) -> Result\<Number>

Writes a i32(signed 4 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeInt32LE(0, 1337);
```

### Buffer.writeInt16LE(Number, Number) -> Result\<Number>

Writes a i16(signed 2 byte integer in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeInt16LE(0, 1337);
```

### Buffer.writeInt8(Number, Number) -> Result\<Number>

Writes a i8(signed 1 byte integer) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
**Note**: For the unsigned equivalent use set().

```cs
const buffer = Buffer.new(1).unwrap();
buffer.writeInt8(0, -12);
```

### Buffer.writeFloatLE(Number, Number) -> Result\<Number>

Writes a float(4 byte signed floating point number in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeFloatLE(0, 14.34);
```

### Buffer.writeDoubleLE(Number, Number) -> Result\<Number>

Writes a double(8 byte signed floating point number in little endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeDoubleLE(0, 14.34);
```

### Buffer.readFloatLE(Number) -> Result\<Number>

Returns the float(signed 4 byte floating point number in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeFloatLE(0, 14.34);
print(buffer.readFloatLE(0).unwrap()) // 14.34
```

### Buffer.readDoubleLE(Number) -> Result\<Number>

Returns the double(signed 8 byte floating point number in little endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeDoubleLE(0, 14.34);
print(buffer.readDoubleLE(0).unwrap()) // 14.34
```

### Buffer.writeUInt64BE(Number, Number) -> Result\<Number>

Writes a u64(unsigned 8 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeUInt64BE(0, 12000);
```

### Buffer.writeUInt32BE(Number, Number) -> Result\<Number>

Writes a u32(unsigned 4 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeUInt32BE(0, 1337);
```

### Buffer.writeUInt16BE(Number, Number) -> Result\<Number>

Writes a u16(unsigned 2 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeUInt16BE(0, 1337);
```

### Buffer.writeInt64BE(Number, Number) -> Result\<Number>

Writes a i64(signed 8 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeInt64BE(0, 12000);
```

### Buffer.writeInt32BE(Number, Number) -> Result\<Number>

Writes a i32(signed 4 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeInt32BE(0, 1337);
```

### Buffer.writeInt16BE(Number, Number) -> Result\<Number>

Writes a i16(signed 2 byte integer in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeInt16BE(0, 1337);
```

### Buffer.writeFloatBE(Number, Number) -> Result\<Number>

Writes a float(4 byte signed floating point number in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeFloatBE(0, 14.34);
```

### Buffer.writeDoubleBE(Number, Number) -> Result\<Number>

Writes a double(8 byte signed floating point number in big endian) at the index(the first argument).
Returns a result with the set value or an error incase the byte size from the start index would exceed the buffer bounds.
```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeDoubleBE(0, 14.34);
```

### Buffer.readUInt64BE(Number) -> Result\<Number>

Returns the u64(unsigned 8 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.
**Note**: The maximum value supported is: `9007199254740992`, if a read of a larger value is attempted a error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeUInt64BE(0, 12000);
print(buffer.readUInt64BE(0).unwrap()) // 12000
```

### Buffer.readUInt32BE(Number) -> Result\<Number>

Returns the u32(unsigned 4 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeUInt32BE(0, 1337);
print(buffer.readUInt32BE(0).unwrap()) // 1337
```

### Buffer.readUInt16BE(Number) -> Result\<Number>

Returns the u16(unsigned 2 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeUInt16BE(0, 1337);
print(buffer.readUInt16BE(0).unwrap()) // 1337
```

### Buffer.readInt64BE(Number) -> Result\<Number>

Returns the i64(signed 8 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeInt64BE(0, -12000);
print(buffer.readInt64BE(0).unwrap()) // -12000
```

### Buffer.readInt32BE(Number) -> Result\<Number>

Returns the i32(signed 4 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeInt32BE(0, -1337);
print(buffer.readInt32BE(0).unwrap()) // -1337
```

### Buffer.readInt16BE(Number) -> Result\<Number>

Returns the i16(signed 2 byte integer in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(2).unwrap();
buffer.writeInt16BE(0, -1337);
print(buffer.readInt16BE(0).unwrap()) // -1337
```

### Buffer.readFloatBE(Number) -> Result\<Number>

Returns the float(signed 4 byte floating point number in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(4).unwrap();
buffer.writeFloatBE(0, 14.34);
print(buffer.readFloatBE(0).unwrap()) // 14.34
```

### Buffer.readDoubleBE(Number) -> Result\<Number>

Returns the double(signed 8 byte floating point number in big endian) value given the starting index.
If the given index + byte length does exceed the buffer bounds an error is returned.

```cs
const buffer = Buffer.new(8).unwrap();
buffer.writeDoubleBE(0, 14.34);
print(buffer.readDoubleBE(0).unwrap()) // 14.34
```