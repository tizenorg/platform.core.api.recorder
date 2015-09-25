/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



#ifndef __TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__
#define	__TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__
#include <camera.h>
#include <mm_camcorder.h>
#include <recorder.h>
#include <mmsvc_core.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union _mediaSource{
	camera_h camera;
}mediasource;

typedef enum {
	_RECORDER_EVENT_TYPE_STATE_CHANGE,
	_RECORDER_EVENT_TYPE_RECORDING_LIMITED,
	_RECORDER_EVENT_TYPE_RECORDING_STATUS,
	_RECORDER_EVENT_TYPE_INTERRUPTED,
	_RECORDER_EVENT_TYPE_AUDIO_STREAM,
	_RECORDER_EVENT_TYPE_ERROR,
	_RECORDER_EVENT_TYPE_NUM
}_recorder_event_e;

typedef struct _callback_cb_info {
	GThread *thread;
	gint running;
	gint fd;
	gpointer user_cb[MMSVC_RECORDER_EVENT_TYPE_NUM];
	gpointer user_cb_completed[MMSVC_RECORDER_EVENT_TYPE_NUM];
	gpointer user_data[MMSVC_RECORDER_EVENT_TYPE_NUM];
	gchar recvMsg[MM_MSG_MAX_LENGTH];
	gchar recvApiMsg[MM_MSG_MAX_LENGTH];
	gchar recvEventMsg[MM_MSG_MAX_LENGTH];
	GCond *pCond;
	GMutex *pMutex;
	gint *activating;
} callback_cb_info_s;

typedef struct _recorder_cli_s{
	intptr_t remote_handle;
	callback_cb_info_s *cb_info;
}recorder_cli_s;

typedef struct _camera_cli_s{
	intptr_t remote_handle;
	MMHandleType client_handle;
	callback_cb_info_s *cb_info;
}camera_cli_s;

int __convert_recorder_error_code(const char *func, int code);
#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__


