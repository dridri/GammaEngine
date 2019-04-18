#ifndef GE_RENDERBUFFER_H
#define GE_RENDERBUFFER_H

#include "Image.h"

namespace GE {

class Instance;
template <typename T> class ProxyWindow;
class BaseWindow;
typedef ProxyWindow< BaseWindow > Window;

class RenderBuffer
{
public:
	void AssociateSize( Window* window );

	virtual void Clear( bool color = true, bool depth = true, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f ) = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	Image* colorBuffer();
	Image* depthBuffer();

protected:
	RenderBuffer( uint32_t width, uint32_t height );
	~RenderBuffer();

	Window* mAssociatedWindow;
	uint32_t mWidth;
	uint32_t mHeight;
	Image* mColorBuffer;
	Image* mDepthBuffer;
};

} // namespace GE

#endif // GE_RENDERBUFFER_H
