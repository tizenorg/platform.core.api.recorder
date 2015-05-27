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
#include <audio-session-manager-types.h>
#include <mm_camcorder.h>
#include <mm_types.h>
#include <math.h>
#include <camera.h>
#include <recorder_private.h>
#include <dlog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_RECORDER"

#define LOWSET_DECIBEL -300.0
#define RECORDER_PATH_RECORDER_RESOURCE    "/usr/share/sounds/mm-camcorder/recorder_resource"


/*
 * camera_private function
*/
int _camera_get_mm_handle(camera_h camera , MMHandleType *handle);
int _camera_set_relay_mm_message_callback(camera_h camera, MMMessageCallback callback, void *user_data);
int _camera_set_use(camera_h camera, bool used);
bool _camera_is_used(camera_h camera);
/*
 * end of camera_private function
 */

static int __mm_audio_stream_cb(MMCamcorderAudioStreamDataType *stream, void *user_param);
static int __mm_recorder_msg_cb(int message, void *param, void *user_data);


static int __convert_error_code_camera_to_recorder(int code)
{
	int new_code = code;

	switch (code) {
	case CAMERA_ERROR_INVALID_STATE :
		new_code = RECORDER_ERROR_INVALID_STATE;
		break;
	case CAMERA_ERROR_DEVICE:
		new_code = RECORDER_ERROR_DEVICE;
		break;
	case CAMERA_ERROR_SOUND_POLICY:
		new_code = RECORDER_ERROR_SOUND_POLICY;
		break;
	case CAMERA_ERROR_SECURITY_RESTRICTED:
		new_code = RECORDER_ERROR_SECURITY_RESTRICTED;
		break;
	default:
		break;
	}

	return new_code;
}


int __convert_recorder_error_code(const char *func, int code)
{
	int ret = RECORDER_ERROR_INVALID_OPERATION;
	const char *errorstr = NULL;

	switch (code) {
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
		ret = RECORDER_ERROR_INVALID_PARAMETER;
		errorstr = "INVALID_PARAMETER";
		break;
	case MM_ERROR_COMMON_INVALID_PERMISSION :
		ret = RECORDER_ERROR_PERMISSION_DENIED;
		errorstr = "ERROR_PERMISSION_DENIED";
		break;
	case MM_ERROR_CAMCORDER_NOT_INITIALIZED :
	case MM_ERROR_CAMCORDER_INVALID_STATE :
		ret = RECORDER_ERROR_INVALID_STATE;
		errorstr = "INVALID_STATE";
		break;
	case MM_ERROR_CAMCORDER_DEVICE :
	case MM_ERROR_CAMCORDER_DEVICE_NOT_FOUND :
	case MM_ERROR_CAMCORDER_DEVICE_BUSY :
	case MM_ERROR_CAMCORDER_DEVICE_OPEN :
	case MM_ERROR_CAMCORDER_DEVICE_IO :
	case MM_ERROR_CAMCORDER_DEVICE_TIMEOUT :
	case MM_ERROR_CAMCORDER_DEVICE_WRONG_JPEG :
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
	case MM_ERROR_POLICY_BLOCKED_BY_CALL:
		ret = RECORDER_ERROR_SOUND_POLICY_BY_CALL;
		errorstr = "ERROR_SOUND_POLICY_BY_CALL";
		break;
	case MM_ERROR_POLICY_BLOCKED_BY_ALARM:
		ret = RECORDER_ERROR_SOUND_POLICY_BY_ALARM;
		errorstr = "ERROR_SOUND_POLICY_BY_ALARM";
		break;
	case MM_ERROR_POLICY_RESTRICTED:
		ret = RECORDER_ERROR_SECURITY_RESTRICTED;
		errorstr = "ERROR_RESTRICTED";
		break;
	case MM_ERROR_CAMCORDER_DEVICE_REG_TROUBLE:
		ret = RECORDER_ERROR_ESD;
		errorstr = "ERROR_ESD";
		break;
	case MM_ERROR_OUT_OF_STORAGE:
		ret = RECORDER_ERROR_OUT_OF_STORAGE;
		errorstr = "OUT_OF_STORAGE";
		break;
	case MM_ERROR_COMMON_OUT_OF_ARRAY:
	case MM_ERROR_COMMON_OUT_OF_RANGE:
	case MM_ERROR_COMMON_ATTR_NOT_EXIST:
	case MM_ERROR_CAMCORDER_NOT_SUPPORTED:
		ret = RECORDER_ERROR_NOT_SUPPORTED;
		errorstr = "NOT_SUPPORTED";
		break;
	default:
		ret = RECORDER_ERROR_INVALID_OPERATION;
		errorstr = "INVALID_OPERATION";
		break;
	}

	LOGE("[%s] %s(0x%08x) : core frameworks error code(0x%08x)", func, errorstr, ret, code);

	return ret;
}


static recorder_state_e __recorder_state_convert(MMCamcorderStateType mm_state)
{
	recorder_state_e state = RECORDER_STATE_NONE;
	switch (mm_state) {
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
	case MM_CAMCORDER_STATE_CAPTURING:
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


static int __mm_recorder_msg_cb(int message, void *param, void *user_data)
{
	recorder_s * handle = (recorder_s *)user_data;
	MMMessageParamType *m = (MMMessageParamType *)param;
	recorder_state_e previous_state;
	recorder_recording_limit_type_e type;
	int recorder_error = 0;

	switch (message) {
	case MM_MESSAGE_READY_TO_RESUME:
		LOGW("not supported message");
		break;
	case MM_MESSAGE_CAMCORDER_STATE_CHANGED:
	case MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_ASM:
	case MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_SECURITY:
		previous_state = handle->state;
		handle->state = __recorder_state_convert(m->state.current);
		recorder_policy_e policy = RECORDER_POLICY_NONE;
		if (message == MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_ASM) {
			switch (m->state.code) {
			case ASM_EVENT_SOURCE_CALL_START:
			case ASM_EVENT_SOURCE_CALL_END:
				policy = RECORDER_POLICY_SOUND_BY_CALL;
				LOGE("RECORDER_POLICY_SOUND_BY_CALL");
				break;
			case ASM_EVENT_SOURCE_ALARM_START:
			case ASM_EVENT_SOURCE_ALARM_END:
				policy = RECORDER_POLICY_SOUND_BY_ALARM;
				LOGE("RECORDER_POLICY_SOUND_BY_ALARM");
				break;
			default:
				policy = RECORDER_POLICY_SOUND;
				LOGE("RECORDER_POLICY_SOUND");
				break;
			}
		} else if (message == MM_MESSAGE_CAMCORDER_STATE_CHANGED_BY_SECURITY) {
			policy = RECORDER_POLICY_SECURITY;
			LOGE("RECORDER_POLICY_SECURITY");
		}

		if (previous_state != handle->state && handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE]) {
			((recorder_state_changed_cb)handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE])(previous_state, handle->state, policy, handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE]);
		}

		/* should change intermediate state MM_CAMCORDER_STATE_READY is not valid in capi , change to NULL state */
		if (policy != RECORDER_POLICY_NONE &&
		    (m->state.current == MM_CAMCORDER_STATE_PAUSED || m->state.current == MM_CAMCORDER_STATE_NULL)) {
			if (handle->user_cb[_RECORDER_EVENT_TYPE_INTERRUPTED]) {
				((recorder_interrupted_cb)handle->user_cb[_RECORDER_EVENT_TYPE_INTERRUPTED])(policy, previous_state, handle->state, handle->user_data[_RECORDER_EVENT_TYPE_INTERRUPTED]);
			} else {
				LOGW("_RECORDER_EVENT_TYPE_INTERRUPTED cb is NULL");
			}
		}
		break;
	case MM_MESSAGE_CAMCORDER_MAX_SIZE:
	case MM_MESSAGE_CAMCORDER_NO_FREE_SPACE:
	case MM_MESSAGE_CAMCORDER_TIME_LIMIT:
		if (MM_MESSAGE_CAMCORDER_MAX_SIZE == message) {
			type = RECORDER_RECORDING_LIMIT_SIZE;
		} else if (MM_MESSAGE_CAMCORDER_NO_FREE_SPACE == message) {
			type = RECORDER_RECORDING_LIMIT_FREE_SPACE;
		} else {
			type = RECORDER_RECORDING_LIMIT_TIME;
		}
		if (handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED]) {
			((recorder_recording_limit_reached_cb)handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED])(type, handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED]);
		}
		break;
	case MM_MESSAGE_CAMCORDER_RECORDING_STATUS:
		if (handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS]) {
			((recorder_recording_status_cb)handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS])(m->recording_status.elapsed, m->recording_status.filesize, handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS]);
		}
		break;
	case MM_MESSAGE_CAMCORDER_VIDEO_CAPTURED:
	case MM_MESSAGE_CAMCORDER_AUDIO_CAPTURED:
		if (handle->type == _RECORDER_TYPE_AUDIO) {
			MMCamRecordingReport *report = (MMCamRecordingReport *)m->data;
			if (report != NULL && report->recording_filename) {
				free(report->recording_filename);
				report->recording_filename = NULL;
			}
			if (report) {
				free(report);
				report = NULL;
			}
		}
		break;
	case MM_MESSAGE_CAMCORDER_ERROR:
		switch (m->code) {
		case MM_ERROR_CAMCORDER_DEVICE:
		case MM_ERROR_CAMCORDER_DEVICE_TIMEOUT:
		case MM_ERROR_CAMCORDER_DEVICE_WRONG_JPEG:
			recorder_error = RECORDER_ERROR_DEVICE;
			break;
		case MM_ERROR_CAMCORDER_GST_CORE:
		case MM_ERROR_CAMCORDER_GST_LIBRARY:
		case MM_ERROR_CAMCORDER_GST_RESOURCE:
		case MM_ERROR_CAMCORDER_GST_STREAM:
		case MM_ERROR_CAMCORDER_GST_NEGOTIATION:
		case MM_ERROR_CAMCORDER_GST_FLOW_ERROR:
		case MM_ERROR_CAMCORDER_ENCODER:
		case MM_ERROR_CAMCORDER_ENCODER_BUFFER:
		case MM_ERROR_CAMCORDER_ENCODER_WORKING:
		case MM_ERROR_CAMCORDER_MNOTE_CREATION:
		case MM_ERROR_CAMCORDER_MNOTE_ADD_ENTRY:
		case MM_ERROR_CAMCORDER_INTERNAL:
		case MM_ERROR_FILE_NOT_FOUND:
		case MM_ERROR_FILE_READ:
			recorder_error = RECORDER_ERROR_INVALID_OPERATION;
			break;
		case MM_ERROR_CAMCORDER_LOW_MEMORY:
		case MM_ERROR_CAMCORDER_MNOTE_MALLOC:
			recorder_error = RECORDER_ERROR_OUT_OF_MEMORY;
			break;
		case MM_ERROR_CAMCORDER_DEVICE_REG_TROUBLE:
			recorder_error = RECORDER_ERROR_ESD;
			break;
		case MM_ERROR_OUT_OF_STORAGE:
			recorder_error = RECORDER_ERROR_OUT_OF_STORAGE;
			break;
		default:
			recorder_error = RECORDER_ERROR_INVALID_OPERATION;
			break;
		}

		if (recorder_error != 0 && handle->user_cb[_RECORDER_EVENT_TYPE_ERROR]) {
			((recorder_error_cb)handle->user_cb[_RECORDER_EVENT_TYPE_ERROR])(recorder_error, handle->state, handle->user_data[_RECORDER_EVENT_TYPE_ERROR]);
		}
		break;
	case MM_MESSAGE_CAMCORDER_CURRENT_VOLUME:
		if (handle->last_max_input_level < m->rec_volume_dB) {
			handle->last_max_input_level = m->rec_volume_dB;
		}
		break;
	default:
		break;
	}

	return 1;
}


static int __mm_audio_stream_cb(MMCamcorderAudioStreamDataType *stream, void *user_param)
{
	if (user_param == NULL || stream == NULL) {
		return 0;
	}

	recorder_s *handle = (recorder_s *)user_param;
	audio_sample_type_e format = AUDIO_SAMPLE_TYPE_U8;

	if (stream->format == MM_CAMCORDER_AUDIO_FORMAT_PCM_S16_LE) {
		format = AUDIO_SAMPLE_TYPE_S16_LE;
	}

	if( handle->user_cb[_RECORDER_EVENT_TYPE_AUDIO_STREAM] ){
		((recorder_audio_stream_cb)(handle->user_cb[_RECORDER_EVENT_TYPE_AUDIO_STREAM]))(stream->data, stream->length, format,
												 stream->channel, stream->timestamp,
												 handle->user_data[_RECORDER_EVENT_TYPE_AUDIO_STREAM]);
	}

	return 1;
}


static int _recorder_check_and_set_attribute(recorder_h recorder, const char *attribute_name, int set_value)
{
	bool reset_pipeline = false;
	bool restore_set = false;
	int ret = MM_ERROR_NONE;
	int ret2 = MM_ERROR_NONE;
	int current_value = -1;
	int current_audio_disable = 0;

	recorder_s *handle = (recorder_s *)recorder;
	MMCamcorderStateType mmstate = MM_CAMCORDER_STATE_NONE;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	mm_camcorder_get_state(handle->mm_handle, &mmstate);
	if (mmstate >= MM_CAMCORDER_STATE_RECORDING) {
		LOGE("invalid state %d", mmstate);
		return RECORDER_ERROR_INVALID_STATE;
	}

	if (handle->type == _RECORDER_TYPE_AUDIO && mmstate == MM_CAMCORDER_STATE_PREPARE) {
		mm_camcorder_get_attributes(handle->mm_handle, NULL,
					    MMCAM_AUDIO_DISABLE, &current_audio_disable,
					    attribute_name, &current_value,
					    NULL);

		if (current_value != set_value) {
			LOGD("try to reset pipeline");

			ret = mm_camcorder_stop(handle->mm_handle);
			if (ret != MM_ERROR_NONE) {
				LOGE("mm_camcorder_stop failed 0x%x", ret);
				return __convert_recorder_error_code(attribute_name, ret);
			}

			ret = mm_camcorder_unrealize(handle->mm_handle);
			if (ret != MM_ERROR_NONE) {
				LOGE("mm_camcorder_unrealize failed 0x%x", ret);
				mm_camcorder_start(handle->mm_handle);
				return __convert_recorder_error_code(attribute_name, ret);
			}

			reset_pipeline = true;
		}
	}

	if (!strcmp(attribute_name, MMCAM_AUDIO_ENCODER)) {
		if (set_value == RECORDER_AUDIO_CODEC_DISABLE) {
			ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
							  MMCAM_AUDIO_DISABLE, true,
						          NULL);
		} else {
			ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
							  MMCAM_AUDIO_DISABLE, false,
							  MMCAM_AUDIO_ENCODER, set_value,
							  NULL);
		}
	} else {
		ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
						  attribute_name, set_value,
						  NULL);
	}

	if (ret != MM_ERROR_NONE) {
		LOGE("set [%s] failed 0x%x", attribute_name, ret);
	}

	if (reset_pipeline) {
		ret2 = mm_camcorder_realize(handle->mm_handle);
		if (ret2 == MM_ERROR_NONE) {
			ret2 = mm_camcorder_start(handle->mm_handle);
			if (ret2 == MM_ERROR_NONE) {
				LOGW("restart pipeline done.");
			} else {
				LOGE("mm_camcorder_start failed 0x%x", ret2);
				mm_camcorder_unrealize(handle->mm_handle);
			}
		} else {
			LOGE("mm_camcorder_realize failed 0x%x", ret2);
		}

		if (ret2 != MM_ERROR_NONE) {
			restore_set = true;
			/* determine return value */
			if (ret == MM_ERROR_NONE) {
				ret = ret2;
			}
		}
	}

	if (restore_set) {
		ret2 = mm_camcorder_set_attributes(handle->mm_handle, NULL,
						   MMCAM_AUDIO_DISABLE, current_audio_disable,
						   attribute_name, current_value,
						   NULL);
		LOGW("restore attribute set : 0x%x", ret2);

		ret2 = mm_camcorder_realize(handle->mm_handle);
		LOGW("restore mm_camcorder_realize : 0x%x", ret2);

		ret2 = mm_camcorder_start(handle->mm_handle);
		LOGW("restore mm_camcorder_realize : 0x%x", ret2);
		if (ret2 != MM_ERROR_NONE) {
			ret2 = mm_camcorder_unrealize(handle->mm_handle);
			LOGW("restore mm_camcorder_unrealize : 0x%x", ret2);
		}
	}

	return __convert_recorder_error_code(attribute_name, ret);
}


int recorder_create_videorecorder(camera_h camera, recorder_h *recorder)
{
	int ret = MM_ERROR_NONE;
	int resource_fd = -1;
	recorder_s *handle = NULL;
	int preview_format = MM_PIXEL_FORMAT_NV12;
	int camera_device_count = 0;

	if (camera == NULL) {
		LOGE("NULL pointer camera handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (recorder == NULL) {
		LOGE("NULL pointer recorder handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	/* Check already used in another recorder */
	if (_camera_is_used(camera)) {
		LOGE("[%s] camera is using in another recorder.", __func__);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	resource_fd = open(RECORDER_PATH_RECORDER_RESOURCE, O_RDONLY);
	if (resource_fd < 0) {
		LOGE("failed to open recorder resource : errno [%d]", errno);
		if (errno == EPERM || errno == EACCES) {
			LOGE("permission denied");
			return RECORDER_ERROR_PERMISSION_DENIED;
		} else {
			LOGE("invalid operation");
			return RECORDER_ERROR_INVALID_OPERATION;
		}
	}

	close(resource_fd);
	resource_fd = -1;

	LOGW("permission check done");

	handle = (recorder_s*)malloc( sizeof(recorder_s) );
	if(handle == NULL){
		LOGE("[%s] malloc error", __func__);
		return RECORDER_ERROR_OUT_OF_MEMORY;
	}

	memset(handle, 0 , sizeof(recorder_s));
	handle->src_type = _RECORDER_SOURCE_TYPE_CAMERA;
	handle->last_max_input_level = LOWSET_DECIBEL;
	handle->changed_preview_format = -1;
	handle->mm_source.camera = camera;

	_camera_get_mm_handle(camera, &handle->mm_handle);
	_camera_set_relay_mm_message_callback(camera, __mm_recorder_msg_cb , (void*)handle);
	handle->type = _RECORDER_TYPE_VIDEO;
	recorder_get_state((recorder_h)handle, (recorder_state_e*)&handle->state);

	mm_camcorder_get_attributes(handle->mm_handle, NULL,
				    MMCAM_CAMERA_FORMAT, &preview_format,
				    NULL);
	handle->origin_preview_format = preview_format;
	mm_camcorder_get_attributes(handle->mm_handle, NULL,
				    MMCAM_RECOMMEND_PREVIEW_FORMAT_FOR_RECORDING, &preview_format,
				    NULL);
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL,
				    MMCAM_CAMERA_DEVICE_COUNT, &camera_device_count,
				    NULL);
	if (ret != MM_ERROR_NONE) {
		free(handle);
		handle = NULL;
		LOGE("get device count error");
		return __convert_recorder_error_code(__func__, ret);
	}
	if (camera_device_count == 0) {
		free(handle);
		handle = NULL;
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	} else {
		handle->camera_device_count = camera_device_count;
	}

	_camera_set_use(camera, true);
	if (handle->state == RECORDER_STATE_CREATED) {
		ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
						  MMCAM_CAMERA_FORMAT, preview_format,
						  NULL);
		if (ret == MM_ERROR_NONE) {
			handle->changed_preview_format = preview_format;
		}
	}
	*recorder = (recorder_h)handle;

	return RECORDER_ERROR_NONE;
}


int recorder_create_audiorecorder(recorder_h *recorder)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = NULL;
	MMCamPreset info;
	int camera_device_count = 0;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	info.videodev_type = MM_VIDEO_DEVICE_NONE;

	handle = (recorder_s *)malloc(sizeof(recorder_s));
	if (handle == NULL) {
		LOGE("OUT_OF_MEMORY(0x%08x)", RECORDER_ERROR_OUT_OF_MEMORY);
		return RECORDER_ERROR_OUT_OF_MEMORY;
	}

	memset(handle, 0, sizeof(recorder_s));

	handle->last_max_input_level = LOWSET_DECIBEL;

	ret = mm_camcorder_create(&handle->mm_handle, &info);
	if (ret != MM_ERROR_NONE) {
		free(handle);
		handle = NULL;
		LOGE("mm_camcorder_create fail");
		return __convert_recorder_error_code(__func__, ret);
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_MODE, MM_CAMCORDER_MODE_AUDIO,
					  NULL);

	if (ret != MM_ERROR_NONE) {
		mm_camcorder_destroy(handle->mm_handle);
		free(handle);
		handle = NULL;
		LOGE("AUDIO mode setting fail");
		return __convert_recorder_error_code(__func__, ret);
	}
	ret = mm_camcorder_get_attributes(handle->mm_handle ,NULL,
				    MMCAM_CAMERA_DEVICE_COUNT, &camera_device_count, NULL);
	if (ret != MM_ERROR_NONE) {
		mm_camcorder_destroy(handle->mm_handle);
		free(handle);
		handle = NULL;
		LOGE("get device count error");
		return __convert_recorder_error_code(__func__, ret);
	} else {
		handle->camera_device_count = camera_device_count;
	}
	handle->state = RECORDER_STATE_CREATED;
	handle->mm_source.camera = NULL;
	handle->type = _RECORDER_TYPE_AUDIO;

	mm_camcorder_set_message_callback(handle->mm_handle, __mm_recorder_msg_cb, (void*)handle);

	*recorder = (recorder_h)handle;

	return RECORDER_ERROR_NONE;
}


int recorder_get_state(recorder_h recorder, recorder_state_e *state)
{
	int ret = MM_ERROR_NONE;
	MMCamcorderStateType mmstate;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (state == NULL) {
		LOGE("NULL pointer state");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	recorder_s *handle = (recorder_s*)recorder;

	ret = mm_camcorder_get_state(handle->mm_handle, &mmstate);
	if (ret != MM_ERROR_NONE) {
		return __convert_recorder_error_code(__func__, ret);
	}

	*state = __recorder_state_convert(mmstate);

	return RECORDER_ERROR_NONE;
}


int recorder_destroy(recorder_h recorder)
{
	recorder_s *handle = NULL;
	int ret = MM_ERROR_NONE;
	int preview_format;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle = (recorder_s *)recorder;

	if (handle->type == _RECORDER_TYPE_VIDEO) {
		/* set to unsed */
		_camera_set_use(handle->mm_source.camera, false);
		ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
						  MMCAM_CAMERA_FORMAT, &preview_format,
						  NULL);

		/* preview format was changed? */
		if (ret == MM_ERROR_NONE && preview_format == handle->changed_preview_format) {
			ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
							  MMCAM_CAMERA_FORMAT, handle->origin_preview_format,
							  NULL);
		}

		if (ret == MM_ERROR_NONE) {
			_camera_set_relay_mm_message_callback(handle->mm_source.camera, NULL, NULL);
		}
	} else {
		ret = mm_camcorder_destroy(handle->mm_handle);
	}

	if (ret == MM_ERROR_NONE) {
		free(handle);
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_prepare(recorder_h recorder)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamcorderStateType mmstate;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (handle->type == _RECORDER_TYPE_VIDEO) {
		return __convert_error_code_camera_to_recorder(camera_start_preview(handle->mm_source.camera));
	}

	ret = mm_camcorder_get_state(handle->mm_handle, &mmstate);

	if (ret == MM_ERROR_NONE && mmstate < MM_CAMCORDER_STATE_READY) {
		ret = mm_camcorder_realize(handle->mm_handle);
		if (ret != MM_ERROR_NONE) {
			LOGE("mm_camcorder_realize fail");
			return __convert_recorder_error_code(__func__, ret);
		}
	}

	ret = mm_camcorder_start(handle->mm_handle);
	if (ret != MM_ERROR_NONE) {
		LOGE("mm_camcorder_start fail");
		mm_camcorder_unrealize(handle->mm_handle);
		return __convert_recorder_error_code(__func__, ret);
	}

	return RECORDER_ERROR_NONE;
}


int recorder_unprepare(recorder_h recorder)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamcorderStateType mmstate;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_state(handle->mm_handle, &mmstate);
	if (ret == MM_ERROR_NONE && mmstate == MM_CAMCORDER_STATE_PREPARE) {
		ret = mm_camcorder_stop(handle->mm_handle);
		if( ret != MM_ERROR_NONE){
			LOGE("mm_camcorder_stop fail");
		}
	}

	if (ret == MM_ERROR_NONE) {
		ret = mm_camcorder_unrealize(handle->mm_handle);
		if (ret != MM_ERROR_NONE) {
			LOGE("mm_camcorder_unrealize fail");
			mm_camcorder_start(handle->mm_handle);
		}
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_start(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return __convert_recorder_error_code(__func__, mm_camcorder_record(handle->mm_handle));
}


int recorder_pause(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return __convert_recorder_error_code(__func__, mm_camcorder_pause(handle->mm_handle));
}


int recorder_commit(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return __convert_recorder_error_code(__func__, mm_camcorder_commit(handle->mm_handle));
}


int recorder_cancel(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return __convert_recorder_error_code(__func__, mm_camcorder_cancel(handle->mm_handle));
}


int recorder_set_video_resolution(recorder_h recorder, int width, int height)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s*)recorder;
	recorder_state_e state;

	if (handle == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	recorder_get_state(recorder, &state);
	if (state > RECORDER_STATE_READY) {
		LOGE("RECORDER_ERROR_INVALID_STATE (state:%d)", state);
		return RECORDER_ERROR_INVALID_STATE;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_WIDTH, width,
					  MMCAM_VIDEO_HEIGHT, height,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_get_video_resolution(recorder_h recorder, int *width, int *height)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s*)recorder;

	if (!handle) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}

	if (!width || !height) {
		LOGE("NULL pointer width = [%p], height = [%p]", width, height);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_WIDTH, width,
					  MMCAM_VIDEO_HEIGHT, height,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_foreach_supported_video_resolution(recorder_h recorder,
                                                recorder_supported_video_resolution_cb foreach_cb, void *user_data)
{
	int i = 0;
	int ret = MM_ERROR_NONE;
	recorder_s * handle = (recorder_s*)recorder;
	MMCamAttrsInfo video_width;
	MMCamAttrsInfo video_height;

	if (!handle) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}

	if (!foreach_cb) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_VIDEO_WIDTH, &video_width);
	ret |= mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_VIDEO_HEIGHT, &video_height);

	if (ret != MM_ERROR_NONE ) {
		return __convert_recorder_error_code(__func__, ret);
	}

	for (i = 0 ; i < video_width.int_array.count ; i++) {
		if (!foreach_cb(video_width.int_array.array[i], video_height.int_array.array[i], user_data)) {
			break;
		}
	}

	return RECORDER_ERROR_NONE;
}


int recorder_get_audio_level(recorder_h recorder, double *level)
{
	recorder_s *handle = (recorder_s *)recorder;
	recorder_state_e state;

	if (recorder == NULL || level == NULL) {
		LOGE("NULL pointer %p %p", recorder, level);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	recorder_get_state(recorder, &state);
	if (state < RECORDER_STATE_RECORDING) {
		LOGE("RECORDER_ERROR_INVALID_STATE(0x%08x)", RECORDER_ERROR_INVALID_STATE);
		return RECORDER_ERROR_INVALID_STATE;
	}

	*level = handle->last_max_input_level;
	handle->last_max_input_level = LOWSET_DECIBEL;

	return RECORDER_ERROR_NONE;
}


int recorder_set_filename(recorder_h recorder,  const char *filename)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamcorderStateType mmstate = MM_CAMCORDER_STATE_NONE;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (filename == NULL) {
		LOGE("filename is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	mm_camcorder_get_state(handle->mm_handle, &mmstate);
	if (mmstate >= MM_CAMCORDER_STATE_RECORDING) {
		LOGE("invalid state %d", mmstate);
		return RECORDER_ERROR_INVALID_STATE;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_FILENAME, filename, strlen(filename),
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_get_filename(recorder_h recorder,  char **filename)
{
	int ret = MM_ERROR_NONE;
	char *record_filename = NULL;
	int record_filename_size;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (filename == NULL) {
		LOGE("filename is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_FILENAME, &record_filename, &record_filename_size,
					  NULL);
	if (ret == MM_ERROR_NONE && record_filename) {
		*filename = strdup(record_filename);
	} else {
		LOGE("internal return (0x%08x), get filename p:%p", ret, record_filename);
		*filename = NULL;
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_set_file_format(recorder_h recorder, recorder_file_format_e format)
{
	int format_table[6] = { MM_FILE_FORMAT_3GP, /* RECORDER_FILE_FORMAT_3GP */
	                        MM_FILE_FORMAT_MP4, /* RECORDER_FILE_FORMAT_MP4 */
	                        MM_FILE_FORMAT_AMR, /* RECORDER_FILE_FORMAT_AMR */
	                        MM_FILE_FORMAT_AAC, /* RECORDER_FILE_FORMAT_ADTS */
	                        MM_FILE_FORMAT_WAV, /* RECORDER_FILE_FORMAT_WAV */
	                        MM_FILE_FORMAT_OGG  /* RECORDER_FILE_FORMAT_OGG */
	};

	if (format < RECORDER_FILE_FORMAT_3GP || format > RECORDER_FILE_FORMAT_OGG) {
		LOGE("invalid format %d", format);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_FILE_FORMAT, format_table[format]);
}


int recorder_get_file_format(recorder_h recorder, recorder_file_format_e *format)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;
	int mm_format;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (format == NULL) {
		LOGE("format is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_FILE_FORMAT, &mm_format,
					  NULL);
	if (ret == MM_ERROR_NONE) {
		switch (mm_format) {
		case MM_FILE_FORMAT_3GP:
			*format = RECORDER_FILE_FORMAT_3GP;
			break;
		case MM_FILE_FORMAT_MP4 :
			*format = RECORDER_FILE_FORMAT_MP4;
			break;
		case MM_FILE_FORMAT_AMR :
			*format = RECORDER_FILE_FORMAT_AMR;
			break;
		case MM_FILE_FORMAT_AAC :
			*format = RECORDER_FILE_FORMAT_ADTS;
			break;
		case MM_FILE_FORMAT_WAV:
			*format = RECORDER_FILE_FORMAT_WAV;
			break;
		case MM_FILE_FORMAT_OGG:
			*format = RECORDER_FILE_FORMAT_OGG;
			break;
		default :
			ret = MM_ERROR_CAMCORDER_INTERNAL;
			break;
		}
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void* user_data)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE] = user_data;

	return RECORDER_ERROR_NONE;
}


int recorder_unset_state_changed_cb(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_STATE_CHANGE] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_STATE_CHANGE] = NULL;

	return RECORDER_ERROR_NONE;
}


int recorder_set_interrupted_cb(recorder_h recorder, recorder_interrupted_cb callback, void *user_data)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_INTERRUPTED] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_INTERRUPTED] = user_data;

	return RECORDER_ERROR_NONE;
}


int recorder_unset_interrupted_cb(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_INTERRUPTED] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_INTERRUPTED] = NULL;

	return RECORDER_ERROR_NONE;
}


int recorder_set_audio_stream_cb(recorder_h recorder, recorder_audio_stream_cb callback, void* user_data)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_audio_stream_callback(handle->mm_handle, __mm_audio_stream_cb, handle);
	if (ret == MM_ERROR_NONE){
		handle->user_cb[_RECORDER_EVENT_TYPE_AUDIO_STREAM] = callback;
		handle->user_data[_RECORDER_EVENT_TYPE_AUDIO_STREAM] = user_data;
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_unset_audio_stream_cb(recorder_h recorder)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_AUDIO_STREAM] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_AUDIO_STREAM] = NULL;

	ret = mm_camcorder_set_audio_stream_callback(handle->mm_handle, NULL, NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_set_error_cb(recorder_h recorder, recorder_error_cb callback, void *user_data)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_ERROR] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_ERROR] = user_data;

	return RECORDER_ERROR_NONE;
}


int recorder_unset_error_cb(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_ERROR] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_ERROR] = NULL;

	return RECORDER_ERROR_NONE;
}


int recorder_set_recording_status_cb(recorder_h recorder, recorder_recording_status_cb callback, void* user_data)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = user_data;

	return RECORDER_ERROR_NONE;
}


int recorder_unset_recording_status_cb(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_STATUS] = NULL;

	return RECORDER_ERROR_NONE;
}


int recorder_set_recording_limit_reached_cb(recorder_h recorder, recorder_recording_limit_reached_cb callback, void* user_data)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL) {
		LOGE("NULL pointer callback");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = callback;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = user_data;

	return RECORDER_ERROR_NONE;
}


int recorder_unset_recording_limit_reached_cb(recorder_h recorder)
{
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	handle->user_cb[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = NULL;
	handle->user_data[_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = NULL;

	return RECORDER_ERROR_NONE;
}


int recorder_foreach_supported_file_format(recorder_h recorder, recorder_supported_file_format_cb foreach_cb, void *user_data)
{
	int i = 0;
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamAttrsInfo info;
	int format;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (foreach_cb == NULL) {
		LOGE("NULL pointer foreach_cb");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_FILE_FORMAT, &info);
	if (ret != MM_ERROR_NONE) {
		LOGE("mm_camcorder_get_attribute_info failed 0x%x", ret);
		return __convert_recorder_error_code(__func__, ret);;
	}

	for (i = 0 ; i < info.int_array.count ; i++) {
		switch (info.int_array.array[i]) {
		case MM_FILE_FORMAT_3GP:
			format = RECORDER_FILE_FORMAT_3GP;
			break;
		case MM_FILE_FORMAT_MP4 :
			format = RECORDER_FILE_FORMAT_MP4;
			break;
		case MM_FILE_FORMAT_AMR :
			format = RECORDER_FILE_FORMAT_AMR;
			break;
		case MM_FILE_FORMAT_AAC:
			format = RECORDER_FILE_FORMAT_ADTS;
			break;
		case MM_FILE_FORMAT_WAV:
			format = RECORDER_FILE_FORMAT_WAV;
			break;
		default :
			format = -1;
			break;
		}

		if (format != -1 && !foreach_cb(format,user_data)) {
			break;
		}
	}

	return RECORDER_ERROR_NONE;
}


int recorder_attr_set_size_limit(recorder_h recorder, int kbyte)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_MAX_SIZE, kbyte,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_set_time_limit(recorder_h recorder, int second)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_TIME_LIMIT, second,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_set_audio_device(recorder_h recorder, recorder_audio_device_e device)
{
	if (device < RECORDER_AUDIO_DEVICE_MIC || device > RECORDER_AUDIO_DEVICE_MODEM) {
		LOGE("invalid device %d", device);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_AUDIO_DEVICE, device);
}


int recorder_set_audio_encoder(recorder_h recorder, recorder_audio_codec_e codec)
{
	recorder_s *handle = (recorder_s *)recorder;
	int audio_table[4] = { MM_AUDIO_CODEC_AMR,      /* RECORDER_AUDIO_CODEC_AMR */
			       MM_AUDIO_CODEC_AAC,      /* RECORDER_AUDIO_CODEC_AAC */
			       MM_AUDIO_CODEC_VORBIS,   /* RECORDER_AUDIO_CODEC_VORBIS */
			       MM_AUDIO_CODEC_WAVE      /* RECORDER_AUDIO_CODEC_PCM */
	};

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (codec != RECORDER_AUDIO_CODEC_DISABLE &&
	    (codec < RECORDER_AUDIO_CODEC_AMR || codec > RECORDER_AUDIO_CODEC_PCM)) {
		LOGE("invalid parameter : codec %d", codec);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (handle->type == _RECORDER_TYPE_AUDIO && codec == RECORDER_AUDIO_CODEC_DISABLE) {
		LOGE("AUDIO_CODEC_DISABLE is not supported in audio mode");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_AUDIO_ENCODER, codec == RECORDER_AUDIO_CODEC_DISABLE ? RECORDER_AUDIO_CODEC_DISABLE : audio_table[codec]);
}


int recorder_get_audio_encoder(recorder_h recorder, recorder_audio_codec_e *codec)
{
	int ret = MM_ERROR_NONE;
	int mm_codec = 0;
	int audio_disable = 0;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (codec == NULL) {
		LOGE("codec is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_ENCODER, &mm_codec,
					  MMCAM_AUDIO_DISABLE, &audio_disable,
					  NULL);

	if (ret == MM_ERROR_NONE && audio_disable == 0) {
		switch (mm_codec) {
		case MM_AUDIO_CODEC_AMR :
			*codec = RECORDER_AUDIO_CODEC_AMR;
			break;
		case MM_AUDIO_CODEC_AAC :
			*codec = RECORDER_AUDIO_CODEC_AAC;
			break;
		case MM_AUDIO_CODEC_VORBIS:
			*codec = RECORDER_AUDIO_CODEC_VORBIS;
			break;
		case MM_AUDIO_CODEC_WAVE:
			*codec = RECORDER_AUDIO_CODEC_PCM;
			break;
		default :
			ret = MM_ERROR_CAMCORDER_INTERNAL;
			break;
		}
	} else if (ret == MM_ERROR_NONE && audio_disable) {
		*codec = RECORDER_AUDIO_CODEC_DISABLE;
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_set_video_encoder(recorder_h recorder, recorder_video_codec_e codec)
{
	int ret = MM_ERROR_NONE;
	int video_table[4] = { MM_VIDEO_CODEC_H263,     /* RECORDER_VIDEO_CODEC_H263 */
			       MM_VIDEO_CODEC_H264,     /* RECORDER_VIDEO_CODEC_H264 */
			       MM_VIDEO_CODEC_MPEG4,    /* RECORDER_VIDEO_CODEC_MPEG4 */
			       MM_VIDEO_CODEC_THEORA    /* RECORDER_VIDEO_CODEC_THEORA */
	};
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}

	if (codec < RECORDER_VIDEO_CODEC_H263 || codec > RECORDER_VIDEO_CODEC_THEORA) {
		LOGE("invalid codec %d", codec);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_ENCODER, video_table[codec],
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_get_video_encoder(recorder_h recorder, recorder_video_codec_e *codec)
{
	int ret = MM_ERROR_NONE;
	int mm_codec = 0;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	if (codec == NULL) {
		LOGE("codec is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_ENCODER, &mm_codec,
					  NULL);
	if (ret == MM_ERROR_NONE) {
		switch(mm_codec) {
		case MM_VIDEO_CODEC_H263 :
			*codec = RECORDER_VIDEO_CODEC_H263;
			break;
		case MM_VIDEO_CODEC_H264 :
			*codec = RECORDER_VIDEO_CODEC_H264;
			break;
		case MM_VIDEO_CODEC_MPEG4 :
			*codec = RECORDER_VIDEO_CODEC_MPEG4;
			break;
		case MM_VIDEO_CODEC_THEORA:
			*codec = RECORDER_VIDEO_CODEC_THEORA;
			break;
		default :
			ret = MM_ERROR_CAMCORDER_INTERNAL;
			break;
		}
	}

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_set_audio_samplerate(recorder_h recorder, int samplerate)
{
	if (samplerate < 1) {
		LOGE("invalid samplerate %d", samplerate);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_AUDIO_SAMPLERATE, samplerate);
}


int recorder_attr_set_audio_encoder_bitrate(recorder_h recorder, int bitrate)
{
	if (bitrate < 1) {
		LOGE("invalid bitrate %d", bitrate);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_AUDIO_ENCODER_BITRATE, bitrate);
}


int recorder_attr_set_video_encoder_bitrate(recorder_h recorder, int bitrate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_ENCODER_BITRATE, bitrate,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_size_limit(recorder_h recorder, int *kbyte)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_MAX_SIZE, kbyte,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_time_limit(recorder_h recorder, int *second)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_TARGET_TIME_LIMIT, second,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_audio_device(recorder_h recorder, recorder_audio_device_e *device)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_DEVICE, device,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_audio_samplerate(recorder_h recorder, int *samplerate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_SAMPLERATE, samplerate,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_audio_encoder_bitrate(recorder_h recorder, int *bitrate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_ENCODER_BITRATE, bitrate,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_video_encoder_bitrate(recorder_h recorder, int *bitrate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_VIDEO_ENCODER_BITRATE, bitrate,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int recorder_foreach_supported_audio_encoder(recorder_h recorder, recorder_supported_audio_encoder_cb foreach_cb, void *user_data)
{
	int i = 0;
	int ret = MM_ERROR_NONE;
	int codec;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamAttrsInfo info;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (foreach_cb == NULL) {
		LOGE("foreach_cb is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_AUDIO_ENCODER, &info);
	if (ret != MM_ERROR_NONE) {
		return __convert_recorder_error_code(__func__, ret);
	}

	for (i = 0 ; i < info.int_array.count ; i++) {
		switch (info.int_array.array[i]) {
		case MM_AUDIO_CODEC_AMR:
			codec = RECORDER_AUDIO_CODEC_AMR;
			break;
		case MM_AUDIO_CODEC_AAC :
			codec = RECORDER_AUDIO_CODEC_AAC;
			break;
		case MM_AUDIO_CODEC_VORBIS:
			codec = RECORDER_AUDIO_CODEC_VORBIS;
			break;
		case MM_AUDIO_CODEC_WAVE:
			codec = RECORDER_AUDIO_CODEC_PCM;
			break;
		default :
			codec = -1;
			break;
		}
		if (codec != -1 && !foreach_cb(codec,user_data)) {
			break;
		}
	}

	return RECORDER_ERROR_NONE;
}


int recorder_foreach_supported_video_encoder(recorder_h recorder, recorder_supported_video_encoder_cb foreach_cb, void *user_data)
{
	int i = 0;
	int ret = MM_ERROR_NONE;
	int codec;
	recorder_s *handle = (recorder_s *)recorder;
	MMCamAttrsInfo info;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	if (foreach_cb == NULL) {
		LOGE("foreach_cb is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attribute_info(handle->mm_handle, MMCAM_VIDEO_ENCODER, &info);
	if (ret != MM_ERROR_NONE) {
		return __convert_recorder_error_code(__func__, ret);
	}

	for (i = 0 ; i < info.int_array.count ; i++) {
		switch (info.int_array.array[i]){
		case MM_VIDEO_CODEC_H263 :
			codec = RECORDER_VIDEO_CODEC_H263;
			break;
		case MM_VIDEO_CODEC_H264 :
			codec = RECORDER_VIDEO_CODEC_H264;
			break;
		case MM_VIDEO_CODEC_MPEG4 :
			codec = RECORDER_VIDEO_CODEC_MPEG4;
			break;
		case MM_VIDEO_CODEC_THEORA :
			codec = RECORDER_VIDEO_CODEC_THEORA;
			break;
		default :
			codec = -1;
			break;
		}

		if (codec != -1 && !foreach_cb(codec,user_data)) {
			break;
		}
	}

	return RECORDER_ERROR_NONE;
}


int recorder_attr_set_mute(recorder_h recorder, bool enable)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_VOLUME, enable ? 0.0 : 1.0,
					  NULL);

	return  __convert_recorder_error_code(__func__, ret);
}


bool recorder_attr_is_muted(recorder_h recorder)
{
	int ret = MM_ERROR_NONE;
	double volume = 1.0;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return false;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_VOLUME, &volume,
					  NULL);

	set_last_result(__convert_recorder_error_code(__func__, ret));

	if (volume == 0.0) {
		return true;
	} else {
		return false;
	}
}


int recorder_attr_set_recording_motion_rate(recorder_h recorder, double rate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_CAMERA_RECORDING_MOTION_RATE, rate,
					  NULL);

	return  __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_get_recording_motion_rate(recorder_h recorder, double *rate)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	if (rate == NULL) {
		LOGE("rate is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_CAMERA_RECORDING_MOTION_RATE, rate,
					  NULL);

	return  __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_set_audio_channel(recorder_h recorder, int channel_count)
{
	if (channel_count < 1) {
		LOGE("invalid channel %d", channel_count);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	return _recorder_check_and_set_attribute(recorder, MMCAM_AUDIO_CHANNEL, channel_count);
}


int recorder_attr_get_audio_channel(recorder_h recorder, int *channel_count)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (channel_count == NULL) {
		LOGE("channel_count is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_AUDIO_CHANNEL, channel_count,
					  NULL);

	return  __convert_recorder_error_code(__func__, ret);
}


int recorder_attr_set_orientation_tag(recorder_h recorder, recorder_rotation_e orientation)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	if (orientation > RECORDER_ROTATION_270) {
		LOGE("invalid orientation %d", orientation);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_set_attributes(handle->mm_handle, NULL,
					  MMCAM_RECORDER_TAG_ENABLE, true,
					  MMCAM_TAG_VIDEO_ORIENTATION, orientation,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}


int  recorder_attr_get_orientation_tag(recorder_h recorder, recorder_rotation_e *orientation)
{
	int ret = MM_ERROR_NONE;
	recorder_s *handle = (recorder_s *)recorder;

	if (handle == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (handle->camera_device_count == 0) {
		LOGE("RECORDER_ERROR_NOT_SUPPORTED");
		return RECORDER_ERROR_NOT_SUPPORTED;
	}
	if (orientation == NULL) {
		LOGE("orientation is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	ret = mm_camcorder_get_attributes(handle->mm_handle, NULL,
					  MMCAM_TAG_VIDEO_ORIENTATION, orientation,
					  NULL);

	return __convert_recorder_error_code(__func__, ret);
}
