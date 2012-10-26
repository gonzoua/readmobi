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

#include "pdb.h"
#include "bytestream.h"
#include "utils.h"

#define MIN_PDB_HEADER_SIZE 50

/*
 * Just to be on the safe side
 */
#ifdef PDB_HEADER_READ_2
#undef  PDB_HEADER_READ_2
#endif

#ifdef PDB_HEADER_READ_4
#undef  PDB_HEADER_READ_4
#endif

#define PDB_HEADER_READ_2(v, ptr) do {     \
    (v) = bs_read_2((ptr)); (ptr) += 2;     \
} while (0);

#define PDB_HEADER_READ_4(v, ptr) do {     \
    (v) = bs_read_4((ptr)); (ptr) += 4;     \
} while (0);

pdb_header_t*
pdb_header_alloc()
{
    pdb_header_t *h = malloc(sizeof(pdb_header_t));
    memset(h, 0, sizeof(*h));
    return h;
}

void
pdb_header_free(pdb_header_t* header)
{
    if (header->pdb_records)
        free(header->pdb_records);
    free(header);
}

void
pdb_header_print(pdb_header_t* h)
{
    printf("PDB Header\n");
    printf("  DB Name: %s\n", h->pdb_db_name);
    printf("  Attributes:");
    if (h->pdb_attributes & PDB_HEADER_READONLY)
        printf(" READONLY");
    if (h->pdb_attributes & PDB_HEADER_DIRTY_APPINFO)
        printf(" DIRTY_APPINFO");
    if (h->pdb_attributes & PDB_HEADER_BACKUP)
        printf(" BACKUP");
    if (h->pdb_attributes & PDB_HEADER_NEWER_OK)
        printf(" NEWER_OK");
    if (h->pdb_attributes & PDB_HEADER_RESET_AFTER_INSTALL)
        printf(" RESET");
    if (h->pdb_attributes & PDB_HEADER_NO_BEAM)
        printf(" NO_BEAM");
    printf(" (%04x)\n", h->pdb_attributes);
    printf("  Version: %d\n", h->pdb_version);
    printf("  Creation time: %d\n", h->pdb_ctime);
    printf("  Modification time: %d\n", h->pdb_mtime);
    printf("  Last backup time: %d\n", h->pdb_btime);
    printf("  Modification #: %d\n", h->pdb_modification);
    printf("  App info offset: %d\n", h->pdb_app_info_offset);
    printf("  Sort info offset: %d\n", h->pdb_sort_info_offset);
    printf("  Type: %08x (%s)\n", h->pdb_type, id2string(h->pdb_type));
    printf("  Creator: %08x (%s)\n", h->pdb_creator, id2string(h->pdb_creator));
    printf("  UID seed: %08x\n", h->pdb_uid_seed);
    printf("  Next record: %d\n", h->pdb_next_record);
    printf("  # of Records: %d\n", h->pdb_num_records);
}

void
pdb_header_print_records(pdb_header_t* h)
{
    int i;
    if (h->pdb_num_records) {
        printf("PDB Records:\n");
        for (i = 0; i < h->pdb_num_records; i++) {
            printf("  <");
            printf("offset=%d,attr=0x%08x,id=%06d",
                    h->pdb_records[i].rec_offset,
                    h->pdb_records[i].rec_attributes,
                    h->pdb_records[i].rec_id);
            printf(">\n");
        }
    }
}

off_t
pdb_header_read(pdb_header_t* h, unsigned char *ptr, off_t size)
{
    unsigned char *orig_ptr = ptr;

    if (size < MIN_PDB_HEADER_SIZE)
        return (-1);

    memcpy(h->pdb_db_name, ptr, sizeof(h->pdb_db_name));
    h->pdb_db_name[sizeof(h->pdb_db_name)-1] = 0;
    ptr += sizeof(h->pdb_db_name);

    PDB_HEADER_READ_2(h->pdb_attributes, ptr);
    PDB_HEADER_READ_2(h->pdb_version, ptr);
    PDB_HEADER_READ_4(h->pdb_ctime, ptr);
    PDB_HEADER_READ_4(h->pdb_mtime, ptr);
    PDB_HEADER_READ_4(h->pdb_btime, ptr);
    PDB_HEADER_READ_4(h->pdb_modification, ptr);
    PDB_HEADER_READ_4(h->pdb_app_info_offset, ptr);
    PDB_HEADER_READ_4(h->pdb_sort_info_offset, ptr);
    PDB_HEADER_READ_4(h->pdb_type, ptr);
    PDB_HEADER_READ_4(h->pdb_creator, ptr);
    PDB_HEADER_READ_4(h->pdb_uid_seed, ptr);
    PDB_HEADER_READ_4(h->pdb_next_record, ptr);
    PDB_HEADER_READ_2(h->pdb_num_records, ptr);

    return (ptr - orig_ptr);
}

off_t
pdb_header_read_records(pdb_header_t* h, unsigned char *ptr, off_t size)
{
    unsigned char *orig_ptr = ptr;
    int i;

    if (size < MIN_PDB_HEADER_SIZE)
        return (-1);

    if (h->pdb_num_records) {
        h->pdb_records = malloc(sizeof(pdb_record_t)*h->pdb_num_records);
        /* TODO: ENOMEM here */
        if (!h->pdb_records)
            return (-1);
        for (i = 0; i < h->pdb_num_records; i++) {
            h->pdb_records[i].rec_offset = bs_read_4(ptr);
            ptr += 4;
            h->pdb_records[i].rec_attributes = bs_read_1(ptr);
            ptr += 1;
            h->pdb_records[i].rec_id = (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
            ptr += 3;
        }
    }

    /* Calculate record sizes */
    for (i = 0; i < h->pdb_num_records; i++) {
        /* last record */
        if ((i + 1) ==  h->pdb_num_records)
            h->pdb_records[i].rec_size = size - h->pdb_records[i].rec_offset;
        else
            h->pdb_records[i].rec_size = 
                h->pdb_records[i+1].rec_offset - h->pdb_records[i].rec_offset;

    }

    /* Trailing two bytes */
    ptr += 2;

    return (ptr - orig_ptr);
}
