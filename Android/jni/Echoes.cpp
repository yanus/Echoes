#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include "fmod.h"
#include "fmod_errors.h"

#define MAXSOUNDS 128
#define MAXCHANNELS 16

unsigned int numsounds;
FMOD_CHANNEL *channelWait;
FMOD_RESULT result;
FMOD_SYSTEM *fmodSystem;
FMOD_DSP *dspflange, *dsppitch;
FMOD_CHANNELGROUP *groupScary, *masterGroup;
FMOD_SOUND *sounds[MAXSOUNDS];
FMOD_CHANNEL *channels[MAXSOUNDS];

#define ERRCHECK(x) \
		{ \
	FMOD_RESULT _result = x; \
	if (_result != FMOD_OK) \
	{ \
		__android_log_print(ANDROID_LOG_ERROR, "fmod", "FMOD error! (%d) %s\n%s:%d", _result, FMOD_ErrorString(_result), __FILE__, __LINE__); \
		exit(-1); \
	} \
		}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cInit(JNIEnv *env, jobject thiz)
{
	int numdrivers;
	unsigned int version;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS caps;
	char name[256];

	// Initialize global variables
	numsounds = 0;
	channelWait = 0;

	result = FMOD_System_Create(&fmodSystem);
	ERRCHECK(result);

	result = FMOD_System_Init(fmodSystem, MAXCHANNELS, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	// Create effects
	result = FMOD_System_CreateDSPByType(fmodSystem, FMOD_DSP_TYPE_FLANGE, &dspflange);
	ERRCHECK(result);
	result = FMOD_DSP_SetParameter(dspflange, FMOD_DSP_FLANGE_WETMIX, 0.8f);
	ERRCHECK(result);
	result = FMOD_DSP_SetParameter(dspflange, FMOD_DSP_FLANGE_DRYMIX, 0.2f);
	ERRCHECK(result);
	result = FMOD_System_CreateDSPByType(fmodSystem, FMOD_DSP_TYPE_PITCHSHIFT, &dsppitch);
	ERRCHECK(result);
	result = FMOD_DSP_SetParameter(dsppitch, FMOD_DSP_PITCHSHIFT_PITCH, 2.0f);
	ERRCHECK(result);
	
	FMOD_DSP *dspreverb;
	FMOD_System_CreateDSPByType(fmodSystem, FMOD_DSP_TYPE_SFXREVERB, &dspreverb);
	FMOD_DSP_SetParameter(dspreverb, FMOD_DSP_SFXREVERB_ROOM, -1800.0f);

	result = FMOD_System_CreateChannelGroup(fmodSystem, "Scary", &groupScary);
	ERRCHECK(result);

	FMOD_ChannelGroup_AddDSP(groupScary, dspflange, 0);
	FMOD_ChannelGroup_AddDSP(groupScary, dsppitch, 0);

	result = FMOD_System_GetMasterChannelGroup(fmodSystem, &masterGroup);
	ERRCHECK(result);
	
	FMOD_ChannelGroup_AddDSP(masterGroup, dspreverb, 0);

	result = FMOD_ChannelGroup_AddGroup(masterGroup, groupScary);
	ERRCHECK(result);
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cEnd(JNIEnv *env, jobject thiz)
{
	for (unsigned int i = 0; i < numsounds; ++i)
	{
		result = FMOD_Sound_Release(sounds[i]);
		ERRCHECK(result);
	}

	result = FMOD_System_Release(fmodSystem);
	ERRCHECK(result);
}

extern "C" JNIEXPORT jint JNICALL Java_no_uia_yannis11_echoes_MainActivity_cCreateSound(JNIEnv * env, jobject thiz, jstring filename)
{
	jboolean isCopy;
	const char *szFilename = env->GetStringUTFChars(filename, &isCopy);

	if (numsounds < MAXSOUNDS)
	{
		result = FMOD_System_CreateSound(fmodSystem, szFilename, FMOD_SOFTWARE, 0, &sounds[numsounds]);
		ERRCHECK(result);
		return numsounds++;
	}
	else
	{
		return MAXSOUNDS;
	}
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cStop(JNIEnv * env, jobject thiz, jint jindex)
{
	unsigned int index = jindex;
	FMOD_BOOL playing = false;
	if (channels[index])
	{
		result = FMOD_Channel_IsPlaying(channels[index], &playing);
		//ERRCHECK(result);

		if (playing)
		{
			result = FMOD_Channel_Stop(channels[index]);
			ERRCHECK(result);
		}
	}
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cStopAll(JNIEnv * env, jobject thiz)
{
	FMOD_ChannelGroup_Stop(masterGroup);
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cPlay(JNIEnv * env, jobject thiz, jint jindex, jboolean jloop, jboolean jwait, jchar jeffect)
{
	unsigned int index = jindex;
	bool loop = jloop, wait = jwait;
	char effect = jeffect;
	Java_no_uia_yannis11_echoes_MainActivity_cStop(env, thiz, index);

	FMOD_Sound_SetMode(sounds[index], loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, sounds[index], false, &channels[index]);
	ERRCHECK(result);
	if (wait) channelWait = channels[index]; else channelWait = 0;

	switch (effect)
	{
	case '0':
		FMOD_Channel_SetChannelGroup(channels[index], masterGroup);
		break;
	case '1':
		result = FMOD_Channel_SetChannelGroup(channels[index], groupScary);
		break;
	case '2':
		float fr;
		result = FMOD_Channel_GetFrequency(channels[index], &fr);
		ERRCHECK(result);
		result = FMOD_Channel_SetFrequency(channels[index], -0.5f*fr);
		ERRCHECK(result);
		FMOD_Channel_SetChannelGroup(channels[index], groupScary);
		FMOD_Channel_SetVolume(channels[index], 0.3f);
		break;
	}
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cSetPaused(JNIEnv * env, jobject thiz, jboolean paused)
{
	if (channelWait)
	{
		FMOD_Channel_SetPaused(channelWait, (bool)paused);
	}
}

extern "C" JNIEXPORT jboolean JNICALL Java_no_uia_yannis11_echoes_MainActivity_cIsWaiting(JNIEnv * env, jobject thiz)
{
	FMOD_BOOL playing = false, recording = false;

	if (channelWait)
	{
		result = FMOD_Channel_IsPlaying(channelWait, &playing);
		//ERRCHECK(result);
	}
	else
	{
		result = FMOD_System_IsRecording(fmodSystem, 0, &recording);
		ERRCHECK(result);
	}

	if (!playing && !recording)
	{
		channelWait = 0;
		return false;
	}
	return true;
}

extern "C" JNIEXPORT jint JNICALL Java_no_uia_yannis11_echoes_MainActivity_cRecord(JNIEnv * env, jobject thiz, jint jlength)
{
	unsigned int length = jlength;

	FMOD_CREATESOUNDEXINFO exinfo;

	if (numsounds < MAXSOUNDS)
	{
		memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.numchannels = 1;
		exinfo.format = FMOD_SOUND_FORMAT_PCM16;
		exinfo.defaultfrequency = 44100;
		exinfo.length = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * length;

		result = FMOD_System_CreateSound(fmodSystem, 0, FMOD_3D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &sounds[numsounds]);
		ERRCHECK(result);

		result = FMOD_System_RecordStart(fmodSystem, 0, sounds[numsounds], false);
		ERRCHECK(result);

		return numsounds++;
	}
	else
	{
		return MAXSOUNDS;
	}
}

extern "C" JNIEXPORT void JNICALL Java_no_uia_yannis11_echoes_MainActivity_cUpdate(JNIEnv * env, jobject thiz)
{
	FMOD_System_Update(fmodSystem);
}
