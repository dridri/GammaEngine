#include "OpenGL43RenderBuffer.h"
#include "OpenGL43Instance.h"
#include "OpenGL43Window.h"

using namespace GE;


extern "C" GE::RenderBuffer* CreateRenderBuffer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new OpenGL43RenderBuffer( instance, width, height );
}


OpenGL43RenderBuffer::OpenGL43RenderBuffer( Instance* instance, uint32_t width, uint32_t height )
	: RenderBuffer( width, height )
	, mInstance( instance )
	, mReady( false )
{
}


OpenGL43RenderBuffer::~OpenGL43RenderBuffer()
{
}


void OpenGL43RenderBuffer::Clear( bool color, bool depth, float r, float g, float b, float a )
{
	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
	glClearColor( r, g, b, a );
	glClear( ( color ? GL_COLOR_BUFFER_BIT : 0 ) | ( depth ? GL_DEPTH_BUFFER_BIT : 0 ) );
}


void OpenGL43RenderBuffer::Compute()
{
	glGenFramebuffers( 1, (GLuint*)&mFBO );
	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer->serverReference( mInstance ), 0 );

	glBindTexture( GL_TEXTURE_2D, mDepthBuffer->serverReference( mInstance ) );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mDepthBuffer->width(), mDepthBuffer->height(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthBuffer->serverReference( mInstance ), 0 );

	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


void OpenGL43RenderBuffer::Bind()
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


void OpenGL43RenderBuffer::Unbind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}
