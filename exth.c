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
#include <ctype.h>

#include "exth.h"
#include "bytestream.h"
#include "utils.h"

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

static exth_record_desc_t record_descs[] = {
    { EXTH_REC_DRM_SERVER_ID,     EXTH_REC_TYPE_STRING,   "drm_server_id"},
    { EXTH_REC_COMMERCE_ID,       EXTH_REC_TYPE_STRING,   "drm_commerce_id"},
    { EXTH_REC_EBOOKBASE_BOOK_ID, EXTH_REC_TYPE_STRING,   "drm_ebookbase_book_id"},
    { EXTH_REC_AUTHOR,            EXTH_REC_TYPE_STRING,   "author"},
    { EXTH_REC_PUBLISHER,         EXTH_REC_TYPE_STRING,   "publisher"},
    { EXTH_REC_IMPRINT,           EXTH_REC_TYPE_STRING,   "imprint"},
    { EXTH_REC_DESCRIPTION,       EXTH_REC_TYPE_STRING,   "description"},
    { EXTH_REC_ISBN,              EXTH_REC_TYPE_STRING,   "isbn"},
    { EXTH_REC_SUBJECT,           EXTH_REC_TYPE_STRING,   "subject"},
    { EXTH_REC_PUBLIHING_DATE,    EXTH_REC_TYPE_STRING,   "publishing_date"},
    { EXTH_REC_REVIEW,            EXTH_REC_TYPE_STRING,   "review"},
    { EXTH_REC_CONTRIBUTOR,       EXTH_REC_TYPE_STRING,   "contributor"},
    { EXTH_REC_RIGHTS,            EXTH_REC_TYPE_STRING,   "rights"},
    { EXTH_REC_SUBJECT_CODE,      EXTH_REC_TYPE_STRING,   "subject_code"},
    { EXTH_REC_TYPE,              EXTH_REC_TYPE_STRING,   "type"},
    { EXTH_REC_SOURCE,            EXTH_REC_TYPE_STRING,   "source"},
    { EXTH_REC_ASIN,              EXTH_REC_TYPE_STRING,   "asin"},
    { EXTH_REC_VER,               EXTH_REC_TYPE_STRING,   "version_number"},
    { EXTH_REC_SAMPLE,            EXTH_REC_TYPE_INT,      "sample"},
    { EXTH_REC_START_READING,     EXTH_REC_TYPE_INT,      "startreading"},
    { EXTH_REC_ADULT,             EXTH_REC_TYPE_STRING,   "adult"},
    { EXTH_REC_RETAIL_PRICE,      EXTH_REC_TYPE_STRING,   "retail_price"},
    { EXTH_REC_RETAIL_PRICE_CURRENCY, EXTH_REC_TYPE_STRING,   "retail_price_currency"},
    { EXTH_REC_DICT_SHORT_NAME,   EXTH_REC_TYPE_STRING,   "dict_short_name"},
    { EXTH_REC_COVER_OFFSET,      EXTH_REC_TYPE_INT,      "coveroffset"},
    { EXTH_REC_THUMB_OFFSET,      EXTH_REC_TYPE_INT,      "thumboffset"},
    { EXTH_REC_HAS_FAKE_COVER,    EXTH_REC_TYPE_INT,      "hasfakecover"},
    { EXTH_REC_CREATOR_SOFTWARE,  EXTH_REC_TYPE_INT,      "creator_software"},
    { EXTH_REC_CREATOR_MAJ_VER,   EXTH_REC_TYPE_INT,      "creator_maj_ver"},
    { EXTH_REC_CREATOR_MIN_VER ,  EXTH_REC_TYPE_INT,      "creator_min_ver"},
    { EXTH_REC_CREATOR_BUILD,     EXTH_REC_TYPE_INT,      "creator_build"},
    { EXTH_REC_WATERMARK,         EXTH_REC_TYPE_STRING,   "watermark"},
    { EXTH_REC_TPROOF_KEYS,       EXTH_REC_TYPE_STRING,   "tamper_proof_keys"},
    { EXTH_REC_FONT_SIGN,         EXTH_REC_TYPE_BYTES,    "font_signature"},
    { EXTH_REC_CLIP_LIMIT,        EXTH_REC_TYPE_INT,      "clipping_limit"},
    { EXTH_REC_PUBLISHER_LIMIT,   EXTH_REC_TYPE_STRING,   "publisher_limit"},
    { EXTH_REC_TTS_FLAG,          EXTH_REC_TYPE_INT,      "tts_flag"},
    { EXTH_REC_CDE_TYPE,          EXTH_REC_TYPE_STRING,   "cde_type"},
    { EXTH_REC_LAST_UPDATE,       EXTH_REC_TYPE_STRING,   "last_update_time"},
    { EXTH_REC_UPDATED_TITLE,     EXTH_REC_TYPE_STRING,   "updated_title"},
    { EXTH_REC_NONE,              EXTH_REC_TYPE_UNKNOWN,  NULL},
};

static const exth_record_desc_t *
exth_get_record_desc(uint32_t rec_type)
{
    int i;
    for (i = 0; record_descs[i].exth_rec_type != EXTH_REC_NONE; i++) {
        if (record_descs[i].exth_rec_type == rec_type)
            return (&record_descs[i]);
    }

    return (NULL);
}

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

static void
exth_record_print_content(exth_record_t *rec, int kind, const char *indent)
{
    uint32_t val = 0;
    int i, c;
    int record_size = rec->exth_rec_length - 8;

    switch (kind) {
        case EXTH_REC_TYPE_STRING:
            printf("%s", indent);
            for (i = 0; i < record_size; i++) {
                printf("%c", rec->exth_rec_data[i]);
                if (rec->exth_rec_data[i] == '\n')
                    printf("%s", indent);
            }
            break;
        case EXTH_REC_TYPE_INT:
            printf("%s", indent);
            if (record_size > 4) {
                printf("<int longer then 32 bits>");
            }
            for (i = 0; i < record_size; i++)
                val = (val << 8) | rec->exth_rec_data[i];
            printf("%d", val);
            break;
        case EXTH_REC_TYPE_UNKNOWN:
        case EXTH_REC_TYPE_BYTES:
        default:
            c = 0;
            while (c < record_size) {
                printf("%s", indent);

                for (i = 0; i < 16; i++) {
                    if (c + i >= record_size) {
                        printf("   ");
                        continue;
                    }
                    printf("%02x ", rec->exth_rec_data[c + i]);
                }

                printf(" | ");
                for (i = 0; i < 16; i++) {
                    if (c + i >= record_size) {
                        printf(" ");
                        continue;
                    }
                    if (isprint(rec->exth_rec_data[c + i]))
                        printf("%c", rec->exth_rec_data[c + i]);
                    else
                        printf(".");
                }

                c += 16;

                printf("\n");
            }
            break;

    }
}

void
exth_header_print(exth_header_t* h)
{
    printf("EXTH header\n");
    printf("  Identifier: %08x (%s)\n", h->exth_identifier, id2string(h->exth_identifier));
    printf("  Length: %d\n", h->exth_length);
    printf("  Record count: %d\n", h->exth_record_count);
}

void
exth_header_print_records(exth_header_t* h)
{
    int i;
    const exth_record_desc_t *desc;
    exth_record_kind_t kind = EXTH_REC_TYPE_UNKNOWN;

    printf("EXTH records\n");

    for (i = 0; i < h->exth_record_count; i++) {
        kind = EXTH_REC_TYPE_UNKNOWN;
        desc = exth_get_record_desc(h->exth_records[i].exth_rec_type);
        printf("  <type=%d(%s),data_length=%d>\n",
                h->exth_records[i].exth_rec_type,
                desc ? desc->exth_rec_name : "unknown",
                h->exth_records[i].exth_rec_length - 8);
        if (desc != NULL)
            kind = desc->exth_rec_kind;
        exth_record_print_content(&h->exth_records[i], kind, "    ");
        printf("\n");
    }
}



off_t
exth_header_read(exth_header_t* h, unsigned char *ptr, off_t size)
{
    unsigned char *orig_ptr = ptr;
    int i;
    int data_size;

    if (size < MIN_EXTH_HEADER_SIZE)
        return (-1);

    EXTH_HEADER_READ_4(h->exth_identifier, ptr);
    if (h->exth_identifier != EXTH_ID)
        return (-1);
    EXTH_HEADER_READ_4(h->exth_length, ptr);
    EXTH_HEADER_READ_4(h->exth_record_count, ptr);

    if (h->exth_record_count)
        h->exth_records = malloc(sizeof(exth_record_t)*h->exth_record_count);

    for (i = 0; i < h->exth_record_count; i++) {
        EXTH_HEADER_READ_4(h->exth_records[i].exth_rec_type, ptr);
        EXTH_HEADER_READ_4(h->exth_records[i].exth_rec_length, ptr);
        data_size = h->exth_records[i].exth_rec_length - 8;
        if (data_size > 0) {
            h->exth_records[i].exth_rec_data = malloc(data_size);
            if (h->exth_records[i].exth_rec_data == NULL)
                return (-1);
            memcpy(h->exth_records[i].exth_rec_data, ptr, data_size);
        }
        ptr += h->exth_records[i].exth_rec_length - 8;
    }

    return (ptr - orig_ptr);
}
