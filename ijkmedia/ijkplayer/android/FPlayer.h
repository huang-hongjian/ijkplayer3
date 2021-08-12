#ifndef __FPLAYER_H__
#define __FPLAYER_H__
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include <unistd.h>
#include "j4a/class/java/util/ArrayList.h"
#include "j4a/class/android/os/Bundle.h"
#include "j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.h"
#include "j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.h"
#include "j4a/class/tv/danmaku/ijk/media/player/misc/IIjkIOHttp.h"
#include "ijksdl/ijksdl_log.h"
#include "../ff_ffplay.h"
#include "ffmpeg_api_jni.h"
#include "ijkplayer_android_def.h"
#include "ijkplayer_android.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_codec_android_mediadef.h"
#include "ijkavformat/ijkavformat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char   BOOL;
typedef void* object;

typedef void (* FPLAYER_CB)( void*weak_thiz , int what, int arg1, int arg2, int obj);


void FMediaPlayer_setDataSourceAndHeaders(char* path,void* keys, void* values);


void FMediaPlayer_setDataSourceFd(int fd);


void FMediaPlayer_setDataSourceCallback(void* callback);


void FMediaPlayer_setDataSourceIjkIOHttpCallback(  void* callback); 

void FMediaPlayer_setVideoSurface(  void* jsurface,int x,int y,int width,int height);


void FMediaPlayer_prepareAsync();


void FMediaPlayer_start();

void FMediaPlayer_stop();


void FMediaPlayer_pause();


void FMediaPlayer_seekTo( long msec);


BOOL FMediaPlayer_isPlaying();


long FMediaPlayer_getCurrentPosition();


long FMediaPlayer_getDuration();

long FMediaPlayer_getBit_Rate();
void FMediaPlayer_release();


void FMediaPlayer_native_setup( void* weak_this);
void FMediaPlayer_reset();

void FMediaPlayer_setLoopCount( int loop_count);


int FMediaPlayer_getLoopCount();


float FMediaPlayer_getPropertyFloat( int id, float default_value);

void FMediaPlayer_setPropertyFloat( int id, float value);


long FMediaPlayer_getPropertyLong( int id, long default_value);

void FMediaPlayer_setPropertyLong(int id, long value);

void FMediaPlayer_setStreamSelected(int stream, BOOL selected);

void FMediaPlayer_setVolume( float leftVolume, float rightVolume);

int FMediaPlayer_getAudioSessionId();

void FMediaPlayer_setOption( int category, object name, object value);

void FMediaPlayer_setOptionLong( int category, object name, long value);

char* FMediaPlayer_getColorFormatName( int mediaCodecColorFormat);


char* FMediaPlayer_getVideoCodecInfo();

char* FMediaPlayer_getAudioCodecInfo();


void* FMediaPlayer_getMediaMeta();

void FMediaPlayer_native_init();
void FMediaPlayer_native_deinit();
void FMediaPlayer_native_setup( void* weak_this);

void FMediaPlayer_native_finalize( char* name, char* value);

void FMediaPlayer_native_profileBegin( char* libName);

void FMediaPlayer_native_profileEnd();

void FMediaPlayer_native_setLogLevel( int level);
void FMediaPlayer_setPostEvent(void* fmediaplayerclass,void* post_event);


#ifdef __cplusplus
extern };
#endif


#endif