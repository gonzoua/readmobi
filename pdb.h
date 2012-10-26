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
#ifndef __PDB_H__
#define __PDB_H__

/* Header Attributes */
#define PDB_HEADER_READONLY             0x0002
#define PDB_HEADER_DIRTY_APPINFO        0x0004
#define PDB_HEADER_BACKUP               0x0008
#define PDB_HEADER_NEWER_OK             0x0010
#define PDB_HEADER_RESET_AFTER_INSTALL  0x0020
#define PDB_HEADER_NO_BEAM              0x0040

/* Record Attributes */
#define PDB_RECORD_SECRET               0x10
#define PDB_RECORD_INUSE                0x20
#define PDB_RECORD_DIRTY                0x40
#define PDB_RECORD_DELETE_ON_SYNC       0x80

struct pdb_record
{
    uint32_t        rec_offset;
    uint32_t        rec_size;
    uint8_t         rec_attributes;
    uint32_t        rec_id;
};

typedef struct pdb_record pdb_record_t;

struct pdb_header {
    char            pdb_db_name[32];
    uint16_t        pdb_attributes;
    uint16_t        pdb_version;
    uint32_t        pdb_ctime;
    uint32_t        pdb_mtime;
    uint32_t        pdb_btime; /** backup time */
    uint32_t        pdb_modification;
    uint32_t        pdb_app_info_offset;;
    uint32_t        pdb_sort_info_offset;;
    uint32_t        pdb_type;
    uint32_t        pdb_creator;
    uint32_t        pdb_uid_seed;
    uint32_t        pdb_next_record;
    uint16_t        pdb_num_records;
    pdb_record_t    *pdb_records;
};

typedef struct pdb_header pdb_header_t;

pdb_header_t* pdb_header_alloc();
void pdb_header_free(pdb_header_t*);
void pdb_header_print(pdb_header_t*);
void pdb_header_print_records(pdb_header_t*);
off_t pdb_header_read(pdb_header_t*, unsigned char *, off_t);
off_t pdb_header_read_records(pdb_header_t*, unsigned char *, off_t);

#endif /* __PDB_H__ */
