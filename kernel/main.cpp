#include "frame_buffer_config.h"

extern "C" void KernelMain(const struct FrameBufferConfig *config)
{
	
	while ( 1 ) __asm__("hlt");
}
