#include <stdint.h>

class PixelWriter
{
private:
	FrameBufferConfig* m_f_buf_conf;

public:
	PixelWriter(FrameBufferConfig& conf);
	~PixelWriter();

	void Write(uint8_t pos_x, uint8_t pos_y);
}
