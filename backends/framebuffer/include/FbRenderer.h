/*
 * The GammaEngine Library 2.0 is a multiplatform -based game engine
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

#ifndef FRAMEBUFFERRENDERER_H
#define FRAMEBUFFERRENDERER_H

#include <vector>
#include "Renderer.h"
#include "Object.h"
#include "Light.h"

namespace GE {
	class Instance;
	class Object;
	class Camera;
	class Matrix;
}
using namespace GE;

typedef struct DrawElementsIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t firstIndex;
	uint32_t baseVertex;
	uint32_t baseInstance;
} DrawElementsIndirectCommand;

typedef struct DrawArraysIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t baseInstance;
} DrawArraysIndirectCommand;

class FbRenderer : public Renderer
{
public:
	FbRenderer( Instance* instance = nullptr );
	~FbRenderer();

	virtual int LoadVertexShader( const std::string& file );
	virtual int LoadVertexShader( const void* data, size_t size );
	virtual int LoadGeometryShader( const std::string& file );
	virtual int LoadGeometryShader( const void* data, size_t size );
	virtual int LoadFragmentShader( const std::string& file );
	virtual int LoadFragmentShader( const void* data, size_t size );

	virtual void setRenderMode( const RenderMode& mode );
	virtual void setDepthTestEnabled( bool en );
	virtual void setBlendingEnabled( bool en );

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

protected:
	Instance* mInstance;

	Matrix* mMatrixProjection;
	Matrix* mMatrixView;
	float* mMatrixObjects;
	uint32_t mMatrixObjectsSize;
	std::vector< Object* > mObjects;
	std::vector< Light* > mLights;

	int mRenderMode;
	bool mDepthTestEnabled;
	bool mBlendingEnabled;

	static bool s2DActive;
};

#endif // FRAMEBUFFERRENDERER_H
