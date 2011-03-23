/*
 * mp3.h - MJPEG creator tool (https://github.com/nanoant/mjpeg)
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

typedef uint32_t mp3header_t;

#define	MPEGVersion2_5 0
#define	MPEGReserved   1
#define	MPEGVersion2   2
#define	MPEGVersion1   3

#define MPEGLayer1        3
#define MPEGLayer2        2
#define MPEGLayer3        1
#define MPEGLayerReserved 0

#define MPEGChannelsStereo      0
#define MPEGChannelsJointStereo 1
#define MPEGChannelsDualChannel 2
#define MPEGChannelsMono        3

#define MPEGFrameSync(f)   ((f) & 0xFFE00000)
#define MPEGCheck(f)      (((f) & 0xFFE00000) == 0xFFE00000)
#define MPEGBitrate(f)    (((f) &     0xf000) >> 12)
#define MPEGPadding(f)    (((f) &      0x200) >> 9)
#define MPEGVersion(f)    (((f) &   0x180000) >> 19)
#define MPEGLayer(f)      (((f) &    0x60000) >> 17)
#define MPEGSamplerate(f) (((f) &      0xC00) >> 10)
#define MPEGChannels(f)   (((f) &       0x60) >> 6)

typedef struct {
	char     tagid [3];   /* 0-2     3 Tag identifier. Must contain "TAG" string if Tag is valid. */
	char     name  [30];  /* 3-32   30 Song Name */
	char     artist[30];  /* 33-62  30 Artist */
	char     album [30];  /* 63-92  30 Album */
	uint32_t year;        /* 93-96   4 Year */
	char     comment[30]; /* 97-126 30 Comment */
	uint8_t  genre;       /* 127     1 Genre */
} __attribute__((packed)) MP3ID3TAG1;

typedef struct {
	char     tagid[3];         /* 0-2  TAG identifier. It contains of string "ID3" */
	uint16_t version;          /* 3-4  TAG version */
	uint8_t  flags;            /* 5    Flags */
	uint8_t  synchSafeSize[4]; /* 6-9  Size of TAG MSB */
} __attribute__((packed)) MP3ID3TAG2;

mp3header_t freadmp3header(FILE *fin);
size_t fwritemp3header(FILE *fout, mp3header_t h);
int mp3bitrate(mp3header_t h);
int mp3samplerate(mp3header_t h);
size_t mp3framesize(mp3header_t h);
double mp3framelength(mp3header_t h);
