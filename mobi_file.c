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
#include <unistd.h>

#include "compress.h"
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
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;

    /* 
     * Is it valid MOBIBOOK file?
     */

    if ((f->file_pdb_header->pdb_type != PDB_MOBI_TYPE) ||
            (f->file_pdb_header->pdb_creator != PDB_MOBI_CREATOR))
        return (-1);

    bytes_read = pdb_header_read_records(f->file_pdb_header,
            ptr + file_pos, file_size);
    if (bytes_read < 0) {
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;
 
    f->file_mobi_header = mobi_header_alloc();
    bytes_read = mobi_header_read(f->file_mobi_header, 
            (ptr + file_pos), file_size);
    if (bytes_read < 0) {
        return (-1);
    }

    file_size -= bytes_read;
    file_pos += bytes_read;

    if (f->file_mobi_header->mobi_exth_flags & MOBI_EXTH_PRESENT) {
        f->file_exth_header = exth_header_alloc();
        bytes_read = exth_header_read(f->file_exth_header, 
                (ptr + file_pos), file_size);
        if (bytes_read < 0) {
            return (-1);
        }

        file_size -= bytes_read;
        file_pos += bytes_read;
    }

    f->file_data = ptr;
    f->file_size = size;

    return 0;
}

off_t
mobi_file_record_offset(mobi_file_t* f, uint32_t num)
{
    pdb_header_t *h = f->file_pdb_header;

    if (num < h->pdb_num_records)
        return (h->pdb_records[num].rec_offset);

    return (-1);
}

size_t
mobi_file_record_size(mobi_file_t* f, uint32_t num)
{
    pdb_header_t *h = f->file_pdb_header;

    if (num < h->pdb_num_records)
        return (h->pdb_records[num].rec_size);

    /* size_t is signed, can't use -1 here */
    return (0);
}

int
mobi_file_print_text(int fd, mobi_file_t* f)
{
    int rec;
    int trailing_entry;
    unsigned char ch;
    uint32_t te_mask;
    uint32_t te_size;
    uint32_t te_size_size;
    uint32_t overlap_size;
    uint32_t total_size;
    uint32_t prev_overlap_size;
    off_t record_offset;
    size_t record_size;
    uint32_t first_content_record = 
        f->file_mobi_header->mobi_first_content_rec;
    uint32_t last_content_record =
        f->file_mobi_header->mobi_last_content_rec;

    unsigned char *chunk;
    int chunk_size;

    chunk = malloc(MOBI_CHUNK_SIZE);

    te_mask = f->file_mobi_header->mobi_extra_record_data_flags;
    prev_overlap_size = 0;
    total_size = 0;

    if (first_content_record == MOBI_NO_RECORD)
        first_content_record = 1;

    if (last_content_record == MOBI_NO_RECORD)
        last_content_record = f->file_pdb_header->pdb_num_records - 1;

    for (rec = first_content_record; rec < last_content_record; rec++) {
        record_offset = mobi_file_record_offset(f, rec);
        record_size = mobi_file_record_size(f, rec);

        if ((record_offset < 0) || (record_size == 0)) {
            fprintf(stderr, "%s: record #%d not found\n", __func__, rec);
            goto fail;
        }

        /*
         * All trailing bits but 1 are common format <data>,<size>
         * Size is reverse-format variable-length int:
         *  - big endian
         *  - 7 bits per byte
         *  - 8th bit set marks end of the number
         */
        for (trailing_entry = 15; trailing_entry > 0; trailing_entry--) {
            if (te_mask & (1 << trailing_entry)) {
                te_size = 0;
                te_size_size = 1;

                do {
                    ch = f->file_data[record_offset + record_size - te_size_size];
                    te_size |= ((ch & 0x7f) << (7 * (te_size_size - 1)));
                    te_size_size++;
                } while (!(ch & 0x80));

                if (te_size > record_size) {
                    fprintf(stderr, 
                            "%s: trailing etry size is too long "
                            "in record #%d: record_size = %zd, TE size: %d\n",
                            __func__, rec, record_size, te_size);
                    goto fail;
                }

                record_size -= te_size;
            }
        }

        overlap_size = 0;
        if (te_mask & 1) {
            /*
             * Multibyte overlap
             */
            ch = f->file_data[record_offset + record_size - 1];
            overlap_size = ch & 0x3;
            record_size -= overlap_size + 1;
        }

        if ((chunk_size = palmdoc_decompress(f->file_data + record_offset,
                    record_size, chunk, MOBI_CHUNK_SIZE)) < 0) {
            goto fail;
        }
        else {
            write(fd, chunk + prev_overlap_size, 
                    chunk_size - prev_overlap_size);
            if (overlap_size)
                write(fd, f->file_data + record_offset + record_size, 
                        overlap_size);

            total_size += chunk_size - prev_overlap_size + overlap_size;

            prev_overlap_size = overlap_size;

            /* End of file? */
            if (total_size >= f->file_mobi_header->mobi_text_length) 
                break;

        }
    }

    free(chunk);
    return (0);

fail:
    free(chunk);
    return (-1);
}
