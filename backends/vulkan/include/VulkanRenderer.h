/* WARNING
 * THIS IS A DRAFT, ALL THE REFERENCES USED HERE ARE PROBABLY ALL WRONG
 * All the code here is based on Mantle API samples, which Vulkan is more or less based on.
 * You should not use this file as a reference or example, and let it here
 * until Vulkan is officialy released
 * WARNING
 */

/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <vector>
#include <vulkan/vulkan.h>
#include "Renderer.h"
#include "VulkanObject.h"
#include "Light.h"

namespace GE {
	class Instance;
	class Object;
	class Camera;
	class Matrix;
	class VulkanFramebuffer;
}
using namespace GE;

class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer( Instance* instance = nullptr );
	~VulkanRenderer();

	virtual int LoadVertexShader( const std::string& file );
	virtual int LoadVertexShader( const void* data, size_t size );
	virtual int LoadGeometryShader( const std::string& file );
	virtual int LoadGeometryShader( const void* data, size_t size );
	virtual int LoadFragmentShader( const std::string& file );
	virtual int LoadFragmentShader( const void* data, size_t size );

	virtual void setVertexDefinition( const VertexDefinition& vertexDefinition );
	virtual void setRenderMode( const RenderMode& mode );
	virtual void setDepthTestEnabled( bool en );
	virtual void setBlendingEnabled( bool en );
	virtual void setBlendingMode( BlendingMode source, BlendingMode dest );

	virtual void AddObject( Object* obj );
	virtual void AddLight( Light* light );

	virtual void Compute();
	virtual void Draw();
	virtual void Look( Camera* cam );

	virtual Matrix* projectionMatrix();

	virtual uintptr_t attributeID( const std::string& name );
	virtual uintptr_t uniformID( const std::string& name );
	virtual void uniformUpload( const uintptr_t id, const float f );
	virtual void uniformUpload( const uintptr_t id, const Vector2f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector3f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector4f& v );
	virtual void uniformUpload( const uintptr_t id, const Matrix& v );

	void PopulateCommandBuffer( VkCommandBuffer buffer );

private:
	uint8_t* loadShader( const std::string& filename, size_t* sz );
	void createPipeline();
	void VertexPoolAppend( VertexBase** pVertices, uint32_t& pVerticesPoolSize, uint32_t& pVerticesCount, VertexBase* append, uint32_t count );

	bool mReady;
	VulkanInstance* mInstance;

	Matrix* mMatrixProjection;
	Matrix* mMatrixView;
	float* mMatrixObjects;
	VertexDefinition mVertexDefinition;
	std::vector< VulkanObject* > mObjects;
	std::vector< Light* > mLights;

	std::map< VulkanFramebuffer*, VkCommandBuffer > mRenderCommandBuffers;
	std::map< VulkanFramebuffer*, uint64_t > mFramebuffersHashes;
	VkCommandBuffer mCommandBuffer;
// 	VK_MEMORY_REF mPipelineRef;
	VkShaderModule mVertexShader;
	VkShaderModule mFragmentShader;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;

	VkDescriptorPool mDescriptorPool;
	std::pair< VkDeviceMemory, VkBuffer > mUniformsBuffer;
	VkDescriptorSet mUniformsDescriptorSet;

	uint32_t mTotalObjectsInstances;
	int mRenderMode;
};

#endif // VULKANRENDERER_H
