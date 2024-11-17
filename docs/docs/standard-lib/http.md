---
layout: default
title: HTTP
nav_order: 9
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

To make use of the HTTP module an import is required. Along with the methods described below, this module also defines constants representing all standard response codes and their associated messages, the standard set of HTTP methods, and common request headers and values.

```js
import HTTP;
```

### HTTP.get(String, list: headers -> Optional, Number: timeout -> Optional) -> Result\<Response>

Sends a HTTP GET request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a Response upon success.

```cs
HTTP.get("https://httpbin.org/get");
HTTP.get("https://httpbin.org/get", ["Content-Type: application/json"]);
HTTP.get("https://httpbin.org/get", ["Content-Type: application/json"], 1);

{"content": "...", "headers": ["...", "..."], "statusCode": 200}
```

### HTTP.post(String, dictionary: postArgs -> Optional, list: headers -> Optional, Number: timeout -> Optional) -> Result\<Response>

Sends a HTTP POST request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a Response upon success.

```cs
HTTP.post("https://httpbin.org/post");
HTTP.post("https://httpbin.org/post", {"test": 10});
HTTP.post("https://httpbin.org/post", {"test": 10}, ["Content-Type: application/json"]);
HTTP.post("https://httpbin.org/post", {"test": 10}, ["Content-Type: application/json"], 1);
```

### HTTP.put(String, dictionary: postArgs -> Optional, list: headers -> Optional, Number: timeout -> Optional) -> Result\<Response>

Sends a HTTP POST request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a Response upon success.

```cs
HTTP.put("https://httpbin.org/put");
HTTP.put("https://httpbin.org/put", {"test": 10});
HTTP.put("https://httpbin.org/put", {"test": 10}, ["Content-Type: application/json"]);
HTTP.put("https://httpbin.org/put", {"test": 10}, ["Content-Type: application/json"], 1);
```

### HTTP.head(String, list: headers -> Optional, Number: timeout -> Optional) -> Result\<Response>

Sends a HTTP HEAD request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a Response upon success. 

```cs
HTTP.head("https://httpbin.org/get");
HTTP.head("https://httpbin.org/get", ["Content-Type: application/json"]);
HTTP.head("https://httpbin.org/get", ["Content-Type: application/json"], 1);

{"content": "", "headers": ["...", "..."], "statusCode": 200}
```

### HTTP.options(String, list: headers -> Optional, Number: timeout -> Optional) -> Result\<Response>

Sends a HTTP OPTIONS request to a given URL. Timeout is given in seconds.
Returns a Result and unwraps to a Response upon success. 

```cs
HTTP.head("https://httpbin.org/options");
HTTP.head("https://httpbin.org/options", ["Content-Type: application/json"]);
HTTP.head("https://httpbin.org/options", ["Content-Type: application/json"], 1);

{"content": "", "headers": ["...", "..."], "statusCode": 404}
```

### HTTP.newClient(Dict) -> HttpClient

Creates a new HTTP client with a given set of options.

```cs
const opts = {
    "timeout": 20,
    "headers": [
        "Content-Type: application/json", 
        "Accept: application/json",
        "User-Agent: Dictu"
    ],
    "insecure": false,
    "keyFile": "",
    "certFile": "",
    "keyPasswd": ""
};
var httpClient = HTTP.newClient(opts);
```

### httpClient.get(String) -> Result\<Response>

Sends a HTTP GET request to a given URL.
Returns a Result and unwraps to a Response upon success.

```cs
httpClient.get("https://httpbin.org/get");

{"content": "...", "headers": ["...", "..."], "statusCode": 200}
```

### httpClient.post(String, Dict: postArgs) -> Result\<Response>

Sends a HTTP POST request to a given URL.
Returns a Result and unwraps to a Response upon success.

```cs
httpClient.post("https://httpbin.org/post");
httpClient.post("https://httpbin.org/post", {"test": 10});
```

### httpClient.put(String, Dict: postArgs) -> Result\<Response>

Sends a HTTP PUT request to a given URL.
Returns a Result and unwraps to a Response upon success.

```cs
httpClient.put("https://httpbin.org/put");
httpClient.put("https://httpbin.org/put", {"test": 10});
```

### httpClient.head(String) -> Result\<Response>

Sends a HTTP HEAD request to a given URL.
Returns a Result and unwraps to a Response upon success.

```cs
httpClient.head("https://httpbin.org/get");

{"content": "", "headers": ["...", "..."], "statusCode": 200}
```

```cs
httpClient.options("https://httpbin.org/options");

{"content": "", "headers": ["...", "..."], "statusCode": 404}
```

### Response

All HTTP requests return a Result that unwraps a Response object on success, or nil on error.
The Response object returned has 3 public attributes, "content", "headers" and "statusCode". "content" is the actual content returned from the
HTTP request as a string, "headers" is a list of all the response headers and "statusCode" is a number denoting the status code from
the response

#### Quick Reference Table
##### Attributes

| Attribute  | Description                                       |
| ---------- | ------------------------------------------------- |
| content    | Raw string content returned from the HTTP request |
| headers    | A list of headers returned from the HTTP request  |
| statusCode | The status code returned from the HTTP request    |

##### Methods

| Method | Description                           |
| ------ | ------------------------------------- |
| json   | Convert the content attribute to JSON |

Example response from [httpbin.org](https://httpbin.org)

```json
// GET
const response = HTTP.get("https://httpbin.org/get").unwrap();
print(response.content);
{
  "args": {}, 
  "headers": {
    "Accept": "*/*", 
    "Accept-Encoding": "gzip", 
    "Host": "httpbin.org", 
    "X-Amzn-Trace-Id": "Root=1-620ff6d1-24de015127aa59770abce026"
  }, 
  "origin": "...", 
  "url": "https://httpbin.org/get"
}
print(response.headers);
["HTTP/2 200 ", "date: Fri, 18 Feb 2022 19:43:13 GMT", "content-type: application/json", "content-length: 254", "server: gunicorn/19.9.0", "access-control-allow-origin: *", "access-control-allow-credentials: true"]

print(response.statusCode);
200

// POST
const response = HTTP.post("https://httpbin.org/post").unwrap();
print(response.content);
{
  "args": {}, 
  "data": "", 
  "files": {}, 
  "form": {}, 
  "headers": {
    "Accept": "*/*", 
    "Accept-Encoding": "gzip", 
    "Content-Length": "0", 
    "Content-Type": "application/x-www-form-urlencoded", 
    "Host": "httpbin.org", 
    "X-Amzn-Trace-Id": "Root=1-620ff777-311a6db4398c1f5325a22f8a"
  }, 
  "json": null, 
  "origin": "...", 
  "url": "https://httpbin.org/post"
}

print(response.headers);
["HTTP/2 200 ", "date: Fri, 18 Feb 2022 19:45:59 GMT", "content-type: application/json", "content-length: 404", "server: gunicorn/19.9.0", "access-control-allow-origin: *", "access-control-allow-credentials: true"]

print(response.statusCode);
200
```

#### Response.json() -> Result\<Value>

To quickly convert the raw string contained within the Response object we can use the helper `.json` method.
This works exactly the same as `JSON.parse()` and will return a Result object.
