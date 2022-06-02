#include "VulkanDeferredRenderer.h"
#include "VulkanFramebuffer.h"

extern "C" GE::DeferredRenderer* CreateDeferredRenderer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new VulkanDeferredRenderer( static_cast<VulkanInstance*>(instance), width, height );
}

VulkanDeferredRenderer::VulkanDeferredRenderer( VulkanInstance* instance, uint32_t width, uint32_t height )
// 	: VulkanRenderer2D( instance, width, height )
{
}


VulkanDeferredRenderer::~VulkanDeferredRenderer()
{
}


void VulkanDeferredRenderer::AddLight( Light* light )
{
}


void VulkanDeferredRenderer::AddSunLight( Light* sun_light )
{
}


void VulkanDeferredRenderer::setAmbientColor( const Vector4f& color )
{
}


void VulkanDeferredRenderer::setExtraBuffersCount( uint32_t count )
{
}


int VulkanDeferredRenderer::LoadVertexShader( const std::string& file )
{
}


int VulkanDeferredRenderer::LoadVertexShader( const void* data, size_t size )
{
}


int VulkanDeferredRenderer::LoadFragmentShader( const std::string& file )
{
}


int VulkanDeferredRenderer::LoadFragmentShader( const void* data, size_t size )
{
}


void VulkanDeferredRenderer::Compute()
{
}


void VulkanDeferredRenderer::Bind()
{
}


void VulkanDeferredRenderer::Unbind()
{
}


void VulkanDeferredRenderer::Render()
{
}


void VulkanDeferredRenderer::Look( Camera* cam )
{
}


Matrix* VulkanDeferredRenderer::projectionMatrix()
{
}


void VulkanDeferredRenderer::Update( Light* light )
{
}
