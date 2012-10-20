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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mobi.h"
#include "bytestream.h"

#define MIN_MOBI_HEADER_SIZE (16 + 0xf8)

mobi_header_t*
mobi_header_alloc()
{
	mobi_header_t *h = malloc(sizeof(mobi_header_t));
	memset(h, 0, sizeof(*h));
	return h;
}

void
mobi_header_free(mobi_header_t* header)
{
	free(header);
}

void
mobi_header_print(mobi_header_t* h)
{
    printf("MOBI header\n");
    printf("  Compression: %d", h->mobi_compression);
    switch (h->mobi_compression) {
        case 1:
            printf(" (no compression)");
            break;
        case 2:
            printf(" (PalmDOC compression)");
            break;
        case 17480:
            printf(" (HUFF/CDC compression)");
            break;
        default:
            printf(" (Unknown compression)");
            break;
    }
    printf("\n");
    printf("  Uncompressed text length: %d\n", h->mobi_text_length);
    printf("  Record count: %d\n", h->mobi_record_count);
    printf("  Record size: %d\n", h->mobi_record_size);
    printf("  Encryptin type: %d\n", h->mobi_encryption_type);
}

off_t
mobi_header_read(mobi_header_t* h, unsigned char *ptr, off_t size)
{
	unsigned char *orig_ptr = ptr;

	if (size < MIN_MOBI_HEADER_SIZE)
		return (-1);

	h->mobi_compression = bs_read_2(ptr);
	ptr += 2;

    /* Skip 2 unused bytes */
	ptr += 2;

	h->mobi_text_length = bs_read_4(ptr);
	ptr += 4;

	h->mobi_record_count = bs_read_2(ptr);
	ptr += 2;

	h->mobi_record_size = bs_read_2(ptr);
	ptr += 2;

	h->mobi_encryption_type = bs_read_2(ptr);
	ptr += 2;

    /* zeroes */
	ptr += 2;

	return (ptr - orig_ptr);
}
