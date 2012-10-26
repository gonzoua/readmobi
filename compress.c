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

int
palmdoc_decompress(unsigned char *ptr_in, size_t size_in,
        unsigned char* ptr_out, size_t size_out)
{
    size_t pos_in = 0;
    size_t pos_out = 0;
    unsigned char cmd;
    uint8_t length;
    uint16_t distance;
    int i;

    while ((pos_in < size_in) && (pos_out < size_out) ) {
        cmd = ptr_in[pos_in++];
        /* Literal */
        if ((cmd == 0) || (cmd > 8 && cmd < 0x80)) {
            ptr_out[pos_out++] = cmd;
        }
        /* Literals */
        if ((cmd > 0 && cmd < 9)) {
            memcpy(ptr_out + pos_out, ptr_in + pos_in, cmd);
            pos_in += cmd;
            pos_out += cmd;
        }
        /* Length, distance */
        if ((cmd >= 0x80 && cmd < 0xc0)) {
            /* Take 6 bits of first byte */
            distance = cmd & 0x3f;
            /* .. and 5 bits of next byte; */
            distance <<= 5;
            distance |= (ptr_in[pos_in] >> 3) & 0x1f;
            /* 3 last bits are length */
            length = (ptr_in[pos_in] & 7) + 3;
            pos_in++;
            /*
             * Copy byte by byte because there might be overlap
             * between destination and source
             */
            for (i = 0; i < length; i++) {
                ptr_out[pos_out] = ptr_out[pos_out - distance];
                pos_out++;
            }
        }
        /* Byte pair */
        if (cmd >= 0xc0) { 
            ptr_out[pos_out++] = ' ';
            ptr_out[pos_out++] = cmd ^ 0x80;
        }
    }

    return (pos_out);
}
