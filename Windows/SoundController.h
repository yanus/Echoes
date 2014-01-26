#include "stdafx.h"

#define MAXSOUNDS 128
#define MAXCHANNELS 16

class SoundController
{
public:
	SoundController();
	~SoundController();

	unsigned int createSound(const char *filename);
	void play(unsigned int index, bool loop, bool wait, char effect='0');
	void stop(unsigned int index);
	unsigned int record(unsigned int length);
	void enableEffect(unsigned int sound, bool enabled);
	bool isWaiting();
	void update() { system->update(); }
	void setPaused(bool paused);

private:
	void ERRCHECK(FMOD_RESULT result);
	unsigned int numsounds;
	FMOD::Sound *sounds[MAXSOUNDS];
	FMOD::Channel *channels[MAXSOUNDS];
	FMOD::ChannelGroup *groupScary, *masterGroup;
	FMOD::System *system;
	FMOD_RESULT result;

	FMOD::DSP *dspflange, *dsppitch;

	FMOD::Channel *channelWait;

	void error(const char *msg);
};