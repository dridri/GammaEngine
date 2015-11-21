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

#ifndef OPENGLES20DEFERREDRENDERER_H
#define OPENGLES20DEFERREDRENDERER_H

#include <mutex>
#include <vector>
#include <unordered_map>
#include "DeferredRenderer.h"
#include "OpenGLES20Renderer2D.h"
#include "Object.h"
#include "Light.h"

namespace GE {
	class Instance;
	class Object;
	class Camera;
	class Matrix;
}
using namespace GE;

class OpenGLES20DeferredRenderer : public DeferredRenderer, private OpenGLES20Renderer2D
{
public:
	OpenGLES20DeferredRenderer( Instance* instance, uint32_t width, uint32_t height );
	~OpenGLES20DeferredRenderer();

	virtual void AddLight( Light* light );
	virtual void AddSunLight( Light* sun_light );
	virtual void setAmbientColor( const Vector4f& color );

	virtual void Compute();
	virtual void Bind();
	virtual void Unbind();
	virtual void Render();
	virtual void Look( Camera* cam );

	virtual void Update( Light* light = nullptr );

	virtual void AssociateSize( Window* window ) { mAssociatedWindow = window; };

private:
	typedef struct LightData {
		Vector4f position;
		Vector4f direction;
		Vector4f color;
		Vector4f data;
	} __attribute__((packed, aligned(16))) LightData;

	void ComputeCommandList();
	uint8_t* loadShader( const std::string& filename, size_t* sz = 0 );

	bool mCommandListReady;
	Vector4f mAmbientColor;
	std::vector< Light* > mLights;
	std::vector< Light* > mSunLights;
	LightData* mLightsData;
	std::unordered_map< Light*, uint32_t > mLightsDataIndices;

	Vertex* mSphereVertices;
	uint32_t mSphereVerticesCount;
	uint32_t* mSphereIndices;
	uint32_t mSphereIndicesCount;

	std::mutex mRenderMutex;
	uint32_t mLightsDataIDi;
	uint32_t mLightsDataID[2];
	uint32_t mVAOs[2];
	uint32_t mFBO;
	uint32_t mTextureDiffuse;
	uint32_t mTextureDepth;
	uint32_t mTextureNormals;
	uint32_t mTexturePositions;
};

#endif // OPENGLES20DEFERREDRENDERER_H
