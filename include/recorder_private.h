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

typedef enum {
	_RECORDER_TYPE_AUDIO = 0,
	_RECORDER_TYPE_VIDEO
}_recorder_type_e;

typedef enum {
	_RECORDER_SOURCE_TYPE_UNKNOWN,
	_RECORDER_SOURCE_TYPE_CAMERA,
}_recorder_source_type_e;

typedef struct _recorder_s{
	MMHandleType mm_handle;
	mediasource mm_source;
	void* user_cb[_RECORDER_EVENT_TYPE_NUM];
	void* user_data[_RECORDER_EVENT_TYPE_NUM];
	unsigned int state;
	int camera_device_count;
	_recorder_type_e type;
	_recorder_source_type_e src_type;
	int origin_preview_format;
	int changed_preview_format;
	double last_max_input_level;
} recorder_s;

int __convert_recorder_error_code(const char *func, int code);
#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MULTIMEDIA_RECORDER_PRIVATE_H__


