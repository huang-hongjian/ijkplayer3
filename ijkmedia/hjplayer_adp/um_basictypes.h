/**************************************************************************
*
*  Copyright(c) 2017,UNIONMAN TECHNOLOGY CO.,LID
*  All rights reserved
*
*  AUTHOR�� //�ļ��Ĵ�����
*
*  File: um_basictypes.h
*
*  Description: //�ļ��Ĺ�������
*
*  History:
*      Date        	Author         	Version   		Descripion
*
**************************************************************************/
#ifndef __UM_BASICTYPES_H__
#define __UM_BASICTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef char UM_S8;             /* 8λ�з����� */
typedef unsigned char UM_U8;    /* 8λ�޷����� */

typedef short UM_S16;           /* 16λ�з����� */
typedef unsigned short UM_U16;  /* 16λ�޷����� */

typedef int UM_S32;             /* 32λ�з����� */
typedef unsigned int UM_U32;    /* 32λ�޷����� */

typedef long long UM_S64;       /* 64λ�з����� */
typedef unsigned long long UM_U64;  /* 64λ�޷����� */

typedef long UM_LONG;               /* �з��ų����� */
typedef unsigned long UM_ULONG;     /* �޷��ų����� */

typedef char UM_CHAR;           /* �ַ��� */
typedef UM_U8   UM_BYTE;        /* �ֽ����� */

typedef float UM_FLOAT;         /* ������ */
typedef double UM_DOUBLE;       /* ˫���ȸ����� */

typedef UM_U16	UM_WORD;        /* �� */
typedef UM_U32	UM_DWORD;       /* ˫�� */
#define UM_VOID void              /* �� */ /*typedef void UM_VOID ���ַ�ʽg++��������֧��*/


typedef enum
{
    UM_FALSE    = (0==1),
    UM_TRUE     = (0==0)
} UM_BOOL; /*��������*/

typedef  UM_VOID*  UM_HANDLE ;


#define UM_KEY_NAME_LEN_MAX     (128)
#define UM_KEY_VALUE_LEN_MAX    (1024)

typedef struct UM_KEY_VALUE_S {
    UM_CHAR key[UM_KEY_NAME_LEN_MAX];
    UM_CHAR value[UM_KEY_VALUE_LEN_MAX];
} UM_KEY_VALUE_S;


/******��������*******/
#define UM_NULL      (0)
#define UM_SUCCESS ((UM_S32)0)
#define UM_FAILURE ((UM_S32)-1)

typedef struct
{
	UM_U32 u32StreamType;
	UM_U32 u32APid;
	UM_U32 u32VPid;
	UM_U32 u32AStreamNum;
	UM_U32 u32VStreamNum;
	UM_U32 u32AdecType;
	UM_VOID*  pACodecContext;
	UM_U32 u32VdecType;
	UM_U32 u32DispFmt;
	UM_U32 u32VoMode;
    UM_U32 frameRate;
	UM_U32 x;
	UM_U32 y;
	UM_U32 w;
	UM_U32 h;
       UM_U32 samplerate; ///< samples per second
       UM_U32 channels;    ///< number of audio channels

}FPLAYER_ATTR_S;

typedef struct UM_PHAL_CREATE_PARAM_S
{
       UM_BOOL     bUsed;
       UM_HANDLE hAvPlayer;
       UM_HANDLE hWindow;
       UM_HANDLE hTrack;
       FPLAYER_ATTR_S attr;
} UM_PHAL_CREATE_PARAM_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __UM_BASICTYPES_H__ */

