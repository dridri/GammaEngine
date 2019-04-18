#ifndef GE_OPENGLES20RENDERBUFFER_H
#define GE_OPENGLES20RENDERBUFFER_H

#include "RenderBuffer.h"

namespace GE {

class OpenGLES20RenderBuffer : public RenderBuffer
{
public:
	OpenGLES20RenderBuffer( Instance* instance, uint32_t width, uint32_t height );
	~OpenGLES20RenderBuffer();

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

#endif // GE_OPENGLES20RENDERBUFFER_H
