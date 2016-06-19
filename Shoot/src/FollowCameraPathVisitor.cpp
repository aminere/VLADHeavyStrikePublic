/* 

Amine Rehioui
Created: August 14th 2011

*/

#include "Shoot.h"	

#include "FollowCameraPathVisitor.h"

#include "Camera.h"
#include "CameraPath.h"

namespace shoot
{
	DEFINE_OBJECT(FollowCameraPathVisitor);

	//! constructor
	FollowCameraPathVisitor::FollowCameraPathVisitor()
		: m_pCamera(NULL)
		, m_fTimer(0.0f)
		, m_fDuration(0.0f)
		, m_CurrentElement(0)
	{
	}

	//! Reads/Writes struct properties from/to a stream
	void FollowCameraPathVisitor::Serialize(PropertyStream& stream)
	{
		super::Serialize(stream);

		stream.Serialize(PT_Link, "CameraPath", &m_Path);
	}

	//! visits a particular entity
	void FollowCameraPathVisitor::Visit(Entity* pTarget)
	{
		m_pCamera = DYNAMIC_CAST(pTarget, Camera);		
		SHOOT_ASSERT(m_pCamera, "FollowCameraPathVisitor target is not of type Camera");

		m_Path.Init(pTarget);

		if(m_pCamera && m_Path.Get() && (m_Path->GetChildCount() > 1))
		{
			m_CurrentElement = 0;
			m_fTimer = 0.0f;
			m_fDuration = m_Path->GetElement(m_CurrentElement)->GetFollowDuration();

			super::Visit(pTarget);
		}
	}

	//! updates the visitor	
	void FollowCameraPathVisitor::Update()
	{
		if(!m_bActive)
		{
			return;
		}

		f32 fRatio = m_fTimer/m_fDuration;
		m_pCamera->SetPosition(m_Path->GetPosition(m_CurrentElement, m_CurrentElement+1, fRatio));
		m_pCamera->SetLookAt(m_Path->GetLookAt(m_CurrentElement, m_CurrentElement+1, fRatio));

		if(m_fTimer < m_fDuration)
		{
			m_fTimer += g_fDeltaTime;
		}
		else if(m_CurrentElement < s32(m_Path->GetChildCount())-2)
		{
			++m_CurrentElement;
			f32 newDuration = m_Path->GetElement(m_CurrentElement)->GetFollowDuration();
			f32 fOverTime = (m_fTimer-m_fDuration)*(m_fDuration/newDuration);			
			m_fDuration = newDuration-fOverTime;
			m_fTimer = fOverTime + g_fDeltaTime;
		}
		else
		{
			m_iPlayCount++;
			switch(m_ePlaybackType)
			{
			case PT_Once:
				Leave();
				break;

			case PT_Loop:
				if((m_iMaxPlayCount < 0) || (m_iPlayCount < m_iMaxPlayCount))
				{
					m_CurrentElement = 0;
					f32 newDuration = m_Path->GetElement(m_CurrentElement)->GetFollowDuration();
					f32 fOverTime = (m_fTimer-m_fDuration)*(m_fDuration/newDuration);			
					m_fDuration = newDuration-fOverTime;
					m_fTimer = fOverTime + g_fDeltaTime;
				}
				else
				{
					Leave();
				}
				break;

			case PT_Toggle:
				SHOOT_WARNING(false, "FollowCameraPathVisitor Unsupported PlaybackType: PT_Toggle");
				Leave();
				break;
				
			default:
				Leave();
			}
		}		
	}
}

