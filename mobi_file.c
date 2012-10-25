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
#include "exth.h"
#include "pdb.h"
#include "mobi_file.h"

mobi_file_t*
mobi_file_alloc()
{
    mobi_file_t *f;
    f = malloc(sizeof(mobi_file_t));
    memset(f, 0, sizeof(mobi_file_t));

    return (f);
}

void mobi_file_free(mobi_file_t* f)
{
    if (f->file_pdb_header) {
        pdb_header_free(f->file_pdb_header);
        f->file_pdb_header = NULL;
    }

    if (f->file_mobi_header) {
        mobi_header_free(f->file_mobi_header);
        f->file_mobi_header = NULL;
    }

    if (f->file_exth_header) {
        exth_header_free(f->file_exth_header);
        f->file_exth_header = NULL;
    }
}

int
mobi_file_load(mobi_file_t *f, unsigned char *ptr, size_t size)
{
    off_t file_size = size;
    off_t file_pos = 0;
    off_t bytes_read = 0;

    f->file_pdb_header = pdb_header_alloc();

    bytes_read = pdb_header_read(f->file_pdb_header, ptr, file_size);
    if (bytes_read < 0) {
        // fprintf(stderr, "pdb_header_read failed\n");
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;
 
    f->file_mobi_header = mobi_header_alloc();
    bytes_read = mobi_header_read(f->file_mobi_header, 
            (ptr + file_pos), file_size);
    if (bytes_read < 0) {
        // fprintf(stderr, "mobi_header_read failed\n");
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;

    f->file_exth_header = exth_header_alloc();
    bytes_read = exth_header_read(f->file_exth_header, 
            (ptr + file_pos), file_size);
    if (bytes_read < 0) {
        // fprintf(stderr, "exth_header_read failed\n");
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;

    return 0;
}

off_t
mobi_file_record_offset(mobi_file_t* f, uint32_t id)
{
    int i;
    pdb_header_t *h = f->file_pdb_header;

    for (i = 0; i < h->pdb_num_records; i++) {
        if (h->pdb_records[i].rec_id == id)
            return (h->pdb_records[i].rec_offset);
    }

    return (-1);
}

size_t
mobi_file_record_size(mobi_file_t* f, uint32_t id)
{
    int i;
    pdb_header_t *h = f->file_pdb_header;

    for (i = 0; i < h->pdb_num_records; i++) {
        if (h->pdb_records[i].rec_id  == id)
            return (h->pdb_records[i].rec_size);
    }

    return (-1);
}
