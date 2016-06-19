/* 

Amine Rehioui
Created: April 5th 2011

*/

#ifndef _ANIMATION_VISITOR_H_INCLUDED_
#define _ANIMATION_VISITOR_H_INCLUDED_

#include "Visitor.h"

namespace shoot
{
	//! AnimationVisitor class
	class AnimationVisitor : public Visitor
	{
	public:

		DECLARE_OBJECT(AnimationVisitor, Visitor);

		//! playback types
		enum E_PlaybackType
		{
			PT_Once,
			PT_Toggle,
			PT_Loop			
		};

		//! constructor
		AnimationVisitor();

		//! destructor
		virtual ~AnimationVisitor();

		//! Reads/Writes struct properties from/to a stream
		virtual void Serialize(PropertyStream& stream);		

		//! visits a particular entity
		virtual void Visit(Entity* pTarget);

	protected:

		s32 m_iPlayCount;

		// properties		
		E_PlaybackType m_ePlaybackType;
		s32 m_iMaxPlayCount;
	};
}

#endif // _ANIMATION_VISITOR_H_INCLUDED_

