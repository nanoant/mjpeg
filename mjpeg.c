/*
 * mjpeg.c - MJPEG creator tool (https://github.com/nanoant/mjpeg)
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
#include <stdlib.h>
#include <string.h>

#include "riff.h"
#include "mp3.h"
#include "jpeg.h"

#define DEFAULT_FPS 25

void help(const char *program)
{
	fprintf(stderr, "Usage: %s [-f fps] [-o output.avi] [-s input.mp3] input1.jpg [input2.jpg ...]\n", program);
}

int main(int argc, char const *argv[])
{
	int argi, fps = DEFAULT_FPS, width, height;
	const char *outPath = NULL, *sndPath = NULL;
	fpos_t riffPos, hdrlPos, strlPos, moviPos, sndDataPos, sndFmtPos;
	size_t riffSize, hdrlSize, strlSize, moviSize, read, sndFmtSize;
	AVIH avih;
	STRH strh;
	BMPH bmph;
	VPRP vprp;
	WAVH wavh;
	ADPCMH adpcmh;
	MP3H mp3h;
	int frame = 0;
	FILE *out, *idx, *snd = NULL, *in;
	mp3header_t mp3 = 0;
	double videoFrameLength, audio = 0, video = 0;

	/* read command line */
	for(argi = 1; argi < argc && *argv[argi] == '-'; argi++) {
		if(!strcmp(argv[argi], "-h")) {
			help(argv[0]);
			return 0;
		} else if(!strcmp(argv[argi], "-o") && argi + 1 < argc) {
			outPath = argv[++argi];
		} else if(!strcmp(argv[argi], "-s") && argi + 1 < argc) {
			sndPath = argv[++argi];
		} else if(!strcmp(argv[argi], "-f") && argi + 1 < argc) {
			fps = atoi(argv[++argi]);
			if(fps == 0) {
				fprintf(stderr, "Error: Invalid FPS value `%s'.\n", argv[argi]);
				return 255;
			}
		}
	}

	if(argi >= argc) {
		help(argv[0]);
		return 255;
	}

	if(!jpeg_size(argv[argi], &width, &height)) {
		fprintf(stderr, "Error: Invalid JPEG file `%s'.\n", argv[argi]);
		return 1;
	}

	if(outPath && !(out = fopen(outPath, "w+b"))) {
		fprintf(stderr, "Error: Cannot open output `%s'.\n", outPath);
		return 2;
	} else if(!out) {
		out = stdout;
	}

	videoFrameLength = 1.0 / fps;

	if(!(idx = tmpfile())) {
		fprintf(stderr, "Error: Cannot create temporary index for `%s'.\n", outPath ?: "(stdout)");
		return 3;
	}
	if(sndPath && !(snd = fopen(sndPath, "rb"))) {
		fprintf(stderr, "Error: Cannot open input `%s'.\n", sndPath);
		return 4;
	}

	if(snd) {
		if((mp3 = freadmp3header(snd))) {
			fprintf(stderr, "MP3 `%s' sample rate: %d, bitrate: %d, length: %lu, padding: %d\n", sndPath,
				mp3samplerate(mp3), mp3bitrate(mp3), mp3framesize(mp3), MPEGPadding(mp3));
			fseek(snd, 0, SEEK_SET);
		} else {
			FOURCC fcc;
			uint32_t size;
			uint16_t cbsize;
			fseek(snd, 0, SEEK_SET);
			if(freadchunk(&fcc, &size, snd) && fcc == FOURCC_RIFF &&
			   freadcc(&fcc, snd) && fcc == FOURCC_WAVE &&
			   freadchunk(&fcc, &size, snd) && fcc == FOURCC_FMT && (sndFmtSize = size) &&
			   fgetpos(snd, &sndFmtPos) == 0 &&
			   fread(&wavh, 1, sizeof(wavh), snd) && wavh.format == WAVE_FORMAT_ADPCM &&
			   fread(&cbsize, 1, sizeof(cbsize), snd) && (cbsize >= sizeof(adpcmh)) &&
			   fread(&adpcmh, 1, sizeof(adpcmh), snd) && fseek(snd, cbsize - sizeof(adpcmh), SEEK_CUR) == 0) {
				/* skip all headers until data */
				while(freadchunk(&fcc, &size, snd) && fcc != FOURCC_DATA) {
					fseek(snd, size, SEEK_CUR);
				}
				if(fcc == FOURCC_DATA) {
					fprintf(stderr, "WAV `%s' sample rate: %d, bitrate: %d, format: %d, channels: %d, samples per block: %d, block align: %d bytes, data: %d bytes, blocks: %.12g\n", sndPath,
						wavh.samplesPerSec, wavh.bitsPerSample, wavh.format, wavh.channels, adpcmh.samplesPerBlock,
						wavh.blockAlign, size, (float)size / (float)wavh.blockAlign);
					fgetpos(snd, &sndDataPos);
				}
			} else {
				fclose(snd), snd = NULL;
			}
		}
	}

	fgetpos(out, &riffPos);
	fwritechunk(FOURCC_RIFF, 0, out);
	riffSize = fwritecc(FOURCC_AVI, out);

		fgetpos(out, &hdrlPos);
		riffSize += fwritechunk(FOURCC_LIST, 0, out);
		hdrlSize = fwritecc(FOURCC_HDRL, out);

		hdrlSize += fwritechunk(FOURCC_AVIH, sizeof(avih), out);
		memset(&avih, 0, sizeof(avih));
		avih.microSecPerFrame = 1000000 / fps;
		avih.maxBytesPerSec = 45000;
		avih.flags = AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE;
		avih.totalFrames = argc - argi;
		avih.streams = snd ? 2 : 1;
		avih.width = width;
		avih.height = height;
		avih.suggestedBufferSize = 1024*1024;
		hdrlSize += fwrite(&avih, 1, sizeof(avih), out);

		fprintf(stderr, "AVI `%s' %dx%d %d frames\n", outPath, avih.width, avih.height, avih.totalFrames);

			fgetpos(out, &strlPos);
			hdrlSize += fwritechunk(FOURCC_LIST, 0, out);
			strlSize = fwritecc(FOURCC_STRL, out);

				strlSize += fwritechunk(FOURCC_STRH, sizeof(strh), out);
				memset(&strh, 0, sizeof(strh));
				strh.type = FOURCC_VIDS;
				strh.handler = CC("MJPG");
				strh.scale   = 1;
				strh.rate    = fps;
				strh.quality = (uint32_t)-1;
				strh.length  = avih.totalFrames;
				strh.suggestedBufferSize = avih.suggestedBufferSize;
				strh.frame.right  = avih.width;
				strh.frame.bottom = avih.height;
				strlSize += fwrite(&strh, 1, sizeof(strh), out);

				strlSize += fwritechunk(FOURCC_STRF, sizeof(bmph), out);
				memset(&bmph, 0, sizeof(bmph));
				bmph.size     = sizeof(bmph);
				bmph.width    = avih.width;
				bmph.height   = avih.height;
				bmph.planes   = 1;
				bmph.bitCount = 24;
				bmph.imgSize  = bmph.width * bmph.height * bmph.bitCount / 8;
				bmph.compression = CC("MJPG");
				strlSize += fwrite(&bmph, 1, sizeof(bmph), out);

				strlSize += fwritechunk(FOURCC_VPRP, sizeof(vprp), out);
				memset(&vprp, 0, sizeof(vprp));
				vprp.verticalRefreshRate = fps;
				vprp.hTotalInT           = avih.width;
				vprp.vTotalInLines       = avih.height;
				vprp.frameAspectRatio    = ASPECT_3_2;
				vprp.frameWidthInPixels  = avih.width;
				vprp.frameHeightInLines  = avih.height;
				vprp.fieldsPerFrame      = 1;
				vprp.field.compressedBMHeight = avih.height;
				vprp.field.compressedBMWidth  = avih.width;
				vprp.field.validBMHeight      = avih.height;
				vprp.field.validBMWidth       = avih.width;
				strlSize += fwrite(&vprp, 1, sizeof(vprp), out);

			fupdate(out, &strlPos, strlSize);
			hdrlSize += strlSize;

			if(snd) {
				fgetpos(out, &strlPos);
				hdrlSize += fwritechunk(FOURCC_LIST, 0, out);
				strlSize = fwritecc(FOURCC_STRL, out);

				if(mp3) {
					strlSize += fwritechunk(FOURCC_STRH, sizeof(strh), out);
					memset(&strh, 0, sizeof(strh));
					strh.type = FOURCC_AUDS;
					strh.scale = 1;
					strh.rate = mp3bitrate(mp3) * 1000 / 8;
					strh.quality = 10000;
					strh.initialFrames = 1;
					strh.length = avih.totalFrames * videoFrameLength / mp3framelength(mp3);
					strh.suggestedBufferSize = 1024*1024;
					strh.sampleSize = 1;
					strlSize += fwrite(&strh, 1, sizeof(strh), out);

					strlSize += fwritechunk(FOURCC_STRF, sizeof(mp3h), out);
					memset(&mp3h, 0, sizeof(mp3h));
					mp3h.wavh.format = WAVE_FORMAT_MPEGLAYER3;
					mp3h.wavh.channels = MPEGChannels(mp3) == MPEGChannelsMono ? 1 : 2;
					mp3h.wavh.samplesPerSec = mp3samplerate(mp3);
					mp3h.wavh.avgBytesPerSec = strh.rate;
					mp3h.wavh.blockAlign = 1;
					mp3h.size = sizeof(mp3h) - sizeof(mp3h.wavh) - sizeof(mp3h.size);
					mp3h.id = MPEGLAYER3_ID_MPEG;
					mp3h.flags = MPEGLAYER3_FLAG_PADDING_ISO;
					strlSize += fwrite(&mp3h, 1, sizeof(mp3h), out);
				} else {
					strlSize += fwritechunk(FOURCC_STRH, sizeof(strh), out);
					memset(&strh, 0, sizeof(strh));
					strh.type = FOURCC_AUDS;
					strh.scale = 253;
					strh.rate = wavh.samplesPerSec / wavh.bitsPerSample;
					strh.quality = (uint32_t)-1;
					strh.initialFrames = 0;
					strh.length = avih.totalFrames * wavh.samplesPerSec / fps / adpcmh.samplesPerBlock;
					strh.suggestedBufferSize = 12288 /* ??? FFmpeg tells so */;
					strh.sampleSize = wavh.blockAlign;
					strlSize += fwrite(&strh, 1, sizeof(strh), out);

					fsetpos(snd, &sndFmtPos);
					strlSize += fwritechunk(FOURCC_STRF, sndFmtSize, out);
					strlSize += fcopy(snd, out, sndFmtSize);
					fsetpos(snd, &sndDataPos);
				}

				fupdate(out, &strlPos, strlSize);
				hdrlSize += strlSize;
			}

		fupdate(out, &hdrlPos, hdrlSize);
		riffSize += hdrlSize;

		fgetpos(out, &moviPos);
		riffSize += fwritechunk(FOURCC_LIST, 0, out);
		moviSize = fwritecc(FOURCC_MOVI, out);

		while(1) {
			if(frame + argi >= argc) break;

			while(snd && audio < video + videoFrameLength * 2) {
				if(mp3) {
					/* read next mp3 frame */
					if(!(mp3 = freadmp3header(snd))) {
						fseek(snd, 0, SEEK_SET);
						mp3 = freadmp3header(snd);
					}
					size_t bufSize = mp3framesize(mp3) - sizeof(mp3);
					uint32_t buf[bufSize + 1];
					buf[bufSize] = 0;
					if(fread(buf, 1, bufSize, snd) == bufSize) {
						IDX1 idx1 = { CC("01wb"), 0, moviSize, bufSize + sizeof(mp3) };
						fwrite(&idx1, 1, sizeof(idx1), idx);
						moviSize += fwritechunk(CC("01wb"), bufSize + sizeof(mp3), out);
						moviSize += fwritemp3header(out, mp3);
						moviSize += fwrite(buf, 1, bufSize + (bufSize % 2), out);
					}
					audio += mp3framelength(mp3);
				} else {
					/* read next wav chunk */
					size_t copied;
					IDX1 idx1 = { CC("01wb"), 0, moviSize, wavh.blockAlign};
					fwrite(&idx1, 1, sizeof(idx1), idx);
					moviSize += fwritechunk(CC("01wb"), wavh.blockAlign, out);
					moviSize += copied = fcopy(snd, out, wavh.blockAlign);
					if(copied == 0) {
						fsetpos(snd, &sndDataPos);
						moviSize += fcopy(snd, out, wavh.blockAlign);
					}
					audio += (double)adpcmh.samplesPerBlock / (double)wavh.samplesPerSec;
				}
			}

			in = fopen(argv[frame + argi], "rb");
			if(!in) {
				moviSize += fwritechunk(CC("00dc"), 0, out);
			} else {
				IDX1 idx1 = { CC("00dc"), 0, moviSize, 0 };
				uint32_t buf[1024];
				fseek(in, 0, SEEK_END);
				idx1.size = ftell(in);
				fwrite(&idx1, 1, sizeof(idx1), idx);
				moviSize += fwritechunk(CC("00dc"), idx1.size, out);
				fseek(in, 0, SEEK_SET);
				while(1) {
					read = fread(buf, 1, sizeof(buf), in);
					if(read > 0) {
						/* NOTE: Little trick here, ensure we write 2 byte padded data */
						moviSize += fwrite(buf, 1, read + (read % 2), out);
					}
					if(read < sizeof(buf)) break;
				}
			}
			video += videoFrameLength;
			frame ++;
			fclose(in);
		}

		fupdate(out, &moviPos, moviSize);
		riffSize += moviSize;

		/* rewrite index */
		if(idx) {
			uint32_t buf[1024];
			moviSize += fwritechunk(FOURCC_IDX1, ftell(idx), out);
			fseek(idx, 0, SEEK_SET);
			while(1) {
				read = fread(buf, 1, sizeof(buf), idx);
				if(read > 0) {
					/* NOTE: Little trick here, ensure we write 2 byte padded data */
					riffSize += fwrite(buf, 1, read + (read % 2), out);
				}
				if(read < sizeof(buf)) break;
			}
		}

	fupdate(out, &riffPos, riffSize);

	if(out && out != stdout) fclose(out);
	if(idx) fclose(idx);
	if(snd) fclose(snd);

	return 0;
}