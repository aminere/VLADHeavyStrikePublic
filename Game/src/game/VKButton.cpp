/* 

Amine Rehioui
Created: November 11th 2013

*/

#include "ShootTest.h"

#include "VKButton.h"

#include "Text.h"

#include "EventManager.h"

namespace shoot
{
	DEFINE_OBJECT(VKButton);
	
	//! Constructor
	VKButton::VKButton()
		// properties
		: m_Letter("A")
	{
	}

	//! serializes the entity to/from a PropertyStream
	void VKButton::Serialize(PropertyStream& stream)
	{
		super::Serialize(stream);
		stream.Serialize(PT_String, "Letter", &m_Letter);
	}

	//! called during the initialization of the entity
	void VKButton::Init()
	{
		super::Init();

		if(Text* pLetter = static_cast<Text*>(GetChildByName("Letter")))
		{
			pLetter->SetText(m_Letter.c_str());
		}
	}

	//! returns true if the button is touched
	bool VKButton::IsTouched() const
	{
		Vector2 vTouchPos = InputManager::Instance()->GetTouchState().vPosition;
		Vector3 vPos = const_cast<VKButton*>(this)->GetTransformationMatrix().GetTranslation();
		AABBox2D bbox = GetBoundingBox() + Vector2(vPos.X, vPos.Y);
		return bbox.Contains(vTouchPos);
	}

	//! sends the UI event
	void VKButton::SendUIEvent()
	{
		UIEvent* pEvent = snew UIEvent();
		pEvent->m_Sender = this;
		pEvent->m_Command = std::string("VK ") + m_Letter;
		EventManager::Instance()->SendEvent(pEvent);
	}
}

