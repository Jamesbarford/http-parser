# HTTP Request/Response Parser

An implementation for parsing an http request/response. Internally the parser does not allocate memory from the heap. The aim is for a simple function call requiring as little setup as possible.

## HTTP Request

The primary `struct` `http_request_t` has the following fields:

- http_method: `GET`, `HEAD`, `POST`, `PUT`, `PATCH`, `CONNECT`, `DELETE`, `TRACE`, `OPTIONS`
- path: the requested path i.e `/name-of/resource`
- http_version: `"2"`, `"1.1"`, `"1"`
- headers: An array of headers in key value pairings;

### Example:

`main.c` can accept a file with an http header and will attempt the use the `http_parser` to parse the headers as a demonstration

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "http_parser.h"

void some_function(void) {
  char buf[BUFSIZ];
  int socket_fd, bytes;
  http_request_t req;

  while ((bytes = read(socket_fd, buf, BUFSIZ)) > 0);

  if (bytes == 0) {
		// handle error
    return;
  }

  parse_request(buf, &req);

  printf("%s %s HTTP/%s\n", req.http_method, req.path, req.http_version);
	
  for (int i = 0; i < req.num_headers; ++i) {
    printf("%s: %s\n", req.headers[i].key, req.headers[i].value);
  }
}
```

## Methods

```c
void parse_request(char *request_raw, http_request_t *req);
```
- `buf` a character array used internally
- `req_raw` the raw response
- `req` a `http_response_t`, which on success will be populated explicity with the HTTP Version, http Method and the path.

## HTTP Response

A very simliar the request parser.

The primary struct `http_response_t` consists of the following fields:

- `http_version` - `"1"`, `"1.1"`, `"2"`
- `status_code` char * i.e `"200"`, `"400"`.
- `status_text` char * i.e "OK", "Bad Request"

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "http_parser.h"

void some_function(void) {
  char buf[BUFSIZ];
  int socket_fd, bytes;
  http_response_t res;

  while ((bytes = read(socket_fd, buf, BUFSIZ)) > 0);

  if (bytes == 0) {
		// handle error
  return;
  }

  parse_response(buf, &res);

  printf("HTTP/%s %s %s\n", res.http_version, res.status_code, res.status_text);
	
  for (int i = 0; i < res.num_headers; ++i) {
    printf("%s: %s\n", res.headers[i].key, res.headers[i].value);
  }
}
```
## Methods

```c
void parse_response(char *resonse_raw, http_response_t *res);
```
- `buf` a character array used internally
- `req_raw` the raw response
- `req` a `http_response_t`, which on success will be populated explicitly with the HTTP version, Status code and Status Text

### Utils

As a convenience for getting a header, will return a pointer to a header or `NULL`.

```c
headers_kv_t *find_header(headers_kv_t *headers, int num_headers, char *key);
```
