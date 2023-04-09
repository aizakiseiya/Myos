#pragma once 

#include <stdint.h>

enum PixelFormat {
	kPixelRGBResvBitPerColor,
	kPixelBGRResvBitPerColor,
};

typedef struct {
	uint8_t* frame_buffer;
	uint32_t pixcels_per_scan_line;
	uint32_t horizontal_resolution;
	uint32_t vertical_resolution;
	enum PixelFormat pixel_format;
} FrameBufferConfig;
