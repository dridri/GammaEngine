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

#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include <stdint.h>
#include "Vector.h"
#include "Vertex.h"
#include "Matrix.h"

namespace GE {

class Instance;
class Object;
class Light;
class Camera;
class VertexDefinition;

class Renderer
{
public:
	typedef enum {
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip
	} RenderMode;
	typedef enum {
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	} BlendingMode;

	virtual ~Renderer(){};

	virtual int LoadVertexShader( const std::string& file ) = 0;
	virtual int LoadVertexShader( const void* data, size_t size ) = 0;
	virtual int LoadGeometryShader( const std::string& file ) = 0;
	virtual int LoadGeometryShader( const void* data, size_t size ) = 0;
	virtual int LoadFragmentShader( const std::string& file ) = 0;
	virtual int LoadFragmentShader( const void* data, size_t size ) = 0;

	virtual void setVertexDefinition( const VertexDefinition& vertexDefinition ) = 0;
	virtual void setRenderMode( const RenderMode& mode ) = 0;
	virtual void setDepthTestEnabled( bool en ) = 0;
	virtual void setBlendingEnabled( bool en ) = 0;
	virtual void setBlendingMode( BlendingMode source, BlendingMode dest ) = 0;

	virtual void AddObject( Object* obj ) = 0;
	virtual void AddLight( Light* light ) = 0;

	virtual void Compute() = 0;
	virtual void Draw() = 0;
	virtual void Draw( uint32_t inddicesOffset, uint32_t indicesCount, uint32_t verticesOffset, uint32_t verticesCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0 ) = 0;
	virtual void Look( Camera* cam ) = 0;

	virtual Matrix* projectionMatrix() = 0;

	virtual uintptr_t attributeID( const std::string& name ) = 0;
	virtual uintptr_t uniformID( const std::string& name ) = 0;
	virtual void uniformUpload( const uintptr_t id, const float f ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector2f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector3f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector4f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Matrix& v ) = 0;

	virtual void UpdateVertexArray( VertexBase* data, uint32_t offset, uint32_t count ) = 0;
	virtual void UpdateIndicesArray( uint32_t* data, uint32_t offset, uint32_t count ) = 0;
};

} // namespace GE

#endif // RENDERER_H
