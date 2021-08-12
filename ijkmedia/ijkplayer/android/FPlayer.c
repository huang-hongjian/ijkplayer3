#include "FPlayer.h"

#define USE_FP_EVENT 1
#define CHECK_GOTO(condition__,msg,go) \
do{\
    if(!condition__) \
    {\
          MPTRACE("hongjian %s  :%s\n", __func__,msg);\
          goto go;\
    }\
}while(0)


#define F_CHECK_MPRET_GOTO(retval, label) \
    CHECK_GOTO((retval != EIJK_INVALID_STATE), "", label); \
    CHECK_GOTO((retval != EIJK_OUT_OF_MEMORY), "", label); \
    CHECK_GOTO((retval == 0), "", label);


typedef struct FMediaPlayer {
    
    void* NativeMediaPlayer;
    void* FMediaPlayerClass;
     void (*Post_Event)(void*weak_thiz,  int what, int arg1, int arg2, void* obj);
    //void (*Post_Event)( FPlayerEvent*);
    void* NativeIjkIOHttp;
    void (*postEventFromNative)(void* weak_this, int what, int arg1, int arg2, void obj);
    char* (*onSelectCodec)(void* weakThiz, char* mimeType, int profile, int level) ;
    int (*onNativeInvoke)(int what, void args);
    int stop_flag;
    struct FMediaPlayer* oldplayer;
} FMediaPlayer;
static FMediaPlayer Fplayer={0};

typedef struct ff_player_fields_t {
    pthread_mutex_t mutex;

} ff_player_fields_t;
static ff_player_fields_t g_clazz;

static int inject_callback(void *opaque, int type, void *data, size_t data_size);
static BOOL mediacodec_select_callback(void *opaque, ijkmp_mediacodecinfo_context *mcc);
 int FMediaPlayer_Report_FpEvent(int msg,void* arg,void* arg2,void*obj);
 int FMediaPlayer_SendSqmMsg(int msg);
static IjkMediaPlayer * get_fplayer(void)
{
    return (IjkMediaPlayer *)Fplayer.NativeMediaPlayer;
}

static void set_fplayer(void* player)
{
     Fplayer.NativeMediaPlayer=(IjkMediaPlayer *)player;

}


static IjkMediaPlayer *get_media_player()
{
    pthread_mutex_lock(&g_clazz.mutex);

    IjkMediaPlayer *mp = get_fplayer();
    if (mp) {
        //MPTRACE("$$$$$$$$$$$$$$%s$$$$$$$$$$$$\n",__func__);
        ijkmp_inc_ref(mp);
    }

    pthread_mutex_unlock(&g_clazz.mutex);
    return mp;
}

static IjkMediaPlayer *set_media_player( IjkMediaPlayer *mp)
{
    pthread_mutex_lock(&g_clazz.mutex);

    IjkMediaPlayer *old = get_fplayer();
            MPTRACE("%s old=%p mp=%p\n",__func__,old,mp);

    if (mp) {
        ijkmp_inc_ref(mp);
    }
      if(old) 
        Fplayer.oldplayer = old;
    else 
        Fplayer.oldplayer = NULL;
    
    set_fplayer((void*) mp);

    pthread_mutex_unlock(&g_clazz.mutex);

    // NOTE: ijkmp_dec_ref may block thread
    if (old != NULL ) {
        MPTRACE("************ hongjian old != NULL \n");
        ijkmp_dec_ref_p(&old);
    }

    return old;
}

int ff_is_multicast(char*url)
{
     if(strstr(url,"igmp://") ||strstr(url,"IGMP://") ||strstr(url,"udp://") ||strstr(url,"UDP://"))
        return 1;

     return 0;
}

static int message_loop(void *arg);

void
FMediaPlayer_setDataSourceAndHeaders(
     char* path,
     object keys, object values)
{
    MPTRACE("hongjian %s\n", __func__);
    int retval = -1;

    IjkMediaPlayer *mp = get_media_player();
    av_log(NULL,AV_LOG_ERROR,"@unionman  mediaplayer = 0x%x",mp);
    CHECK_GOTO(path, "mpjni: setDataSource: null path", LABEL_RETURN);
    CHECK_GOTO(mp, "mpjni: setDataSource: null mp", LABEL_RETURN);

    ALOGV("setDataSource: path %s", path);
    retval = ijkmp_set_data_source(mp, path);

    F_CHECK_MPRET_GOTO(retval,  LABEL_RETURN);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_setDataSourceFd( int fd)
{
    MPTRACE("%s\n", __func__);
    int retval = 0;
    int dupFd = 0;
    char uri[128];
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(fd > 0, "mpjni: setDataSourceFd: null fd", LABEL_RETURN);
    CHECK_GOTO(mp, "mpjni: setDataSourceFd: null mp", LABEL_RETURN);

    dupFd = dup(fd);

    ALOGV("setDataSourceFd: dup(%d)=%d\n", fd, dupFd);
    snprintf(uri, sizeof(uri), "pipe:%d", dupFd);
    retval = ijkmp_set_data_source(mp, uri);

    F_CHECK_MPRET_GOTO(retval, LABEL_RETURN);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_setDataSourceCallback(object callback)
{
    MPTRACE("hongjian %s\n", __func__);
    return ;
}

//Android SDK 21֮���������ӿ�
void
FMediaPlayer_setDataSourceIjkIOHttpCallback(object callback) {
    MPTRACE("hongjian %s\n", __func__);
   return ;
}

void
FMediaPlayer_setVideoSurface(object surface)
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    ijkmp_android_set_surface(surface);
LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_prepareAsync()
{
    MPTRACE("hongjian %s\n", __func__);
    int retval = 0;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: prepareAsync: null mp", LABEL_RETURN);
#if USE_FP_EVENT
        ijkmp_set_start_flag(mp, 0);
        FMediaPlayer_Report_FpEvent(FFP_MSG_PLAY_PREPARE,NULL,NULL,mp);
#endif    
    retval = ijkmp_prepare_async(mp);
    F_CHECK_MPRET_GOTO(retval,  LABEL_RETURN);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_start()
{
    int ret = -1;
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: start: null mp", LABEL_RETURN);

    ret = ijkmp_start(mp);
    ff_send_exitplay_msg = 1;
    FMediaPlayer_SendSqmMsg(FFP_MSG_SQM_START);
    if(!ret)
        Fplayer.stop_flag = 0;
#if USE_FP_EVENT	
    if(ff_gPauseFlg==0 && ff_gCurrentEventState != FFP_MSG_PAUSE_MESSAGE)
    {
        if(ff_gCurrentEventState != FFP_MSG_PLAY_START && ff_gCurrentEventState != FFP_MSG_SEEK_START  && ff_gCurrentEventState != FFP_MSG_SEEK_END)
           ; //IjkMediaPlayer_Report_FpEvent(env,  thiz,FFP_MSG_PLAY_STARTUP,NULL,NULL,NULL);//²»Ì«Ìý»°£¬ÀÏÊÇÂÒ·¢£¬²»ÒªÁË
    }
    else if(ff_gPauseFlg == 1)
    {
        FMediaPlayer_Report_FpEvent(FFP_MSG_RESUME_MESSAGE,NULL,NULL,mp);
        ff_gPauseFlg = 0;
    }
#endif    
LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return ret;
}
 void
FMediaPlayer_fastplay(int speed)
{
    MPTRACE("%s  speed=%d\n", __func__,speed);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: start: null mp", LABEL_RETURN);

    ijkmp_fastplay(mp,speed);
    if(speed !=1)
        FMediaPlayer_SendSqmMsg(FFP_MSG_SQM_FASTFORWARD_OR_REWIND);
    else
        FMediaPlayer_SendSqmMsg(FFP_MSG_SQM_START);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}
void
FMediaPlayer_stop()
{
    int ret = -1;
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    //IjkMediaPlayer *tmp_mp=mp;

    CHECK_GOTO(mp,  "mpjni: stop: null mp", LABEL_RETURN);

    ret = ijkmp_stop(mp);
    if(!ret)
        Fplayer.stop_flag = 1;
  
LABEL_RETURN:
   // ijkmp_dec_ref_p(&tmp_mp);
    ijkmp_dec_ref_p(&mp);
    
}

void
FMediaPlayer_pause()
{
    MPTRACE("%s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: pause: null mp", LABEL_RETURN);

    ijkmp_pause(mp);
    FMediaPlayer_SendSqmMsg(FFP_MSG_SQM_PAUSE);
#if USE_FP_EVENT
    if(ff_gCurrentEventState!=FFP_MSG_PAUSE_MESSAGE)
        {
        ff_gPauseFlg = 1;
        FMediaPlayer_Report_FpEvent(FFP_MSG_PAUSE_MESSAGE,NULL,NULL,mp);
        }
#endif
LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_seekTo(long msec)
{
    MPTRACE("%s msec=%ld\n", __func__,msec);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: seekTo: null mp", LABEL_RETURN);

#if USE_FP_EVENT
    FMediaPlayer_Report_FpEvent(FFP_MSG_SEEK_START,msec,NULL,mp);
 #endif   
    ijkmp_seek_to(mp, msec);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

BOOL
FMediaPlayer_isPlaying()
{
    MPTRACE("hongjian %s\n", __func__);
    BOOL retval = 0;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: isPlaying: null mp", LABEL_RETURN);

    retval = ijkmp_is_playing(mp) ? 1 : 0;

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}

long
FMediaPlayer_getCurrentPosition()
{
    MPTRACE(" %s\n", __func__);
    long retval = 0;
    IjkMediaPlayer *mp = get_fplayer();
    
    CHECK_GOTO(mp,  "mpjni: getCurrentPosition: null mp", LABEL_RETURN);
    if(Fplayer.stop_flag)
    {
        mp=NULL;
        return ff_gCurrentPosition;
    }
    retval = ijkmp_get_current_position(mp);
    MPTRACE(" %s %ld\n", __func__,retval);
    ff_gCurrentPosition = retval;	
LABEL_RETURN:
    //ijkmp_dec_ref_p(&mp);
    return retval;
}

long
FMediaPlayer_getDuration()
{
    //MPTRACE("hongjian %s\n", __func__);
    long retval = 0;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: getDuration: null mp", LABEL_RETURN);

    retval = ijkmp_get_duration(mp);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}

long
FMediaPlayer_getBit_Rate()
{
    //MPTRACE("hongjian %s\n", __func__);
    long retval = 0;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: getBit_Rate: null mp", LABEL_RETURN);

    //retval = ijkmp_get_bit_rate(mp);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}


void
FMediaPlayer_release()
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    if (!mp)
        return;
    //ijkmp_android_set_surface(env, mp, NULL,0,0,0,0);//��ʱע�Ͳ��ã��ú�˼��surface
    
    // explicit shutdown mp, in case it is not the last mp-ref here
    ijkmp_shutdown(mp);
    //only delete weak_thiz at release
    object weak_thiz = (object) ijkmp_set_weak_thiz(mp, NULL );
    free(weak_thiz);
    MPTRACE("hongjian set_media_player( NULL);--->");
    set_media_player( NULL);
    ff_send_exitplay_msg = 0;
     ff_gMediaType = -1;

     ff_gChannelPort = 0;
     ff_gStbPort = 0;
     ff_gBitRate = 0;
     ff_gPlayerID = -1;

     ff_gCurrentPosition = 0;
     ff_gFirstFrameTime = 0;
     ff_gCurrentEventState = 0;
     ff_gCacheTime = 0;
      ff_gPauseFlg = 0;    //jni_set_media_data_source( NULL);
    MPTRACE("hongjian set_media_player( NULL);<---");
    ijkmp_dec_ref_p(&mp);
}

void FMediaPlayer_native_setup( object weak_this);
void
FMediaPlayer_reset()
{

    MPTRACE("%s\n", __func__);

#if 1
    IjkMediaPlayer *mp = get_media_player();
    if (!mp)
        return;
    if(ff_send_exitplay_msg)
    {
        FMediaPlayer_SendSqmMsg( FFP_MSG_SQM_EXITPLAY);
#if USE_FP_EVENT
        FMediaPlayer_Report_FpEvent(FFP_MSG_PLAY_QUIT,NULL,NULL,mp);
       
#endif
     ff_send_exitplay_msg = 0;
     ff_gMediaType = -1;

     ff_gChannelPort = 0;
     ff_gStbPort = 0;
     ff_gBitRate = 0;
     ff_gPlayerID = -1;

     ff_gCurrentPosition = 0;
     ff_gFirstFrameTime = 0;
     ff_gCurrentEventState = 0;
     ff_gCacheTime = 0;
      ff_gPauseFlg = 0;
    }
    jobject weak_thiz = (jobject) ijkmp_set_weak_thiz(mp, NULL );
    FMediaPlayer_release();
    FMediaPlayer_native_setup( weak_thiz);
LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
#else 

    IjkMediaPlayer *mp = get_media_player();
    if (!mp)
        return;

    object weak_thiz = (object) ijkmp_set_weak_thiz(mp, NULL );

    FMediaPlayer_release();
    //MPTRACE("hongjian FMediaPlayer_native_setup--->;");
    FMediaPlayer_native_setup( weak_thiz);
    //MPTRACE("hongjian FMediaPlayer_native_setup<---;");
    ijkmp_dec_ref_p(&mp);
 #endif   
}

void
FMediaPlayer_setLoopCount( int loop_count)
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: setLoopCount: null mp", LABEL_RETURN);

    ijkmp_set_loop(mp, loop_count);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

int
FMediaPlayer_getLoopCount()
{
    int loop_count = 1;
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: getLoopCount: null mp", LABEL_RETURN);

    loop_count = ijkmp_get_loop(mp);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return loop_count;
}

float
FMediaPlayer_getPropertyFloat(int id, float default_value)
{
    float value = default_value;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: getPropertyFloat: null mp", LABEL_RETURN);

    value = ijkmp_get_property_float(mp, id, default_value);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return value;
}

void
FMediaPlayer_setPropertyFloat(int id, float value)
{
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: setPropertyFloat: null mp", LABEL_RETURN);

    ijkmp_set_property_float(mp, id, value);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return;
}

long
FMediaPlayer_getPropertyLong(int id, long default_value)
{
    jlong value = default_value;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: getPropertyLong: null mp", LABEL_RETURN);

    value = ijkmp_get_property_int64(mp, id, default_value);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return value;
}

void
FMediaPlayer_setPropertyLong( int id, long value)
{
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: setPropertyLong: null mp", LABEL_RETURN);

    ijkmp_set_property_int64(mp, id, value);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return;
}

void
FMediaPlayer_setStreamSelected( int stream, BOOL selected)
{
    IjkMediaPlayer *mp = get_media_player();
    int ret = 0;
    CHECK_GOTO(mp,  "mpjni: setStreamSelected: null mp", LABEL_RETURN);

    ret = ijkmp_set_stream_selected(mp, stream, selected);
    if (ret < 0) {
        ALOGE("failed to %s %d", selected ? "select" : "deselect", stream);
        goto LABEL_RETURN;
    }

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return;
}

void
FMediaPlayer_setVolume(float leftVolume, float rightVolume)
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: setVolume: null mp", LABEL_RETURN);

   // ijkmp_android_set_volume(NULL, mp, leftVolume, rightVolume);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

int
FMediaPlayer_getAudioSessionId()
{
    jint audio_session_id = 0;
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: getAudioSessionId: null mp", LABEL_RETURN);

    audio_session_id = ijkmp_android_get_audio_session_id(NULL, mp);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return audio_session_id;
}

void
FMediaPlayer_setOption( int category, object name, object value)
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    const char *c_name = NULL;
    const char *c_value = NULL;
    CHECK_GOTO(mp,  "mpjni: setOption: null mp", LABEL_RETURN);

    c_name =(char *)name;
    CHECK_GOTO(c_name,  "mpjni: setOption: name.string oom", LABEL_RETURN);

    if (value) {
        c_value =(char *)value;
        CHECK_GOTO(c_name,  "mpjni: setOption: name.string oom", LABEL_RETURN);
    }

    ijkmp_set_option(mp, category, c_name, c_value);

LABEL_RETURN:

    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_setOptionLong( int category, object name, long value)
{
    MPTRACE("hongjian %s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    const char *c_name = NULL;
    CHECK_GOTO(mp, "mpjni: setOptionLong: null mp", LABEL_RETURN);

    c_name = (char*) name;
    CHECK_GOTO(c_name,  "mpjni: setOptionLong: name.string oom", LABEL_RETURN);

    ijkmp_set_option_int(mp, category, c_name, value);

LABEL_RETURN:
   
    ijkmp_dec_ref_p(&mp);
}

char*
FMediaPlayer_getColorFormatName(int mediaCodecColorFormat)
{
    const char *codec_name = SDL_AMediaCodec_getColorFormatName(mediaCodecColorFormat);
    if (!codec_name)
        return NULL ;

    return  codec_name;
}

char*
FMediaPlayer_getVideoCodecInfo()
{
    MPTRACE("hongjian %s\n", __func__);

    int ret = 0;
    char *codec_info = NULL;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp, "mpjni: getVideoCodecInfo: null mp", LABEL_RETURN);

    ret = ijkmp_get_video_codec_info(mp, &codec_info);
    if (ret < 0 || !codec_info)
        goto LABEL_RETURN;

LABEL_RETURN:

    ijkmp_dec_ref_p(&mp);
    return codec_info;
}

char*
FMediaPlayer_getAudioCodecInfo()
{
    MPTRACE("hongjian %s\n", __func__);

    int ret = 0;
    char *codec_info = NULL;
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: getAudioCodecInfo: null mp", LABEL_RETURN);

    ret = ijkmp_get_audio_codec_info(mp, &codec_info);
    if (ret < 0 || !codec_info)
        goto LABEL_RETURN;

    
LABEL_RETURN:
 

    ijkmp_dec_ref_p(&mp);
    return codec_info;
}

inline static void fillMetaInternal(IjkMediaMeta *meta, const char *key, const char *default_value)
{
#if 0 
    const char *value = ijkmeta_get_string_l(meta, key);
    if (value == NULL )
        value = default_value;

    J4AC_Bundle__putString__withCString__catchAll(env, jbundle, key, value);
#endif
}

object
FMediaPlayer_getMediaMeta()
{
    MPTRACE("hongjian %s\n", __func__);

    return NULL;
}

void
FMediaPlayer_native_init()
{
    MPTRACE("%s\n", __func__);
    
    pthread_mutex_init(&g_clazz.mutex, NULL );

    ijkmp_global_init();
    //ijkmp_global_set_inject_callback(inject_callback);
 
    //memset(&Fplayer,0,sizeof(FMediaPlayer));
    //Fplayer.NativeMediaPlayer=NULL;

    //FFmpegApi_global_init(env);

}
void
FMediaPlayer_native_deinit()
{
    MPTRACE("hongjian %s\n", __func__);
    ijkmp_global_uninit();


    //pthread_mutex_lock(&g_clazz.mutex);
    //memset(&Fplayer,0,sizeof(Fplayer));
    //pthread_mutex_unlock(&g_clazz.mutex);    
    pthread_mutex_destroy(&g_clazz.mutex);

}

void
FMediaPlayer_native_setup(object weak_this)
{
    MPTRACE("%s\n", __func__);
    IjkMediaPlayer *mp = ijkmp_android_create(message_loop);
    CHECK_GOTO(mp, "mpjni: native_setup: ijkmp_create() failed", LABEL_RETURN);

    ijkmp_global_set_log_level(3);
    set_media_player(mp);
    void* weak_this_=(void*)malloc(sizeof(void*));
    weak_this_ = weak_this;
    ijkmp_set_weak_thiz(mp, weak_this_);
    //ijkmp_set_inject_opaque(mp, ijkmp_get_weak_thiz(mp));
    //ijkmp_set_ijkio_inject_opaque(mp, ijkmp_get_weak_thiz(mp));
    //ijkmp_android_set_mediacodec_select_callback(mp, mediacodec_select_callback, ijkmp_get_weak_thiz(mp));
    //MPTRACE("FMediaPlayer_native_setup    ref_count=%d\n",mp->ref_count);
LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

void
FMediaPlayer_native_finalize( char* name, char* value)
{
    MPTRACE("hongjian %s\n", __func__);
    FMediaPlayer_release();
}

// NOTE: support to be called from read_thread
static int
inject_callback(void *opaque, int what, void *data, size_t data_size)
{

    object     jbundle = NULL;
    int         ret     = 0;

    return ret;
}

static BOOL mediacodec_select_callback(void *opaque, ijkmp_mediacodecinfo_context *mcc)
{

    return 0;
}

static void fplayer_post_event(void*weak_this, int what, int arg1, int arg2)
{
    pthread_mutex_lock(&g_clazz.mutex);
    if(!Fplayer.Post_Event)
    {
        ALOGE("Fplayer.Post_Event=null\n");
        goto ret ;

    }

    Fplayer.Post_Event(  weak_this,what,  arg1, arg2,NULL);
ret:
    pthread_mutex_unlock(&g_clazz.mutex);
    return ;
}

static void fplayer_post_event2(void *weak_thiz,int what, int arg1, int arg2, void * obj)
{

    if(!Fplayer.Post_Event)
    {
        ALOGE("Fplayer.Post_Event=null\n");
        goto ret ;

    }

    pthread_mutex_lock(&g_clazz.mutex);
    Fplayer.Post_Event( weak_thiz, what,  arg1,  arg2,  obj);
ret:
    pthread_mutex_unlock(&g_clazz.mutex);
    return ;
}
static int32_t swapInt32(int32_t value)  
{  
     return ((value & 0x000000FF) << 24) |  
               ((value & 0x0000FF00) << 8) |  
               ((value & 0x00FF0000) >> 8) |  
               ((value & 0xFF000000) >> 24) ;  
}
static void message_loop_n( IjkMediaPlayer *mp)
{

    CHECK_GOTO(mp, "message_loop_n  null mp", LABEL_RETURN);

    while (1) {
        AVMessage msg;
        int retval = ijkmp_get_msg(mp, &msg, 1);
        if (retval < 0)
            break;

        // block-get should never return 0
        assert(retval > 0);

        switch (msg.what) {
        case FFP_MSG_FLUSH:
            MPTRACE("FFP_MSG_FLUSH:\n");
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_NOP, 0, 0);
            break;
        case FFP_MSG_ERROR:
            MPTRACE("FFP_MSG_ERROR: %d\n", msg.arg1);
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_ERROR, MEDIA_ERROR_IJK_PLAYER, msg.arg1);
            break;
        case FFP_MSG_PREPARED:
            MPTRACE("FFP_MSG_PREPARED:\n");
#if USE_FP_EVENT
	      FMediaPlayer_Report_FpEvent( FFP_MSG_PREPARE_COMPLETED,NULL,NULL,mp);

#endif
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_PREPARED, 0, 0);
            break;
        case FFP_MSG_COMPLETED:
            MPTRACE("FFP_MSG_COMPLETED");
            ff_is_completed = 1;
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_PLAYBACK_COMPLETE, 0, 0);
            break;
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            MPTRACE("FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_SET_VIDEO_SIZE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_SAR_CHANGED:
            MPTRACE("FFP_MSG_SAR_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
            fplayer_post_event(Fplayer.FMediaPlayerClass,MEDIA_SET_VIDEO_SAR, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_VIDEO_RENDERING_START:
            MPTRACE("FFP_MSG_VIDEO_RENDERING_START:\n");
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_INFO, MEDIA_INFO_VIDEO_RENDERING_START, 0);
            break;
        case FFP_MSG_AUDIO_RENDERING_START:
            MPTRACE("FFP_MSG_AUDIO_RENDERING_START:\n");
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_INFO, MEDIA_INFO_AUDIO_RENDERING_START, 0);
            break;
        case FFP_MSG_VIDEO_ROTATION_CHANGED:
            MPTRACE("FFP_MSG_VIDEO_ROTATION_CHANGED: %d\n", msg.arg1);
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_INFO, MEDIA_INFO_VIDEO_ROTATION_CHANGED, msg.arg1);
            break;
        case FFP_MSG_BUFFERING_START:
            MPTRACE("FFP_MSG_BUFFERING_START:\n");
#if USE_FP_EVENT
            if(!ff_is_multicast(ff_gUrl))
                FMediaPlayer_Report_FpEvent(FFP_MSG_BUFFER_START,NULL,NULL,mp);     
#endif
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_INFO, MEDIA_INFO_BUFFERING_START, 0);
            break;
        case FFP_MSG_BUFFERING_END:
            MPTRACE("FFP_MSG_BUFFERING_END:\n");
#if USE_FP_EVENT
            if(!ff_is_multicast(ff_gUrl))
                FMediaPlayer_Report_FpEvent(FFP_MSG_BUFFER_END,NULL,NULL,mp);

#endif
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_INFO, MEDIA_INFO_BUFFERING_END, 0);
            break;
        case FFP_MSG_BUFFERING_UPDATE:
            // MPTRACE("FFP_MSG_BUFFERING_UPDATE: %d, %d", msg.arg1, msg.arg2);
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_BUFFERING_UPDATE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
#if USE_FP_EVENT
            if (ijkmp_get_start_flag(mp))
                FMediaPlayer_Report_FpEvent(FFP_MSG_PLAYABE_REPORT,msg.arg1,NULL,mp);

 #endif           
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            ff_gCacheTime = (int)msg.arg1;
            break;
        case FFP_MSG_SEEK_COMPLETE:
            MPTRACE("FFP_MSG_SEEK_COMPLETE:\n");
#if USE_FP_EVENT
            FMediaPlayer_Report_FpEvent(FFP_MSG_SEEK_END,NULL,NULL,mp);

#endif
            fplayer_post_event(Fplayer.FMediaPlayerClass, MEDIA_SEEK_COMPLETE, 0, 0);
            break;
        case FFP_MSG_PLAYBACK_STATE_CHANGED:
            break;
        case FFP_MSG_TIMED_TEXT:
            if (msg.obj) {
                //jstring text = (*env)->NewStringUTF(env, (char *)msg.obj);
                //fplayer_post_event2(Fplayer.FMediaPlayerClass, MEDIA_TIMED_TEXT, 0, 0, text);
                //J4A_DeleteLocalRef__p(env, &text);
            }
            else {
                fplayer_post_event2(Fplayer.FMediaPlayerClass, MEDIA_TIMED_TEXT, 0, 0, NULL);
            }
            break;
		case FFP_MSG_UDRM_ERROR:
            fplayer_post_event2(Fplayer.FMediaPlayerClass, MEDIA_INFO_UMDRM_ERR, msg.arg1, 0, NULL);
            ALOGE("umplayer FFP_MSG_UDRM_ERROR(%d)\n", msg.arg1);
            break;
        case FFP_MSG_UDRM_CALLBACK:
            fplayer_post_event2(Fplayer.FMediaPlayerClass,MEDIA_INFO_UMDRM_CALLBACK, msg.arg1, 0, NULL);
            ALOGE("umplayer FFP_MSG_UDRM_CALLBACK(%d)\n", msg.arg1);
			break;
        case FFP_MSG_UDRM_AUTH_FAIL:
            fplayer_post_event2(Fplayer.FMediaPlayerClass,MEDIA_INFO_UMDRM_AUTH_FAIL, 0, 0, NULL);
            ALOGE("umplayer MEDIA_INFO_UMDRM_AUTH_FAIL\n");
            break;
        case FFP_MSG_UDRM_BIND_FAIL:
            fplayer_post_event2(Fplayer.FMediaPlayerClass,MEDIA_INFO_UMDRM_BIND_FAIL, 0, 0, NULL);
            ALOGE("umplayer MEDIA_INFO_UMDRM_BIND_FAIL\n");
            break;
		case FFP_MSG_BE_STREAM:
			fplayer_post_event(Fplayer.FMediaPlayerClass,FFP_MSG_BE_STREAM, msg.arg1, msg.arg2);
			break;
        case FFP_MSG_SQM_START_INFO:
                {
                
                struct sqm_start_info *info = (struct sqm_start_info *)msg.arg1;
                if(!info)
                    break;
                memcpy(ff_gChannelURL,info->url,strlen(info->url)+1);
                memcpy(ff_gChannelIp,info->srcip,strlen(info->srcip)+1);
                ff_gChannelPort = info->srcport;
                int ipp=swapInt32(inet_addr(ff_gChannelIp));
                //224.0.0.255 ~ 239.255.255.255 网络字节序是倒序的，需要大小端转换
                if( strstr(ff_gChannelURL,"http://"))
                {
                   ff_gMediaType =3;
                     if(strstr(ff_gUrl,".m3u8"))
                    {
                         strcpy(ff_gChannelURL,ff_gUrl);
                    }
                }
                else if(ipp > 0xe00000ff && ipp <=0xefffffff)
                {
                    ff_gMediaType =2;
                     if(strstr(ff_gChannelURL,"rtp://"))
                    {
                         strcpy(ff_gChannelURL,ff_gUrl);
                         
                    }
                }
                else{
                    ff_gMediaType =1;
                }
                ALOGE("ChannelIp=%s,IP=%x,ChannelURL=%s\n", ff_gChannelIp,ipp,ff_gChannelURL);
                ff_gStbPort = info->stbport;
                free(info);
                
            break;
            }
		/*
        case FFP_MSG_TCP_READ_TIMEOUT:
            {
                if(ff_is_completed==0)
                {
                   post_event2(env, weak_thiz, FFP_MSG_TCP_READ_TIMEOUT, 0, 0, NULL);
                   ALOGE("umplayer FFP_MSG_TCP_READ_TIMEOUT\n");
                }

             break;
            }
        */
case FFP_MSG_FIRST_FRAME_DISPLAYED:
#if USE_FP_EVENT
            ff_gFirstFrameTime = (long)*((int*)msg.arg1);
            FMediaPlayer_Report_FpEvent(FFP_MSG_PLAY_START,NULL,NULL,mp);
#endif
            break;
        case FFP_MSG_BLURREDSCREEN_START:
#if USE_FP_EVENT
            
            FMediaPlayer_Report_FpEvent(FFP_MSG_BLURREDSCREEN_START,NULL,NULL,mp);
#endif
            break;
        case FFP_MSG_BLURREDSCREEN_END:
#if USE_FP_EVENT
            FMediaPlayer_Report_FpEvent(FFP_MSG_BLURREDSCREEN_END,msg.arg1,NULL,mp);
#endif            
            break;
        case FFP_MSG_UNLOAD_START:
#if USE_FP_EVENT
            ALOGE("umplayer FFP_MSG_UNLOAD_START\n");
            if(ff_is_multicast(ff_gUrl))
                FMediaPlayer_Report_FpEvent(FFP_MSG_UNLOAD_START,NULL,NULL,mp);
#endif            
            break;
        case FFP_MSG_UNLOAD_END:
 #if USE_FP_EVENT
            ALOGE("umplayer FFP_MSG_UNLOAD_END\n");
            if(ff_is_multicast(ff_gUrl))
                FMediaPlayer_Report_FpEvent(FFP_MSG_UNLOAD_END,NULL,NULL,mp);
#endif           
            break;
        default:
            fplayer_post_event2(Fplayer.FMediaPlayerClass , msg.what, 0, 0,(object)0);
            ALOGE("unknown FFP_MSG_xxx(%d)\n", msg.what);
            break;
        }
        msg_free_res(&msg);
    }

LABEL_RETURN:
    ;
}

static int message_loop(void *arg)
{
    MPTRACE("hongjian %s\n", __func__);

	if(arg ==NULL) {
		av_log(NULL,AV_LOG_ERROR,"@unionman ijkplayer == null");
	} else {
		av_log(NULL,AV_LOG_ERROR,"@unionman ijkplayer != null");
	}
    IjkMediaPlayer *mp = (IjkMediaPlayer*) arg;
    CHECK_GOTO(mp,  "mpjni: native_message_loop: null mp", LABEL_RETURN);

    message_loop_n(mp);

LABEL_RETURN:
    MPTRACE("message_loop exit");
    ijkmp_dec_ref_p(&mp);
    


    return 0;
}

// ----------------------------------------------------------------------------
void monstartup(const char *libname);
void moncleanup(void);

void
FMediaPlayer_native_profileBegin( char* libName)
{
    MPTRACE("hongjian %s\n", __func__);

    const char *c_lib_name = NULL;
    static int s_monstartup = 0;

    if (!libName)
        return;

    if (s_monstartup) {
        ALOGW("monstartup already called\b");
        return;
    }

    c_lib_name =(char*) libName;
    CHECK_GOTO(c_lib_name, "mpjni: monstartup: libName.string oom", LABEL_RETURN);

    s_monstartup = 1;
    monstartup(c_lib_name);
    ALOGD("monstartup: %s\n", c_lib_name);

LABEL_RETURN:
    return ;
  
}

void
FMediaPlayer_native_profileEnd()
{
    MPTRACE("hongjian %s\n", __func__);
    static int s_moncleanup = 0;

    if (s_moncleanup) {
        ALOGW("moncleanu already called\b");
        return;
    }

    s_moncleanup = 1;
    moncleanup();
    ALOGD("moncleanup\n");
}

void
FMediaPlayer_native_setLogLevel( int level)
{
    MPTRACE("%s(%d)\n", __func__, level);
    ijkmp_global_set_log_level(level);
    ALOGD("moncleanup\n");
}
void 
FMediaPlayer_setPostEvent(void* FMediaPlayerClass,void*  post_event)
{
    if(!post_event)
    {
       ALOGD("FMediaPlayer_setPostEvent post_event=null\n");
       pthread_mutex_lock(&g_clazz.mutex);
       Fplayer.FMediaPlayerClass=NULL;
       Fplayer.Post_Event=NULL;
       pthread_mutex_unlock(&g_clazz.mutex);
	return ;
    }
    pthread_mutex_lock(&g_clazz.mutex);
    ALOGD("FMediaPlayer_setPostEvent post_event=%x FMediaPlayerClass=%x\n",post_event,FMediaPlayerClass);
    Fplayer.FMediaPlayerClass=FMediaPlayerClass;
    Fplayer.Post_Event=post_event;
    pthread_mutex_unlock(&g_clazz.mutex);
    return ;
}

 int FMediaPlayer_SendSqmMsg(int msg)
{

	int retval = 0;
    jobject    jbundle   = NULL;
    int        ret       = -1;
	jobject    weak_thiz = NULL;

	MPTRACE("%s\n", __func__);
    IjkMediaPlayer *mp = get_media_player();
    CHECK_GOTO(mp,  "mpjni: IjkMediaPlayer_SendSqmMsg: null mp", LABEL_RETURN);

	switch(msg)
	{
		case FFP_MSG_SQM_START:
		{
                    fp_sqm_start_event event;
                    strcpy(event.ChannelIp , ff_gChannelIp);
                    event.ChannelPort = ff_gChannelPort;
                    strcpy(event.ChannelURL ,ff_gChannelURL);
                    event.StbPort = ff_gStbPort;
                    event.MediaType = ff_gMediaType;
                    ALOGE("IjkMediaPlayer_SendSqmMsg  msg = %d",msg);
                    fplayer_post_event2(Fplayer.FMediaPlayerClass, msg, 0, 0,&event );
                    //real_data->is_handled = J4AC_IjkMediaPlayer__onNativeInvoke(env, weak_thiz, msg, jbundle);
        			
			break;
		}    

    		case FFP_MSG_SQM_EXITPLAY:
		{
                    ALOGE("IjkMediaPlayer_SendSqmMsg  msg = %d",msg);
			fplayer_post_event(Fplayer.FMediaPlayerClass,msg, 0, 0 );
			break;
		}
		case FFP_MSG_SQM_FASTFORWARD_OR_REWIND:
		{
                    ALOGE("IjkMediaPlayer_SendSqmMsg  msg = %d",msg);
			fplayer_post_event(Fplayer.FMediaPlayerClass, msg, 0, 0 );
			break;
		}
		case FFP_MSG_SQM_PAUSE:
		{
                    ALOGE("IjkMediaPlayer_SendSqmMsg  msg = %d",msg);
			fplayer_post_event(Fplayer.FMediaPlayerClass, msg, 0, 0 );
			break;
		}
		case FFP_MSG_SQM_PCRBITTRATE:
		{
                    ALOGE("IjkMediaPlayer_SendSqmMsg  msg = %d",msg);
			int bittrate = 0;
			fplayer_post_event(Fplayer.FMediaPlayerClass, msg, bittrate, 0 );
			break;
		}
		default:
		    break;

	}
	
LABEL_RETURN:
     ijkmp_dec_ref_p(&mp);
    return ret;	
}
#define ONE_DAY (24*60*60)
int ff_get_timeshift_playtime(IjkMediaPlayer  *mp)
{

    int ret = 0;
    long curr_pos = ijkmp_get_current_position(mp);
    ALOGE("%s %d curr_pos=%ld",__FUNCTION__,__LINE__,curr_pos);
    if(curr_pos == -1)
        return 0;
    int pos = (curr_pos+8*60*60)%ONE_DAY;
    ALOGE("%s %d pos=%d",__FUNCTION__,__LINE__,pos);
    int64_t current_time = av_gettime();
    int time = (current_time/1000000+8*60*60)%ONE_DAY;
    
    
    if( pos -time > 20*60*60 )//¿çÓòkua yu
    {
          ret =  (time+ONE_DAY) - pos;
    }
    else if(pos -time >0 && pos -time < 60)
    {
         ret = 0;
    }
    else
    {
         ret = time - pos;
    }
    ALOGE("%s %d pos=%d,time=%d,ret=%d",__FUNCTION__,__LINE__,pos,time,ret);
    return ret;

}
static int ff_get_diff_time(int start_s,int end_s)//end_s>start_s
{
    int ret = 0;
    int pos = (start_s+8*60*60)%ONE_DAY;
    ALOGE("%s %d pos=%d,start_s=%d,end_s=%d",__FUNCTION__,__LINE__,pos,start_s,end_s);
    int current_time = end_s;
    int time = (current_time+8*60*60)%ONE_DAY;
    
    if( pos -time > 20*60*60 )//¿çÓòkua yu
    {
          ret =  (time+ONE_DAY) - pos;
    }
    else if(pos -time >0 && pos -time < 60)
    {
         ret = 0;
    }
    else
    {
         ret = time - pos;
    }
    ALOGE("%s %d pos=%d,time=%d,ret=%d",__FUNCTION__,__LINE__,pos,time,ret);
    return ret;    
}
 int FMediaPlayer_Report_FpEvent(int msg,void* arg,void* arg2,void*obj)
{

	int retval = 0;
    jobject    jbundle   = NULL;
    int        ret       = -1;
	jobject    weak_thiz = NULL;
    IjkMediaPlayer *mp =NULL;
    int is_dec_ref = 0;
    int64_t current_time = av_gettime();

    mp = obj;

    CHECK_GOTO(mp, "mpjni: IjkMediaPlayer_Report_FpEvent: null mp", LABEL_RETURN);

    
    switch(msg)
        {
		case FFP_MSG_PLAY_PREPARE:
			{
			fp_play_perpare_event perpare_event;
			strcpy(perpare_event.type ,"PLAY_PREPARE");
			perpare_event.id = ff_gPlayerID;
			strcpy(perpare_event.url,ff_gUrl);
			perpare_event.start_time = current_time/1000;
			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PLAY_PREPARE, 0, 0,&perpare_event);
			ff_gCurrentEventState = FFP_MSG_PLAY_PREPARE;
			break;
			}

		case FFP_MSG_PLAY_STARTUP:
			 {
			  fp_play_startup_event play_startup_event;
			  strcpy(play_startup_event.type ,"PLAY_STARTUP");
			  play_startup_event.id = ff_gPlayerID;
			  play_startup_event.time = current_time/1000;

			  fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PLAY_STARTUP, 0, 0,&play_startup_event);
			  ff_gCurrentEventState = FFP_MSG_PLAY_STARTUP;

			   break;
			  }
			 
		case FFP_MSG_RESUME_MESSAGE:
			{
			  fp_resume_event resume_event;
                      int program_type = -1;
			  strcpy(resume_event.type ,"RESUME_MESSAGE");
			  resume_event.id = ff_gPlayerID;
                      resume_event.time = current_time/1000;
                      ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                      
                      if(IS_LIVE(program_type))
                          resume_event.play_time = (resume_event.time/1000+8*60*60)%ONE_DAY;
                      else if(IS_TIMESHIFT(program_type))
                          resume_event.play_time = ff_get_timeshift_playtime(mp);
                      else
			      resume_event.play_time = (int) ff_gCurrentPosition/1000;
			  
			  strcpy(resume_event.url,ff_gUrl);
			  ijkmp_invoke2(INVOKE_GET_PARAMETER_BITRATE,&resume_event.bitrate,0,(jobject)mp);
			  ijkmp_invoke2(INVOKE_GET_PARAMETER_WIDTH_HEIGTH,&resume_event.width,&resume_event.height,(jobject)mp);
			  strcpy(resume_event.program_name , "unknow");
			  strcpy(resume_event.program_type , "unknow");
			  ijkmp_invoke2(INVOKE_GET_PARAMETER_DECODE_NAME,resume_event.video_codec,resume_event.audio_codec,(jobject)mp);

			  fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_RESUME_MESSAGE, 0, 0,&resume_event);
			  ff_gCurrentEventState = FFP_MSG_RESUME_MESSAGE;
	
			  break;
			}

		case FFP_MSG_PAUSE_MESSAGE:
			{
			fp_pause_event pause_event;
                    int program_type = -1;
			strcpy(pause_event.type ,"PAUSE_MESSAGE");
			pause_event.id = ff_gPlayerID;
            
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    pause_event.time =  current_time/1000;
                    if(IS_LIVE(program_type))
                        pause_event.play_time = (pause_event.time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    pause_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        pause_event.play_time = (int)ijkmp_get_current_position(mp)/1000;
			

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PAUSE_MESSAGE, 0, 0,&pause_event);
			ff_gCurrentEventState = FFP_MSG_PAUSE_MESSAGE;

			 break;
			}
		case FFP_MSG_SEEK_START:
			{
			fp_seek_start_event  seek_start_event;
                    int program_type = -1;
                    long seek_time = arg;
			strcpy(seek_start_event.type ,"SEEK_START");
			seek_start_event.id = ff_gPlayerID;
			seek_start_event.start_time = current_time/1000;
                    MPTRACE("start_time:%ld\n",seek_start_event.start_time);
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    ALOGE("INVOKE_GET_PROGRAM_TYPE program_type=%d",program_type);
                    if(IS_LIVE(program_type))
                        seek_start_event.play_time = ff_get_diff_time(seek_time,seek_start_event.start_time/1000%ONE_DAY);
                    else if(IS_TIMESHIFT(program_type))
			    seek_start_event.play_time =ff_get_diff_time(seek_time,seek_start_event.start_time/1000%ONE_DAY);
                    else
                        seek_start_event.play_time = (int)ijkmp_get_current_position(mp)/1000;

                    ALOGE("INVOKE_GET_PROGRAM_TYPE seek_start_event.play_time=%d",seek_start_event.play_time);

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_SEEK_START, 0, 0,&seek_start_event);
			ff_gCurrentEventState = FFP_MSG_SEEK_START;

			break;
			}

		case FFP_MSG_PLAY_QUIT:
			{
			fp_play_quit_event  play_quit_event;
                    int program_type = -1;
			strcpy(play_quit_event.type ,"PLAY_QUIT");
			play_quit_event.id = ff_gPlayerID;
			play_quit_event.time = current_time/1000;
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    
                    if(IS_LIVE(program_type))
                        play_quit_event.play_time = (play_quit_event.time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    play_quit_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        play_quit_event.play_time = (int)ijkmp_get_current_position(mp)/1000;

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PLAY_QUIT, 0, 0,&play_quit_event);
			ff_gCurrentEventState = FFP_MSG_PLAY_QUIT;

			break;
			}  

		case FFP_MSG_PREPARE_COMPLETED:
			{
			fp_prepare_completed_event prepare_completed_event;
			strcpy(prepare_completed_event.type ,"PREPARE_COMPLETED");
			prepare_completed_event.id = ff_gPlayerID;
			prepare_completed_event.time =  current_time/1000;

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PREPARE_COMPLETED ,0, 0,&prepare_completed_event);
			ff_gCurrentEventState = FFP_MSG_PREPARE_COMPLETED;

			break;
			}

		case FFP_MSG_BUFFER_START:
			{
                    if(ff_gCurrentEventState == FFP_MSG_SEEK_START || ff_gCurrentEventState == FFP_MSG_SEEK_END)
                        break;
			fp_buffer_start_event  buffer_start_event;
                    int program_type = -1;
			strcpy(buffer_start_event.type ,"BUFFER_START");
			buffer_start_event.id = ff_gPlayerID;
			buffer_start_event.start_time = current_time/1000;
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    
                    if(IS_LIVE(program_type))
                        buffer_start_event.play_time = (buffer_start_event.start_time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    buffer_start_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        buffer_start_event.play_time = (int)ijkmp_get_current_position(mp)/1000;
			
			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_BUFFER_START ,0, 0,&buffer_start_event);
			ff_gCurrentEventState = FFP_MSG_BUFFER_START;

			break;
			}

		case FFP_MSG_BUFFER_END:
			{
                     if(FFP_MSG_SEEK_START == ff_gCurrentEventState || 
                       FFP_MSG_SEEK_END == ff_gCurrentEventState ||
                       FFP_MSG_BUFFER_START  != ff_gCurrentEventState)
                       {
                             break;
                     }
			fp_buffer_end_event  buffer_end_event;
			strcpy(buffer_end_event.type ,"BUFFER_END");
			buffer_end_event.id = ff_gPlayerID;
			buffer_end_event.end_time = current_time/1000;

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_BUFFER_END ,0, 0,&buffer_end_event);
			ff_gCurrentEventState = FFP_MSG_BUFFER_END;

			break;
			} 

		case FFP_MSG_PLAYABE_REPORT:
			{
			fp_playabe_report_event  playabe_report_event;
                    int program_type = -1;
			strcpy(playabe_report_event.type ,"PLAYABE_REPORT");
			playabe_report_event.id = ff_gPlayerID;
			playabe_report_event.time = current_time/1000;
			playabe_report_event.seconds = (ff_gCacheTime+500)/1000;
			playabe_report_event.bytes = arg;
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                     ALOGE("INVOKE_GET_PROGRAM_TYPE program_type=%d",program_type);
                    if(IS_LIVE(program_type))
                        playabe_report_event.play_time = (playabe_report_event.time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    playabe_report_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        playabe_report_event.play_time =  ijkmp_get_current_position(mp)/1000;
                    ALOGE("INVOKE_GET_PROGRAM_TYPE playabe_report_event.play_time=%d",playabe_report_event.play_time);

			playabe_report_event.pre_fec = 0;
			playabe_report_event.after_fec = 0;

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PLAYABE_REPORT ,0, 0,&playabe_report_event); 
			//ff_gCurrentEventState = FFP_MSG_PLAYABE_REPORT;

			break;
			}

		case FFP_MSG_SEEK_END:
			{

			fp_seek_end_event  seek_end_event;
                    int program_type = -1;
			strcpy(seek_end_event.type ,"SEEK_END");
			seek_end_event.id = ff_gPlayerID;
			seek_end_event.end_time = current_time/1000;
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    ALOGE("INVOKE_GET_PROGRAM_TYPE program_type=%d",program_type);
                    if(IS_LIVE(program_type))
                        seek_end_event.play_time = (seek_end_event.end_time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    seek_end_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        seek_end_event.play_time = (int)ijkmp_get_current_position(mp)/1000;

                    ALOGE("INVOKE_GET_PROGRAM_TYPE seek_end_event.play_time=%d",seek_end_event.play_time);
                    

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_SEEK_END ,0, 0,&seek_end_event); 
			ff_gCurrentEventState = FFP_MSG_SEEK_END;

			break;
			}

		case FFP_MSG_PLAY_START:
			{
			fp_play_start_event play_start_event;
                    int program_type = -1;
			strcpy(play_start_event.type ,"PLAY_START");
			play_start_event.id = ff_gPlayerID;
                    ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                    play_start_event.end_time = current_time/1000;
                    
                    if(IS_LIVE(program_type))
                        play_start_event.play_time = (play_start_event.end_time/1000+8*60*60)%ONE_DAY;
                    else if(IS_TIMESHIFT(program_type))
			    play_start_event.play_time = ff_get_timeshift_playtime(mp);
                    else
                        play_start_event.play_time =  ff_gCurrentPosition/1000;
                    
			//play_start_event.play_time =  ff_gCurrentPosition/1000;
			
			ijkmp_invoke2(INVOKE_GET_PARAMETER_BITRATE,&play_start_event.bitrate,0,(jobject)mp);
			ijkmp_invoke2(INVOKE_GET_PARAMETER_WIDTH_HEIGTH,&play_start_event.width,&play_start_event.height,(jobject)mp);
			strcpy(play_start_event.program_name , "unknow");
			strcpy(play_start_event.program_type , "unknow");
			ijkmp_invoke2(INVOKE_GET_PARAMETER_FRAMERATE,&play_start_event.framerate,0,(jobject)mp);
			ijkmp_invoke2(INVOKE_GET_PARAMETER_DECODE_NAME,play_start_event.video_codec,play_start_event.audio_codec,(jobject)mp);

			fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_PLAY_START,0,0,&play_start_event);
                    ALOGE("umplayer FFP_MSG_PLAY_START\n");
                    ijkmp_set_start_flag(mp, 1);
			ff_gCurrentEventState = FFP_MSG_PLAY_START;

			break;
			}
             case FFP_MSG_ERROR_MESSAGE:
                    {
                        fp_play_error_event error_event ;
                        strcpy(error_event.type ,"ERROR_MESSAGE");
                        error_event.id = ff_gPlayerID;
                        error_event.error_code = 10001;
                        error_event.time = current_time/1000;

                        fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_ERROR_MESSAGE,0,0,&error_event);
                        ff_gCurrentEventState = FFP_MSG_ERROR_MESSAGE;

                        break;
                   }
                 case FFP_MSG_UNLOAD_START:
			{
                    if(ff_gCurrentEventState == FFP_MSG_SEEK_START || ff_gCurrentEventState == FFP_MSG_SEEK_END)
                        break;				
				fp_unload_start_event start_event ;
                           int program_type = -1;
				strcpy(start_event.type ,"UNLOAD_START");
				start_event.id = ff_gPlayerID;
			      start_event.start_time = current_time/1000;
                           ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                           
                           if(IS_LIVE(program_type))
                                start_event.play_time = (start_event.start_time/1000+8*60*60)%ONE_DAY;
                          else if(IS_TIMESHIFT(program_type))
            		            start_event.play_time = ff_get_timeshift_playtime(mp);
                          else
                               start_event.play_time = (int)ijkmp_get_current_position(mp)/1000;

				fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_UNLOAD_START,0,0,&start_event);
				ff_gCurrentEventState = FFP_MSG_UNLOAD_START;

				break;
			}
		    case FFP_MSG_UNLOAD_END:
			{
                         if(ff_gCurrentEventState == FFP_MSG_SEEK_START ||
                            ff_gCurrentEventState == FFP_MSG_SEEK_END||
                            ff_gCurrentEventState  != FFP_MSG_UNLOAD_START)
                              break;

				fp_unload_end_event end_event ;
				strcpy(end_event.type ,"UNLOAD_END");
				end_event.id = ff_gPlayerID;
			       end_event.end_time = current_time/1000;

				fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_UNLOAD_END,0,0,&end_event);
				ff_gCurrentEventState = FFP_MSG_UNLOAD_END;

				break;
			}
                 case FFP_MSG_BLURREDSCREEN_START:
                 {
				fp_blurredscren_start_event start_event ;
                          int program_type = -1;
				strcpy(start_event.type ,"BLURREDSCREEN_START");
				start_event.id = ff_gPlayerID;
			      start_event.start_time = current_time/1000;
                          ijkmp_invoke2(INVOKE_GET_PROGRAM_TYPE,&program_type,0,(jobject)mp);
                          
                            if(IS_LIVE(program_type))
                                start_event.play_time = (start_event.start_time/1000+8*60*60)%ONE_DAY;
                            else if(IS_TIMESHIFT(program_type))
        			    start_event.play_time = ff_get_timeshift_playtime(mp);
                            else
                               start_event.play_time = (int)ijkmp_get_current_position(mp)/1000;
                           

				fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_BLURREDSCREEN_START,0,0,&start_event);
				ff_gCurrentEventState = FFP_MSG_BLURREDSCREEN_START;

				break;
                 }
                 case FFP_MSG_BLURREDSCREEN_END:
                  {
				fp_blurredscren_end_event end_event ;
				strcpy(end_event.type ,"BLURREDSCREEN_END");
				end_event.id = ff_gPlayerID;
			      end_event.end_time = current_time/1000;
                          end_event.ratio = (int)arg;
			
				fplayer_post_event2(Fplayer.FMediaPlayerClass,FFP_MSG_BLURREDSCREEN_END,0,0,&end_event);
				ff_gCurrentEventState = FFP_MSG_BLURREDSCREEN_END;
				
				break;
                 }

			default:
			
				break;

	}

LABEL_RETURN:
    return ret;	
}

void FMediaPlayer_invoke(int cmd,object obj)
{
    MPTRACE("%s\n", __func__);
    char* o ;
    switch(cmd)
    {
      case INVOKE_FREE_AVPLAYER:
      case INVOKE_SET_HAL_WINDOWS_SIZE:
        ijkmp_invoke( cmd, obj);
        break;
      case INVOKE_SET_PARAMETER_PLAYERID:
      {
            //jclass integer_class = (*env)->FindClass(env,"java/lang/Integer");
            //jmethodID int_value_methodID = (*env)->GetMethodID(env,integer_class, "intValue", "()I");
            //jint result = (*env)->CallIntMethod(env,obj, int_value_methodID);
            ff_gPlayerID =obj ; 
            break;
      }
      default:
        break;

    }
    return ;
}

typedef struct __FPlayer_API
{
char* module;
void (*setDataSourceAndHeaders)(char* ,void* , void* );
void (*setDataSourceFd)(int );
void (*setDataSourceCallback)(void* );
void (*setDataSourceIjkIOHttpCallback)(  void* ); 
void (*setVideoSurface)(  void* ,int ,int ,int ,int );
void (*prepareAsync)();
void (*start)(void);
void (*stop)(void);
void (*pause)(void);
void (*seekTo)( long );
BOOL (*isPlaying)(void);
long (*getCurrentPosition)();
long (*getDuration)(void);
long (*getBitRate)(void);
void (*release)(void);
void (*native_setup)( void* );
void (*reset)(void);
void (*setLoopCount)( int );
int   (*getLoopCount)(void);
float (*getPropertyFloat)( int , float );
void (*setPropertyFloat)( int , float );
long (*getPropertyLong)( int , long );
void (*setPropertyLong)(int , long );
void (*setStreamSelected)(int , int );
void (*setVolume)( float , float );
int   (*getAudioSessionId)(void);
void (*setOption)( int , object , object );
void (*setOptionLong)( int , object , long );
char* (*getColorFormatName)( int );
char* (*getVideoCodecInfo)(void);
char* (*getAudioCodecInfo)(void);
void* (*getMediaMeta)(void);
void (*native_init)(void);
void (*native_deinit)(void);
void (*native_finalize)( char* , char* );
void (*native_profileBegin)( char* );
void (*native_profileEnd)(void);
void (*native_setLogLevel)( int );
void (*setPostEvent)(void*,void* );
void (*invoke)(int ,object );
int   (*report_fpevent)(int ,void* ,void* ,void*);
int (*sendSqmMsg)(int );
void (*fastplay)(int);
}FPlayer_API;

FPlayer_API fplayer={
.module="fplayer",
.setDataSourceAndHeaders = FMediaPlayer_setDataSourceAndHeaders,
.setDataSourceFd=FMediaPlayer_setDataSourceFd,
.setDataSourceCallback=FMediaPlayer_setDataSourceCallback,
.setDataSourceIjkIOHttpCallback=FMediaPlayer_setDataSourceIjkIOHttpCallback,
.setVideoSurface=FMediaPlayer_setVideoSurface,
.prepareAsync=FMediaPlayer_prepareAsync,
.start=FMediaPlayer_start,
.stop= FMediaPlayer_stop,
.pause= FMediaPlayer_pause,
.seekTo= FMediaPlayer_seekTo,
.isPlaying= FMediaPlayer_isPlaying,
.getCurrentPosition= FMediaPlayer_getCurrentPosition,
.getDuration= FMediaPlayer_getDuration,
.getBitRate=FMediaPlayer_getBit_Rate,
.release= FMediaPlayer_release,
.native_setup= FMediaPlayer_native_setup,
.reset=FMediaPlayer_reset,
.setLoopCount= FMediaPlayer_setLoopCount,
.getLoopCount= FMediaPlayer_getLoopCount,
.getPropertyFloat= FMediaPlayer_getPropertyFloat,
.setPropertyFloat= FMediaPlayer_setPropertyFloat,
.getPropertyLong= FMediaPlayer_getPropertyLong,
.setPropertyLong= FMediaPlayer_setPropertyLong,
.setStreamSelected= FMediaPlayer_setStreamSelected,
.setVolume= FMediaPlayer_setVolume,
.getAudioSessionId= FMediaPlayer_getAudioSessionId,
.setOption= FMediaPlayer_setOption,
.setOptionLong= FMediaPlayer_setOptionLong,
.getColorFormatName= FMediaPlayer_getColorFormatName,
.getVideoCodecInfo= FMediaPlayer_getVideoCodecInfo,
.getAudioCodecInfo= FMediaPlayer_getAudioCodecInfo,
.getMediaMeta= FMediaPlayer_getMediaMeta,
.native_init= FMediaPlayer_native_init,
.native_deinit= FMediaPlayer_native_deinit,
.native_finalize= FMediaPlayer_native_finalize,
.native_profileBegin= FMediaPlayer_native_profileBegin,
.native_profileEnd= FMediaPlayer_native_profileEnd,
.native_setLogLevel= FMediaPlayer_native_setLogLevel,
.setPostEvent= FMediaPlayer_setPostEvent,
.invoke = FMediaPlayer_invoke,
.report_fpevent =  FMediaPlayer_Report_FpEvent,
.sendSqmMsg = FMediaPlayer_SendSqmMsg,
.fastplay = FMediaPlayer_fastplay
};

