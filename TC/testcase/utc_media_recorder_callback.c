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



#include <tet_api.h>
#include <media/recorder.h>

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

 
static void utc_media_recorder_foreach_supported_audio_encoder_p(void);
static void utc_media_recorder_foreach_supported_audio_encoder_n(void);
static void utc_media_recorder_foreach_supported_file_format_p(void);
static void utc_media_recorder_foreach_supported_file_format_n(void);
static void utc_media_recorder_foreach_supported_video_encoder_p(void);
static void utc_media_recorder_foreach_supported_video_encoder_n(void);
static void utc_media_recorder_set_recording_limit_reached_cb_p(void);
static void utc_media_recorder_set_recording_limit_reached_cb_n(void);
static void utc_media_recorder_set_recording_status_cb_p(void);
static void utc_media_recorder_set_recording_status_cb_n(void);
static void utc_media_recorder_set_state_changed_cb_p(void);
static void utc_media_recorder_set_state_changed_cb_n(void); 
static void utc_media_recorder_unset_recording_limit_reached_cb_p(void);
static void utc_media_recorder_unset_recording_limit_reached_cb_n(void);
static void utc_media_recorder_unset_recording_status_cb_p(void);
static void utc_media_recorder_unset_recording_status_cb_n(void);
static void utc_media_recorder_unset_state_changed_cb_p(void);
static void utc_media_recorder_unset_state_changed_cb_n(void); 


struct tet_testlist tet_testlist[] = {
	{ utc_media_recorder_foreach_supported_audio_encoder_p , 1 },
	{ utc_media_recorder_foreach_supported_audio_encoder_n , 2 },
	{ utc_media_recorder_foreach_supported_file_format_p , 1 },
	{ utc_media_recorder_foreach_supported_file_format_n , 2 },
	{ utc_media_recorder_foreach_supported_video_encoder_p , 1 },
	{ utc_media_recorder_foreach_supported_video_encoder_n , 2 }, 
	{ utc_media_recorder_set_recording_limit_reached_cb_p , 1 },
	{ utc_media_recorder_set_recording_limit_reached_cb_n , 2 },
	{ utc_media_recorder_set_recording_status_cb_p , 1 },
	{ utc_media_recorder_set_recording_status_cb_n , 2 },
	{ utc_media_recorder_set_state_changed_cb_p , 1 },
	{ utc_media_recorder_set_state_changed_cb_n , 2 }, 
	{ utc_media_recorder_unset_recording_limit_reached_cb_p , 1 },
	{ utc_media_recorder_unset_recording_limit_reached_cb_n , 2 },
	{ utc_media_recorder_unset_recording_status_cb_p , 1 },
	{ utc_media_recorder_unset_recording_status_cb_n , 2 },
	{ utc_media_recorder_unset_state_changed_cb_p , 1 },
	{ utc_media_recorder_unset_state_changed_cb_n , 2 }, 
	{ NULL, 0 },
};

recorder_h recorder;

static void startup(void)
{
	/* start of TC */
	int ret;
	ret = recorder_create_audiorecorder(&recorder);	
	if( ret != 0 )
		dts_fail("recorder_create_audiorecorder", "Could not create audio recorder"); 	
}

static void cleanup(void)
{
	/* end of TC */
	recorder_destroy(recorder);	
}

bool _audio_encoder_cb(recorder_audio_codec_e codec , void *user_data){
	return false;
}

bool _file_format_cb(recorder_file_format_e format , void *user_data){
	return false;
}

bool _video_encoder_cb(recorder_video_codec_e codec ,  void *user_data){
	return false;
}

void _limit_reached_cb(recorder_recording_limit_type_e type,  void *user_data){
}

void _recording_statis_cb(int elapsed_time, int file_size,  void *user_data){
}

void _state_changed_cb(recorder_state_e previous , recorder_state_e current , bool by_asm,  void *user_data)
{
}


static void utc_media_recorder_foreach_supported_audio_encoder_p(void)
{
	int ret;
	ret = recorder_foreach_supported_audio_encoder(recorder, _audio_encoder_cb, NULL);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_foreach_supported_audio_encoder_n(void)
{
	int ret;
	ret = recorder_foreach_supported_audio_encoder(recorder, NULL, NULL);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");			
}

static void utc_media_recorder_foreach_supported_file_format_p(void)
{
	int ret;
	ret = recorder_foreach_supported_file_format(recorder, _file_format_cb, NULL);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_foreach_supported_file_format_n(void)
{
	int ret;
	ret = recorder_foreach_supported_file_format(recorder, NULL, NULL);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");			
}

static void utc_media_recorder_foreach_supported_video_encoder_p(void)
{
	int ret;
	ret = recorder_foreach_supported_video_encoder(recorder, _video_encoder_cb, NULL);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_foreach_supported_video_encoder_n(void)
{
	int ret;
	ret = recorder_foreach_supported_video_encoder(recorder, NULL, NULL);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");			
}

static void utc_media_recorder_set_recording_limit_reached_cb_p(void)
{
	int ret;
	ret = recorder_set_recording_limit_reached_cb(recorder, _limit_reached_cb, NULL);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "fail recording limited reached cb set");
}

static void utc_media_recorder_set_recording_limit_reached_cb_n(void)
{
	int ret;
	ret = recorder_set_recording_limit_reached_cb(recorder, NULL, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");

}

static void utc_media_recorder_set_recording_status_cb_p(void)
{
	int ret;
	ret = recorder_set_recording_status_cb(recorder, _recording_statis_cb, NULL);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "error");	
}

static void utc_media_recorder_set_recording_status_cb_n(void)
{
	int ret;
	ret = recorder_set_recording_status_cb(recorder, NULL, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");
}

static void utc_media_recorder_set_state_changed_cb_p(void)
{
	int ret;
	ret = recorder_set_state_changed_cb(recorder, _state_changed_cb, NULL);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_set_state_changed_cb_n(void)
{
	int ret;
	ret = recorder_set_state_changed_cb(recorder, NULL, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");

}

static void utc_media_recorder_unset_recording_limit_reached_cb_p(void)
{
	int ret;
	ret = recorder_unset_recording_limit_reached_cb(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "fail unset limited callback");
}

static void utc_media_recorder_unset_recording_limit_reached_cb_n(void)
{
	int ret;
	ret = recorder_unset_recording_limit_reached_cb(NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");
}

static void utc_media_recorder_unset_recording_status_cb_p(void)
{
	int ret;
	ret = recorder_unset_recording_status_cb(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "fail unset limited callback");
}

static void utc_media_recorder_unset_recording_status_cb_n(void)
{
	int ret;
	ret = recorder_unset_recording_status_cb(NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");

}

static void utc_media_recorder_unset_state_changed_cb_p(void)
{
	int ret;
	ret = recorder_unset_state_changed_cb(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "fail unset limited callback");
}

static void utc_media_recorder_unset_state_changed_cb_n(void)
{
	int ret;
	ret = recorder_unset_state_changed_cb(NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");
}
