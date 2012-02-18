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
#include <stdlib.h>
#include <string.h>
#include <mm.h>
#include <mm_camcorder.h>
#include <mm_types.h>
#include <math.h>
#include <camera.h>
#include <recorder.h>
#include <recorder_private.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_RECORDER"

/*
 *	camera_private
 * It should be sync with camera_private.h
*/
int mm_message_callback(int message, void *param, void *user_data);

typedef enum {
	_CAMERA_EVENT_TYPE_STATE_CHANGE,
	_CAMERA_EVENT_TYPE_FOCUS_CHANGE,	
	_CAMERA_EVENT_TYPE_CAPTURE_COMPLETE,
	_CAMERA_EVENT_TYPE_PREVIEW,
	_CAMERA_EVENT_TYPE_CAPTURE,	
	_CAMERA_EVENT_TYPE_ERROR,		
	_CAMERA_EVENT_TYPE_NUM
}_camera_event_e;

typedef struct _camera_s{
	MMHandleType mm_handle;
	
	void* user_cb[_CAMERA_EVENT_TYPE_NUM];
	void* user_data[_CAMERA_EVENT_TYPE_NUM];
	unsigned long display_handle;
	int state;
	
} camera_s;
/*
 * end of camera_private
 */


int _convert_recorder_error_code(const char *func, int code){
	int ret = RECORDER_ERROR_INVALID_OPERATION;
	char *errorstr = NULL;
	
	switch(code)
	{
		case RECORDER_ERROR_INVALID_PARAMETER:
			ret = RECORDER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";			
			break;			
		case MM_ERROR_NONE:
			ret = RECORDER_ERROR_NONE;
			errorstr = "ERROR_NONE";
			break;
		case MM_ERROR_CAMCORDER_INVALID_ARGUMENT :
		case MM_ERROR_COMMON_INVALID_ATTRTYPE :
		case MM_ERROR_COMMON_INVALID_PERMISSION :
		case MM_ERROR_COMMON_OUT_OF_ARRAY :
		case MM_ERROR_COMMON_OUT_OF_RANGE :
		case MM_ERROR_COMMON_ATTR_NOT_EXIST :
			ret = RECORDER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";			
			break;
		case MM_ERROR_CAMCORDER_NOT_INITIALIZED :
		case MM_ERROR_CAMCORDER_INVALID_STATE :
			ret = RECORDER_ERROR_INVALID_STATE;
			errorstr = "INVALID_STATE";			
			break;

		case MM_ERROR_CAMCORDER_DEVICE :
		case MM_ERROR_CAMCORDER_DEVICE_NOT_FOUND :
		case MM_ERROR_CAMCORDER_DEVICE_BUSY	:
		case MM_ERROR_CAMCORDER_DEVICE_OPEN :
		case MM_ERROR_CAMCORDER_DEVICE_IO :
		case MM_ERROR_CAMCORDER_DEVICE_TIMEOUT	:
		case MM_ERROR_CAMCORDER_DEVICE_REG_TROUBLE :
		case MM_ERROR_CAMCORDER_DEVICE_WRONG_JPEG	 :
		case MM_ERROR_CAMCORDER_DEVICE_LACK_BUFFER :
			ret = RECORDER_ERROR_DEVICE;
			errorstr = "ERROR_DEVICE";			
			break;

		case MM_ERROR_CAMCORDER_GST_CORE :
		case MM_ERROR_CAMCORDER_GST_LIBRARY :
		case MM_ERROR_CAMCORDER_GST_RESOURCE :
		case MM_ERROR_CAMCORDER_GST_STREAM :
		case MM_ERROR_CAMCORDER_GST_STATECHANGE :
		case MM_ERROR_CAMCORDER_GST_NEGOTIATION :
		case MM_ERROR_CAMCORDER_GST_LINK :
		case MM_ERROR_CAMCORDER_GST_FLOW_ERROR :
		case MM_ERROR_CAMCORDER_ENCODER :
		case MM_ERROR_CAMCORDER_ENCODER_BUFFER :
		case MM_ERROR_CAMCORDER_ENCODER_WRONG_TYPE :
		case MM_ERROR_CAMCORDER_ENCODER_WORKING :
		case MM_ERROR_CAMCORDER_INTERNAL :
		case MM_ERROR_CAMCORDER_NOT_SUPPORTED :
		case MM_ERROR_CAMCORDER_RESPONSE_TIMEOUT :
		case MM_ERROR_CAMCORDER_CMD_IS_RUNNING :	
		case MM_ERROR_CAMCORDER_DSP_FAIL :
		case MM_ERROR_CAMCORDER_AUDIO_EMPTY :
		case MM_ERROR_CAMCORDER_CREATE_CONFIGURE :
		case MM_ERROR_CAMCORDER_FILE_SIZE_OVER :
		case MM_ERROR_CAMCORDER_DISPLAY_DEVICE_OFF :
		case MM_ERROR_CAMCORDER_INVALID_CONDITION :
			ret = RECORDER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";			
			break;
		case MM_ERROR_CAMCORDER_RESOURCE_CREATION :
		case MM_ERROR_COMMON_OUT_OF_MEMORY:	
			ret = RECORDER_ERROR_OUT_OF_MEMORY;
			errorstr = "OUT_OF_MEMORY";			
			break;

		case MM_ERROR_POLICY_BLOCKED:
			ret = RECORDER_ERROR_SOUND_POLICY;
			errorstr = "ERROR_SOUND_POLICY";			
			break;

		default:
			ret = RECORDER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
		
	}

	LOGE( "[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	

	return ret;
}

recorder_state_e _recorder_state_convert(MMCamcorderStateType mm_state )
{
	recorder_state_e state = RECORDER_STATE_NONE;
	switch( mm_state ){
		case MM_CAMCORDER_STATE_NONE:	
			state = RECORDER_STATE_NONE;
			break;
		case MM_CAMCORDER_STATE_NULL:
			state = RECORDER_STATE_CREATED;
			break;
		case MM_CAMCORDER_STATE_READY:
			state = RECORDER_STATE_CREATED;
			break;
		case MM_CAMCORDER_STATE_PREPARE:
			state = RECORDER_STATE_READY;
			break;
		case MM_CAMCORDER_STATE_RECORDING:
			state = RECORDER_STATE_RECORDING;
			break;
		case MM_CAMCORDER_STATE_PAUSED:
			state = RECORDER_STATE_PAUSED;
			break;
		default:
			state = RECORDER_STATE_NONE;
			break;
	}

	return state;
}

int mm_recorder_msg_cb(int message, void *param, void *user_data){

	recorder_s * handle = (recorder_s*)user_data;
	MMMessageParamType *m = (MMMessageParamType*)param;
	recorder_state_e previous_state; 

	switch(message){
		case MM_MESSAGE_CAMCORDER_STATE_CHANGED:
		case MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_ASM:
				previous_state = handle->state;
				handle->state = _recorder_state_convert(m->state.current);
				
				if( previous_state != handle->state && handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE] ){
					((recorder_state_changed_cb)handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE])(previous_state, handle->state, message == MM_MESSAGE_CAMCORDER_STATE_CHANGED ? 0 : 1 , handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE]);
				}
				
				// should change intermediate state MM_CAMCORDER_STATE_READY is not valid in capi , change to NULL state
				if( message == MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_ASM ){
					if( m->state.previous == MM_CAMCORDER_STATE_PREPARE && m->state.current == MM_CAMCORDER_STATE_PREPARE ){
						mm_camcorder_unrealize(handle->mm_handle);
					}
				}
				
				break;
		case MM_MESSAGE_CAMCORDER_MAX_SIZE:
		case MM_MESSAGE_CAMCORDER_NO_FREE_SPACE:			
		case MM_MESSAGE_CAMCORDER_TIME_LIMIT:
			{
				recorder_recording_limit_type_e type ;
				if( MM_MESSAGE_CAMCORDER_MAX_SIZE == message )
					type = RECORDER_RECORDING_LIMIT_SIZE;
				else if( MM_MESSAGE_CAMCORDER_NO_FREE_SPACE == message)
					type = RECORDER_RECORDING_LIMIT_FREE_SPACE;
				else
					type = RECORDER_RECORDING_LIMIT_TIME;
				if( handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] ){
					((recorder_recording_limit_reached_cb)handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED])(type, handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED]);
				}
			}			
			break;
		case MM_MESSAGE_CAMCORDER_RECORDING_STATUS:
			if( handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS] ){
				((recorder_recording_status_cb)handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS])( m->recording_status.elapsed, m->recording_status.filesize, handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS]);
			}
			break;
		case MM_MESSAGE_CAMCORDER_CAPTURED :
		{
			MMCamRecordingReport *report = (MMCamRecordingReport *)m ->data;
			int mode;
			mm_camcorder_get_attributes(handle->mm_handle ,NULL,MMCAM_MODE, &mode, NULL);

			if( mode != MM_CAMCORDER_MODE_IMAGE){
				if( report && report->recording_filename ){
					free(report->recording_filename );
					report->recording_filename = NULL;
				}
				if( report ){
					free(report);
					report = NULL;
				}
			}
			break;
		}


			
	}

	return 1;
}

int recorder_create_videorecorder( camera_h camera, recorder_h* recorder){
	
	if( camera == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	

 	int ret;
	recorder_s * handle;
	camera_state_e camera_state;	
	int preview_format;	

	camera_get_state(camera, &camera_state);
	if( camera_state != CAMERA_STATE_CREATED){
		LOGE("[%s] camera state(%d) is not CAMERA_STATE_CREATED ", __func__, camera_state);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	
	handle = (recorder_s*)malloc( sizeof(recorder_s) );
	if(handle == NULL){
		LOGE("[%s] malloc error", __func__);
		return RECORDER_ERROR_OUT_OF_MEMORY;
	}

	memset(handle, 0 , sizeof(recorder_s));		
		
	handle->mm_handle = ((camera_s*)camera)->mm_handle;
	handle->camera = camera;
	handle->state = RECORDER_STATE_CREATED;

	mm_camcorder_set_message_callback(handle->mm_handle, mm_recorder_msg_cb, (void*)handle);

	//((camera_s*)camera)->relay_callback = mm_recorder_msg_cb;
	//((camera_s*)camera)->relay_user_data = handle;
	handle->type = _RECORDER_TYPE_VIDEO;
	*recorder = (recorder_h)handle;

	preview_format = MM_PIXEL_FORMAT_YUYV;
	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL, MMCAM_CAMERA_FORMAT, &preview_format, NULL);
	handle->origin_preview_format = preview_format;
	preview_format = MM_PIXEL_FORMAT_NV12;	
	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL, MMCAM_RECOMMEND_PREVIEW_FORMAT_FOR_RECORDING, &preview_format, NULL);
	
	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL, 
																MMCAM_MODE , MM_CAMCORDER_MODE_VIDEO, 
																MMCAM_CAMERA_FORMAT, preview_format,
																(void*)NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_create_audiorecorder( recorder_h* recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
	
 	int ret;
	recorder_s * handle;
	MMCamPreset info;
	info.videodev_type= MM_VIDEO_DEVICE_NONE;
	
	handle = (recorder_s*)malloc( sizeof(recorder_s) );
	if(handle==NULL){
		LOGE( "[%s] OUT_OF_MEMORY(0x%08x)", __func__, RECORDER_ERROR_OUT_OF_MEMORY);
		return RECORDER_ERROR_OUT_OF_MEMORY;
	}
	
	memset(handle, 0 , sizeof(recorder_s));
	
	ret = mm_camcorder_create(&handle->mm_handle, &info);
	if( ret != MM_ERROR_NONE){
		free(handle);
		LOGE("[%s] mm_camcorder_create fail", __func__);
		return _convert_recorder_error_code(__func__, ret);
	}
	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL, 
																MMCAM_MODE , MM_CAMCORDER_MODE_AUDIO, 
																(void*)NULL);

	if( ret != MM_ERROR_NONE){
		mm_camcorder_destroy(handle->mm_handle);
		free(handle);
		LOGE("[%s] AUDIO mode setting fail", __func__);
		return _convert_recorder_error_code(__func__, ret);
	}


	handle->state = RECORDER_STATE_CREATED;
	mm_camcorder_set_message_callback(handle->mm_handle, mm_recorder_msg_cb, (void*)handle);
	handle->camera = NULL;
	handle->type = _RECORDER_TYPE_AUDIO;

	*recorder = (recorder_h)handle;

	return RECORDER_ERROR_NONE;

}


int recorder_get_state(recorder_h recorder, recorder_state_e * state){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
	if( state == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	

	recorder_s *handle = (recorder_s*)recorder;

	MMCamcorderStateType mmstate ;
	recorder_state_e capi_state;
	mm_camcorder_get_state(handle->mm_handle, &mmstate);	
	capi_state = _recorder_state_convert(mmstate);

	*state = capi_state;
	return CAMERA_ERROR_NONE;
	
}

int recorder_destroy( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
	recorder_s * handle;
	int ret;

	handle = (recorder_s *) recorder;
	if( handle->type == _RECORDER_TYPE_VIDEO ){
		//camera object mode change
		ret = mm_camcorder_set_attributes(handle->mm_handle, NULL, 
																MMCAM_MODE , MM_CAMCORDER_MODE_IMAGE, 
																MMCAM_CAMERA_FORMAT,  handle->origin_preview_format,
																MMCAM_IMAGE_ENCODER , MM_IMAGE_CODEC_JPEG, 
																MMCAM_CAPTURE_FORMAT, MM_PIXEL_FORMAT_ENCODED,
																MMCAM_CAPTURE_COUNT, 1, 
																(void*)NULL);
		
		mm_camcorder_set_message_callback(handle->mm_handle, mm_message_callback, (void*)handle->camera);
	}else{
		ret = mm_camcorder_destroy(handle->mm_handle);
	}

	if(ret == MM_ERROR_NONE)
		free(handle);

	return _convert_recorder_error_code(__func__, ret);

}

int recorder_prepare( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
 	int ret = 0;
	recorder_s *handle = (recorder_s*)recorder;

	MMCamcorderStateType mmstate ;
	mm_camcorder_get_state(handle->mm_handle, &mmstate);	

	if( mmstate !=  MM_CAMCORDER_STATE_READY){
		
		ret = mm_camcorder_realize(handle->mm_handle);	
		if( ret != MM_ERROR_NONE){
			LOGE("[%s] mm_camcorder_realize fail", __func__);
			return _convert_recorder_error_code(__func__, ret);
		}
		
	}
	
	ret = mm_camcorder_start(handle->mm_handle);
	
	if( ret != MM_ERROR_NONE){
		LOGE("[%s] mm_camcorder_start fail", __func__);	
		mm_camcorder_unrealize(handle->mm_handle);
		return _convert_recorder_error_code(__func__, ret);
	}	

	return RECORDER_ERROR_NONE;
}

int recorder_unprepare( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
 	int ret = 0;
	recorder_s *handle = (recorder_s*)recorder;

	MMCamcorderStateType mmstate ;
	mm_camcorder_get_state(handle->mm_handle, &mmstate);	
	
	if( mmstate ==  MM_CAMCORDER_STATE_PREPARE){
		ret = mm_camcorder_stop(handle->mm_handle);	
		if( ret != MM_ERROR_NONE){
			LOGE("[%s] mm_camcorder_stop fail", __func__);	
			return _convert_recorder_error_code(__func__, ret);
		}
	}
	ret = mm_camcorder_unrealize(handle->mm_handle);
	return _convert_recorder_error_code(__func__, ret);
}

int recorder_start( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
 	int ret;
	recorder_s *handle = (recorder_s*)recorder;
	ret = mm_camcorder_record(handle->mm_handle);
	return _convert_recorder_error_code(__func__, ret);
}

int recorder_pause( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s *handle = (recorder_s*)recorder;
	ret = mm_camcorder_pause(handle->mm_handle);

	return _convert_recorder_error_code(__func__, ret);
}

int recorder_commit( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
 	int ret;
	recorder_s *handle = (recorder_s*)recorder;
	ret = mm_camcorder_commit(handle->mm_handle);
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_cancel( recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
 	int ret;
	recorder_s *handle = (recorder_s*)recorder;
	ret = mm_camcorder_cancel(handle->mm_handle);
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_set_filename(recorder_h recorder,  const char *filename){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	g_return_val_if_fail(filename != NULL, RECORDER_ERROR_INVALID_PARAMETER);			
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL,  MMCAM_TARGET_FILENAME  , filename , strlen(filename), NULL);
	return _convert_recorder_error_code(__func__, ret);

}

int recorder_get_filename(recorder_h recorder,  char **filename){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	g_return_val_if_fail(filename != NULL, RECORDER_ERROR_INVALID_PARAMETER);			
	int ret;
	recorder_s * handle = (recorder_s*)recorder;

	char *record_filename;
	int record_filename_size;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_TARGET_FILENAME , &record_filename, &record_filename_size, NULL);
	if( ret == CAMERA_ERROR_NONE ){
		*filename = strdup(record_filename);
	}

	return _convert_recorder_error_code(__func__, ret);
	
}


int recorder_set_file_format(recorder_h recorder, recorder_file_format_e format){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	int format_table[3] = { MM_FILE_FORMAT_3GP ,  // RECORDER_FILE_FORMAT_3GP,
											  MM_FILE_FORMAT_MP4 , //RECORDER_FILE_FORMAT_MP4,
											  MM_FILE_FORMAT_AMR //RECORDER_FILE_FORMAT_AMR,
											};
	
	if( format < RECORDER_FILE_FORMAT_3GP || format > RECORDER_FILE_FORMAT_AMR )
		return RECORDER_ERROR_INVALID_PARAMETER;
	
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_FILE_FORMAT  , format_table[format], NULL);
	return _convert_recorder_error_code(__func__, ret);
}

int recorder_get_file_format(recorder_h recorder, recorder_file_format_e *format){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	g_return_val_if_fail(format != NULL, RECORDER_ERROR_INVALID_PARAMETER);		
	
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	int mm_format;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_FILE_FORMAT  , &mm_format, NULL);

	if( ret == 0 ){
		switch( mm_format ){
			case MM_FILE_FORMAT_3GP:
				*format = RECORDER_FILE_FORMAT_3GP;
				break;
			case MM_FILE_FORMAT_MP4 :
				*format = RECORDER_FILE_FORMAT_MP4;
				break;
			case MM_FILE_FORMAT_AMR :
				*format = RECORDER_FILE_FORMAT_AMR;
				break;				
			default :
				ret = MM_ERROR_CAMCORDER_INTERNAL;
				break;
		}
	}	
	return _convert_recorder_error_code(__func__, ret);
}



int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void* user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
	recorder_s *handle = (recorder_s*)recorder;
	if( callback == NULL )
		return RECORDER_ERROR_INVALID_PARAMETER;
	
	handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE] = user_data;

	return RECORDER_ERROR_NONE;
	
}

int recorder_unset_state_changed_cb(recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	recorder_s *handle = (recorder_s*)recorder;

	handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE] = NULL;

	return RECORDER_ERROR_NONE;	
}

int recorder_set_recording_status_cb(recorder_h recorder, recorder_recording_status_cb callback, void* user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	recorder_s *handle = (recorder_s*)recorder;
	if( callback == NULL )
		return RECORDER_ERROR_INVALID_PARAMETER;
	
	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = user_data;

	return RECORDER_ERROR_NONE;
}

int recorder_unset_recording_status_cb(recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	recorder_s *handle = (recorder_s*)recorder;	
	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = NULL;
	
	return RECORDER_ERROR_NONE;
	
}

int recorder_set_recording_limit_reached_cb(recorder_h recorder, recorder_recording_limit_reached_cb callback, void* user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	recorder_s *handle = (recorder_s*)recorder;
	if( callback == NULL )
		return RECORDER_ERROR_INVALID_PARAMETER;
	
	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = user_data;

	return RECORDER_ERROR_NONE;
	
}

int recorder_unset_recording_limit_reached_cb(recorder_h recorder){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	recorder_s *handle = (recorder_s*)recorder;
	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = NULL;

	return RECORDER_ERROR_NONE;	
}

int recorder_foreach_supported_file_format(recorder_h recorder, recorder_supported_file_format_cb foreach_cb , void *user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	if( foreach_cb == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);			
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	MMCamAttrsInfo info;
	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_FILE_FORMAT , &info);
	if( ret != RECORDER_ERROR_NONE )
		return ret;
	
	int i;
	for( i=0 ; i < info.int_array.count ; i++)
	{

		int format;
		
		switch(  info.int_array.array[i] ){
			case MM_FILE_FORMAT_3GP:
				format = RECORDER_FILE_FORMAT_3GP;
				break;
			case MM_FILE_FORMAT_MP4 :
				format = RECORDER_FILE_FORMAT_MP4;
				break;
			case MM_FILE_FORMAT_AMR :
				format = RECORDER_FILE_FORMAT_AMR;
				break;				
			default :
				format = -1;
		}	
	
		if ( format != -1 && !foreach_cb(format,user_data) )
			break;
	}
	return RECORDER_ERROR_NONE;
	
}



int recorder_attr_set_size_limit(recorder_h recorder,  int kbyte){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, ""  , kbyte, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_set_time_limit(recorder_h recorder,  int second){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_TARGET_TIME_LIMIT , second, NULL);
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_attr_set_audio_device(recorder_h recorder , recorder_audio_device_e device){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_DEVICE , device, NULL);
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_set_audio_encoder(recorder_h recorder, recorder_audio_codec_e  codec){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		

	if( codec < RECORDER_AUDIO_CODEC_AMR || codec > RECORDER_AUDIO_CODEC_AAC)
		return RECORDER_ERROR_INVALID_PARAMETER;
		

	int audio_table[2] = { MM_AUDIO_CODEC_AMR , //RECORDER_AUDIO_CODEC_AMR
											MM_AUDIO_CODEC_AAC  //RECORDER_AUDIO_CODEC_AAC
										};

	
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	if( handle->state != RECORDER_STATE_CREATED )
		return RECORDER_ERROR_INVALID_STATE;
	
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_ENCODER , audio_table[codec], NULL);

	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_get_audio_encoder(recorder_h recorder, recorder_audio_codec_e *codec){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	g_return_val_if_fail(codec != NULL, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	int mm_codec = 0;
	
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_ENCODER , &mm_codec , NULL);
	if( ret == 0 ){
		switch( mm_codec ){
			case MM_AUDIO_CODEC_AMR :
				*codec = RECORDER_AUDIO_CODEC_AMR;
				break;
			case MM_AUDIO_CODEC_AAC :
				*codec = RECORDER_AUDIO_CODEC_AAC;
				break;
			default :
				ret = MM_ERROR_CAMCORDER_INTERNAL;
				break;
		}
	}
		
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_set_video_encoder(recorder_h recorder, recorder_video_codec_e  codec){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;

	int video_table[3] = { MM_VIDEO_CODEC_H263,		//RECORDER_VIDEO_CODEC_H263,			/**< H263 codec		*/
											MM_VIDEO_CODEC_H264, 	//RECORDER_VIDEO_CODEC_H264,			/**< H264 codec		*/
											MM_VIDEO_CODEC_MPEG4 	//RECORDER_VIDEO_CODEC_MPEG4,			/**< MPEG4 codec	*/
										};
	if( codec < RECORDER_VIDEO_CODEC_H263 || codec > RECORDER_VIDEO_CODEC_MPEG4 )
		return RECORDER_ERROR_INVALID_PARAMETER;
	
	recorder_s * handle = (recorder_s*)recorder;
	if( handle->state != RECORDER_STATE_CREATED )
		return RECORDER_ERROR_INVALID_STATE;

	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_VIDEO_ENCODER   , video_table[codec], NULL);
	return _convert_recorder_error_code(__func__, ret);
}



int recorder_get_video_encoder(recorder_h recorder, recorder_video_codec_e *codec){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	
	if( codec == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);	

	int ret;
	int mm_codec = 0;

	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_VIDEO_ENCODER , &mm_codec, NULL);
	if( ret == 0 ){
		switch( mm_codec ){
			case MM_VIDEO_CODEC_H263 :
				*codec = RECORDER_VIDEO_CODEC_H263;
				break;
			case MM_VIDEO_CODEC_H264 :
				*codec = RECORDER_VIDEO_CODEC_H264;
				break;
			case MM_VIDEO_CODEC_MPEG4 :
				*codec = RECORDER_VIDEO_CODEC_MPEG4;
				break;				
			default :
				ret = MM_ERROR_CAMCORDER_INTERNAL;
				break;
		}
	}
	
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_set_audio_samplerate(recorder_h recorder, int samplerate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_SAMPLERATE  , samplerate, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_set_audio_encoder_bitrate(recorder_h recorder,  int bitrate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_ENCODER_BITRATE  , bitrate, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_set_video_encoder_bitrate(recorder_h recorder,  int bitrate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_set_attributes(handle->mm_handle ,NULL, MMCAM_VIDEO_ENCODER_BITRATE  , bitrate, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_get_size_limit(recorder_h recorder,  int *kbyte){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, ""  , kbyte, NULL);
	return _convert_recorder_error_code(__func__, ret);	
}

int recorder_attr_get_time_limit(recorder_h recorder,  int *second){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_TARGET_TIME_LIMIT , second, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_get_audio_device(recorder_h recorder , recorder_audio_device_e *device){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_DEVICE , device, NULL);
	return _convert_recorder_error_code(__func__, ret);	
}



int recorder_attr_get_audio_samplerate(recorder_h recorder, int *samplerate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_SAMPLERATE , samplerate, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_attr_get_audio_encoder_bitrate(recorder_h recorder,  int *bitrate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_AUDIO_ENCODER_BITRATE , bitrate, NULL);
	return _convert_recorder_error_code(__func__, ret);
}

int recorder_attr_get_video_encoder_bitrate(recorder_h recorder,  int *bitrate){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL, MMCAM_VIDEO_ENCODER_BITRATE , bitrate, NULL);
	return _convert_recorder_error_code(__func__, ret);
	
}

int recorder_foreach_supported_audio_encoder(recorder_h recorder, recorder_supported_audio_encoder_cb foreach_cb , void *user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	if( foreach_cb == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);		
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	MMCamAttrsInfo info;
	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_AUDIO_ENCODER , &info);
	if( ret != RECORDER_ERROR_NONE )
		return _convert_recorder_error_code(__func__, ret);
	
	int i;
	for( i=0 ; i < info.int_array.count ; i++)
	{
		int codec;
		
		switch(  info.int_array.array[i] ){
			case MM_AUDIO_CODEC_AMR:
				codec = RECORDER_AUDIO_CODEC_AMR;
				break;
			case MM_AUDIO_CODEC_AAC :
				codec = RECORDER_AUDIO_CODEC_AAC;
				break;
			default :
				codec = -1;
		}	
		if( codec != -1 && !foreach_cb(codec,user_data) )
			break;
	}
	return RECORDER_ERROR_NONE;	
}
int recorder_foreach_supported_video_encoder(recorder_h recorder, recorder_supported_video_encoder_cb foreach_cb , void *user_data){
	
	if( recorder == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);
	if( foreach_cb == NULL) return _convert_recorder_error_code(__func__, RECORDER_ERROR_INVALID_PARAMETER);
	int ret;
	recorder_s * handle = (recorder_s*)recorder;
	MMCamAttrsInfo info;
	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_VIDEO_ENCODER , &info);
	if( ret != RECORDER_ERROR_NONE )
		return _convert_recorder_error_code(__func__, ret);
	
	int i;
	for( i=0 ; i < info.int_array.count ; i++)
	{
		int codec;
		
		switch(  info.int_array.array[i] ){
			case MM_VIDEO_CODEC_H263 :
				codec = RECORDER_VIDEO_CODEC_H263;
				break;
			case MM_VIDEO_CODEC_H264 :
				codec = RECORDER_VIDEO_CODEC_H264;
				break;
			case MM_VIDEO_CODEC_MPEG4 :
				codec = RECORDER_VIDEO_CODEC_MPEG4;
				break;		
			default :
				codec = -1;
		}	
	
		if ( codec != -1 &&  !foreach_cb(codec,user_data) )
			break;
	}
	return RECORDER_ERROR_NONE;
	
}

