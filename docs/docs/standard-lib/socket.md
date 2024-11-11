---
layout: default
title: Socket
nav_order: 21
parent: Standard Library
---

# Socket
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Socket

To make use of the Socket module an import is required.

```js
import Socket;
```

### Constants

| Constant              | Description                             |
| --------------------- | --------------------------------------- |
| Socket.AF_INET        | AF_INET protocol family                 |
| Socket.SOCK_STREAM    | SOCK_STREAM protocol type               |
| Socket.SOCK_DGRAM     | SOCK_DGRAM protocol type                |
| Socket.SOCK_RAW       | SOCK_RAW protocol type                  |
| Socket.SOCK_SEQPACKET | Sequenced, reliable, two-way connection |
| Socket.SOL_SOCKET     | SOL_SOCKET option level                 |
| Socket.SO_REUSEADDR   | SO_REUSEADDR allow socket reuse         |
| Socket.SO_BROADCAST   | Allow sending to dgram sockets          |

### Socket.create(Number: family, Number: type) -> Result\<Socket>

Create a new socket object given a socket type and socket family.
This will return a Result and unwrap to a new socket object in which the rest of the methods are ran on.

```cs
var result = Socket.create(Socket.AF_INET, Socket.SOCK_STREAM);
if (!result.success()) {
    print(result.unwrapError());
    // ...
}

var socket = result.unwrap();
```

### socket.bind(String, Number) -> Result\<Nil>

This will bind a given socket object to an IP and port number.
Returns a Result type and on success will unwrap to nil.

```cs
var result = socket.bind("127.0.0.1", 1000);
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

### socket.connect(String, Number) -> Result\<Nil>

This will connect to a socket on a given host and IP.
Returns a Result type and on success will unwrap to nil.

```cs
var result = socket.connect("127.0.0.1", 1000);
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

### socket.listen(Number: backlog -> Optional) -> Result\<Nil>

This will enable connections to be made. The backlog parameter specifies how many
pending connections can be queued before they begin to get rejected. If left unspecified
a reasonable value is chosen. 
Returns a Result type and on success will unwrap to nil.

```cs
var result = socket.listen();
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

### socket.accept() -> Result\<List>

This will accept incoming connections. The socket must be bound to an address an listening for incoming connections before
`.accept()` can be used.
Returns a Result type that wraps a list of two values where the first value is a **new** socket object and the second 
is the address connecting to the socket as a string.

```cs
var [client, address] = socket.accept().unwrap();
print(address); // 127.0.0.1
```

### socket.write(String) -> Result\<Number>

This will write data to the remote client socket.
Returns a Result type and on success will unwrap to a number (amount of chars written).

```cs
var result = socket.write("Some Data");
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

### socket.recv(Number) -> Result\<String>

This will receive data from the client socket. The maximum amount of data to be read at a given
time is specified by the argument passed to `recv()`. 
Returns a Result type and on success will unwrap to a string.

Note: The argument passed to recv should be a relatively small power of 2, such as 2048 or 4096.

```cs
var result = socket.recv(2048);
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

### socket.close()

Closes a socket.

```cs
socket.close();
```

### socket.setsocketopt(Number: level, Number: option) -> Result

Set a socket option on a given socket.
Returns a Result type and on success will unwrap to nil.

```cs
var result = socket.setsockopt(Socket.SOL_SOCKET, Socket.SO_REUSEADDR);
if (!result.success()) {
    print(result.unwrapError());
    // ...
}
```

