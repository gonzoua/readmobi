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
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "mobi.h"
#include "exth.h"
#include "pdb.h"
#include "mobi_file.h"

#define VERSION "1.0"

static void
usage(void)
{
    fprintf(stderr, "Usage: readmobi {-v | -adDeEm | -r id | -t file.mhtml} [file.mobi]\n");
    fprintf(stderr, "\t-a\t\t\tprint all headers/records\n");
    fprintf(stderr, "\t-d\t\t\tprint PDB headers\n");
    fprintf(stderr, "\t-D\t\t\tprint PDB records\n");
    fprintf(stderr, "\t-e\t\t\tprint EXTH header\n");
    fprintf(stderr, "\t-E\t\t\tprint EXTH records\n");
    fprintf(stderr, "\t-m\t\t\tprint MOBI headers\n");
    fprintf(stderr, "\t-r record_id\t\tDump PDB record\n");
    fprintf(stderr, "\t-t out_file\t\tDump text to file (\"-\" for stdout)\n");
    fprintf(stderr, "\t-v\t\t\tprint version and exit\n");
}

int
main(int argc, char **argv)
{
    int fd;
    void *ptr;
    unsigned char *mobi_data;
    const char *mhtml_file = NULL;
    int mhtml_fd;
    mobi_file_t *mobi_file;
    char ch;

    off_t record_offset;
    size_t record_size;
    off_t file_size;

    int print_pdb_header = 0;
    int print_pdb_records = 0;
    int print_mobi_header = 0;
    int print_exth_header = 0;
    int print_exth_records = 0;
    int dump_record = -1;

    while ((ch = getopt(argc, argv, "adDeEmr:t:v?")) != -1) {
        switch (ch) {
            case 'a':
                print_pdb_header = 1;
                print_pdb_records = 1;
                print_mobi_header = 1;
                print_exth_header = 1;
                print_exth_records = 1;
                break;
            case 'd':
                print_pdb_header = 1;
                break;
            case 'D':
                print_pdb_records = 1;
                break;
            case 'e':
                print_exth_header = 1;
                break;
            case 'E':
                print_exth_records = 1;
                break;
            case 'm':
                print_mobi_header = 1;
                break;
            case 'r':
                dump_record = atoi(optarg);
                break;
            case 't':
                mhtml_file = strdup(optarg);
                break;
            case 'v':
                printf("readmobi version %s\n", VERSION); 
                exit(0);
                break;
            case '?':
            default:
                usage();
                exit(0);
                break;
        }
    }

    if ((print_pdb_header || print_pdb_records ||
            print_mobi_header || print_exth_header || 
            print_exth_records) && (dump_record > -1)) {
        fprintf(stderr, "Can't mix -r and -adDeEm options\n");
        exit(0);
    }
    
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        usage();
        exit(1);
    }

    fd = open(argv[0], O_RDONLY);
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
    mobi_file = mobi_file_alloc();

    if (mobi_file_load(mobi_file, mobi_data, file_size) < 0) {
        fprintf(stderr, "Failed to load MOBI file\n");
        exit(1);
    }

    if (print_pdb_header)
        pdb_header_print(mobi_file->file_pdb_header);

    if (print_pdb_records)
        pdb_header_print_records(mobi_file->file_pdb_header);

    if (print_mobi_header)
        mobi_header_print(mobi_file->file_mobi_header);

    if (print_exth_header)
        exth_header_print(mobi_file->file_exth_header);

    if (print_exth_records)
        exth_header_print_records(mobi_file->file_exth_header);

    if (dump_record > -1) {
        record_offset = mobi_file_record_offset(mobi_file, dump_record);
        record_size = mobi_file_record_size(mobi_file, dump_record);

        if ((record_offset > -1) && (record_size > 0)) 
            write(fileno(stdout), mobi_data + record_offset, record_size);
        else
            fprintf(stderr, "PDB record #%d not found\n", dump_record);
    }

    if (mhtml_file != NULL) {
        if (strcmp(mhtml_file, "-") == 0)
            mhtml_fd = fileno(stdout);
        else
            mhtml_fd = open(mhtml_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (mhtml_fd == -1) {
            fprintf(stderr, "Failed to open out file %s\n", mhtml_file);
            perror("open");
            exit(1);
        }

        if (mobi_file_print_text(mhtml_fd, mobi_file) < 0)
            fprintf(stderr, "MOBI uncompress failed\n");
    }

    munmap(ptr, file_size);

    return 0;
}
