#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "http_parser.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Test for http_parser\nUsage: %s <http_header_file> \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	struct stat sb;
	http_request_t req;
	//http_response_t res;

	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror("Failed to open file");
		goto failed;
	}

	if (fstat(fd, &sb) == -1) {
		perror("Failed to stat file\n");
		goto failed;
	}
	
	char *example = mmap(NULL, sb.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (example == MAP_FAILED) {
		perror("Failed to mmap file\n");
		goto failed;
	}

	//parse_response(buf, example, &res);
//	printf("HTTP/%s-%s-%s\n", res.http_version, res.status_code, res.status_text);
//	for (int i = 0; i < res.num_headers; i++) {
//		printf("%s: %s\n", res.headers[i].key, res.headers[i].value);
//	}

	parse_request(example, &req);
	printf("%s %s HTTP/%s\n",
			req.http_method,
			req.path,
			req.http_version
		);

	for (int i = 0; i < req.num_headers; i++) {
		printf("%s: %s\n", req.headers[i].key, req.headers[i].value);
	}

	(void)close(fd);
	(void)munmap(example, sb.st_size); 

	return EXIT_SUCCESS;

failed:
	(void)close(fd);
	exit(EXIT_FAILURE);
}	

