#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TOKEN_SIZE 128

bool try_get_label(FILE* f, char* buf, size_t bufsiz, int* written) {
	int c = fgetc(f);
	if (c != '$') {
		ungetc(c, f);
		return false;
	}

	*written = 0;
	while(*written < bufsiz - 1) {
		c = fgetc(f);
		if (c == EOF || isspace(c)) return *written != 0;

		buf[*written] = c;
		*written += 1;
	}
	fprintf(stderr, "FATAL: Token was too big (more than %lu characters)\n", bufsiz);
	exit(1);
}

bool try_get_hex(FILE* f, char* buf, size_t bufsiz, int* written) {
	int c = fgetc(f);
	ungetc(c, f);
	if (!isxdigit(c)) {
		return false;
	}

	*written = 0;
	while(*written < bufsiz - 1) {
		c = fgetc(f);
		if (c == EOF || isspace(c)) return *written != 0;

		if (!isxdigit(c)) {
			fprintf(stderr, "FATAL: Numeric token had non-hex digits\n");
			exit(1);
		}

		buf[*written] = c;
		*written += 1;
	}
	fprintf(stderr, "FATAL: Token was too big (more than %lu characters)\n", bufsiz);
	exit(1);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: %s <infile> <outfile>\n", argv[0]);
	}
	const char* from_filename = argv[1];
	const char* to_filename = argv[2];
	FILE* from = fopen(from_filename, "r");
	FILE* to = fopen(to_filename, "w");

	char token[MAX_TOKEN_SIZE];
	int len;
	do {
		if (!try_get_label(from, token, MAX_TOKEN_SIZE, &len)) {
			if (feof(from)) break;
			fprintf(stderr, "FATAL: Expected label but got something else\n");
			exit(1);
		}
		fprintf(to, "uint8_t %*s[] = {\n", len, token);

		const int bytes_per_line = 16;
		int bytes_current_line = 0;
		bool is_first = true;
		fputc('\t', to);
		while (try_get_hex(from, token, MAX_TOKEN_SIZE, &len)) {
			if (len != 16 && len != 8 && len != 4 && len != 2) {
				fprintf(stderr, "FATAL: Unexpected word size (%d bits)\n", len * 4);
				exit(1);
			}

			for (int i = 0; i < len;) {
				if (bytes_per_line <= bytes_current_line) {
					fputc(',', to);
					fputc('\n', to);
					fputc('\t', to);
					bytes_current_line = 0;
				} else if (bytes_current_line != 0) {
					fputc(',', to);
					fputc(' ', to);
				}
				fputc('0', to);
				fputc('x', to);
				fputc(token[i++], to);
				fputc(token[i++], to);
				bytes_current_line++;
			}
		}
		fprintf(to, "\n};\n");
	} while(1);
	return 0;
}
