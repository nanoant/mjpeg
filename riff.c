/*
 * riff.c - MJPEG creator tool (https://github.com/nanoant/mjpeg)
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include "riff.h"

const char *fourcc(FOURCC fcc) {
	static char sfcc[8][5];
	static int rot = -1;
	rot = (rot + 1) % 8;
	sfcc[rot][4] = 0;
	*(FOURCC *)sfcc[rot] = fcc;
	return sfcc[rot++];
}

int freadchunk(FOURCC *fcc, uint32_t *size, FILE *in) {
	CHNK chnk;
	if(!fcc || !in || !size) return 0;
	if(!fread(&chnk, sizeof(CHNK), 1, in)) return 0;
	*fcc = chnk.fcc;
	*size = chnk.size;
	return 1;
}

int freadcc(FOURCC *fcc, FILE *in) {
	if(!fcc || !in) return 0;
	return fread(fcc, sizeof(FOURCC), 1, in);
}

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

size_t fcopy(FILE *in, FILE *out, uint32_t size) {
	uint8_t buf[4096]; /* usually one memory page */
	size_t wrote = 0;
	if(in && !out) {
		fseek(in, size, SEEK_CUR);
		return 0;
	}
	for(;;) {
		size_t wants = MIN(size, sizeof(buf));
		size_t read = fread(buf, 1, wants, in);
		if(read > 0 && out) {
			wrote += fwrite(buf, 1, read, out);
		}
		if(read < wants || size == wants) break;
		size -= wants;
	}
	return wrote;
}

size_t fwritechunk(FOURCC fcc, uint32_t size, FILE *out) {
	CHNK chnk;
	if(!out) return 0;
	chnk.fcc = fcc;
	chnk.size = size;
	return fwritesafe(&chnk, sizeof(CHNK), out);
}

size_t fwritecc(FOURCC fcc, FILE *out) {
	if(!out) return 0;
	return fwritesafe(&fcc, sizeof(FOURCC), out);
}

size_t fwritesafe(const void *ptr, size_t size, FILE *out) {
	if(!out) return 0;
	return fwrite(ptr, 1, size, out);
}

long fseeksafe(FILE *out, long pos, int whence) {
	if(!out) return 0;
	return fseek(out, pos, whence);
}

void fgetpossafe(FILE *out, fpos_t *pos) {
	if(!out) return;
	fgetpos(out, pos);
}

int fupdate(FILE *out, fpos_t *pos, uint32_t value) {
	fpos_t back;
	FOURCC fcc;
	uint32_t size;
	if(!out) return 0;
	fgetpos(out, &back);
	fsetpos(out, pos);
	freadchunk(&fcc, &size, out);
	fsetpos(out, pos);
	fwritechunk(fcc, value, out);
	fsetpos(out, &back);
	return 1;
}
