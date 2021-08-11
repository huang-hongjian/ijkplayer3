#ifndef __HJPLAYER__
#define __HJPLAYER__
#include <stdio.h>
#include "MediaFormat.h"
#include <jni.h>
#include "um_basictypes.h"




#define KEY_PARAMETER_VIDEO_POSITION_INFO  (6009)
#define KEY_PARAMETER_SET_SYNC             (1234)


typedef void (*PLAYER_EVENT_CB)(void *handler, PLAYER_EVENT_E evt, uint32_t param1, uint32_t param2);


typedef struct CRect_
{
int left;
int top;
int right;
int bottom;
}CRect;
typedef struct Sync_
{
int mod;
int timeout;
}CSync;


int     HJ_initPlayer                (void);
void    HJ_InitVideo                 (PVIDEO_PARA_T pVideoPara);
void    HJ_InitAudio                 (PAUDIO_PARA_T pAudioPara);
int     HJ_StartPlay                 (void);
int     HJ_Pause                     (void);
int     HJ_Resume                    (void);
int     HJ_TrickPlay                 (void);
int     HJ_StopTrickPlay             (void);
int     HJ_Stop                      (void);
int     HJ_Seek                      (void);
int32_t HJ_GetWriteBuffer            (PLAYER_STREAMTYPE_E type, uint8_t **pBuffer, uint32_t *nSize);
int32_t HJ_WriteData                 (PLAYER_STREAMTYPE_E type, uint8_t *pBuffer, uint32_t nSize, uint64_t timestamp);
void    HJ_SwitchAudioTrack          (uint16_t pid, PAUDIO_PARA_T pAudioPara);
int     HJ_GetIsEos                  (void);
int32_t HJ_GetPlayMode               (void);
int32_t HJ_GetCurrentPts             (void);
void    HJ_GetVideoPixels            (int32_t *width, int32_t *height);
int32_t HJ_GetBufferStatus           (int32_t *total_size, int32_t *datasize);
void    HJ_SetStopMode               (int bHoldLastPic);
void    HJ_SetContentMode            (PLAYER_CONTENTMODE_E contentMode);
void    HJ_SetEventCB                (void *handler, PLAYER_EVENT_CB pfunc);
int32_t HJ_GetAudioBalance           (void);
int     HJ_SetAudioBalance           (PLAYER_CH_E nAudioBalance);
void    HJ_DestoryPlayer            (void);
void    HJ_SetSurfaceTexture         (const void* pVideoSurfaceTexture);
void    HJ_SetParameter              (int32_t key, void* request);
void    HJ_GetParameter              (int32_t key, void* reply);
void*   HJ_SetSurfaceFromApk         (void*env,void*jsurface);
void*   HJ_GetSurfaceFromApk         (void);





#endif
