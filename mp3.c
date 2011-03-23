/*
 * mp3.c - MJPEG creator tool (https://github.com/nanoant/mjpeg)
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

#include "mp3.h"

#define BITRATEFREE 0xfffe
#define BITRATEBAD  0xffff

#define SAMPLERATERESERVED  0xffff

static int bitrates[] = {
	BITRATEFREE, BITRATEFREE, BITRATEFREE, BITRATEFREE, BITRATEFREE,
	32,   32,  32,  32,   8,
	64,   48,  40,  48,  16,
	96,   56,  48,  56,  24,
	128,  64,  56,  64,  32,
	160,  80,  64,  80,  40,
	192,  96,  80,  96,  48,
	224, 112,  96, 112,  56,
	256, 128, 112, 128,  64,
	288, 160, 128, 144,  80,
	320, 192, 160, 160,  96,
	352, 224, 192, 176, 112,
	384, 256, 224, 192, 128,
	416, 320, 256, 224, 144,
	448, 384, 320, 256, 160,
	BITRATEBAD, BITRATEBAD, BITRATEBAD, BITRATEBAD, BITRATEBAD
};

static int samplerates[] = {
	44100, 22050, 11025,
	48000, 24000, 12000,
	32000, 16000, 8000,
	SAMPLERATERESERVED, SAMPLERATERESERVED, SAMPLERATERESERVED
};

static size_t unpacktagv2size(MP3ID3TAG2 *id) {
	return ((size_t)id->synchSafeSize[3]) |
	      (((size_t)id->synchSafeSize[2])<<7) |
	      (((size_t)id->synchSafeSize[1])<<14) |
	      (((size_t)id->synchSafeSize[0])<<21);
}

int mp3bitrate(mp3header_t h) {
	int index = 0;
	int version = MPEGVersion(h);
	if(version == MPEGVersion1) {
		index = 3 - MPEGLayer(h);
	} else if(version == MPEGVersion2 || version == MPEGVersion2_5) {
		index = MPEGLayer(h) == MPEGLayer1 ? 3 : 4;
	}
	return bitrates[index + MPEGBitrate(h) * 5];
}

int mp3samplerate(mp3header_t h) {
	int index = 2;
	int version = MPEGVersion(h);
	if(version == MPEGVersion1) {
		index = 0;
	} else if(version == MPEGVersion2) {
		index = 1;
	}
	return samplerates[index + MPEGSamplerate(h) * 3];
}

size_t mp3framesize(mp3header_t h) {
	if(MPEGLayer(h) == MPEGLayer1) {
		return (12000 * mp3bitrate(h) / mp3samplerate(h) + MPEGPadding(h)) * 4;
	}
	return 144000 * mp3bitrate(h) / mp3samplerate(h) + MPEGPadding(h);
}

double mp3framelength(mp3header_t h) {
	return 1152.0 / (double)mp3samplerate(h);
}

mp3header_t freadmp3header(FILE *fin) {
	MP3ID3TAG1 tagv1;
	MP3ID3TAG2 tagv2;
	fpos_t pos;
	mp3header_t h;

	while(1) {
		fgetpos(fin, &pos);

		if(fread(&h, 1, sizeof(mp3header_t), fin) < sizeof(mp3header_t)) return 0;
		h = ntohl(h);
		if(MPEGCheck(h)) return h;

		fsetpos(fin, &pos);
		fread(&tagv2, 1, sizeof(MP3ID3TAG2), fin);
		if(tagv2.tagid[0] == 'I' &&
		   tagv2.tagid[1] == 'D' &&
		   tagv2.tagid[2] == '3') {
			fseek(fin, unpacktagv2size(&tagv2), SEEK_CUR);
			continue;
		}

		fsetpos(fin, &pos);
		fread(&tagv1, 1, sizeof(MP3ID3TAG1), fin);
		if(tagv1.tagid[0] == 'T' &&
		   tagv1.tagid[1] == 'A' &&
		   tagv1.tagid[2] == 'G') {
			continue;
		}

		return 0;
	}
	return 0;
}

size_t fwritemp3header(FILE *fout, mp3header_t h) {
	h = htonl(h);
	return fwrite(&h, 1, sizeof(mp3header_t), fout);
}
