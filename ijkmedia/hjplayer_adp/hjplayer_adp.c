#include "hjplayer_adp.h"





	int HJ_initPlayer(void)
		{
			
			return UM_SUCCESS;
		}
	void HJ_InitVideo(PVIDEO_PARA_T pVideoPara)
		{
			
		}
	
		void HJ_InitAudio(PAUDIO_PARA_T pAudioPara)
		{
			
		}
	
		int HJ_StartPlay(void)
		{
			
			return  0;
		}
	
		int HJ_Pause(void)
		{
			
			return 0;
		}
	
	
		int HJ_Resume(void)
		{
			return 0;
		}
	
		int HJ_TrickPlay(void)
		{
			return 0;
		}
	
		int HJ_StopTrickPlay(void)
		{
			return 0;
		}
	
		int HJ_Stop(void)
		{
			return 0;
		}
	
		int HJ_Seek(void)
		{
			return 0;
		}
	
		int32_t HJ_GetWriteBuffer(PLAYER_STREAMTYPE_E type, uint8_t **pBuffer, uint32_t *nSize)
		{
			return 0;
		}
	
		int32_t HJ_WriteData(PLAYER_STREAMTYPE_E type, uint8_t *pBuffer, uint32_t nSize, uint64_t timestamp)
		{
			return 0;
		}
	
		void HJ_SwitchAudioTrack(uint16_t pid, PAUDIO_PARA_T pAudioPara)
		{
			return ;
		}
	
		int HJ_GetIsEos()
		{
			return 0;
		}
	
		int32_t HJ_GetPlayMode()
		{
			return 0;
		}
	
		int32_t HJ_GetCurrentPts()
		{
			return 0;
		}
	
		void HJ_GetVideoPixels(int32_t *width, int32_t *height)
		{
			return ;
		}
	
		int32_t HJ_GetBufferStatus(int32_t *total_size, int32_t *datasize)
		{
			return 0;
		}
	
		void HJ_SetStopMode(int bHoldLastPic)
		{
			return ;
		}
	
		void HJ_SetContentMode(PLAYER_CONTENTMODE_E contentMode)
		{
			return ;
		}
	
		int32_t HJ_GetAudioBalance()
		{
			return 0;
		}
	
		int HJ_SetAudioBalance(PLAYER_CH_E nAudioBalance)
		{
			return 0;
		}
	    void HJ_SetSurfaceTexture(const void* pVideoSurfaceTexture)
		{
            return ;

		}
		
		void HJ_SetEventCB(void *handler, PLAYER_EVENT_CB pfunc)
		{
			return ;
		}
	
		void HJ_DestoryPlayer()
		{
			return ;
		}
		void HJ_SetParameter(int32_t key, void* request)
		{
		    
            return ;
		}
		void	HJ_GetParameter 			 (int32_t key, void* reply)
		{
		
            return ;
		}

		void*	HJ_GetSurfaceFromApk		 (void)
			{
           return NULL;
		}
		void*	HJ_SetSurfaceFromApk		 (void*env,void*jsurface)
			{
           return NULL;
		}





