#include "OpenGLES20RenderBuffer.h"
#include "OpenGLES20Instance.h"
#include "OpenGLES20Window.h"

using namespace GE;


extern "C" GE::RenderBuffer* CreateRenderBuffer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new OpenGLES20RenderBuffer( instance, width, height );
}


OpenGLES20RenderBuffer::OpenGLES20RenderBuffer( Instance* instance, uint32_t width, uint32_t height )
	: RenderBuffer( width, height )
	, mInstance( instance )
	, mReady( false )
{
}


OpenGLES20RenderBuffer::~OpenGLES20RenderBuffer()
{
}


void OpenGLES20RenderBuffer::Clear( bool color, bool depth, float r, float g, float b, float a )
{
	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
	glClearColor( r, g, b, a );
	glClear( ( color ? GL_COLOR_BUFFER_BIT : 0 ) | ( depth ? GL_DEPTH_BUFFER_BIT : 0 ) );
}


void OpenGLES20RenderBuffer::Compute()
{
	glGenFramebuffers( 1, (GLuint*)&mFBO );
	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer->serverReference( mInstance ), 0 );

	glBindTexture( GL_TEXTURE_2D, mDepthBuffer->serverReference( mInstance ) );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mDepthBuffer->width(), mDepthBuffer->height(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr );
// 	glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthBuffer->serverReference( mInstance ), 0 );

	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


void OpenGLES20RenderBuffer::Bind()
{
	if ( mAssociatedWindow != nullptr and ( mAssociatedWindow->width() != mWidth or mAssociatedWindow->height() != mHeight ) ) {
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		// TODO
		mReady = false;
	}

	if ( not mReady ) {
		mReady = true;
		Compute();
	}

	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
}


void OpenGLES20RenderBuffer::Unbind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}
