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

#ifndef VULKANOBJECT_H
#define VULKANOBJECT_H

#include <string>
#include <vector>
#include <map>

#include <vulkan.h>

#include "Object.h"
#include "Vertex.h"
#include "Matrix.h"

using namespace GE;

class VulkanObject : public Object
{
public:
	VulkanObject( Vertex* verts = nullptr, uint32_t nVerts = 0 );
	VulkanObject( const std::string filename, Instance* instance = nullptr );
	~VulkanObject();

	virtual void setTexture( Instance* Instance, int unit, Image* texture );
	virtual void UpdateVertices( Instance* instance, Vertex* verts, uint32_t offset, uint32_t count );
	virtual void UploadMatrix( Instance* instance );

	VK_DESCRIPTOR_SET descriptorSet( Instance* instance );
	VK_MEMORY_REF verticesRef( Instance* instance );
	VK_MEMORY_REF indicesRef( Instance* instance );

protected:
	// descriptorSet, descriptorMemRef, vertexDataMemRef, indexMemRef, matrixMemRef associated to Instance*
	std::map< Instance*, std::tuple< VK_DESCRIPTOR_SET, VK_MEMORY_REF, VK_MEMORY_REF, VK_MEMORY_REF, VK_MEMORY_REF > > mVkRefs;

	void AllocateGpu( Instance* instance );
};


#endif // VULKANOBJECT_H
