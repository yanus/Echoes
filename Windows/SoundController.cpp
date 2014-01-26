#include "stdafx.h"

SoundController::SoundController()
{
	int numdrivers;
	unsigned int version;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS caps;
	char name[256];

	// Initialize member variables
	numsounds = 0;
	channelWait = 0;

	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		error("You are using an old version of FMOD");
		return; // TODO: throw exception
	}

	// Get number of sound cards
	result = system->getNumDrivers(&numdrivers);
	ERRCHECK(result);

	if (numdrivers == 0)
	{
		error("No sound output found.");
		return;
	}
	
	result = system->getDriverCaps(0, &caps, 0, &speakermode);
	ERRCHECK(result);

	result = system->setSpeakerMode(speakermode);
	ERRCHECK(result);

	// Increase buffer size if user has Acceleration slider set to off
	if (caps & FMOD_CAPS_HARDWARE_EMULATED)
	{
		result = system->setDSPBufferSize(1024, 10);
		ERRCHECK(result);
	}

	// Get name of driver
	result = system->getDriverInfo(0, name, 256, 0);
	ERRCHECK(result);

	// SigmaTel sound devices crackle for some reason if the format is PCM 16-bit.
    // PCM floating point output seems to solve it.
	if (strstr(name, "SigmaTel"))
	{
		result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
		ERRCHECK(result);
	}

	result = system->init(MAXCHANNELS, FMOD_INIT_NORMAL, 0);

	// If the selected speaker mode isn't supported by this sound card, switch it back to stereo
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		ERRCHECK(result);

		result = system->init(MAXCHANNELS, FMOD_INIT_NORMAL, 0);
		ERRCHECK(result);
	}

	// Create effects
	result = system->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dspflange);
	ERRCHECK(result);
	result = dspflange->setParameter(FMOD_DSP_FLANGE_WETMIX, 0.8f);
	ERRCHECK(result);
	result = dspflange->setParameter(FMOD_DSP_FLANGE_DRYMIX, 0.2f);
	ERRCHECK(result);
	result = system->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &dsppitch);
	ERRCHECK(result);
	result = dsppitch->setParameter(FMOD_DSP_PITCHSHIFT_PITCH, 2.0f);
	ERRCHECK(result);

	FMOD::DSP *dspreverb;
	system->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &dspreverb);
	dspreverb->setParameter(FMOD_DSP_SFXREVERB_ROOM, -1800.0f);

	result = system->createChannelGroup("Scary", &groupScary);
	ERRCHECK(result);

	groupScary->addDSP(dspflange, 0);
	groupScary->addDSP(dsppitch, 0);

	result = system->getMasterChannelGroup(&masterGroup);
	ERRCHECK(result);

	masterGroup->addDSP(dspreverb, 0);

	result = masterGroup->addGroup(groupScary);
	ERRCHECK(result);
}

SoundController::~SoundController()
{
	for (unsigned int i = 0; i < numsounds; ++i)
	{
		result = sounds[i]->release();
		ERRCHECK(result);
	}

	result = system->release();
	ERRCHECK(result);
}

unsigned int SoundController::createSound(const char *filename)
{
	if (numsounds < MAXSOUNDS)
	{
		result = system->createSound(filename, FMOD_SOFTWARE, 0, &sounds[numsounds]);
		ERRCHECK(result);
		return numsounds++;
	}
	else
	{
		// exception
		return MAXSOUNDS;
	}
}

void SoundController::play(unsigned int index, bool loop, bool wait, char effect)
{
	stop(index);

	//sounds[index]->setDefaults(-44100, 1.0, 0.0, 128);
	sounds[index]->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	result = system->playSound(FMOD_CHANNEL_FREE, sounds[index], false, &channels[index]);
	ERRCHECK(result);
	if (wait) channelWait = channels[index]; else channelWait = 0;

	switch (effect)
	{
	case '0':
		result = channels[index]->setChannelGroup(masterGroup);
		break;
	case '1':
		result = channels[index]->setChannelGroup(groupScary);
		break;
	case '2':
		float fr;
		result = channels[index]->getFrequency(&fr);
		ERRCHECK(result);
		result = channels[index]->setFrequency(-0.5f*fr);
		ERRCHECK(result);
		result = channels[index]->setChannelGroup(groupScary);
		channels[index]->setVolume(0.3f);
		break;
	}
}

void SoundController::setPaused(bool paused)
{
	if (channelWait)
	{
		channelWait->setPaused(paused);

	}
}

void SoundController::enableEffect(unsigned int sound, bool enabled)
{
	/*FMOD::Channel *channel = channels[sound];

	bool active;
	result = dspecho->getActive(&active);
	ERRCHECK(result);

	if (enabled && !active)
	{
		result = channel->addDSP(dspecho, 0);
		ERRCHECK(result);
	}
	else if (!enabled && active)
	{
		result = dspecho->remove();
		ERRCHECK(result);
	}*/
}

bool SoundController::isWaiting()
{
	bool playing = false, recording = false;

	if (channelWait)
	{
		channelWait->isPlaying(&playing);
		ERRCHECK(result);
	}
	else
	{
		result = system->isRecording(0, &recording);
		ERRCHECK(result);
	}

	if (!playing && !recording)
	{
		channelWait = 0;
		return false;
	}
	return true;
}

void SoundController::stop(unsigned int index)
{
	bool playing = false;
	channels[index]->isPlaying(&playing);
	ERRCHECK(result);

	if (playing)
	{
		result = channels[index]->stop();
		ERRCHECK(result);
	}
}

void SoundController::stopAll()
{
	masterGroup->stop();
}

unsigned int SoundController::record(unsigned int length)
{
	FMOD_CREATESOUNDEXINFO exinfo;

	if (numsounds < MAXSOUNDS)
	{
		memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.numchannels = 1;
		exinfo.format = FMOD_SOUND_FORMAT_PCM16;
		exinfo.defaultfrequency = 44100;
		exinfo.length = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * length;

		result = system->createSound(0, FMOD_3D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &sounds[numsounds]);
		ERRCHECK(result);

		result = system->recordStart(0, sounds[numsounds], false);
		ERRCHECK(result);

		return numsounds++;
	}
	else
	{
		// exception
		return MAXSOUNDS;
	}
}


void SoundController::ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1); // throw exception
	}
}

void SoundController::error(const char *msg)
{
	printf("Error: %s", msg);
}