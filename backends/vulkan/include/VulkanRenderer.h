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
#include "vulkan.h"
#include "Renderer.h"
#include "VulkanObject.h"
#include "Light.h"

namespace GE {
	class Instance;
	class Object;
	class Camera;
	class Matrix;
}
using namespace GE;

class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer( Instance* instance = nullptr );
	~VulkanRenderer();

	virtual int LoadVertexShader( const std::string& file );
	virtual int LoadVertexShader( const void* data, size_t size );
	virtual int LoadFragmentShader( const std::string& file );
	virtual int LoadFragmentShader( const void* data, size_t size );
	void setRenderMode( int mode );

	void AddObject( Object* obj );
	virtual void AddLight( Light* light );

	void Compute();
	void Draw();
	void Look( Camera* cam );

	virtual Matrix* projectionMatrix();

	virtual uintptr_t attributeID( const std::string& name );
	virtual uintptr_t uniformID( const std::string& name );
	virtual void uniformUpload( const uintptr_t id, const float f );
	virtual void uniformUpload( const uintptr_t id, const Vector2f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector3f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector4f& v );
	virtual void uniformUpload( const uintptr_t id, const Matrix& v );

private:
	uint8_t* loadShader( const std::string& filename, size_t* sz );
	void createPipeline();

	bool mReady;
	Instance* mInstance;

	Matrix* mMatrixProjection;
	Matrix* mMatrixView;
	std::vector< VulkanObject* > mObjects;
	std::vector< Light* > mLights;

	VK_CMD_BUFFER mCmdBuffer;
	VK_PIPELINE mPipeline;
	VK_MEMORY_REF mPipelineRef;
	VK_SHADER mVertexShader;
	VK_SHADER mFragmentShader;

	int mRenderMode;
};

#endif // VULKANRENDERER_H
