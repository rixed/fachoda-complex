#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "sound.h"

static bool with_sound;
static ALCdevice *dev;
static ALCcontext *ctx;

static ALuint buffers[NB_SAMPLES];
static bool buffer_looping[NB_SAMPLES];	// tells whether the sample is supposed to loop
static float buffer_gain[NB_SAMPLES];
static ALuint sources[NB_VOICES];
static vector const *source_pos[NB_VOICES];

#define MAX_DIST 4000.

vector voices_in_my_head = { 0., 1., 0. };	// upstairs...

static char const *al_strerror(ALenum err)
{
	switch (err) {
		case AL_NO_ERROR:          return "No Error";
		case AL_INVALID_NAME:      return "Invalid Name";
		case AL_INVALID_ENUM:	   return "Invalid Enum";
		case AL_INVALID_VALUE:	   return "Invalid Value";
		case AL_INVALID_OPERATION: return "Invalid Operation";
		case AL_OUT_OF_MEMORY:     return "Out Of Memory";
	}
	return "Unknown Error";
}

int opensound(bool with_sound_)
{
	ALenum err;

	with_sound = with_sound_;
	if (! with_sound) return 0;

	dev = alcOpenDevice(NULL);
	if (! dev) {
		fprintf(stderr, "Cannot aclOpenDevice()\n");
		goto exit0;
	}

	static ALCint attrs[] = {
		ALC_MONO_SOURCES, ARRAY_LEN(sources),
		ALC_STEREO_SOURCES, 0,
		0, 0
	};
	ctx = alcCreateContext(dev, attrs);
	if (! ctx) {
		fprintf(stderr, "Cannot alcCreateContext()\n");
		goto exit1;
	}

	alcMakeContextCurrent(ctx);

	alGetError();
	alGenBuffers(ARRAY_LEN(buffers), buffers);
	alGenSources(ARRAY_LEN(sources), sources);
	if ((err = alGetError()) != AL_NO_ERROR) {
		fprintf(stderr, "Cannot genBuffers(): %s\n", al_strerror(err));
		goto exit2;
	}
	for (unsigned s = 0; s < ARRAY_LEN(sources); s++) {
		alSourcef(sources[s], AL_MAX_DISTANCE, MAX_DIST);
		alSourcef(sources[s], AL_REFERENCE_DISTANCE, 100.);
	}
	alSpeedOfSound(34330.);	// our unit of distance is approx the cm

	printf("Sound OK\n");
	return 0;

exit2:
	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
exit1:
	alcCloseDevice(dev);
exit0:
	with_sound = false;
	return -1;
}

static uchar *load_file(char const *fn, size_t *size)
{
	FILE *in = fopen(fn, "r");
	if (! in) {
		fprintf(stderr, "Cannot fopen(%s): %s\n", fn, strerror(errno));
		goto exit0;
	}

	if (0 != fseek(in, 0, SEEK_END)) {
		fprintf(stderr, "Cannot fseek(%s): %s\n", fn, strerror(errno));
		goto exit1;
	}

	long sz = ftell(in);
	if (sz < 0) {
		fprintf(stderr, "Cannot ftell(%s): %s\n", fn, strerror(errno));
		goto exit1;
	}

	rewind(in);

	if (size) *size = sz;

	uchar *buf = malloc(sz);
	if (! buf) {
		fprintf(stderr, "Cannot malloc(%ld)\n", sz);
		goto exit1;
	}

	size_t read = fread(buf, 1, sz, in);
	if (read < (size_t)sz) {
		fprintf(stderr, "Cannot read %ld bytes from %s: %s\n", sz, fn, strerror(errno));
		goto exit2;
	}

	(void)fclose(in);
	return buf;

exit2:
	free(buf);
exit1:
	(void)fclose(in);
exit0:
	return NULL;
}

int loadsample(sample_e samp, char const *fn, bool loop, float gain)
{
	if (! with_sound) return 0;

	ALenum err;

	size_t sz;
	uchar *fbuf = load_file(fn, &sz);
	if (! fbuf) goto exit0;

	assert(samp < ARRAY_LEN(buffers));
	alGetError();
	alBufferData(buffers[samp], AL_FORMAT_MONO8, fbuf, sz, 8000);

	if ((err = alGetError()) != AL_NO_ERROR) {
		fprintf(stderr, "Cannot alBufferData(%s): %s\n", fn, al_strerror(err));
		goto exit1;
	}

	buffer_looping[samp] = loop;
	buffer_gain[samp] = gain;

	free(fbuf);
	return 0;
exit1:
	free(fbuf);
exit0:
	with_sound = false;
	return -1;
}

static vector last_pos;
void update_listener(vector const *pos, vector const *velocity, matrix const *rot)
{
	if (! with_sound) return;

	ALenum err;

	last_pos = *pos;

	alListener3f(AL_POSITION, pos->x, pos->y, pos->z);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set listener's position to %f,%f,%f: %s\n", pos->x, pos->y, pos->z, al_strerror(err));

	alListener3f(AL_VELOCITY, velocity->x, velocity->y, velocity->z);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set listener's velocity to %f,%f,%f: %s\n", velocity->x, velocity->y, velocity->z, al_strerror(err));

	ALfloat at_up[6] = {
		rot->z.x, rot->z.y, rot->z.z,	// "at" vector
		rot->y.x, rot->y.y, rot->y.z,	// "up" vector
	};
	alListenerfv(AL_ORIENTATION, at_up);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set listener's orientation to %f,%f,%f / %f,%f,%f: %s\n", at_up[0], at_up[1], at_up[2], at_up[3], at_up[4], at_up[5], al_strerror(err));

	for (unsigned s = 0; s < ARRAY_LEN(sources); s++) {
		if (! source_pos[s]) continue;
		alSource3f(sources[s], AL_POSITION, source_pos[s]->x, source_pos[s]->y, source_pos[s]->z);
		if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source position at %f,%f,%f: %s\n", source_pos[s]->x, source_pos[s]->y, source_pos[s]->z, al_strerror(err));
	}
}

void playsound(enum voice voice, sample_e samp, float pitch, vector const *pos, bool relative)
{
	if (! with_sound) return;

	ALenum err;
	
	vector d = *pos;
	if (! relative) subv(&d, &last_pos);
	if (norme2(&d) > MAX_DIST*MAX_DIST) return;

	assert(voice < ARRAY_LEN(sources));
	assert(samp < ARRAY_LEN(buffers));

	alGetError();
	alSourceStop(sources[voice]);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot stop source %d: %s\n", voice, al_strerror(err));
	alSourcei(sources[voice], AL_BUFFER, buffers[samp]);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source buffer for sample %d: %s\n", samp, al_strerror(err));
	alSourcei(sources[voice], AL_LOOPING, buffer_looping[samp]);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source looping for voice %d: %s\n", voice, al_strerror(err));
	alSourcef(sources[voice], AL_GAIN, buffer_gain[samp]);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source gain for voice %d: %s\n", voice, al_strerror(err));
	alSourcef(sources[voice], AL_PITCH, pitch);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source pitch %f: %s\n", pitch, al_strerror(err));
	alSource3f(sources[voice], AL_POSITION, pos->x, pos->y, pos->z);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source position at %f,%f,%f: %s\n", pos->x, pos->y, pos->z, al_strerror(err));
	alSourcei(sources[voice], AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot set source base to %s: %s\n", relative?"relative":"absolute", al_strerror(err));
	alSourcePlay(sources[voice]);
	if ((err = alGetError()) != AL_NO_ERROR) fprintf(stderr, "Cannot play source %d: %s\n", voice, al_strerror(err));
	source_pos[voice] = NULL;
}

void attachsound(enum voice voice, sample_e samp, float pitch, vector const *pos, bool relative)
{
	if (! with_sound) return;

	playsound(voice, samp, pitch, pos, relative);
	source_pos[voice] = pos;
}

void exitsound(void)
{
	if (! with_sound) return;

	alDeleteSources(ARRAY_LEN(sources), sources);
	alDeleteBuffers(ARRAY_LEN(buffers), buffers);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	alcCloseDevice(dev);
}
