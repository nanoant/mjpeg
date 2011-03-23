/*
 * riff.h - MJPEG creator tool (https://github.com/nanoant/mjpeg)
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

#define SWAPL(x) (((x) >> 24) | (((x)&0x00ff0000) >> 8) | (((x)&0x0000ff00) << 8) | ((x) << 24))
#define SWAPS(x) (((x) >> 8) | ((x&0xff) << 8))
#define CC(s) (*((uint32_t *)(s)))

#define IS_CCSN(x)     (((x)&0x000000ff) >= '0' && ((x)&0x000000ff) <= '9' && (((x)&0x0000ff00) >> 8) >= '0' && (((x)&0x0000ff00) >> 8) <= '9' && (((x)&0x00ff0000) >> 16) >= 'a' && (((x)&0x00ff0000) >> 16) <= 'z' && ((x) >> 24) >= 'a' && ((x) >> 24) <= 'z')
#define CCSN(x)        ((((x)&0x000000ff) - '0') * 10 + ((((x)&0x0000ff00) >> 8) - '0'))
#define IS_CCSN_T(x,s) ((((x) >> 24) == s[1]) && ((((x)&0x00ff0000) >> 16) == s[0]))
#define CCSN_T(s,n)    (((uint32_t)s[1] << 24) | ((uint32_t)s[0] << 16) | (((((uint32_t)n) % 10)+'0') << 8) | ((((uint32_t)n) / 10)+'0'))

#define FOURCC_RIFF CC("RIFF")

#define FOURCC_AVI  CC("AVI ")
#define FOURCC_JUNK CC("JUNK")
#define FOURCC_LIST CC("LIST")
#define FOURCC_INFO CC("INFO")
#define FOURCC_DXDT CC("DXDT")
#define FOURCC_HDRL CC("hdrl")
#define FOURCC_AVIH CC("avih")
#define FOURCC_STRL CC("strl")
#define FOURCC_STRH CC("strh")
#define FOURCC_STRF CC("strf")
#define FOURCC_STRD CC("strd")
#define FOURCC_STRN CC("strn")
#define FOURCC_VIDS CC("vids")
#define FOURCC_AUDS CC("auds")
#define FOURCC_ODML CC("odml")
#define FOURCC_DMLH CC("dmlh")
#define FOURCC_MOVI CC("movi")
#define FOURCC_IDX1 CC("idx1")
#define FOURCC_VPRP CC("vprp")

#define FOURCC_WAVE CC("WAVE")
#define FOURCC_FMT  CC("fmt ")
#define FOURCC_DATA CC("data")

typedef uint32_t FOURCC;

typedef struct {
	FOURCC   fcc;
	uint32_t size;
} __attribute__((packed)) CHNK;

typedef struct {
	uint32_t microSecPerFrame;
	uint32_t maxBytesPerSec;
	uint32_t paddingGranularity;
	uint32_t flags;
	uint32_t totalFrames;
	uint32_t initialFrames;
	uint32_t streams;
	uint32_t suggestedBufferSize;
	uint32_t width;
	uint32_t height;
	uint32_t reserved[4];
} __attribute__((packed)) AVIH;

#define AVIF_HASINDEX       0x00000010 /* Index at end of file? */
#define AVIF_MUSTUSEINDEX   0x00000020
#define AVIF_ISINTERLEAVED  0x00000100
#define AVIF_TRUSTCKTYPE    0x00000800 /* Use CKType to find key frames? */
#define AVIF_WASCAPTUREFILE 0x00010000
#define AVIF_COPYRIGHTED    0x00020000

typedef struct {
	FOURCC   type;
	FOURCC   handler;
	uint32_t flags;
	uint16_t priority;
	uint16_t language;
	uint32_t initialFrames;
	uint32_t scale;
	uint32_t rate;
	uint32_t start;
	uint32_t length;
	uint32_t suggestedBufferSize;
	uint32_t quality;
	uint32_t sampleSize;
	struct {
		uint16_t left;
		uint16_t top;
		uint16_t right;
		uint16_t bottom;
	} __attribute__((packed)) frame;
} __attribute__((packed)) STRH;

typedef struct {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
} __attribute__((packed)) RGBA;

typedef struct {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} __attribute__((packed)) RGB;

typedef struct {
	uint32_t size;
	int32_t  width;
	int32_t  height;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t imgSize;
	int32_t  xPelsPerMeter;
	int32_t  yPelsPerMeter;
	uint32_t clrUsed;
	uint32_t clrImportant;
} __attribute__((packed)) BMPH;

typedef struct {
	uint16_t format;
	uint16_t channels;
	uint32_t samplesPerSec;
	uint32_t avgBytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
} __attribute__((packed)) WAVH;

typedef struct {
	uint16_t samplesPerBlock;
	uint16_t numCoef;
} __attribute__((packed)) ADPCMH;

#define MPEGLAYER3_ID_MPEG 1

#define MPEGLAYER3_FLAG_PADDING_ISO 0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON  0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF 0x00000002

typedef struct {
	WAVH wavh;
	uint16_t size;
	uint16_t id;
	uint32_t flags;
	uint16_t blockSize;
	uint16_t framesPerBlock;
	uint16_t codecDelay;
} MP3H;

typedef struct {
	uint32_t id;
	uint32_t flags;
	uint32_t offset;
	uint32_t size;
} __attribute__((packed)) IDX1;

typedef struct {
	char time[27];
	uint16_t width;
	uint16_t height;
	uint16_t tlx;
	uint16_t tly;
	uint16_t brx;
	uint16_t bry;
	uint16_t size;
	RGB      colors[4];
} __attribute__((packed)) XSUB;

typedef struct {
	uint32_t compressedBMHeight;   /* 320 */
	uint32_t compressedBMWidth;    /* 480 */
	uint32_t validBMHeight;        /* 320 */
	uint32_t validBMWidth;         /* 480 */
	uint32_t validBMXOffset;       /* 0 */
	uint32_t validBMYOffset;       /* 0 */
	uint32_t videoXOffsetInT;      /* 0 */
	uint32_t videoYValidStartLine; /* 0 */
} __attribute__((packed)) VIDEO_FIELD_DESC;

typedef struct {
	uint32_t videoFormatToken;     /* 0 */
	uint32_t videoStandard;        /* 0 */
	uint32_t verticalRefreshRate;  /* 20 */
	uint32_t hTotalInT;            /* 320 */
	uint32_t vTotalInLines;        /* 480 */
	uint32_t frameAspectRatio;     /* ASPECT_3_2 */
	uint32_t frameWidthInPixels;   /* 320 */
	uint32_t frameHeightInLines;   /* 480 */
	uint32_t fieldsPerFrame;       /* 1 */
	VIDEO_FIELD_DESC field;
} __attribute__((packed)) VPRP;

#define ASPECT_4_3 0x00040003
#define ASPECT_3_2 0x00030002

#define AVIIF_LIST          0x00000001 /* chunk is a 'LIST' */
#define AVIIF_KEYFRAME      0x00000010 /* this frame is a key frame.*/
#define AVIIF_NOTIME        0x00000100 /* this frame doesn't take any time */

#define WAVE_FORMAT_UNKNOWN                 0x0000 /* Microsoft Corporation */
#define WAVE_FORMAT_PCM                     0x0001 /* Microsoft Corporation */
#define WAVE_FORMAT_ADPCM                   0x0002 /* Microsoft Corporation */
#define WAVE_FORMAT_IEEE_FLOAT              0x0003 /* Microsoft Corporation */
#define WAVE_FORMAT_VSELP                   0x0004 /* Compaq Computer Corp. */
#define WAVE_FORMAT_IBM_CVSD                0x0005 /* IBM Corporation */
#define WAVE_FORMAT_ALAW                    0x0006 /* Microsoft Corporation */
#define WAVE_FORMAT_MULAW                   0x0007 /* Microsoft Corporation */
#define WAVE_FORMAT_DTS                     0x0008 /* Microsoft Corporation */
#define WAVE_FORMAT_OKI_ADPCM               0x0010 /* OKI */
#define WAVE_FORMAT_DVI_ADPCM               0x0011 /* Intel Corporation */
#define WAVE_FORMAT_IMA_ADPCM               (WAVE_FORMAT_DVI_ADPCM) /*  Intel Corporation */
#define WAVE_FORMAT_MEDIASPACE_ADPCM        0x0012 /* Videologic */
#define WAVE_FORMAT_SIERRA_ADPCM            0x0013 /* Sierra Semiconductor Corp */
#define WAVE_FORMAT_G723_ADPCM              0x0014 /* Antex Electronics Corporation */
#define WAVE_FORMAT_DIGISTD                 0x0015 /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIGIFIX                 0x0016 /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIALOGIC_OKI_ADPCM      0x0017 /* Dialogic Corporation */
#define WAVE_FORMAT_MEDIAVISION_ADPCM       0x0018 /* Media Vision, Inc. */
#define WAVE_FORMAT_CU_CODEC                0x0019 /* Hewlett-Packard Company */
#define WAVE_FORMAT_YAMAHA_ADPCM            0x0020 /* Yamaha Corporation of America */
#define WAVE_FORMAT_SONARC                  0x0021 /* Speech Compression */
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH     0x0022 /* DSP Group, Inc */
#define WAVE_FORMAT_ECHOSC1                 0x0023 /* Echo Speech Corporation */
#define WAVE_FORMAT_AUDIOFILE_AF36          0x0024 /* Virtual Music, Inc. */
#define WAVE_FORMAT_APTX                    0x0025 /* Audio Processing Technology */
#define WAVE_FORMAT_AUDIOFILE_AF10          0x0026 /* Virtual Music, Inc. */
#define WAVE_FORMAT_PROSODY_1612            0x0027 /* Aculab plc */
#define WAVE_FORMAT_LRC                     0x0028 /* Merging Technologies S.A. */
#define WAVE_FORMAT_DOLBY_AC2               0x0030 /* Dolby Laboratories */
#define WAVE_FORMAT_GSM610                  0x0031 /* Microsoft Corporation */
#define WAVE_FORMAT_MSNAUDIO                0x0032 /* Microsoft Corporation */
#define WAVE_FORMAT_ANTEX_ADPCME            0x0033 /* Antex Electronics Corporation */
#define WAVE_FORMAT_CONTROL_RES_VQLPC       0x0034 /* Control Resources Limited */
#define WAVE_FORMAT_DIGIREAL                0x0035 /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIGIADPCM               0x0036 /* DSP Solutions, Inc. */
#define WAVE_FORMAT_CONTROL_RES_CR10        0x0037 /* Control Resources Limited */
#define WAVE_FORMAT_NMS_VBXADPCM            0x0038 /* Natural MicroSystems */
#define WAVE_FORMAT_CS_IMAADPCM             0x0039 /* Crystal Semiconductor IMA ADPCM */
#define WAVE_FORMAT_ECHOSC3                 0x003A /* Echo Speech Corporation */
#define WAVE_FORMAT_ROCKWELL_ADPCM          0x003B /* Rockwell International */
#define WAVE_FORMAT_ROCKWELL_DIGITALK       0x003C /* Rockwell International */
#define WAVE_FORMAT_XEBEC                   0x003D /* Xebec Multimedia Solutions Limited */
#define WAVE_FORMAT_G721_ADPCM              0x0040 /* Antex Electronics Corporation */
#define WAVE_FORMAT_G728_CELP               0x0041 /* Antex Electronics Corporation */
#define WAVE_FORMAT_MSG723                  0x0042 /* Microsoft Corporation */
#define WAVE_FORMAT_MPEG                    0x0050 /* Microsoft Corporation */
#define WAVE_FORMAT_RT24                    0x0052 /* InSoft, Inc. */
#define WAVE_FORMAT_PAC                     0x0053 /* InSoft, Inc. */
#define WAVE_FORMAT_MPEGLAYER3              0x0055 /* ISO/MPEG Layer3 Format Tag */
#define WAVE_FORMAT_LUCENT_G723             0x0059 /* Lucent Technologies */
#define WAVE_FORMAT_CIRRUS                  0x0060 /* Cirrus Logic */
#define WAVE_FORMAT_ESPCM                   0x0061 /* ESS Technology */
#define WAVE_FORMAT_VOXWARE                 0x0062 /* Voxware Inc */
#define WAVE_FORMAT_CANOPUS_ATRAC           0x0063 /* Canopus, co., Ltd. */
#define WAVE_FORMAT_G726_ADPCM              0x0064 /* APICOM */
#define WAVE_FORMAT_G722_ADPCM              0x0065 /* APICOM */
#define WAVE_FORMAT_DSAT_DISPLAY            0x0067 /* Microsoft Corporation */
#define WAVE_FORMAT_VOXWARE_BYTE_ALIGNED    0x0069 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC8             0x0070 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC10            0x0071 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC16            0x0072 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC20            0x0073 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT24            0x0074 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT29            0x0075 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT29HW          0x0076 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_VR12            0x0077 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_VR18            0x0078 /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_TQ40            0x0079 /* Voxware Inc */
#define WAVE_FORMAT_SOFTSOUND               0x0080 /* Softsound, Ltd. */
#define WAVE_FORMAT_VOXWARE_TQ60            0x0081 /* Voxware Inc */
#define WAVE_FORMAT_MSRT24                  0x0082 /* Microsoft Corporation */
#define WAVE_FORMAT_G729A                   0x0083 /* AT&T Labs, Inc. */
#define WAVE_FORMAT_MVI_MVI2                0x0084 /* Motion Pixels */
#define WAVE_FORMAT_DF_G726                 0x0085 /* DataFusion Systems (Pty) (Ltd) */
#define WAVE_FORMAT_DF_GSM610               0x0086 /* DataFusion Systems (Pty) (Ltd) */
#define WAVE_FORMAT_ISIAUDIO                0x0088 /* Iterated Systems, Inc. */
#define WAVE_FORMAT_ONLIVE                  0x0089 /* OnLive! Technologies, Inc. */
#define WAVE_FORMAT_SBC24                   0x0091 /* Siemens Business Communications Sys */
#define WAVE_FORMAT_DOLBY_AC3_SPDIF         0x0092 /* Sonic Foundry */
#define WAVE_FORMAT_MEDIASONIC_G723         0x0093 /* MediaSonic */
#define WAVE_FORMAT_PROSODY_8KBPS           0x0094 /* Aculab plc */
#define WAVE_FORMAT_ZYXEL_ADPCM             0x0097 /* ZyXEL Communications, Inc. */
#define WAVE_FORMAT_PHILIPS_LPCBB           0x0098 /* Philips Speech Processing */
#define WAVE_FORMAT_PACKED                  0x0099 /* Studer Professional Audio AG */
#define WAVE_FORMAT_MALDEN_PHONYTALK        0x00A0 /* Malden Electronics Ltd. */
#define WAVE_FORMAT_RHETOREX_ADPCM          0x0100 /* Rhetorex Inc. */
#define WAVE_FORMAT_IRAT                    0x0101 /* BeCubed Software Inc. */
#define WAVE_FORMAT_VIVO_G723               0x0111 /* Vivo Software */
#define WAVE_FORMAT_VIVO_SIREN              0x0112 /* Vivo Software */
#define WAVE_FORMAT_DIGITAL_G723            0x0123 /* Digital Equipment Corporation */
#define WAVE_FORMAT_SANYO_LD_ADPCM          0x0125 /* Sanyo Electric Co., Ltd. */
#define WAVE_FORMAT_SIPROLAB_ACEPLNET       0x0130 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_ACELP4800      0x0131 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_ACELP8V3       0x0132 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_G729           0x0133 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_G729A          0x0134 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_KELVIN         0x0135 /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_G726ADPCM               0x0140 /* Dictaphone Corporation */
#define WAVE_FORMAT_QUALCOMM_PUREVOICE      0x0150 /* Qualcomm, Inc. */
#define WAVE_FORMAT_QUALCOMM_HALFRATE       0x0151 /* Qualcomm, Inc. */
#define WAVE_FORMAT_TUBGSM                  0x0155 /* Ring Zero Systems, Inc. */
#define WAVE_FORMAT_MSAUDIO1                0x0160 /* Microsoft Corporation */
#define WAVE_FORMAT_CREATIVE_ADPCM          0x0200 /* Creative Labs, Inc */
#define WAVE_FORMAT_CREATIVE_FASTSPEECH8    0x0202 /* Creative Labs, Inc */
#define WAVE_FORMAT_CREATIVE_FASTSPEECH10   0x0203 /* Creative Labs, Inc */
#define WAVE_FORMAT_UHER_ADPCM              0x0210 /* UHER informatic GmbH */
#define WAVE_FORMAT_QUARTERDECK             0x0220 /* Quarterdeck Corporation */
#define WAVE_FORMAT_ILINK_VC                0x0230 /* I-link Worldwide */
#define WAVE_FORMAT_RAW_SPORT               0x0240 /* Aureal Semiconductor */
#define WAVE_FORMAT_IPI_HSX                 0x0250 /* Interactive Products, Inc. */
#define WAVE_FORMAT_IPI_RPELP               0x0251 /* Interactive Products, Inc. */
#define WAVE_FORMAT_CS2                     0x0260 /* Consistent Software */
#define WAVE_FORMAT_SONY_SCX                0x0270 /* Sony Corp. */
#define WAVE_FORMAT_FM_TOWNS_SND            0x0300 /* Fujitsu Corp. */
#define WAVE_FORMAT_BTV_DIGITAL             0x0400 /* Brooktree Corporation */
#define WAVE_FORMAT_QDESIGN_MUSIC           0x0450 /* QDesign Corporation */
#define WAVE_FORMAT_VME_VMPCM               0x0680 /* AT&T Labs, Inc. */
#define WAVE_FORMAT_TPC                     0x0681 /* AT&T Labs, Inc. */
#define WAVE_FORMAT_OLIGSM                  0x1000 /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLIADPCM                0x1001 /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLICELP                 0x1002 /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLISBC                  0x1003 /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLIOPR                  0x1004 /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_LH_CODEC                0x1100 /* Lernout & Hauspie */
#define WAVE_FORMAT_NORRIS                  0x1400 /* Norris Communications, Inc. */
#define WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS 0x1500 /* AT&T Labs, Inc. */
#define WAVE_FORMAT_DVM                     0x2000 /* FAST Multimedia AG */
#define WAVE_FORMAT_EXTENSIBLE              0xFFFE /* Microsoft */

const char *fourcc(FOURCC fcc);
int freadchunk(FOURCC *fcc, uint32_t *size, FILE *in);
int freadcc(FOURCC *fcc, FILE *in);
size_t fcopy(FILE *in, FILE *out, uint32_t size);
size_t fwritechunk(FOURCC fcc, uint32_t size, FILE *out);
size_t fwritecc(FOURCC fcc, FILE *out);
size_t fwritesafe(const void *ptr, size_t size, FILE *out);
long fseeksafe(FILE *out, long pos, int whence);
void fgetpossafe(FILE *out, fpos_t *pos);
int fupdate(FILE *out, fpos_t *pos, uint32_t value);
