/*-
 * Copyright (c) 2012 Oleksandr Tymoshenko <gonzo@bluezbox.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <stdint.h>

#include "mobi.h"
#include "pdb.h"

static void
usage(void)
{
	printf("Usage: readmobi file.mobi\n");
}

int
main(int argc, const char *argv[])
{
	int fd;
	void *ptr;
	unsigned char *mobi_data;
	off_t file_size;
	off_t file_pos = 0;
	off_t bytes_read;
	pdb_header_t *pdb_header;
	mobi_header_t *mobi_header;

    if (argc < 2) {
		usage();
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	file_size = lseek(fd, 0, SEEK_END);
	ptr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	mobi_data = (unsigned char*)ptr;

	pdb_header = pdb_header_alloc();

	bytes_read = pdb_header_read(pdb_header, mobi_data, file_size);
	if (bytes_read < 0) {
		fprintf(stderr, "pdb_header_read failed\n");
		exit(0);
	}
	pdb_header_print(pdb_header, 0);

    file_size -= bytes_read;
    file_pos += bytes_read;
    
	mobi_header = mobi_header_alloc();
	bytes_read = mobi_header_read(mobi_header, (mobi_data + file_pos), file_size);
	if (bytes_read < 0) {
		fprintf(stderr, "mobi_header_read failed\n");
		exit(0);
	}
	mobi_header_print(mobi_header);

    file_size -= bytes_read;
    file_pos += bytes_read;

	munmap(ptr, file_size);

    return 0;
}
