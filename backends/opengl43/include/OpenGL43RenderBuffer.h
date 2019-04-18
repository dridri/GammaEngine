#ifndef GE_OPENGL43RENDERBUFFER_H
#define GE_OPENGL43RENDERBUFFER_H

#include "RenderBuffer.h"

namespace GE {

class OpenGL43RenderBuffer : public RenderBuffer
{
public:
	OpenGL43RenderBuffer( Instance* instance, uint32_t width, uint32_t height );
	~OpenGL43RenderBuffer();

	void Clear( bool color, bool depth, float r, float g, float b, float a );
	void Bind();
	void Unbind();

protected:
	Instance* mInstance;
	bool mReady;
	uint32_t mFBO;

	void Compute();
};

} // namespace GE

#endif // GE_OPENGL43RENDERBUFFER_H
