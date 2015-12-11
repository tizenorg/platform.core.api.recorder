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
#include <muse_core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORDER_PARSE_STRING_SIZE 10


typedef struct _recorder_cb_info_s {
	gint fd;
	GThread *msg_recv_thread;
	GThread *msg_handler_thread;
	gint msg_recv_running;
	gint msg_handler_running;
	GCond msg_handler_cond;
	GMutex msg_handler_mutex;
	GQueue *msg_queue;
	GList *idle_event_list;
	GCond idle_event_cond;
	GMutex idle_event_mutex;
	gpointer user_cb[MUSE_RECORDER_EVENT_TYPE_NUM];
	gpointer user_data[MUSE_RECORDER_EVENT_TYPE_NUM];
	gchar recv_msg[MUSE_RECORDER_MSG_MAX_LENGTH];
	GCond *api_cond;
	GMutex *api_mutex;
	gint *api_activating;
	gint *api_ret;
	tbm_bufmgr bufmgr;
} recorder_cb_info_s;

typedef struct _recorder_message_s {
	gchar recv_msg[MUSE_RECORDER_MSG_MAX_LENGTH];
	muse_recorder_api_e api;
} recorder_message_s;

typedef struct _recorder_idle_event_s {
	recorder_cb_info_s *cb_info;
	gchar recv_msg[MUSE_RECORDER_MSG_MAX_LENGTH];
	muse_recorder_event_e event;
	GMutex event_mutex;
} recorder_idle_event_s;

typedef struct _recorder_cli_s{
	intptr_t remote_handle;
	recorder_cb_info_s *cb_info;
} recorder_cli_s;

typedef struct _camera_cli_s{
	intptr_t remote_handle;
	MMHandleType client_handle;
	void *cb_info;
} camera_cli_s;

int __convert_recorder_error_code(const char *func, int code);

#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__


