/* 

Amine Rehioui
Created: August 12th 2010

*/

#ifndef _SKY_BOX_ENTITY_H_INCLUDED_
#define _SKY_BOX_ENTITY_H_INCLUDED_

#include "CubeMapResource.h"

namespace shoot
{
	//! Sky Box entity
	class SkyBoxEntity : public Entity3D				 
	{
	public:

		//! Macro to provide type information
		DECLARE_OBJECT(SkyBoxEntity, Entity3D);

		//! constructor
		SkyBoxEntity();

		//! serializes the entity to/from a PropertyStream
		virtual void Serialize(PropertyStream& stream);

		//! called during the initialization of the entity
		virtual void Init();

		//! sets the cube map
		void SetCubeMap(std::string strPath);

	private:

		// properties
		Reference<CubeMapResource> m_CubeMap;
	};
}

#endif // _SKY_BOX_ENTITY_H_INCLUDED_

