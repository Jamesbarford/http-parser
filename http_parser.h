#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define MAX_HEADERS 10

typedef struct headers_kv_t {
	char *key;
	unsigned int key_len;
	char *value;
	unsigned int value_len;
} headers_kv_t;

typedef struct http_request_t {
	char *http_method;
	char *path;
	// http "1", "1.1", "2", "3"
	char http_version[3];
	int num_headers;
	headers_kv_t headers[MAX_HEADERS];
} http_request_t;

typedef struct http_response_t {
	char status_code[4];
	char *status_text;
	char http_version[3];
	int num_headers;
	headers_kv_t headers[MAX_HEADERS];
} http_response_t;

void parse_request(char *req_raw, http_request_t *req);
void parse_response(char *res_raw, http_response_t *res);
headers_kv_t *find_header(headers_kv_t *headers, int num_headers, char *key);

#endif
