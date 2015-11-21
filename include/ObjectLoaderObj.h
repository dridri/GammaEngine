/*
 * <one line to give the library's name and an idea of what it does.>
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

#ifndef OBJECTLOADEROBJ_H
#define OBJECTLOADEROBJ_H


#include <map>
#include "Object.h"


namespace GE {

class File;
class Image;

class ObjectLoaderObj : public ObjectLoader
{
public:
	ObjectLoaderObj();
	virtual TYPE fileType();
	virtual uint32_t magic();
	virtual std::vector< std::string > contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual ObjectLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, bool static_ );
	virtual std::list< Object* > LoadObjects( Instance* instance, File* file, bool static_ );

private:
	typedef struct Material {
// 		uint32_t ambient;
// 		uint32_t diffuse;
// 		uint32_t specular;
		float ambient[4];
		float diffuse[4];
		float specular[4];
		Image* map_Kd;
		Image* map_bump;
	} Material;

	void LoadMaterials( Instance* instance, File* file, std::string filename );

	static ObjectLoaderObj* mBaseInstance;
	std::map< std::string, Material* > mMaterials;
};


} // namespace GE


#endif // OBJECTLOADEROBJ_H
