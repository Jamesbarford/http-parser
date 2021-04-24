#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "http_parser.h"

enum HTTP_KV { KEY, VALUE };

static int parse_headers(char *buf, int offset, char *req_raw, headers_kv_t *headers);

static int __parse_response(char *res_raw, http_response_t *res) {
	int offset = 0, prev_pos = 0;

	// Walk to HTTP/2 <- to get the number
	while (!isdigit(res_raw[offset++]));
	res->http_version[0] = res_raw[offset-1];
	// Check for minor
	if (res_raw[offset] == '.') {
		res->http_version[1] = '.';
		res->http_version[2] = res_raw[++offset];
	}

	// Walk to status code
	while (!isdigit(res_raw[offset++]));
	res->status_code[0] = res_raw[++offset];
	res->status_code[1] = res_raw[++offset];
	res->status_code[2] = res_raw[++offset];
	res->status_code[3] = '\0';

	// Walk to next space
	while (res_raw[offset++] != ' ');
	prev_pos = offset;
	// We've got to the end of the response line
	while (res_raw[offset++] != '\n');

	res_raw[offset - 1] = '\0';
	res->status_text = &res_raw[prev_pos];

	return offset;
}

/**
 * Parse http method, route and version
 * return offset so we don't parse the same line again
 *
 * -1 denotes a failure
 * a positive integer is success
 */
static int __parse_request(char *req_raw, http_request_t *req) {
	int offset = 0, j = 0, prev_pos = 0;

	// GET <- walk to this piece of white space
	while (req_raw[offset++] != ' ');
	req_raw[offset - 1] = '\0';
	req->http_method = &req_raw[prev_pos];
	prev_pos = offset;

	// /name-of/path <- walk to this piece of white space
	while(req_raw[offset++] != ' ');
	req_raw[offset - 1] = '\0';
	req->path = &req_raw[prev_pos];
	prev_pos = offset;

	// HTTP/1.1 <- walk to first number
	while (!isdigit(req_raw[offset++]));
	req->http_version[0] = req_raw[offset - 1];
	// Check for minor
	if (req_raw[offset] == '.') {
		req->http_version[1] = '.';
		req->http_version[2] = req_raw[++offset];
	}

	if (req_raw[offset] == '\n')
		return offset + 1;
	return offset;
}

/**
 * HTTP Key: Value 
 *
 * Assigns a pointer to each key and value. Effectively Chopping a buffer
 * into pieces by removing ':' and '\n' placing a '\0' null terminator.
 *
 * Easy to free as you just have to free the buffer that was passed in, if it was malloced.
 *
 * We track two pointers, one to the buffer's position and one to the raw request headers.
 *
 * i.e:
 * |H|o|s|t|:| |d|e|v|e|l|o|p|e|r|\n
 *  ^       ^   ^                  ^
 *  p1      \0  p2                \0
 *
 * Where p1 (a pointer) the address of the key, and p2 (a pointer) is the addess of the value.
 *
 * @buf - temporary buffer to of which pointers to various parts of the buffer will be assigned for the key
 * and for the value;
 * @req_raw raw request
 * @req pointer to the http_request_t object, which is where the key->values will be stored
 */
static int parse_headers(char *buf, int offset, char *req_raw, headers_kv_t *headers) {
	/**
	 * We don't want to parse the first line of the request again as that is:
	 * GET /path/of/thing HTTP/2
	 *
	 * So get the offset and start from there
	 */
	
	char *hdr_ptr = &req_raw[offset-1];
	int num_headers = 0;
	enum HTTP_KV part = KEY;
	char *ptr;
	ptr = buf;
	int prev_pos = 0;
	int buf_pos = 0;
	int new_line_count = 0;

	for (;;) {	
		hdr_ptr++;
		buf[buf_pos++] = *hdr_ptr;

		switch (*hdr_ptr) {
			case '\0':
			case EOF:
				goto done;

			case '\n':
			case '\r': {
				new_line_count++;
				buf[buf_pos - 1] = '\0';

				if (new_line_count > 2) {
					headers[num_headers-1].value = ptr;
					headers[num_headers-1].key = "body";
					goto done;
				}

				part = KEY;
				ptr = &buf[prev_pos];

				headers[num_headers].value = ptr;
				headers[num_headers].value_len = strlen(ptr);
				ptr = &buf[buf_pos];
				num_headers++;
				break;
			}

			case ':': {
				new_line_count = 0;
				if (part == KEY) {
					buf[buf_pos - 1] = '\0';

					headers[num_headers].key = ptr;
					headers[num_headers].key_len = strlen(ptr);

					part = VALUE;
					prev_pos = buf_pos + 1;
				}
				break;
			}

			default:
				continue;
		}
	}

done:
	return num_headers;
}

headers_kv_t *find_header(http_request_t *req, char *key) {
	for (int i = 0; i < req->num_headers; ++i) {
		if (strcmp(req->headers[i].key, key) == 0)
			return &req->headers[i];	
	}
	return NULL;
}

void parse_request(char *buf,  char *req_raw, http_request_t *req) {
	int offset = __parse_request(req_raw, req);
	int num_headers = parse_headers(buf, offset, req_raw, req->headers);
	req->num_headers = num_headers;
}

void parse_response(char *buf, char *res_raw, http_response_t *res) {
	int offset = __parse_response(res_raw, res);
	int num_headers = parse_headers(buf, offset, res_raw, res->headers);
	res->num_headers = num_headers;
}

