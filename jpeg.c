/*
 * jpeg.c - MJPEG creator tool (https://github.com/nanoant/mjpeg)
 *
 * Copyright (c) 2011 Adam Strzelecki
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>

#define JPEG_MARKER_MASK 0xFF00
#define JPEG_HEAD_MARKER 0xFFD8
#define JPEG_APP0_MARKER 0xFFE0

static uint16_t jpeg_markers[] = {
	0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3,
	0xFFC5, 0xFFC6, 0xFFC7,
	0xFFC9, 0xFFCA, 0xFFCB,
	0xFFCD, 0xFFCE, 0xFFCF
};

typedef struct {
	uint16_t marker;
	uint16_t size;
} __attribute__((packed)) JPEG_CHUNK;

typedef struct {
	uint8_t unknown;
	uint16_t height;
	uint16_t width;
} __attribute__((packed)) JPEG_SIZE;

int jpeg_size(const char *path, int *width, int *height)
{
	JPEG_CHUNK chunk;
	JPEG_SIZE size;
	int ret = 0, i;
	FILE *in = fopen(path, "rb");
	if(!in) return 0;

	if(fread(&chunk.marker, 1, sizeof(chunk.marker), in) == sizeof(chunk.marker)) {
		chunk.marker = ntohs(chunk.marker);
		if(chunk.marker == JPEG_HEAD_MARKER) {
			while(!ret) {
				if(fread(&chunk, 1, sizeof(chunk), in) != sizeof(chunk)) break;
				chunk.marker = ntohs(chunk.marker);
				chunk.size   = ntohs(chunk.size);
				if((chunk.marker & JPEG_MARKER_MASK) != JPEG_MARKER_MASK) break;
				for(i = 0; i < sizeof(jpeg_markers) / sizeof(*jpeg_markers); i++) {
					if(chunk.marker == jpeg_markers[i]) {
						if(fread(&size, 1, sizeof(size), in) != sizeof(size)) {
							ret = 0; break;
						}
						if(width)  *width  = ntohs(size.width);
						if(height) *height = ntohs(size.height);
						ret = 1;
						break;
					}
				}
				fseek(in, chunk.size - sizeof(chunk.size), SEEK_CUR);
			}
		}
	}

	fclose(in);
	return ret;
}
