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
#include <media/camera.h>
#include <stdio.h>
#include <glib.h>
#include <media/recorder.h>

#define MY_ASSERT( fun , test , msg ) \
{\
	if( !test ) \
		dts_fail(fun , msg ); \
}		

static GMainLoop *g_mainloop = NULL;
static GThread *event_thread;
int preview_win = 0;	


static void utc_media_recorder_attribute_test(void);
static void utc_media_recorder_state_change_test(void);
static void utc_media_recorder_recoding_status_cb_test(void);
static void utc_media_recorder_limit_cb_test(void);


static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;


struct tet_testlist tet_testlist[] = {
	{ utc_media_recorder_attribute_test, 1 }, 
	{ utc_media_recorder_state_change_test , 2 }, 
	{ utc_media_recorder_recoding_status_cb_test , 3 },
	{ utc_media_recorder_limit_cb_test , 4 },
	{ NULL, 0 },
};


gpointer GmainThread(gpointer data){
	g_mainloop = g_main_loop_new (NULL, 0);
	g_main_loop_run (g_mainloop);
	
	return NULL;
}


static void startup(void)
{
	if( !g_thread_supported() )
	{
		g_thread_init(NULL);
	}
	
	GError *gerr = NULL;
	event_thread = g_thread_create(GmainThread, NULL, 1, &gerr);
}

static void cleanup(void)
{
	g_main_loop_quit (g_mainloop);
	g_thread_join(event_thread);
}




void record_state_cb(recorder_state_e previous , recorder_state_e current , int by_asm, void *user_data){
	char * state_table[] = {
		"MEDIARECORDER_STATE_NONE",				/**< recorder is not created yet */
		"MEDIARECORDER_STATE_CREATED",				/**< recorder is created, but not initialized yet */
		"MEDIARECORDER_STATE_READY",			/**< ready to record if video recorder is playing preview */
		"MEDIARECORDER_STATE_RECORDING",	/**< While recording */
		"MEDIARECORDER_STATE_PAUSED",			/**< Pause recording */
		"MEDIARECORDER_STATE_NUM"				/**< Number of recorder states */
		};
	printf("%s\n", state_table[current]);
}

int recording_limit_size_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------limit size test----------------------\n");
	printf("-nagative test\n");
	ret = recorder_attr_set_size_limit(recorder, -1);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-0 test\n");
	ret = recorder_attr_set_size_limit(recorder, 0);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_size_limit(recorder,&int_value);
	if( int_value != 0){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-1212 set test\n");
	ret = recorder_attr_set_size_limit(recorder, 1212);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_size_limit(recorder,&int_value);
	if( int_value != 1212){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	return -fail;	
}

int recording_time_limit_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------limit time test----------------------\n");
	
	ret = recorder_attr_set_time_limit(recorder, -1);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-0 test\n");
	ret = recorder_attr_set_time_limit(recorder, 0);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_time_limit(recorder,&int_value);
	if( int_value != 0){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-1212 set test\n");
	ret = recorder_attr_set_time_limit(recorder, 1212);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_time_limit(recorder,&int_value);
	if( int_value != 1212){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	

	return -fail;	
}

int fileformat_test_fail = 0;
bool _file_format_test_cb(recorder_file_format_e format, void *user_data){
	int ret;
	recorder_h recorder = (recorder_h)user_data;
	recorder_file_format_e get_format;
	ret = recorder_set_file_format(recorder, format);
	recorder_get_file_format(recorder, &get_format);

	if( get_format != format){
		printf("FAIL\n");
		fileformat_test_fail++;
	}else
		printf("PASS\n");
	return true;
}


int fileformat_test(recorder_h recorder){
	fileformat_test_fail = 0;
	printf("----------------file format test ------------------\n");
	recorder_foreach_supported_file_format(recorder, _file_format_test_cb, recorder);
	return -fileformat_test_fail;
}

int videoencoder_test_fail = 0;
bool _video_encoder_test_cb(recorder_video_codec_e codec, void *user_data){
	int ret;
	recorder_h recorder = (recorder_h)user_data;
	recorder_video_codec_e get_codec;
	ret = recorder_set_video_encoder(recorder, codec);
	recorder_get_video_encoder(recorder, &get_codec);
	

	if( get_codec != codec){
		printf("FAIL\n");
		videoencoder_test_fail++;
	}else
		printf("PASS\n");
	return true;
}


int video_encoder_test(recorder_h recorder){
	videoencoder_test_fail = 0;
	printf("----------------video encorder test ------------------\n");
	recorder_foreach_supported_video_encoder(recorder, _video_encoder_test_cb, recorder);
	return -videoencoder_test_fail;
}

int audioencoder_test_fail = 0;
bool _audio_encoder_test_cb(recorder_audio_codec_e codec, void *user_data){
	int ret;
	recorder_h recorder = (recorder_h)user_data;
	recorder_audio_codec_e get_codec;
	ret = recorder_set_audio_encoder(recorder, codec);
	recorder_get_audio_encoder(recorder, &get_codec);
	

	if( get_codec != codec){
		printf("FAIL\n");
		audioencoder_test_fail++;
	}else
		printf("PASS\n");
	return true;
}


int audio_encoder_test(recorder_h recorder){
	audioencoder_test_fail = 0;
	printf("----------------audio encorder test ------------------\n");
	recorder_foreach_supported_audio_encoder(recorder, _audio_encoder_test_cb, recorder);
	return -audioencoder_test_fail;
}
 
int recording_audio_device_test(recorder_h recorder)
{
	int ret;
	int fail=0;
	recorder_audio_device_e  int_value;
	// negative test
	printf("-------------audio device test----------------------\n");
	printf("-nagative test\n");
	ret = recorder_attr_set_audio_device (recorder, -1);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-RECORDER_AUDIO_DEVICE_MIC test\n");
	ret = recorder_attr_set_audio_device(recorder, RECORDER_AUDIO_DEVICE_MIC);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_device(recorder,&int_value);
	if( int_value != RECORDER_AUDIO_DEVICE_MIC){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-RECORDER_AUDIO_DEVICE_MODEM  set test\n");
	ret = recorder_attr_set_audio_device(recorder, RECORDER_AUDIO_DEVICE_MODEM );
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_device(recorder,&int_value);
	if( int_value != RECORDER_AUDIO_DEVICE_MODEM){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	return -fail;	
}

int recording_samplerate_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------samplerate test----------------------\n");
	
	ret = recorder_attr_set_audio_samplerate(recorder, -1);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-44100 test\n");
	ret = recorder_attr_set_audio_samplerate(recorder, 44100);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_samplerate(recorder,&int_value);
	if( int_value != 44100){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-1212 set test\n");
	ret = recorder_attr_set_audio_samplerate(recorder, 1212);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_samplerate(recorder,&int_value);
	if( int_value != 1212){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	

	return -fail;	
}


int recording_audio_encoder_bitrate_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------audio encoder bitrate test----------------------\n");
	printf("-negative test\n");
	ret = recorder_attr_set_audio_encoder_bitrate (recorder, -2);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-12200 test\n");
	ret = recorder_attr_set_audio_encoder_bitrate (recorder, 12200);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_encoder_bitrate (recorder,&int_value);
	if( int_value != 12200){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-288000  set test\n");
	ret = recorder_attr_set_audio_encoder_bitrate (recorder, 288000 );
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_encoder_bitrate (recorder,&int_value);
	if( int_value != 288000 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-1212  set test\n");
	ret = recorder_attr_set_audio_encoder_bitrate (recorder, 1212 );
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_audio_encoder_bitrate (recorder,&int_value);
	if( int_value != 1212 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	return -fail;	
}

int recording_video_encoder_bitrate_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------video encoder bitrate test----------------------\n");
	printf("-negative test\n");
	ret = recorder_attr_set_video_encoder_bitrate (recorder, -2);
	if( ret == 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-12200 test\n");
	ret = recorder_attr_set_video_encoder_bitrate (recorder, 12200);
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_video_encoder_bitrate (recorder,&int_value);
	if( int_value != 12200){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");


	printf("-288000  set test\n");
	ret = recorder_attr_set_video_encoder_bitrate (recorder, 288000 );
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_video_encoder_bitrate (recorder,&int_value);
	if( int_value != 288000 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	printf("-1212  set test\n");
	ret = recorder_attr_set_video_encoder_bitrate (recorder, 1212 );
	if( ret != 0 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");
	
	ret = recorder_attr_get_video_encoder_bitrate (recorder,&int_value);
	if( int_value != 1212 ){
		printf("FAIL\n");
		fail++;
	}else
		printf("PASS\n");

	return -fail;	
}


typedef struct{
	bool iscalled;
	bool isready;
	bool isrecording;
	bool ispaused;
	recorder_state_e state;
} state_change_data;

void _state_change_test_cb(recorder_state_e previous, recorder_state_e current, bool by_asm, void *user_data){
	state_change_data * data = (state_change_data*)user_data;
	if( current == RECORDER_STATE_READY )
		data->isready = true;
	if( current == RECORDER_STATE_RECORDING )
		data->isrecording = true;
	if( current == RECORDER_STATE_PAUSED )
		data->ispaused = true;

	data->state = current;

	//printf("state %d\n",current);

}

void utc_media_recorder_state_change_test(void){
	recorder_h recorder;
	state_change_data data;
	int ret;
	recorder_create_audiorecorder(&recorder);
	data.iscalled = false;
	data.isready = false;
	data.isrecording = false;
	data.ispaused = false;
	data.state = 0;
	printf("-------------------------recorder state change test -------------------\n");
	recorder_set_state_changed_cb(recorder, _state_change_test_cb, &data);
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	recorder_prepare(recorder);
	ret = recorder_start(recorder);
	sleep(1);
	ret = recorder_pause(recorder);
	ret =recorder_commit(recorder);
	sleep(2);

	if( data.isready && data.isrecording && data.ispaused && data.state == RECORDER_STATE_READY ){
		printf("PASS\n");
		ret = 0;
	}else{
		printf("FAIL data.isready %d, data.isrecording %d,  data.ispaused %d,  data.state %d \n", data.isready , data.isrecording , data.ispaused , data.state );
		ret = -1;
	}

	recorder_unprepare(recorder);
	recorder_destroy(recorder);	

	
	MY_ASSERT(__func__, ret == 0 , "Fail state change test");
	dts_pass(__func__, "PASS");
		
}


typedef struct {
	int elapsed_time;
	int file_size;
} recording_result;
void _recording_status_test_cb(int elapsed_time, int file_size, void *user_data){
	recording_result *result = (recording_result*)user_data;
	result->elapsed_time = elapsed_time;
	result->file_size = file_size;

}

void utc_media_recorder_recoding_status_cb_test(void){
	recorder_h recorder;
	recording_result result;
	int ret;

	printf("--------------recording status cb test-------------------\n");
	recorder_create_audiorecorder(&recorder);
	recorder_set_recording_status_cb(recorder,_recording_status_test_cb, &result);
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	recorder_prepare(recorder);
	ret = recorder_start(recorder);
	result.elapsed_time = 0;
	result.file_size = 0;
	sleep(3);
	ret = recorder_cancel(recorder);
	ret =recorder_unprepare(recorder);
	ret =recorder_destroy(recorder);	
	if( result.elapsed_time > 0  && result.file_size > 0){
		printf("PASS\n");
		dts_pass(__func__, "PASS");
	}else{
		printf("FAIL\n");
		dts_fail(__func__, "status cb Fail");
	}
}


void utc_media_recorder_attribute_test(void){
	int fail =0;
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	fail = recording_limit_size_test(recorder);
	fail +=recording_time_limit_test(recorder);
	fail +=fileformat_test(recorder);
	fail +=video_encoder_test(recorder);
	fail +=audio_encoder_test(recorder);
	fail +=recording_audio_device_test(recorder);
	fail +=recording_samplerate_test(recorder);
	fail +=recording_audio_encoder_bitrate_test(recorder);
	fail +=recording_video_encoder_bitrate_test(recorder);
	MY_ASSERT(__func__, fail == 0 , "Fail recorder attribute");
	dts_pass(__func__, "PASS");
}



void _recording_status_cb2(int elapsed_time, int file_size, void *user_data){
	//printf("elapsed time :%d , file_size :%d\n", elapsed_time , file_size);

}

void _recording_limited_reached_cb(recorder_recording_limit_type_e type, void *user_data){
	printf("limited!! %d\n", type);
	int *ischeck = (int*)user_data;
	*ischeck = 1;
}


void utc_media_recorder_limit_cb_test(void){
	recorder_h recorder;
	int ischeck = 0;
	int ret =0;
	recorder_state_e state;
	recorder_create_audiorecorder(&recorder);
	printf("------------------------limit cb test -------------------------\n");
	//recorder_set_state_change_cb(recorder, record_state_cb, NULL);
	recorder_set_recording_status_cb(recorder, _recording_status_cb2, NULL);
	recorder_set_recording_limit_reached_cb(recorder, _recording_limited_reached_cb, &ischeck);	
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	recorder_prepare(recorder);

	recorder_get_state(recorder, &state);


	printf("-time limit test\n");
	recorder_attr_set_time_limit(recorder, 2);
	recorder_start(recorder);
	sleep(5);
	recorder_cancel(recorder);
	if( ischeck ){
		printf("PASS\n");
	}else{
		printf("FAIL\n");
		ret--;
	}


	printf("-time unlimit test\n");
	//recorder_unready(recorder);
	recorder_attr_set_time_limit(recorder, 0);		
	//recorder_ready(recorder);
	
	recorder_start(recorder);
	ischeck = 0;
	sleep(5);
	recorder_cancel(recorder);
	if( ischeck ){
		printf("FAIL\n");
		ret--;
	}else{
		printf("PASS\n");
	}


	printf("-size limit test\n");
	ischeck = 0;
	recorder_attr_set_size_limit(recorder, 2);
	recorder_start(recorder);
	sleep(5);
	recorder_cancel(recorder);
	
	if( ischeck ){
		printf("PASS\n");
	}else{
		printf("FAIL\n");
		ret--;
	}

	printf("-size unlimit test\n");
	ischeck = 0;
	recorder_attr_set_size_limit(recorder, 0);
	recorder_start(recorder);
	sleep(5);
	recorder_cancel(recorder);
	
	if( ischeck ){
		printf("FAIL\n");
		ret--;
	}else{
		printf("PASS\n");
	}
	

	recorder_unprepare(recorder);
	recorder_destroy(recorder);

	MY_ASSERT(__func__,  ret == 0 , "fail limit cb");
	dts_pass(__func__, "PASS");
}

