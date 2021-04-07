---
layout: default
title: HTTP
nav_order: 7
parent: Standard Library
---

# HTTP
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## HTTP

To make use of the HTTP module an import is required.

```cs
import HTTP;
```

### HTTP.get(string, list: headers -> optional, number: timeout -> optional)

Sends a HTTP GET request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a dictionary upon success.

```cs
HTTP.get("https://httpbin.org/get");
HTTP.get("https://httpbin.org/get", ["Content-Type: application/json"]);
HTTP.get("https://httpbin.org/get", ["Content-Type: application/json"], 1);

{"content": "...", "headers": ["...", "..."], "statusCode": 200}
```

### HTTP.post(string, dictionary: postArgs -> optional, list: headers -> optional, number: timeout -> optional)

Sends a HTTP POST request to a given URL.Timeout is given in seconds.
Returns a Result and unwraps to a dictionary upon success.

```cs
HTTP.post("https://httpbin.org/post");
HTTP.post("https://httpbin.org/post", {"test": 10});
HTTP.post("https://httpbin.org/post", {"test": 10}, ["Content-Type: application/json"]);
HTTP.post("https://httpbin.org/post", {"test": 10}, ["Content-Type: application/json"], 1);
```

### Response

Both HTTP.get() and HTTP.post() return a Result that unwraps a dictionary on success, or nil on error.
The dictionary returned has 3 keys, "content", "headers" and "statusCode". "content" is the actual content returned from the
HTTP request as a string, "headers" is a list of all the response headers and "statusCode" is a number denoting the status code from
the response

Example response from [httpbin.org](https://httpbin.org)

```json
// GET

{"content": '{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Host": "httpbin.org",
    "X-Amzn-Trace-Id": "Root=1-5e58197f-21f34d683a951fc741f169c6"
  },
  "origin": "...",
  "url": "https://httpbin.org/get"
}
', "headers": ['HTTP/1.1 200 OK', 'Date: Thu, 27 Feb 2020 19:33:19 GMT', 'Content-Type: application/json', 'Content-Length: 220', 'Connection: keep-alive', 'Server: gunicorn/19.9.0', 'Access-Control-Allow-Origin: *', 'Access-Control-Allow-Credentials: true'], "statusCode": 200}


// POST

{"content": '{
  "args": {},
  "data": "",
  "files": {},
  "form": {
    "test": "10"
  },
  "headers": {
    "Accept": "*/*",
    "Content-Length": "8",
    "Content-Type": "application/x-www-form-urlencoded",
    "Host": "httpbin.org",
    "X-Amzn-Trace-Id": "Root=1-5e5819ac-7e6a3cef0546c7606a34aa73"
  },
  "json": null,
  "origin": "...",
  "url": "https://httpbin.org/post"
}
', "headers": ['HTTP/1.1 200 OK', 'Date: Thu, 27 Feb 2020 19:34:04 GMT', 'Content-Type: application/json', 'Content-Length: 390', 'Connection: keep-alive', 'Server: gunicorn/19.9.0', 'Access-Control-Allow-Origin: *', 'Access-Control-Allow-Credentials: true'], "statusCode": 200}
```