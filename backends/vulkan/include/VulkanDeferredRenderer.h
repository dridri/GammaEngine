#ifndef VULKANDEFERREDRENDERER_H
#define VULKANDEFERREDRENDERER_H

#include <stdint.h>
#include <string>
#include "VulkanInstance.h"
#include "DeferredRenderer.h"
#include "Window.h"

namespace GE {

class VulkanDeferredRenderer : public DeferredRenderer
{
public:
	VulkanDeferredRenderer( VulkanInstance* instance, uint32_t width, uint32_t height );
	~VulkanDeferredRenderer();

	virtual void AddLight( Light* light );
	virtual void AddSunLight( Light* sun_light );
	virtual void setAmbientColor( const Vector4f& color );
	virtual void setExtraBuffersCount( uint32_t count );

	virtual int LoadVertexShader( const std::string& file );
	virtual int LoadVertexShader( const void* data, size_t size );
	virtual int LoadFragmentShader( const std::string& file );
	virtual int LoadFragmentShader( const void* data, size_t size );

	virtual void Compute();
	virtual void Bind();
	virtual void Unbind();
	virtual void Render();
	virtual void Look( Camera* cam );

	virtual Matrix* projectionMatrix();
	virtual void Update( Light* light = nullptr );

	virtual void AssociateSize( GE::Window* window ) { mAssociatedWindow = window; };

protected:
	GE::Window* mAssociatedWindow;
};

} // namespace GE

#endif // VULKANDEFERREDRENDERER_H
