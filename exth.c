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

#include "exth.h"
#include "bytestream.h"

#define MIN_EXTH_HEADER_SIZE 12

/*
 * Just to be on the safe side
 */
#ifdef EXTH_HEADER_READ_2
#undef  EXTH_HEADER_READ_2
#endif

#ifdef EXTH_HEADER_READ_4
#undef  EXTH_HEADER_READ_4
#endif

#define EXTH_HEADER_READ_2(v, ptr) do {     \
    (v) = bs_read_2((ptr)); (ptr) += 2;     \
} while (0);

#define EXTH_HEADER_READ_4(v, ptr) do {     \
    (v) = bs_read_4((ptr)); (ptr) += 4;     \
} while (0);

exth_header_t*
exth_header_alloc()
{
	exth_header_t *h = malloc(sizeof(exth_header_t));
	memset(h, 0, sizeof(*h));
	return h;
}

void
exth_header_free(exth_header_t* header)
{
    if (header->exth_records)
        free(header->exth_records);

	free(header);
}

void
exth_header_print(exth_header_t* h)
{
    int i;

    printf("EXTH header\n");
    printf("  Identifier: %08x\n", h->exth_identifier);
    printf("  Length: %d\n", h->exth_length);
    printf("  Record count: %d\n", h->exth_record_count);

    for (i = 0; i < h->exth_record_count; i++) {
        printf("    <type=%d,length=%d>\n",
                h->exth_records[i].exth_rec_type,
                h->exth_records[i].exth_rec_length);
    }
}

off_t
exth_header_read(exth_header_t* h, unsigned char *ptr, off_t size)
{
	unsigned char *orig_ptr = ptr;
    int i;

	if (size < MIN_EXTH_HEADER_SIZE)
		return (-1);

    EXTH_HEADER_READ_4(h->exth_identifier, ptr);
    EXTH_HEADER_READ_4(h->exth_length, ptr);
    EXTH_HEADER_READ_4(h->exth_record_count, ptr);

    if (h->exth_record_count)
        h->exth_records = malloc(sizeof(exth_record_t)*h->exth_record_count);

    for (i = 0; i < h->exth_record_count; i++) {
        EXTH_HEADER_READ_4(h->exth_records[i].exth_rec_type, ptr);
        EXTH_HEADER_READ_4(h->exth_records[i].exth_rec_length, ptr);
        ptr += h->exth_records[i].exth_rec_length - 8;
    }

	return (ptr - orig_ptr);
}
