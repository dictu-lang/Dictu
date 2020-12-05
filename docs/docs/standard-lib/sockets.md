---
layout: default
title: Socket
nav_order: 8
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

```cs
import Socket;
```

### Constants

| Constant             | Description                     |
|----------------------|---------------------------------|
| Socket.errno         | Number of the last error        |
| Socket.AF_INET       | AF_INET protocol family         |
| Socket.SOCK_STREAM   | SOCK_STREAM protocol type       |
| Socket.SOL_SOCKET    | SOL_SOCKET option level         |
| Socket.SO_REUSEADDR  | SO_REUSEADDR allow socket reuse |

### Socket.create(number: family, number: type)

Create a new socket object given a socket type and socket family. This will return
a new socket object in which the rest of the methods are ran on.

```cs
var socket = Socket.create(Socket.AF_INET, Sockket.SOCK_STREAM);
```

### socket.bind(string, number)

This will bind a given socket object to an IP and port number. On failure `nil` is returned
and `errno` is set accordingly.

```cs
if (!socket.bind("host", 10)) {
    print(Socket.strerror());
    // ...
}
```

### socket.listen(number: backlog -> optional)

This will enable connections to be made. The backlog parameter specifies how many
pending connections can be queued before they begin to get rejected. If left unspecified
a reasonable value is chosen. `listen()` will return `nil` on failure and set `errno` accordingly
or `true` on success.

```cs
if (!socket.listen()) {
    print(Socket.strerror());
    // ...
}
```

### socket.accept()

This will accept incoming connections. The socket must be bound to an address an listening for incoming connections before
`.accept()` can be used. `.accept()` returns a list of two values where the first value is a **new** socket object and the second 
is the address connecting to the socket as a string.

```cs
var [client, address] = socket.accept();
print(address); // 127.0.0.1
```

### socket.write(string)

This will write data to the remote client socket. On failure `nil` is returned and `errno` is set accordingly,
otherwise the amount of characters sent is returned.

```cs
if (!socket.write("Some Data")) {
    print(Socket.strerror());
    // ...
}
```

### socket.recv(number)

This will receive data from the client socket. The maximum amount of data to be read at a given
time is specified by the argument passed to `recv()`. On failure `nil` is returned and `errno` is set accordingly,
otherwise a string is returned.

Note: The argument passed to recv should be a relatively small power of 2, such as 2048 or 4096.

```cs
if (!socket.recv(2048)) {
    print(Socket.strerror());
    // ...
}
```

### socket.close()

Closes a socket.

```cs
socket.close();
```

### socket.setsocketopt(number: level, number: option)

Set a socket option on a given socket. On failure `nil` is returned and `errno` is set accordingly,
otherwise a string is returned.

```cs
if (!socket.setsockopt(Socket.SOL_SOCKET, Socket.SO_REUSEADDR)) {
    print(Socket.strerror());
    // ...
}
```

