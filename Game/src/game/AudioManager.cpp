/* 

Amine Rehioui
Created: August 28th 2013

*/

#include "ShootTest.h"

#include "AudioManager.h"

#include "AudioDriver.h"

#include "GameManager.h"

namespace shoot
{
	// Sound literals
	const char* const g_strSoundLiterals[] =
	{
		"S_Pulse",
		"S_Laser",
		"S_Explosion",
		"S_LongExplosion",
		"S_None",
		0
	};
	
	DEFINE_OBJECT(AudioManager);

	//! static vars
	AudioManager* AudioManager::ms_pInstance = NULL;

	//! constructor
	AudioManager::AudioManager()
		: m_MusicType(M_None)
		, m_fMusicVolume(1.0f)
		// properties
		, m_fInGameMusicFactor(.5f)
	{
		SHOOT_ASSERT(!ms_pInstance, "Multiple AudioManager instances detected");
		ms_pInstance = this;

		m_MetalTracks.push_back("aminor_metal.ogg");
		m_MetalTracks.push_back("bminor_metal.ogg");
		m_MetalTracks.push_back("metal_theme_trailer.ogg");
		m_MetalTracks.push_back("heavy-theme.ogg");
		m_MetalTracks.push_back("nu-metal.ogg");

		for(u32 i=0; i<m_MetalTracks.size(); ++i)
		{
			m_RemainingMetalTracks.push_back(i);
		}
	}

	//! destructor
	AudioManager::~AudioManager()
	{
		ms_pInstance = NULL;
	}

	//! serializes the entity to/from a PropertyStream
	void AudioManager::Serialize(PropertyStream& stream)
	{
		super::Serialize(stream);
		
		stream.Serialize(PT_Float, "InGameMusicFactor", &m_fInGameMusicFactor);
		stream.SerializeArray("SoundInfos", &m_SoundInfos, PT_Struct);
	}	

	//! called during the update of the entity
	void AudioManager::Update()
	{
		if(m_Music.IsValid() && !m_Music->IsPlaying())
		{
			//if(m_MusicType == M_Metal)
			{
				SetMusicVolume(m_fMusicVolume);
				m_Music = PickMetalTrack();
				m_Music->Play();
			}
		}
	}

	//! Reads/Writes struct properties from/to a stream
	void AudioManager::SoundInfo::Serialize(PropertyStream& stream)
	{
		stream.Serialize(PT_Enum, "Sound", &m_Sound, ENUM_PARAMS1(g_strSoundLiterals));
		stream.Serialize(PT_File, "Bank", &m_Bank);
		stream.SerializeArray("Names", &m_Names, PT_String);
	}
	
	//! Plays a sound
	void AudioManager::Play(E_Sound sound, bool bStopIfPlaying /*= false*/)
	{
		bool bExists = m_SoundMap.find(sound) != m_SoundMap.end();
		if(!bExists)
		{
			if(SoundInfo* pInfo = GetSoundInfo(sound))
			{
				bool bBankExists = m_SoundBankMap.find(pInfo->m_Bank) != m_SoundBankMap.end();
				SoundBank* pBank = bBankExists ? m_SoundBankMap[pInfo->m_Bank].Get() : AudioDriver::Instance()->CreateSoundBank(pInfo->m_Bank.c_str());
				for(u32 i=0; i<pInfo->m_Names.GetSize(); ++i)
				{
					Sound* pSound = AudioDriver::Instance()->CreateSound(pBank, pInfo->m_Names[i].c_str());
					m_SoundMap[sound].push_back(pSound);
				}
				u32 index = Random::GetInt(0, pInfo->m_Names.GetSize()-1);
				m_SoundMap[sound][index]->Play();

				if(!bBankExists)
				{
					m_SoundBankMap[pInfo->m_Bank] = pBank;
				}
			}
		}
		else
		{
			if(bStopIfPlaying)
			{
				u32 index = Random::GetInt(0, m_SoundMap[sound].size()-1);
				m_SoundMap[sound][index]->Play();
			}
			else
			{
				std::vector<u32> indices;
				for(u32 i=0; i<m_SoundMap[sound].size(); ++i)
				{
					indices.push_back(i);
				}

				std::vector<u32> randomIndices;
				while(!indices.empty())
				{
					u32 randomIndex = Random::GetInt(0, indices.size()-1);
					randomIndices.push_back(indices[randomIndex]);
					indices.erase(indices.begin()+randomIndex);
				}

				s32 soundIndex = -1;
				for(u32 i=0; i<randomIndices.size(); ++i)
				{
					if(!m_SoundMap[sound][randomIndices[i]]->IsPlaying())
					{
						soundIndex = randomIndices[i];
						break;
					}
				}

				if(soundIndex < 0)
				{
					SoundInfo* pInfo = GetSoundInfo(sound);
					u32 index = Random::GetInt(0, pInfo->m_Names.GetSize()-1);                    
					Sound* pSound = AudioDriver::Instance()->CreateSound(m_SoundBankMap[pInfo->m_Bank].Get(), pInfo->m_Names[index].c_str());
					m_SoundMap[sound].push_back(pSound);
					soundIndex = m_SoundMap[sound].size()-1;
                    Log.Print("m_SoundMap[%d].size: %d\n", sound, m_SoundMap[sound].size());
				}

				m_SoundMap[sound][soundIndex]->Play();
			}
		}
	}

	//! Plays a music
	void AudioManager::Play(E_Music music)
	{
		switch(music)
		{
		case M_Metal:
			m_Music = PickMetalTrack();
			break;

		case M_Intro:
			m_Music = AudioDriver::Instance()->CreateMusic("intro.ogg");
			break;

		case M_Outro:
			m_Music = AudioDriver::Instance()->CreateMusic("eminor_arpeggios.ogg");
			break;
		}
		SetMusicVolume(m_fMusicVolume);
		m_Music->Play();
		m_MusicType = music;
	}

	//! returns the SoundInfo
	AudioManager::SoundInfo* AudioManager::GetSoundInfo(E_Sound sound)
	{
		for(u32 i=0; i<m_SoundInfos.GetSize(); ++i)
		{
			if(m_SoundInfos[i].m_Sound == sound)
			{
				return &m_SoundInfos[i];
			}
		}

		return NULL;
	}

	//! picks a random metal track
	Music* AudioManager::PickMetalTrack()
	{
		if(m_RemainingMetalTracks.empty())
		{
			for(u32 i=0; i<m_MetalTracks.size(); ++i)
			{
				m_RemainingMetalTracks.push_back(i);
			}
		}

		u32 slot = Random::GetInt(0, m_RemainingMetalTracks.size()-1);
		u32 track = m_RemainingMetalTracks[slot];
		m_RemainingMetalTracks.erase(m_RemainingMetalTracks.begin()+slot);
		return AudioDriver::Instance()->CreateMusic(m_MetalTracks[track]);
	}	

	//! sets the music volume
	void AudioManager::SetMusicVolume(f32 fVolume)
	{
		GameManager::E_State state = GameManager::Instance()->GetState();
		bool bInGame = (state == GameManager::S_InGame) 
					|| (state == GameManager::S_InGamePause)
					|| (state == GameManager::S_AskExitLevel);
		f32 fFactor = bInGame ? m_fInGameMusicFactor : 1.0f;
		AudioDriver::Instance()->SetMusicVolume(fVolume*fFactor);
		m_fMusicVolume = fVolume;
	}
}

