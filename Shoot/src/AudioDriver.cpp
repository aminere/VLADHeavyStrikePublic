/* 

Amine Rehioui
Created: August 26th 2013

*/

#include "Shoot.h"

#include "AudioDriver.h"

#ifndef NO_SOUND
#include "ck/ck.h"
#include "ck/config.h"
#include "ck/mixer.h"
#include "ck/sound.h"

namespace shoot
{
	//! Constructor
	AudioDriver::AudioDriver()
		: m_pMusicMixer(NULL)
		, m_pSoundMixer(NULL)
	{
	}

	//! Destructor
	AudioDriver::~AudioDriver()
	{
		if(m_pMusicMixer)
		{
			m_pMusicMixer->destroy();
			m_pMusicMixer = NULL;
		}

		if(m_pSoundMixer)
		{
			m_pSoundMixer->destroy();
			m_pSoundMixer = NULL;
		}

		CkUpdate();
		CkShutdown();
	}

	//! driver initialization
	void AudioDriver::Init()
	{
#if SHOOT_PLATFORM != SHOOT_PLATFORM_ANDROID
		CkConfig config;
		CkInit(&config);
#endif

		m_pMusicMixer = CkMixer::newMixer("MusicMixer");
		m_pSoundMixer = CkMixer::newMixer("SoundMixer");
	}

	//! driver update
	void AudioDriver::Update()
	{
		CkUpdate();
	}

	//! create a sound bank
	SoundBank* AudioDriver::CreateSoundBank(const char* strBank)
	{
		return snew SoundBank(strBank);
	}

	//! creates a sound
	Sound* AudioDriver::CreateSound(SoundBank* pBank, const char* strName)
	{
		Sound* pSound = snew Sound(pBank, strName);
		pSound->GetSound()->setMixer(m_pSoundMixer);
		return pSound;
	}

	//! create a music
	Music* AudioDriver::CreateMusic(const char* strOGG)
	{
		Music* pMusic = snew Music(strOGG);
		pMusic->GetSound()->setMixer(m_pMusicMixer);
		return pMusic;
	}

	//! set music status
	void AudioDriver::SetMusicEnabled(bool bEnabled)
	{
		m_pMusicMixer->setVolume(bEnabled ? 1.0f : 0.0f);
	}

	//! set sound status
	void AudioDriver::SetSoundEnabled(bool bEnabled)
	{
		m_pSoundMixer->setVolume(bEnabled ? 1.0f : 0.0f);
	}

	//! set the music volume
	void AudioDriver::SetMusicVolume(f32 fVolume)
	{
		m_pMusicMixer->setVolume(fVolume);
	}

	//! set the sound volume
	void AudioDriver::SetSoundVolume(f32 fVolume)
	{
		m_pSoundMixer->setVolume(fVolume);
	}

	//! pauses all audio
	void AudioDriver::Pause()
	{
		CkSuspend();
	}

	//! resume all audio
	void AudioDriver::Resume()
	{
		CkResume();
	}
}
#else
namespace shoot
{
	// strictly for win XP build where cricket lib is not available
	AudioDriver::AudioDriver() { }
	AudioDriver::~AudioDriver() { }
	void AudioDriver::Init() { }
	void AudioDriver::Update() { }
	SoundBank* AudioDriver::CreateSoundBank(const char* strBank) { return NULL; }
	Sound* AudioDriver::CreateSound(SoundBank* pBank, const char* strName) { return NULL; }
	Music* AudioDriver::CreateMusic(const char* strOGG)	{ return NULL; }
	void AudioDriver::SetMusicEnabled(bool bEnabled) { }
	void AudioDriver::SetSoundEnabled(bool bEnabled) { }
	void AudioDriver::SetMusicVolume(f32 fVolume) { }
	void AudioDriver::SetSoundVolume(f32 fVolume) { }
	void AudioDriver::Pause() { }
	void AudioDriver::Resume() { }
}
#endif // NO_SOUND