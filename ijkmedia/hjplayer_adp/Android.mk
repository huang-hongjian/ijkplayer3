LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := hjplayer_adp.c
LOCAL_MODULE := libHJ_MediaProcessor
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)