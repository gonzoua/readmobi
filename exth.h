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
#ifndef __EXTH_H__
#define __EXTH_H__

#define EXTH_REC_NONE                       0
#define EXTH_REC_DRM_SERVER_ID              1
#define EXTH_REC_COMMERCE_ID                2
#define EXTH_REC_EBOOKBASE_BOOK_ID          3
#define EXTH_REC_AUTHOR                     100
#define EXTH_REC_PUBLISHER                  101
#define EXTH_REC_IMPRINT                    102
#define EXTH_REC_DESCRIPTION                103
#define EXTH_REC_ISBN                       104
#define EXTH_REC_SUBJECT                    105
#define EXTH_REC_PUBLIHING_DATE             106
#define EXTH_REC_REVIEW                     107
#define EXTH_REC_CONTRIBUTOR                108
#define EXTH_REC_RIGHTS                     109
#define EXTH_REC_SUBJECT_CODE               110
#define EXTH_REC_TYPE                       111
#define EXTH_REC_SOURCE                     112
#define EXTH_REC_ASIN                       113
#define EXTH_REC_VER                        114
#define EXTH_REC_SAMPLE                     115
#define EXTH_REC_START_READING              116
#define EXTH_REC_ADULT                      117
#define EXTH_REC_RETAIL_PRICE               118
#define EXTH_REC_RETAIL_PRICE_CURRENCY      119
#define EXTH_REC_DICT_SHORT_NAME            200
#define EXTH_REC_COVER_OFFSET               201
#define EXTH_REC_THUMB_OFFSET               202
#define EXTH_REC_HAS_FAKE_COVER             203
#define EXTH_REC_CREATOR_SOFTWARE           204
#define EXTH_REC_CREATOR_MAJ_VER            205
#define EXTH_REC_CREATOR_MIN_VER            206
#define EXTH_REC_CREATOR_BUILD              207
#define EXTH_REC_WATERMARK                  208
#define EXTH_REC_TPROOF_KEYS                209
#define EXTH_REC_FONT_SIGN                  300
#define EXTH_REC_CLIP_LIMIT                 401
#define EXTH_REC_PUBLISHER_LIMIT            402
#define EXTH_REC_TTS_FLAG                   404
#define EXTH_REC_CDE_TYPE                   501
#define EXTH_REC_LAST_UPDATE                502
#define EXTH_REC_UPDATED_TITLE              503

/* EXTH */
#define EXTH_ID     0x45585448

enum exth_record_kind {
    EXTH_REC_TYPE_UNKNOWN,
    EXTH_REC_TYPE_BYTES,
    EXTH_REC_TYPE_STRING,
    EXTH_REC_TYPE_INT
};

typedef enum exth_record_kind exth_record_kind_t;

struct exth_record_desc
{
    uint32_t            exth_rec_type;
    exth_record_kind_t  exth_rec_kind;
    const char          *exth_rec_name;
};

typedef struct exth_record_desc exth_record_desc_t;

struct exth_record {
    uint32_t    exth_rec_type;
    uint32_t    exth_rec_length;
    uint8_t     *exth_rec_data;
};

typedef struct exth_record exth_record_t;

struct exth_header {
    uint32_t        exth_identifier;
    uint32_t        exth_length;
    uint32_t        exth_record_count;
    exth_record_t   *exth_records;
};

typedef struct exth_header exth_header_t;

exth_header_t* exth_header_alloc();
void exth_header_free(exth_header_t*);
void exth_header_print(exth_header_t*);
void exth_header_print_records(exth_header_t*);
off_t exth_header_read(exth_header_t*, unsigned char *, off_t);

#endif /* __EXTH_H__ */
