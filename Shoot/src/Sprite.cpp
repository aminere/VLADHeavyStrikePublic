/* 

Amine Rehioui
Created: April 17th 2010

*/

#include "Shoot.h"

#include "Sprite.h"

#include "tinyxml2.h"

#include "MaterialProvider.h"

namespace shoot
{
	DEFINE_OBJECT(Sprite);

	//! Constructor
	Sprite::Sprite()
	: m_CurrentAnimation(0)
	, m_CurrentFrame(0)
	, m_bIsStopped(true)
	, m_bIsLooping(false)
	, m_fCurrentFrameDuration(0.0f)
	, m_ppAnimationLiterals(NULL)
	{
	}

	//! Destructor
	Sprite::~Sprite()
	{
		DeleteAnimations();
	}

	//! serializes the entity to/from a PropertyStream
	void Sprite::Serialize(PropertyStream& stream)
	{
		super::Serialize(stream);

		bool bPathChanged = stream.Serialize(PT_File, "Path", &m_strPath);
		bool bAnimChanged = stream.Serialize(PT_Enum, "Animation", &m_CurrentAnimation, ENUM_PARAMS1(m_ppAnimationLiterals));		

		// Update these properties in case they were changed by the editor after initialization
		if(IsInitialized())
		{
			if(bPathChanged)
			{
				SetPath(m_strPath);
			}		

			if(bAnimChanged)
			{
				SetCurrentAnimation(m_CurrentAnimation);
			}
		}				
	}

	//! called during the initialization of the entity
	void Sprite::Init()
	{
		Entity2D::Init();

		SHOOT_ASSERT(m_strPath != "", "Trying to create empty sprite");	

		if(m_aAnimations.GetSize() == 0)
		{
			SetPath(m_strPath);			
		}		
	}

	//! called during the update of the entity	
	void Sprite::Update()
	{
		if(!m_bIsStopped)
		{
			m_fCurrentFrameDuration += g_fDeltaTime;
			if(m_fCurrentFrameDuration > m_aAnimations[m_CurrentAnimation].fFrameDeltaTime)
			{
				m_CurrentFrame++;
				m_fCurrentFrameDuration = 0.0f;

				if(m_CurrentFrame == m_aAnimations[m_CurrentAnimation].aFrames.size())
				{					
					if(m_bIsLooping)
					{
						m_CurrentFrame = 0;
					}
					else
					{
						m_CurrentFrame--; // stay on last frame
						m_bIsStopped = true;
					}
				}

				Texture* pCurrentFrame = m_aAnimations[m_CurrentAnimation].aFrames[m_CurrentFrame].Get();
				GetComponent<GraphicComponent>()->GetMaterial()->SetTexture(0, pCurrentFrame);
				UpdateGeometry();
			}
		}
	}

	//! Reloads the sprite given a sprite resource path
	void Sprite::SetPath(std::string strPath)
	{
		tinyxml2::XMLDocument document;
		if(!document.LoadFile(strPath.c_str()))
		{
			// delete current animations, if any			
			DeleteAnimations();

			// start the serialization process for the new animations
			tinyxml2::XMLElement* pRoot = document.FirstChildElement();
			SHOOT_ASSERT(std::string(pRoot->Value()) == "Animations", "Invalid sprite file");
			PropertyStream animationsStream(SM_Read);
			animationsStream.ReadFromXML(pRoot);
			
			// read the animations
			animationsStream.SerializeArray("Animations", &m_aAnimations, PT_Struct);
				
			// Init the animations
			SHOOT_ASSERT(m_aAnimations.GetSize() > 0, "Sprite has no animations");
			m_ppAnimationLiterals = snew char*[m_aAnimations.GetSize()+1];
			for(u32 i=0; i<m_aAnimations.GetSize(); ++i)
			{
				m_aAnimations[i].Init();

				// Initialize the animation literals needed to show the animation list in the editor
				m_ppAnimationLiterals[i] = snew char[AnimationLiteralSize];
				strcpy(m_ppAnimationLiterals[i], m_aAnimations[i].name.c_str());
			}
			m_ppAnimationLiterals[m_aAnimations.GetSize()] = 0; // last literal should be null

			m_strPath = strPath;
			
			// create graphic component if not present
			GraphicComponent* pGraphic = GetComponent<GraphicComponent>();
			if(!pGraphic)
			{
				pGraphic = snew GraphicComponent();
				pGraphic->SetRenderingPass(GraphicComponent::RP_2D);
				AddComponent(pGraphic, true);
			}

			// sprites need a unique material, to handle frame changes
			pGraphic->SetMaterial(snew Material());
			pGraphic->GetMaterial()->SetFlag(Material::MF_AlphaBlending, true);

			// update geometry
			m_CurrentAnimation = 0;
			m_CurrentFrame = 0;
			Texture* pCurrentFrame = m_aAnimations[m_CurrentAnimation].aFrames[m_CurrentFrame].Get();
			pGraphic->GetMaterial()->SetTexture(0, pCurrentFrame);
			UpdateGeometry();
		}
		else
		{
			SHOOT_ASSERT(0, "Could not load file");
		}		
	}

	//! returns an animation name
	/** \param animIndex: specifies the animation index. Default is -1 to get the current animation name. */
	std::string Sprite::GetAnimationName(s32 animIndex /*= -1*/)
	{
		s32 index = (animIndex >= 0) ? animIndex : m_CurrentAnimation;
		return m_aAnimations[index].name;
	}

	//! sets the current animation from an animation name
	/** \param strName: name of the animation
		\param frameIndex: specifies the starting frame */
	void Sprite::SetCurrentAnimation(const char* strName, u32 frameIndex /*= 0*/)
	{
		for(u32 i=0; i<m_aAnimations.GetSize(); ++i)
		{
			if(m_aAnimations[i].name == strName)
			{
				SetCurrentAnimation(i, frameIndex);
				return;
			}
		}

		SHOOT_ASSERT(0, "Animation not found");
	}

	//! Sets the current animation from an animation index
	/** \param animIndex: index of the animation
		\param frameIndex: specifies the starting frame */
	void Sprite::SetCurrentAnimation(u32 animIndex, u32 frameIndex /*= 0*/)
	{
		m_CurrentAnimation = animIndex;
		m_CurrentFrame = frameIndex;
		m_fCurrentFrameDuration = 0.0f;
	}

	//! Sets the current frame
	void Sprite::SetCurrentFrame(u32 index)
	{
		m_CurrentFrame = index;
		m_fCurrentFrameDuration = 0.0f;

		if(m_bIsStopped)
		{
			Texture* pCurrentFrame = m_aAnimations[m_CurrentAnimation].aFrames[m_CurrentFrame].Get();
			GetComponent<GraphicComponent>()->GetMaterial()->SetTexture(0, pCurrentFrame);
			UpdateGeometry();
		}
	}

	//! Start playback at the specified frame
	/** \param bLoop: specifies if played in a loop
		\param frameIndex: specifies the starting frame. Default is -1 to continue at the current frame. */
	void Sprite::Play(bool bLoop /*= false*/, s32 frameIndex /*= -1*/)
	{
		if(frameIndex >= 0)
		{
			m_CurrentFrame = frameIndex;
		}

		m_fCurrentFrameDuration = 0.0f;
		m_bIsLooping = bLoop;
		m_bIsStopped = false;
	}

	//! stops the playback
	void Sprite::Stop()
	{
		m_bIsStopped = true;
	}

	//! Get the bounding box
	AABBox2D Sprite::GetBoundingBox() const
	{
		Texture* pCurrentFrame = m_aAnimations[m_CurrentAnimation].aFrames[m_CurrentFrame].Get();
		AABBox2D box(Vector2(0.0f, 0.0f), pCurrentFrame->GetSize());
		return box;
	}

	//! Deletes the animations
	void Sprite::DeleteAnimations()
	{
		// delete animation literals
		if(m_ppAnimationLiterals)
		{
			for(u32 i=0; i<m_aAnimations.GetSize(); ++i)
			{
				delete[] m_ppAnimationLiterals[i];
			}
			delete[] m_ppAnimationLiterals;
		}		

		m_aAnimations.Clear();
	}

	// Sprite::Animation functions
	//! Initializes the Animation frames
	void Sprite::Animation::Init()
	{
		fFrameDeltaTime = 1.0f / framesPerSecond;

		for(u32 i=0; i<aFramePaths.GetSize(); ++i)
		{
			Reference<Texture> frameRef(ResourceManager::Instance()->GetResource<Texture>(aFramePaths[i].c_str()));
			frameRef->SetHasMipMaps(false);
			aFrames.push_back(frameRef);
		}
	}	
}

