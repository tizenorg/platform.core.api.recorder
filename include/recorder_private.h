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
#define __TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__
#include <camera.h>
#include <recorder.h>
#include <muse_core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORDER_PARSE_STRING_SIZE 30
#define RECORDER_MSG_LENGTH_MAX    5120

#define RECORDER_MSG_PARAM_SET(param, msg_type, set_value) { \
	param.type = MUSE_TYPE_##msg_type; \
	param.name = #set_value; \
	param.value.value_##msg_type = set_value; \
}

enum {
	_RECORDER_GET_INT_STATE = 0,
	_RECORDER_GET_INT_VIDEO_RESOLUTION,
	_RECORDER_GET_INT_FILE_FORMAT,
	_RECORDER_GET_INT_AUDIO_ENCODER,
	_RECORDER_GET_INT_VIDEO_ENCODER,
	_RECORDER_GET_INT_SIZE_LIMIT,
	_RECORDER_GET_INT_TIME_LIMIT,
	_RECORDER_GET_INT_AUDIO_DEVICE,
	_RECORDER_GET_INT_AUDIO_SAMPLERATE,
	_RECORDER_GET_INT_AUDIO_ENCODER_BITRATE,
	_RECORDER_GET_INT_VIDEO_ENCODER_BITRATE,
	_RECORDER_GET_INT_AUDIO_CHANNEL,
	_RECORDER_GET_INT_ORIENTATION_TAG,
	_RECORDER_GET_INT_MAX
};

enum {
	_RECORDER_GET_DOUBLE_AUDIO_LEVEL = 0,
	_RECORDER_GET_DOUBLE_RECORDING_MOTION_RATE,
	_RECORDER_GET_DOUBLE_MAX
};

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
	gchar recv_msg[RECORDER_MSG_LENGTH_MAX];
	GCond api_cond[MUSE_RECORDER_API_MAX];
	GMutex api_mutex[MUSE_RECORDER_API_MAX];
	gint *api_activating;
	gint *api_ret;
	tbm_bufmgr bufmgr;

	/* get values */
	char *get_filename;
	gint get_int_value[_RECORDER_GET_INT_MAX];
	gdouble get_double_value[_RECORDER_GET_DOUBLE_MAX];
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

typedef struct _recorder_cli_s {
	intptr_t remote_handle;
	recorder_cb_info_s *cb_info;
	camera_h camera;
} recorder_cli_s;

typedef struct _recorder_msg_param {
	int type;
	const char *name;
	union {
		int value_INT;
		double value_DOUBLE;
		const char *value_STRING;
	} value;
} recorder_msg_param;

typedef struct _camera_cli_s {
	intptr_t remote_handle;
	void *cb_info;
} camera_cli_s;

int __convert_recorder_error_code(const char *func, int code);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__ */


