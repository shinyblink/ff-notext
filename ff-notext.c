// ┳━┓┳━┓  ┏┓┓┏━┓┏┓┓┳━┓┓ ┃┏┓┓
// ┣━ ┣━ ━━┃┃┃┃ ┃ ┃ ┣━ ┏╋┛ ┃
// ┇  ┇    ┇┗┛┛━┛ ┇ ┻━┛┇ ┗ ┇
// ff-notext: censor text using tesseract
// Usage: <farbfeld source> | ff-notext | <farbfeld sink>
// made by vifino. ISC (C) vifino 2019

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <tesseract/capi.h>

#include "conversion.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// IO helpers.
static inline void chew(FILE* file, void* buffer, size_t bytes) {
	if (!fread(buffer, bytes, 1, file)) {
		eprintf("wanted more bytes, didn't get any?\n");
		exit(1);
	}
}
static inline void spew(FILE* file, void* buffer, size_t bytes) {
	if (file)
		if (!fwrite(buffer, bytes, 1, file)) {
			eprintf("write failed.\n");
			exit(1);
		}
}

static void ffparse(FILE* food, FILE* out, uint32_t* w, uint32_t* h) {
	char buf[8];
	chew(food, buf, 8);
	if (strncmp(buf, "farbfeld", 8) != 0) {
		eprintf("file is not a farbfeld image?\n");
		exit(1);
	}
	spew(out, buf, 8);

	chew(food, buf, 8);
	*w = ntohl(*(uint32_t*)buf);
	*h = ntohl(*(uint32_t*)(buf + 4));
	if (!w || !h) {
		eprintf("image has zero dimension?\n");
		exit(1);
	}
	spew(out, buf, 8);
}

int main() {
	// initialize tesseract.
	TessBaseAPI* tess = TessBaseAPICreate();
	if (TessBaseAPIInit3(tess, NULL, "eng") != 0) {
		eprintf("[ff-notext] Error initializing Tesseract.");
		return 2;
	}
	//TessBaseAPISetVariable(tess, "tessedit_pageseg_mode", "11");
	//TessBaseAPISetPageSegMode(tess, PSM_AUTO_ONLY);
	TessBaseAPISetVariable(tess, "load_system_dawg", "false");
	TessBaseAPISetVariable(tess, "load_freq_dawg", "false");
	eprintf("[ff-notext] Initialized Tesseract %s\n", TessVersion());

	// parse input image
	uint32_t w, h;
	ffparse(stdin, stdout, &w, &h);

	uint16_t* frame = malloc(w * h * sizeof(uint16_t) * 4);
	if (!frame) return 2;

	uint8_t* rgba32 = malloc(w * h * 4);
	if (!rgba32) return 2;

	size_t x, y;
	uint16_t buf[4] = {0};
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			// nom.
			chew(stdin, buf, 8);
			size_t off = (x + (y * w)) * 4;
			frame[off + 0] = buf[0];
			frame[off + 1] = buf[1];
			frame[off + 2] = buf[2];
			frame[off + 3] = buf[3];

			if (buf[3]) { // only if alpha is set
#ifdef DOCONVERT
				qbeush2ush(buf, buf);
#endif
				rgba32[off + 0] = buf[0] / 256;
				rgba32[off + 1] = buf[1] / 256;
				rgba32[off + 2] = buf[2] / 256;
				rgba32[off + 3] = buf[3] / 256;
			} else {
				rgba32[off + 0] = 0;
				rgba32[off + 1] = 0;
				rgba32[off + 2] = 0;
				rgba32[off + 3] = 0;
			}
		}
	}

	// Look for text, kill it.
	eprintf("[ff-notext] Setting image..\n");
	TessBaseAPISetImage(tess, rgba32, w, h, 4, 4 * w);
	//TessBaseAPISetImage2(tess, image);
	if (TessBaseAPIGetSourceYResolution(tess) < 70) {
		TessBaseAPISetSourceResolution(tess, 70);
	}

	eprintf("[ff-notext] Recognizing...\n");
	if (TessBaseAPIRecognize(tess, NULL) != 0) {
		eprintf("[ff-notext] error in tesseract recognition\n");
		return 3;
	}

	eprintf("[ff-notext] Iterating over results...\n");
	TessResultIterator* ri = TessBaseAPIGetIterator(tess);
	TessPageIteratorLevel level = RIL_WORD;
	if (ri != 0) {
		do {
			int sx, sy;
		  int ex, ey;
			TessPageIteratorBoundingBox(ri, level, &sx, &sy, &ex, &ey);
			if (sx != 0 && sy != 0 && ex != w && ey != h)
				for (y = sy; y <= ey; y++)
					for (x = sx; x <= ex; x++) {
						int off = (x + (y*w)) * 4;
						frame[off + 0] = 0;
						frame[off + 1] = 0;
						frame[off + 2] = 0;
						frame[off + 3] = 0;
					}

			char* word = TessResultIteratorGetUTF8Text(ri, level);
			eprintf("[ff-notext] [(%u,%u),(%u,%u)}]: %s\n", sx, sy, ex, ey, word);
			free(word);
		} while (TessPageIteratorNext(ri, level));
	}
	free(rgba32);

	// Write out.
	spew(stdout, frame, w*h*sizeof(int16_t)*4);
	free(frame);
	return 0;
}
