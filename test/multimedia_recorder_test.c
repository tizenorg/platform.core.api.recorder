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




#include <stdio.h>
#include <Elementary.h>
#include <glib.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <pthread.h>
#include <recorder.h>
#include <camera.h>
#include <mm.h>
#include <mm_camcorder.h>
#include <mm_types.h>

Evas_Object* mEvasWindow;
Ecore_X_Window mXwindow;	
Ecore_X_Window preview_win;

bool record_state_cb(recorder_state_e previous , recorder_state_e current , int by_asm, void *user_data){
	char * state_table[] = {
		"MEDIARECORDER_STATE_NONE",				/**< recorder is not created yet */
		"MEDIARECORDER_STATE_CREATED",				/**< recorder is created, but not initialized yet */
		"MEDIARECORDER_STATE_READY",			/**< prepare to record if video recorder is playing preview */
		"MEDIARECORDER_STATE_RECORDING",	/**< While recording */
		"MEDIARECORDER_STATE_PAUSED",			/**< Pause recording */
		"MEDIARECORDER_STATE_NUM"				/**< Number of recorder states */
		};
	printf("%s\n", state_table[current]);
	return 0;
}

int recording_size_limit_test(recorder_h recorder){
	int ret;
	int fail=0;
	int int_value;
	// negative test
	printf("-------------limit size test----------------------\n");
	printf("-negative test\n");
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
	printf("----------------video encoder test ------------------\n");
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
	printf("----------------audio encoder test ------------------\n");
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
	printf("-negative test\n");
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
	bool isprepare;
	bool isrecording;
	bool ispaused;
	recorder_state_e state;
} state_change_data;

void _state_change_test_cb(recorder_state_e previous, recorder_state_e current, bool by_asm, void *user_data){
	printf(" state change %d => %d\n", previous , current);
	state_change_data * data = (state_change_data*)user_data;
	if( current == RECORDER_STATE_READY )
		data->isprepare = true;
	if( current == RECORDER_STATE_RECORDING )
		data->isrecording = true;
	if( current == RECORDER_STATE_PAUSED )
		data->ispaused = true;

	data->state = current;

	//printf("state %d\n",current);
}

int recorder_state_change_test(){
	char *state_str[] = {	"RECORDER_STATE_NONE",				/**< recorder is not created yet */
		"RECORDER_STATE_CREATED",				/**< recorder is created, but not initialized yet */
		"RECORDER_STATE_READY",			/**< prepare to record if video recorder is playing preview */
		"RECORDER_STATE_RECORDING",	/**< While recording */
		"RECORDER_STATE_PAUSED",			/**< Pause recording */
	};
	recorder_h recorder;
	state_change_data data;
	int ret;
	recorder_state_e state;
	recorder_create_audiorecorder(&recorder);
	data.iscalled = false;
	data.isprepare = false;
	data.isrecording = false;
	data.ispaused = false;
	data.state = 0;
	printf("-------------------------recorder state change test -------------------\n");
	recorder_get_state(recorder, &state);
	printf("state = %s\n", state_str[state]);	
	recorder_set_state_changed_cb(recorder, _state_change_test_cb, &data);
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	ret = recorder_prepare(recorder);
	printf("recorder_prepare ret = %x\n", ret);	
	recorder_get_state(recorder, &state);
	printf("state = %s\n", state_str[state]);
	ret = recorder_start(recorder);
	printf("recorder_start ret = %x\n", ret);
	recorder_get_state(recorder, &state);
	printf("state = %s\n", state_str[state]);
	sleep(1);
	ret = recorder_pause(recorder);
	printf("recorder_pause ret = %x\n", ret);
	recorder_get_state(recorder, &state);
	printf("state = %s\n", state_str[state]);
	
	ret =recorder_commit(recorder);
	sleep(2);

	if( data.isprepare && data.isrecording && data.ispaused && data.state == RECORDER_STATE_READY ){
		printf("PASS\n");
		ret = 0;
	}else{
		printf("FAIL data.isprepare %d, data.isrecording %d,  data.ispaused %d,  data.state %d \n", data.isprepare , data.isrecording , data.ispaused , data.state );
		ret = -1;
	}

	ret = recorder_unprepare(recorder);
	printf("recorder_unprepare ret = %x\n", ret);
	recorder_get_state(recorder, &state);
	printf("state = %s\n", state_str[state]);
	
	ret = recorder_destroy(recorder);	
	printf("recorder_destroy ret = %x\n", ret);	

	return ret;
		
}


typedef struct {
	int elapsed_time;
	int file_size;
} recording_result;
void _recording_status_test_cb(unsigned long long elapsed_time, unsigned long long file_size, void *user_data){
	recording_result *result = (recording_result*)user_data;
	result->elapsed_time = elapsed_time;
	result->file_size = file_size;

}

int recorder_recoding_status_cb_test(){
	recorder_h recorder;
	recording_result result;
	int ret;

	printf("--------------recording status cb test-------------------\n");
	recorder_create_audiorecorder(&recorder);
	recorder_set_recording_status_cb(recorder,_recording_status_test_cb, &result);
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	ret = recorder_prepare(recorder);
	printf("prepare %d\n", ret);
	ret = recorder_start(recorder);
	printf("start %d\n", ret);
	result.elapsed_time = 0;
	result.file_size = 0;
	sleep(3);
	ret = recorder_cancel(recorder);
	printf("cancel %d\n", ret);
	ret =recorder_unprepare(recorder);
	printf("unprepare %d\n", ret);
	ret =recorder_destroy(recorder);	
	printf("destroy %d\n", ret);
	if( result.elapsed_time > 0  && result.file_size > 0){
		printf("PASS\n");
		return 0;
	}else{
		printf("FAIL\n");
		return -1;	
	}
}


int recorder_attribute_test(){
	int fail =0;
	int ret;
	recorder_h recorder;
	ret = recorder_create_audiorecorder(&recorder);
	fail = recording_size_limit_test(recorder);
	fail +=recording_time_limit_test(recorder);
	fail +=fileformat_test(recorder);
	fail +=video_encoder_test(recorder);
	fail +=audio_encoder_test(recorder);
	fail +=recording_audio_device_test(recorder);
	fail +=recording_samplerate_test(recorder);
	fail +=recording_audio_encoder_bitrate_test(recorder);
	fail +=recording_video_encoder_bitrate_test(recorder);
	return fail;
}



void _recording_status_cb2(unsigned long long elapsed_time, unsigned long long file_size, void *user_data){
	//printf("elapsed time :%d , file_size :%d\n", elapsed_time , file_size);
}

void _recording_limit_reached_cb(recorder_recording_limit_type_e type, void *user_data){
	printf("limited!! %d\n", type);
	int *ischeck = (int*)user_data;
	*ischeck = 1;
}


int recorder_limit_cb_test(){
	recorder_h recorder;
	int ischeck = 0;
	int ret =0;
	recorder_create_audiorecorder(&recorder);
	printf("------------------------limit cb test -------------------------\n");
	//recorder_set_state_changed_cb(recorder, record_state_cb, NULL);
	recorder_set_recording_status_cb(recorder, _recording_status_cb2, NULL);
	recorder_set_recording_limit_reached_cb(recorder, _recording_limit_reached_cb, &ischeck);	
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	recorder_prepare(recorder);


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
	//recorder_unprepare(recorder);
	recorder_attr_set_time_limit(recorder, 0);		
	//recorder_prepare(recorder);
	ischeck = 0;
	recorder_set_recording_limit_reached_cb(recorder, _recording_limit_reached_cb, &ischeck);	
	recorder_start(recorder);
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

	return ret;
		
}

int video_recorder_test(){
	camera_h camera ;
	recorder_h recorder;
	int ret;

	printf("-----------------video recorder test--------------------\n");
	camera_create(CAMERA_DEVICE_CAMERA0 , &camera);
	recorder_create_videorecorder(camera, &recorder);
	camera_set_display(camera,CAMERA_DISPLAY_TYPE_X11,GET_DISPLAY(preview_win));
	camera_set_x11_display_rotation(camera, CAMERA_ROTATION_270);
	//camera_set_preview_resolution(camera, 320, 240);
	camera_attr_set_preview_fps(camera, CAMERA_ATTR_FPS_AUTO);
	ret = recorder_set_file_format(recorder,RECORDER_FILE_FORMAT_MP4);
	printf("ret = %x\n", ret);
	ret = recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H263);
	printf("ret = %x\n", ret);	
	ret = recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	printf("ret = %x\n", ret);	
	ret = recorder_set_filename(recorder, "/mnt/nfs/video_recorder_test.mp4");
	printf("ret = %x\n", ret);	
	

	ret=  recorder_prepare(recorder);
	printf("ret = %x\n", ret);	
	ret = recorder_start(recorder);
	printf("ret = %x\n", ret);	
	sleep(10);
	ret = recorder_pause(recorder);
	printf("ret = %x\n", ret);		
	ret = recorder_commit(recorder);
	printf("ret = %x\n", ret);	
	ret =recorder_unprepare(recorder);
	printf("ret = %x\n", ret);	
	ret= recorder_destroy(recorder);
	printf("ret = %x\n", ret);	
	return 0;
}

int mm_test(){
	MMCamPreset info;
	info.videodev_type= MM_VIDEO_DEVICE_NONE;
	MMHandleType camcorder;
	int ret;	
	ret = mm_camcorder_create(&camcorder, &info);
	printf("mm_camcorder_create %x\n", ret);
	ret = mm_camcorder_set_attributes(camcorder, NULL, 
																MMCAM_MODE , MM_CAMCORDER_MODE_AUDIO, 
																(void*)NULL);
	printf("mm_camcorder_set_attributes %x\n", ret);
	
	ret = mm_camcorder_set_attributes(camcorder ,NULL, MMCAM_AUDIO_ENCODER ,MM_AUDIO_CODEC_AAC , NULL);
	printf("mm_camcorder_set_attributes %x\n", ret);

	ret = mm_camcorder_set_attributes(camcorder ,NULL, MMCAM_FILE_FORMAT ,MM_FILE_FORMAT_MP4 , NULL);
	printf("mm_camcorder_set_attributes %x\n", ret);

	ret = mm_camcorder_realize(camcorder);	
	printf("mm_camcorder_realize %x\n", ret);
	
	ret = mm_camcorder_start(camcorder);
	printf("mm_camcorder_start %x\n", ret);
	
	ret = mm_camcorder_record(camcorder);
	printf("mm_camcorder_record %x\n", ret);
	


	ret = mm_camcorder_pause(camcorder);
	printf("mm_camcorder_pause %x\n", ret);

	ret = mm_camcorder_cancel(camcorder);
	printf("mm_camcorder_cancel %x\n", ret);

	ret = mm_camcorder_stop(camcorder);
	printf("mm_camcorder_stop %x\n", ret);

	ret = mm_camcorder_unrealize(camcorder);
	printf("mm_camcorder_unrealize %x\n", ret);	

	ret = mm_camcorder_destroy(camcorder);
	printf("mm_camcorder_destroy %x\n", ret);	

	
	
	return 0;
	
}



int recorder_encoder_test(){
	recorder_h recorder;
	camera_h camera;
	int ret;

	printf("3GP - AMR\n");
	ret=recorder_create_audiorecorder(&recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/audiotest_amr.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(4);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);

	printf("3GP - AAC\n");
	ret=recorder_create_audiorecorder(&recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/audiotest_aac.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(4);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);

	printf("AMR - AMR\n");
	ret=recorder_create_audiorecorder(&recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/audiotest.amr");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(4);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);

	printf("MP4 - AAC\n");
	ret=recorder_create_audiorecorder(&recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_MP4);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/audiotest_aac.mp4");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(4);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);
	

	ret+=camera_create(CAMERA_DEVICE_CAMERA0,&camera);
	ret+=camera_set_x11_display_rotation(camera, CAMERA_ROTATION_270);
	ret+=camera_set_display(camera, CAMERA_DISPLAY_TYPE_X11, GET_DISPLAY(preview_win));


	printf("3GP - AMR- H263\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H263);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_amr_h263.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);


	printf("3GP - AMR- H264\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H264);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_amr_h264.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);
	
	printf("3GP - AMR- MPEG4\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_amr_mpeg4.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);


	printf("3GP - AAC- H263\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H263);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_h263.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);
		

	printf("3GP - AAC- H264\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H264);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_h264.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);	

	printf("3GP - AAC- MPEG4\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_MPEG4.3gp");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);

	printf("MP4 - AAC- H264\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_MP4);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_H264);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_h264.mp4");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);

	printf("MP4 - AAC- MPEG4\n");	
	ret+=recorder_create_videorecorder(camera, &recorder);
	ret+=recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_MP4);
	ret+=recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	ret+=recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	ret+=recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_MPEG4.mp4");
	ret+=recorder_prepare(recorder);
	ret+=recorder_start(recorder);
	sleep(10);
	ret+=recorder_commit(recorder);
	ret += recorder_unprepare(recorder);
	ret+=recorder_destroy(recorder);


	camera_destroy(camera);
	
	
	return ret;
}

 Eina_Bool print_audio_level(void *data){
 	recorder_h recorder = (recorder_h)data;
	if( recorder ){
		double level; 
		recorder_get_audio_level(recorder,&level);
		printf("%g\n", level);
	}
 	return 1;
}

int audio_level_test(){
	recorder_h recorder;
	recorder_create_audiorecorder(&recorder);
	recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	recorder_set_filename(recorder, "/mnt/nfs/test.amr");
	
	recorder_prepare(recorder);
	recorder_start(recorder);
	ecore_timer_add(0.1, print_audio_level, recorder);
	sleep(2);
	return 0;
}

void _camera_state_changed_cb(camera_state_e previous, camera_state_e current,bool by_policy, void *user_data){
	printf("camera state changed %d -> %d\n", previous , current);
}

void _recorder_state_changed_cb(recorder_state_e previous , recorder_state_e current , bool by_policy, void *user_data){
	printf("recorder state changed %d -> %d\n", previous , current);
}

int slow_motion_test(){
	camera_h camera;
	recorder_h recorder;
	int ret;
	ret = camera_create(CAMERA_DEVICE_CAMERA0, &camera);
	printf("camera_create ret = %x\n", ret);
	camera_set_state_changed_cb(camera, _camera_state_changed_cb, NULL);
	ret = recorder_create_videorecorder(camera, &recorder);
	printf("recorder_create_videorecorder ret = %x\n", ret);
	ret = recorder_set_state_changed_cb(recorder, _recorder_state_changed_cb , NULL);
	ret = recorder_set_filename(recorder, "/mnt/nfs/test.3gp");
	printf("recorder_set_filename ret = %x\n", ret);
	ret = recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	printf("recorder_set_audio_encoder ret = %x\n", ret);
	ret = recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	printf("recorder_set_video_encoder ret = %x\n", ret);
	ret = recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	printf("recorder_set_file_format ret = %x\n", ret);
	ret = recorder_attr_set_recording_motion_rate(recorder, 0.5);
	printf("recorder_attr_set_slow_motion_rate ret = %x\n", ret);
	ret = recorder_prepare(recorder);
	printf("recorder_prepare ret = %x\n", ret);
	ret = recorder_start(recorder);
	printf("recorder_start ret = %x\n", ret);
	sleep(10);
	ret = recorder_commit(recorder);
	printf("recorder_commit ret = %x\n", ret);
	ret = recorder_unprepare(recorder);
	printf("recorder_unprepare ret = %x\n", ret);
	ret = recorder_destroy(recorder);
	printf("recorder_destroy ret = %x\n", ret);
	return 0;
}

void _capturing_cb(camera_image_data_s* image, camera_image_data_s* postview, camera_image_data_s* thumbnail, void *user_data){
	printf("capturing callback!\n");
}

void _capture_completed_cb(void *user_data){
	printf("capture completed callback\n");
}


int recording_capture_test(){
	camera_h camera;
	recorder_h recorder;
	int ret;
	ret = camera_create(CAMERA_DEVICE_CAMERA0, &camera);
	printf("camera_create ret = %x\n", ret);
	camera_set_state_changed_cb(camera, _camera_state_changed_cb, NULL);
	ret = recorder_create_videorecorder(camera, &recorder);
	printf("recorder_create_videorecorder ret = %x\n", ret);
	ret = recorder_set_state_changed_cb(recorder, _recorder_state_changed_cb , NULL);
	ret = recorder_set_filename(recorder, "/mnt/nfs/test.3gp");
	printf("recorder_set_filename ret = %x\n", ret);
	ret = recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	printf("recorder_set_audio_encoder ret = %x\n", ret);
	ret = recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	printf("recorder_set_video_encoder ret = %x\n", ret);
	ret = recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	printf("recorder_set_file_format ret = %x\n", ret);

	camera_set_display(camera, CAMERA_DISPLAY_TYPE_X11, GET_DISPLAY(preview_win));
	//camera_set_preview_resolution(camera, 640, 480);
	//camera_set_capture_resolution(camera, 640, 480);

	ret = recorder_prepare(recorder);
	printf("recorder_prepare ret = %x\n", ret);
	ret = recorder_start(recorder);
	printf("recorder_start ret = %x\n", ret);
	sleep(10);
	ret = camera_start_capture(camera, _capturing_cb , _capture_completed_cb, NULL);
	printf("camera_start_capture ret =%x\n", ret);
	sleep(10);

	ret = recorder_commit(recorder);
	printf("recorder_commit ret = %x\n", ret);
	ret = recorder_unprepare(recorder);
	printf("recorder_unprepare ret = %x\n", ret);
	ret = recorder_destroy(recorder);
	printf("recorder_destroy ret = %x\n", ret);
	return 0;
}


void _audio_stream_cb(void* stream, int size, audio_sample_type_e format, int channel, unsigned int timestamp, void *user_data){
	printf("size = %d[%d]( %d )\n", size, format, timestamp);
}

int audio_stream_cb_test(){
	recorder_h recorder;
	int ret = 0;
	ret = recorder_create_audiorecorder(&recorder);
	printf(" create ret =%d\n", ret);
	ret = recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AMR);
	printf(" create2 ret =%d\n", ret);
	ret = recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_AMR);
	printf(" create3 ret =%d\n", ret);
	ret = recorder_set_filename(recorder, "/mnt/nfs/test.amr");
	printf(" create4 ret =%d\n", ret);
	ret = recorder_set_audio_stream_cb(recorder, _audio_stream_cb, NULL);
	printf(" recorder_set_audio_stream_cb ret =%d\n", ret);
	ret = recorder_prepare(recorder);
	printf(" recorder_prepare ret =%d\n", ret);
	ret = recorder_set_audio_stream_cb(recorder, _audio_stream_cb, NULL);
	printf(" recorder_set_audio_stream_cb ret =%d\n", ret);
	ret = recorder_start(recorder);
	printf(" recorder_start ret =%d\n", ret);
	sleep(10);
	ret = recorder_commit(recorder);
	printf(" recorder_commit ret =%d\n", ret);
	return 0;
}


void _camera_capturing_cb_modechange(camera_image_data_s* image, camera_image_data_s* postview, camera_image_data_s* thumbnail, void *user_data){
	printf("capturing callback\n");
}
void _camera_capture_completed_cb_modechange(void *user_data){
	printf("capture complete\n");
}

void _camera_capturing_cb_modechange2(camera_image_data_s* image, camera_image_data_s* postview, camera_image_data_s* thumbnail, void *user_data){
	printf("in recording capture callback\n");
}
void _camera_capture_completed_cb_modechange2(void *user_data){
	printf("in recording capture complete\n");
}


int modechange_test(){
	camera_h camera;
	recorder_h recorder;
	int ret;
	ret = camera_create(CAMERA_DEVICE_CAMERA0, &camera);
	printf("camera_create %d\n", ret);
	ret = recorder_create_videorecorder(camera, &recorder);
	printf("recorder_create_videorecorder %d\n", ret);
	camera_set_display(camera, CAMERA_DISPLAY_TYPE_X11 , GET_DISPLAY(preview_win));

	ret =recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	printf("recorder_set_file_format %d\n", ret);
	ret =recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	printf("recorder_set_audio_encoder %d\n", ret);
	ret =recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	printf("recorder_set_video_encoder %d\n", ret);
	ret =recorder_set_filename(recorder , "/mnt/nfs/videotest_AAC_MPEG4.3gp");
	printf("recorder_set_filename %d\n", ret);

	ret = camera_start_preview(camera);
	printf("camera_start_preview %d\n", ret);
	ret = camera_start_capture(camera, _camera_capturing_cb_modechange, _camera_capture_completed_cb_modechange, NULL);
	printf("camera_start_capture %d\n", ret);
	camera_state_e state;
	camera_get_state(camera, &state);
	while( state == CAMERA_STATE_CAPTURING ){
		sleep(1);
		camera_get_state(camera, &state);
		printf("camera state = %d\n", state);
	}

	ret =recorder_set_audio_encoder(recorder, RECORDER_AUDIO_CODEC_AAC);
	printf("recorder_set_audio_encoder %d\n", ret);
	ret =recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	printf("recorder_set_video_encoder %d\n", ret);

	ret = camera_start_preview(camera);
	printf("camera_start_preview %d\n", ret);

	ret = recorder_start(recorder);
	printf("recorder_start %d\n", ret);
	sleep(5);
	ret = camera_start_capture(camera, _camera_capturing_cb_modechange2, _camera_capture_completed_cb_modechange2, NULL);
	sleep(5);

	ret = recorder_commit(recorder);
	printf("recorder_commit %d\n", ret);
	return 0;
}


void* test_main(void *arg){
	int ret = 0;
	//ret = recorder_encoder_test();
	/*
	ret = recorder_attribute_test();
	ret += recorder_state_change_test();
	ret += recorder_recoding_status_cb_test();
	ret += recorder_limit_cb_test();
	ret += video_recorder_test();
	ret = mm_test();
	*/

	//audio_level_test();
	//slow_motion_test();
	//recording_capture_test();
	//audio_stream_cb_test();
	//modechange_test();
	recorder_recoding_status_cb_test();

	if( ret == 0 )
		printf("--------------RECORDER TEST ALL PASS--------------------------\n");
	else
		printf("--------------RECORDER TEST FAIL %d--------------------------\n", -ret);


	return 0;
}

int main(int argc, char ** argv)
{
	
	elm_init(argc, argv);



	mEvasWindow = elm_win_add(NULL, "VIDEO OVERLAY", ELM_WIN_BASIC);
	elm_win_title_set(mEvasWindow, "video overlay window");
	elm_win_borderless_set(mEvasWindow, 0);
	evas_object_resize(mEvasWindow, 800, 480);
	evas_object_move(mEvasWindow, 0, 0);
	evas_object_show(mEvasWindow);	

	//To support full-screen
	elm_win_rotation_set(mEvasWindow, 270);
	//elm_win_fullscreen_set(mEvasWindow, 1);
	
	evas_object_color_set(mEvasWindow, 0,0,0,0);
	preview_win = elm_win_xwindow_get(mEvasWindow);

	fprintf(stderr, "end of elm\n");

	pthread_t gloop_thread;

	pthread_create(&gloop_thread, NULL, test_main,  NULL);



	elm_run();
	elm_shutdown();
	

	return 0;
}
