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
#ifndef __MOBI_H__
#define __MOBI_H__

#define MOBI_EXTRA_INDEXES  6
/* Set if there is EXTH header */
#define MOBI_EXTH_PRESENT   0x40

#define EXTH_EXTRA_MULTIBYTE        (1 << 0)
#define EXTH_EXTRA_TBS_INDEX        (1 << 1)
#define EXTH_EXTRA_UNCROSSABLE      (1 << 2)

#define MOBI_NO_INDEX       0xffffffffU
#define MOBI_NO_RECORD      0xffffU

/* 'MOBI' */
#define MOBI_ID             0x4d4f4249

struct mobi_header
{
    /* Old PalmDOC header */
    uint16_t    mobi_compression;
    uint32_t    mobi_text_length;
    uint16_t    mobi_record_count;
    uint16_t    mobi_record_size;
    uint16_t    mobi_encryption_type;
    /* Actual MOBI header */
    uint32_t    mobi_identifier; /* Should be 'MOBI' */
    uint32_t    mobi_header_length;
    uint32_t    mobi_type;
    uint32_t    mobi_text_encoding;
    uint32_t    mobi_uid;
    uint32_t    mobi_file_version;
    uint32_t    mobi_ortographic_index;
    uint32_t    mobi_inflection_index;
    uint32_t    mobi_index_names;
    uint32_t    mobi_index_keys;
    uint32_t    mobi_extra_index[MOBI_EXTRA_INDEXES];
    uint32_t    mobi_first_nonbook_index;
    uint32_t    mobi_full_name_offset;
    uint32_t    mobi_full_name_length;
    uint32_t    mobi_locale;
    uint32_t    mobi_dict_input_lang;
    uint32_t    mobi_dict_output_lang;
    uint32_t    mobi_min_version;
    uint32_t    mobi_first_image_rec;
    uint32_t    mobi_huffman_record_offset;
    uint32_t    mobi_huffman_record_count;
    uint32_t    mobi_huffman_table_offset;
    uint32_t    mobi_huffman_table_count;
    uint32_t    mobi_exth_flags;
    uint32_t    mobi_drm_offset;
    uint32_t    mobi_drm_count;
    uint32_t    mobi_drm_size;
    uint32_t    mobi_drm_flags;
    uint32_t    mobi_first_content_rec;
    uint32_t    mobi_last_content_rec;
    uint32_t    mobi_fcis_rec;
    uint32_t    mobi_flis_rec;
    uint32_t    mobi_extra_record_data_flags;
    uint32_t    mobi_indx_record_offset;
};

typedef struct mobi_header mobi_header_t;

mobi_header_t* mobi_header_alloc();
void mobi_header_free(mobi_header_t*);
void mobi_header_print(mobi_header_t*);
off_t mobi_header_read(mobi_header_t*, unsigned char *, off_t);

#endif /* __MOBI_H__ */
