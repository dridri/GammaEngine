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

#ifndef OPENGLES20RENDERER_H
#define OPENGLES20RENDERER_H

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

class OpenGLES20Renderer : public Renderer
{
public:
	OpenGLES20Renderer( Instance* instance = nullptr );
	~OpenGLES20Renderer();

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

	const std::map< Object*, uint32_t >& objectsVerticesStart() const {return mObjectsVerticesStart; }
	void UpdateVertexArray( VertexBase* data, uint32_t offset, uint32_t count );

protected:
	uint8_t* loadShader( const std::string& filename, size_t* sz = 0 );
	void createPipeline();
	void VertexPoolAppend( VertexBase** pVertices, uint32_t& pVerticesPoolSize, uint32_t& pVerticesCount, VertexBase* append, uint32_t count );

	bool mReady;
	Instance* mInstance;

	Matrix* mMatrixProjection;
	Matrix* mMatrixView;
	float* mMatrixObjects;
	uint32_t mMatrixObjectsSize;
	std::vector< Object* > mObjects;
	std::vector< Light* > mLights;
	std::map< Object*, uint32_t > mObjectsVerticesStart;

	int mRenderMode;
	bool mDepthTestEnabled;
	bool mBlendingEnabled;
	int32_t mBlendingSrc;
	int32_t mBlendingDst;

	uint32_t mShader;
	uint32_t mVertexShader;
	uint32_t mFragmentShader;

	uint32_t mIBO;
	uint32_t mVBO;

	int32_t mVertexTexcoordID;
	int32_t mVertexColorID;
	int32_t mVertexNormalID;
	int32_t mVertexPositionID;
	int32_t mMatrixProjectionID;
	int32_t mMatrixViewID;
	int32_t mMatrixObjectID;
	int32_t mFloatTimeID;
	int32_t mIntInstanceID;

	static bool s2DActive;
};

#endif // OPENGLES20RENDERER_H
