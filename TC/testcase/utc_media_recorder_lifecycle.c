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

static void utc_media_recorder_create_audiorecorder_p(void);
static void utc_media_recorder_create_audiorecorder_n(void); 
static void utc_media_recorder_cancel_p(void);
static void utc_media_recorder_cancel_n(void);
static void utc_media_recorder_commit_p(void);
static void utc_media_recorder_commit_n(void);
static void utc_media_recorder_create_videorecorder_p(void);
static void utc_media_recorder_create_videorecorder_n(void);
static void utc_media_recorder_destroy_p(void);
static void utc_media_recorder_destroy_n(void); 
static void utc_media_recorder_pause_p(void);
static void utc_media_recorder_pause_n(void);
static void utc_media_recorder_ready_p(void);
static void utc_media_recorder_ready_n(void); 
static void utc_media_recorder_start_p(void);
static void utc_media_recorder_start_n(void);
static void utc_media_recorder_unready_p(void);
static void utc_media_recorder_unready_n(void); 


struct tet_testlist tet_testlist[] = {
	{ utc_media_recorder_create_audiorecorder_p, 1 },
	{ utc_media_recorder_create_audiorecorder_n, 2 }, 
	{ utc_media_recorder_cancel_p , 1 },
	{ utc_media_recorder_cancel_n , 2 },
	{ utc_media_recorder_commit_p , 1 },
	{ utc_media_recorder_commit_n , 2 },
	{ utc_media_recorder_create_videorecorder_p , 1 },
	{ utc_media_recorder_create_videorecorder_n , 2 },
	{ utc_media_recorder_destroy_p , 1 },
	{ utc_media_recorder_destroy_n , 2 }, 
	{ utc_media_recorder_pause_p , 1 },
	{ utc_media_recorder_pause_n , 2 },
	{ utc_media_recorder_ready_p , 1 },
	{ utc_media_recorder_ready_n , 2 }, 
	{ utc_media_recorder_start_p , 1 },
	{ utc_media_recorder_start_n , 2 },
	{ utc_media_recorder_unready_p , 1 },
	{ utc_media_recorder_unready_n , 2 },  
	{ NULL, 0 },
};

static void startup(void)
{
	/* start of TC */
}

static void cleanup(void)
{
	/* end of TC */
}
 
/**
 * @brief Negative test case of recorder_create_audiorecorder()
 */
static void utc_media_recorder_create_audiorecorder_n(void)
{
	int ret;
	ret = recorder_create_audiorecorder(NULL);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");

}

static void utc_media_recorder_create_audiorecorder_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "create_audiorecorder is faild");
	recorder_destroy(recorder);
}

 
static void utc_media_recorder_cancel_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	ret |= recorder_start(recorder);	
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	sleep(2);
	ret = recorder_cancel(recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "record commit is fail");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);
}

static void utc_media_recorder_cancel_n(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	ret = recorder_cancel(recorder);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "invalid state");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);
}


static void utc_media_recorder_commit_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	ret |= recorder_start(recorder);	
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	sleep(2);
	ret = recorder_commit(recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "record commit is fail");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);
}

static void utc_media_recorder_commit_n(void)
{
	int ret;
	recorder_h recorder;
	ret =recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	ret = recorder_commit(recorder);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "invalid state");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);
}
 
static void utc_media_recorder_create_videorecorder_p(void)
{
	recorder_h recorder;
	camera_h camera;
	int ret;
	ret = camera_create(CAMERA_DEVICE_CAMERA0, &camera);
	dts_check_eq(__func__, ret, CAMERA_ERROR_NONE, "create camera is faild");
	
	ret = recorder_create_videorecorder(camera, &recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "create recorder is faild");	
	recorder_destroy(recorder);
	camera_destroy(camera);
}

static void utc_media_recorder_create_videorecorder_n(void)
{
	recorder_h recorder;
	int ret;
	ret = recorder_create_videorecorder(NULL, &recorder);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");	
}

static void utc_media_recorder_destroy_p(void)
{
	recorder_h recorder;
	int ret;
	ret = recorder_create_audiorecorder(&recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "create recorder is faild");	
	ret = recorder_destroy(recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "recorder_destroy is faild");	
}

static void utc_media_recorder_destroy_n(void)
{
	int ret;
	ret = recorder_destroy(NULL);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "not allow NULL");		
}
 
static void utc_media_recorder_pause_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	ret |= recorder_start(recorder);	
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	sleep(2);
	ret = recorder_pause(recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "record commit is fail");
	recorder_cancel(recorder);
	recorder_unprepare(recorder);
	recorder_destroy(recorder);	

}

static void utc_media_recorder_pause_n(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	ret = recorder_pause(recorder);
	dts_check_ne(__func__, ret, RECORDER_ERROR_NONE, "invalid state");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);	
}

static void utc_media_recorder_ready_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	ret = recorder_prepare(recorder);
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "invalid state");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);	
}

static void utc_media_recorder_ready_n(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);	
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "recorder prepare is faild");
	ret = recorder_prepare(recorder);	
	dts_check_eq(__func__, ret, RECORDER_ERROR_NONE, "invalid state");
	recorder_unprepare(recorder);
	recorder_destroy(recorder);	
}
 
static void utc_media_recorder_start_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder create fail");
	ret = recorder_start(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder start record fail");
	recorder_cancel(recorder);
	recorder_unprepare(recorder);
	recorder_destroy(recorder);	
}

static void utc_media_recorder_start_n(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder create fail");

	ret = recorder_start(recorder);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "expect invalid state");
	recorder_destroy(recorder);
}

static void utc_media_recorder_unready_p(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	ret |= recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret |= recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_AMR);
	ret |= recorder_prepare(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder create fail");
	ret = recorder_unprepare(recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder unready fail");	
	recorder_destroy(recorder);	
}

static void utc_media_recorder_unready_n(void)
{
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "recorder create fail");

	ret = recorder_unprepare(recorder);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "expect invalid state");
	recorder_destroy(recorder);
}
 


