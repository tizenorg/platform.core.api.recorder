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
#include <mm_types.h>
#include <recorder.h>
#include <mused_recorder.h>
#include <mused_recorder_msg.h>
#include <mmsvc_core_ipc.h>
#include <recorder_private.h>
#include <glib.h>
#include <mmsvc_core.h>
#include <mmsvc_core_msg_json.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_RECORDER2"

static void _client_user_callback(callback_cb_info_s * cb_info, mmsvc_recorder_event_e event )
{
	char *recvMsg = cb_info->recvMsg;
	LOGD("get event %d", event);

	switch (event) {
		case MMSVC_RECORDER_EVENT_TYPE_STATE_CHANGE:
		{
			int cb_previous, cb_current, cb_by_policy;
			mmsvc_recorder_msg_get(cb_previous, recvMsg);
			mmsvc_recorder_msg_get(cb_current, recvMsg);
			mmsvc_recorder_msg_get(cb_by_policy, recvMsg);
			((recorder_state_changed_cb)cb_info->user_cb[event])((recorder_state_e)cb_previous,
													(recorder_state_e)cb_current,
													(bool)cb_by_policy,
													cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_RECORDING_LIMITED:
		{
			int cb_type;
			mmsvc_recorder_msg_get(cb_type, recvMsg);
			((recorder_recording_limit_reached_cb)cb_info->user_cb[event])((recorder_recording_limit_type_e)cb_type,
															cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_RECORDING_STATUS:
		{
			double cb_elapsed_time;
			double cb_file_size;
			mmsvc_recorder_msg_get(cb_elapsed_time, recvMsg);
			mmsvc_recorder_msg_get(cb_file_size, recvMsg);
			((recorder_recording_status_cb)cb_info->user_cb[event])((unsigned long long)cb_elapsed_time,
															(unsigned long long)cb_file_size,
															cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_INTERRUPTED:
		{
			int cb_policy, cb_previous, cb_current;
			mmsvc_recorder_msg_get(cb_policy, recvMsg);
			mmsvc_recorder_msg_get(cb_previous, recvMsg);
			mmsvc_recorder_msg_get(cb_current, recvMsg);
			((recorder_interrupted_cb)cb_info->user_cb[event])((recorder_policy_e)cb_policy,
														(recorder_state_e)cb_previous,
														(recorder_state_e)cb_current,
														cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_AUDIO_STREAM:
		{
			mmsvc_recorder_transport_info_s transport_info;
			int tKey = 0;
			int cb_size = 0;
			int cb_format;
			int cb_channel;
			int cb_timestamp;
			unsigned char *stream = NULL;
			mmsvc_recorder_msg_get(tKey, recvMsg);

			if (tKey != 0) {
				transport_info.tbm_key = tKey;
				LOGE("Read key_info INFO : %d", transport_info.tbm_key);

				if(mmsvc_recorder_ipc_init_tbm(&transport_info) == FALSE) {
					LOGE("Initialize TBM ERROR!!");
					break;
				}

				if(mmsvc_recorder_ipc_import_tbm(&transport_info) == FALSE) {
					LOGE("Import TBM Key ERROR!!");
					break;
				} else {
					mmsvc_recorder_msg_get(cb_size, recvMsg);
					if (cb_size > 0) {
						stream = (unsigned char *)transport_info.bo_handle.ptr;
					}
				}
			} else {
				LOGE("Get KEY INFO sock msg ERROR!!");
				break;
			}
			mmsvc_recorder_msg_get(cb_format, recvMsg);
			mmsvc_recorder_msg_get(cb_channel, recvMsg);
			mmsvc_recorder_msg_get(cb_timestamp, recvMsg);
			((recorder_audio_stream_cb)cb_info->user_cb[event])((void *)stream,
													cb_size,
													(audio_sample_type_e)cb_format,
													cb_channel,
													(unsigned int)cb_timestamp,
													cb_info->user_data[event]);
			//unref tbm after hand over the buffer.
			mmsvc_recorder_ipc_unref_tbm(&transport_info);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_ERROR:
		{
			int cb_error, cb_current_state;
			mmsvc_recorder_msg_get(cb_error, recvMsg);
			mmsvc_recorder_msg_get(cb_current_state, recvMsg);
			((recorder_error_cb)cb_info->user_cb[event])((recorder_error_e)cb_error,
													(recorder_state_e)cb_current_state,
													cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER:
		{
			int cb_codec;
			mmsvc_recorder_msg_get(cb_codec, recvMsg);
			((recorder_supported_audio_encoder_cb)cb_info->user_cb[event])((recorder_audio_codec_e)cb_codec,
																	cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT:
		{
			int cb_format;
			mmsvc_recorder_msg_get(cb_format, recvMsg);
			((recorder_supported_file_format_cb)cb_info->user_cb[event])((recorder_file_format_e)cb_format,
																	cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER:
		{
			int cb_codec;
			mmsvc_recorder_msg_get(cb_codec, recvMsg);
			((recorder_supported_video_encoder_cb)cb_info->user_cb[event])((recorder_video_codec_e)cb_codec,
																	cb_info->user_data[event]);
			break;
		}
		case MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION:
		{
			int width;
			int height;
			mmsvc_recorder_msg_get(width, recvMsg);
			mmsvc_recorder_msg_get(height, recvMsg);
			((recorder_supported_video_resolution_cb)cb_info->user_cb[event])(width,
																	height,
																	cb_info->user_data[event]);
			break;
		}

		default:
			LOGE("Unknonw event");
			break;
	}
	return;
}

static void *_client_cb_handler(gpointer data)
{
	int ret;
	int api;
	int num_token = 0;
	int i = 0;
	int str_pos = 0;
	int prev_pos = 0;
	callback_cb_info_s *cb_info = data;
	char *recvMsg = cb_info->recvMsg;
	char parseStr[MMSVC_PARSE_STRING_SIZE][MMSVC_MSG_MAX_LENGTH] = {{0,0},};

	while (g_atomic_int_get(&cb_info->running)) {
		ret = mmsvc_core_ipc_recv_msg(cb_info->fd, recvMsg);
		if (ret <= 0)
			break;
		recvMsg[ret] = '\0';
		str_pos = 0;
		prev_pos = 0;
		num_token = 0;
		memset(parseStr, 0, MMSVC_PARSE_STRING_SIZE * MMSVC_MSG_MAX_LENGTH);
		LOGD("recvMSg : %s, length : %d", recvMsg, ret);

		/* Need to split the combined entering msgs.
		    This module supports up to 5 combined msgs. */
		for (str_pos = 0; str_pos < ret; str_pos++) {
			if(recvMsg[str_pos] == '}') {
				strncpy(&(parseStr[num_token][0]), recvMsg + prev_pos, str_pos - prev_pos + 1);
				LOGD("splitted msg : %s, Index : %d", &(parseStr[num_token][0]), num_token);
				prev_pos = str_pos+1;
				num_token++;
			}
		}
		LOGD("num_token : %d", num_token);
		/* Re-construct to the useful single msg. */
		for (i = 0; i < num_token; i++) {

			if (i >= MMSVC_PARSE_STRING_SIZE)
				break;
			if (mmsvc_recorder_msg_get(api, &(parseStr[i][0]))) {
				if(api < MMSVC_RECORDER_API_MAX){
					LOGD("api : %d, wait ends.", api);
					g_mutex_lock(&(cb_info->pMutex[api]));
					/* The api msgs should be distinguished from the event msg. */
					memset(cb_info->recvApiMsg, 0, strlen(cb_info->recvApiMsg));
					strcpy(cb_info->recvApiMsg, &(parseStr[i][0]));
					LOGD("cb_info->recvApiMsg : %s", cb_info->recvApiMsg);
					cb_info->activating[api] = 1;
					g_cond_signal(&(cb_info->pCond[api]));
					g_mutex_unlock(&(cb_info->pMutex[api]));
					if(api == MMSVC_RECORDER_API_DESTROY) {
						g_atomic_int_set(&cb_info->running, 0);
						LOGD("close client cb handler");
					}

				} else if(api == MMSVC_RECORDER_CB_EVENT) {
					int event;
					if (mmsvc_recorder_msg_get(event, &(parseStr[i][0]))) {
						LOGD("go callback : %d", event);
						_client_user_callback(cb_info, event);
					}
				}
			}else{
				LOGD("mmsvc_recorder_msg_get FAIL");
			}
		}
	}
	LOGD("client cb exit");

	return NULL;
}

static callback_cb_info_s *_client_callback_new(gint sockfd)
{
	callback_cb_info_s *cb_info;
	GCond *recorder_cond;
	GMutex *recorder_mutex;
	gint *recorder_activ;
	g_return_val_if_fail(sockfd > 0, NULL);

	cb_info = g_new0(callback_cb_info_s, 1);
	recorder_cond = g_new0(GCond, MMSVC_RECORDER_API_MAX);
	recorder_mutex = g_new0(GMutex, MMSVC_RECORDER_API_MAX);
	recorder_activ = g_new0(gint, MMSVC_RECORDER_API_MAX);

	g_atomic_int_set(&cb_info->running, 1);
	cb_info->fd = sockfd;
	cb_info->pCond = recorder_cond;
	cb_info->pMutex = recorder_mutex;
	cb_info->activating = recorder_activ;
	cb_info->thread =
		g_thread_new("callback_thread", _client_cb_handler,
			     (gpointer) cb_info);

	return cb_info;
}

static int client_wait_for_cb_return(mmsvc_recorder_api_e api, callback_cb_info_s *cb_info, int time_out)
{
	int ret = RECORDER_ERROR_NONE;
	gint64 end_time;

	LOGD("Enter api : %d", api);
	g_mutex_lock(&(cb_info->pMutex[api]));

	end_time = g_get_monotonic_time() + time_out * G_TIME_SPAN_SECOND;
	if (cb_info->activating[api] != 1) {
		if (g_cond_wait_until(&(cb_info->pCond[api]), &(cb_info->pMutex[api]), end_time)) {
			LOGD("cb_info->recvApiMsg : %s", cb_info->recvApiMsg);
			if (!mmsvc_recorder_msg_get(ret, cb_info->recvApiMsg)) {
				ret = RECORDER_ERROR_INVALID_OPERATION;
			} else {
				LOGD("API %d passed successfully", api);
			}
		} else {
			ret = RECORDER_ERROR_INVALID_OPERATION;
		}
	} else {
		LOGD("condition is already checked for the api : %d.", api);
		cb_info->activating[api] = 0;
		if (!mmsvc_recorder_msg_get(ret, cb_info->recvApiMsg)) {
			ret = RECORDER_ERROR_INVALID_OPERATION;
		} else {
			LOGD("Already checked condition, Wait passed, ret : 0x%x", ret);
		}
	}
	g_mutex_unlock(&(cb_info->pMutex[api]));
	LOGD("ret of api %d : 0x%x", api, ret);
	return ret;
}

static void _client_callback_destroy(callback_cb_info_s * cb_info)
{
	g_return_if_fail(cb_info != NULL);

	LOGI("%p Callback destroyed", cb_info->thread);
	g_thread_join(cb_info->thread);
	g_thread_unref(cb_info->thread);


	if (cb_info->pCond) {
		g_free(cb_info->pCond);
	}
	if (cb_info->pMutex) {
		g_free(cb_info->pMutex);
	}
	g_free(cb_info);
}

int recorder_create_videorecorder(camera_h camera, recorder_h *recorder)
{
	if (camera == NULL) {
		LOGE("NULL pointer camera handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (recorder == NULL) {
		LOGE("NULL pointer recorder handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int sock_fd = -1;
	char *sndMsg;
	int ret = RECORDER_ERROR_NONE;
	camera_cli_s *camera_pc = (camera_cli_s *)camera;
	recorder_cli_s *pc;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_CREATE;
	mmsvc_api_client_e api_client = MMSVC_RECORDER;
	intptr_t camera_handle = (intptr_t)camera_pc->remote_handle;
	intptr_t handle;
	int recorder_type = MMSVC_RECORDER_TYPE_VIDEO;

	pc = g_new0(recorder_cli_s, 1);
	if (pc == NULL) {
		return 0;
	}

	LOGD("remote handle : 0x%x", camera_handle);
	sock_fd = mmsvc_core_client_new();
	sndMsg = mmsvc_core_msg_json_factory_new(api, "client", api_client,
											      MUSED_TYPE_INT, PARAM_RECORDER_TYPE, recorder_type,
											      MUSED_TYPE_POINTER, PARAM_CAMERA_HANDLE, camera_handle,
											      0);
	mmsvc_core_ipc_send_msg(sock_fd, sndMsg);
	LOGD("sock_fd : %d, msg : %s", sock_fd, sndMsg);
	mmsvc_core_msg_json_factory_free(sndMsg);

	pc->cb_info = _client_callback_new(sock_fd);
	LOGD("cb info : %d", pc->cb_info->fd);

	ret = client_wait_for_cb_return(api, pc->cb_info, CALLBACK_TIME_OUT);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get_pointer(handle, pc->cb_info->recvMsg);
		if (handle == 0) {
			LOGE("Receiving Handle Failed!!");
			goto ErrorExit;
		} else {
			pc->remote_handle = handle;
		}
		LOGD("recorder create 0x%x", pc->remote_handle);
		*recorder = (recorder_h) pc;
	} else
		goto ErrorExit;
	LOGD("ret : 0x%x", ret);
	return ret;

ErrorExit:
	g_free(pc);
	LOGD("ErrorExit!!! ret value : %d", ret);
	return ret;
}


int recorder_create_audiorecorder(recorder_h *recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int sock_fd = -1;
	char *sndMsg;
	int ret = RECORDER_ERROR_NONE;
	recorder_cli_s *pc = NULL;

	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_CREATE;
	mmsvc_api_client_e api_client = MMSVC_RECORDER;
	int recorder_type = MMSVC_RECORDER_TYPE_AUDIO;

	sock_fd = mmsvc_core_client_new();
	sndMsg = mmsvc_core_msg_json_factory_new(api, "client", api_client,
											      MUSED_TYPE_INT, PARAM_RECORDER_TYPE, recorder_type,
											      0);
	mmsvc_core_ipc_send_msg(sock_fd, sndMsg);
	LOGD("sock_fd : %d, msg : %s", sock_fd, sndMsg);
	mmsvc_core_msg_json_factory_free(sndMsg);

	pc = g_new0(recorder_cli_s, 1);
	if (pc == NULL) {
		return 0;
	}

	pc->cb_info = _client_callback_new(sock_fd);
	LOGD("cb info : %d", pc->cb_info->fd);

	ret = client_wait_for_cb_return(api, pc->cb_info, CALLBACK_TIME_OUT);
	if (ret == RECORDER_ERROR_NONE) {
		intptr_t handle = 0;
		mmsvc_recorder_msg_get_pointer(handle, pc->cb_info->recvMsg);
		if (handle == 0) {
			LOGE("Receiving Handle Failed!!");
			goto ErrorExit;
		} else {
			pc->remote_handle = handle;
		}
		LOGD("recorder create 0x%x", pc->remote_handle);
		*recorder = (recorder_h)pc;
	} else
		goto ErrorExit;
	LOGD("ret : 0x%x", ret);
	return ret;

ErrorExit:
	g_free(pc);
	LOGD("ret value : %d", ret);
	return ret;
}


int recorder_get_state(recorder_h recorder, recorder_state_e *state)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (state == NULL) {
		LOGE("NULL pointer state");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_STATE;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_state;

	LOGD("Enter, remote_handle : %x", pc->remote_handle);
	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_state, pc->cb_info->recvMsg);
		*state = (recorder_state_e)get_state;
	}
	LOGD("ret : 0x%x, get_state : %d", ret, get_state);
	return ret;
}


int recorder_destroy(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_DESTROY;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	if (pc == NULL) {
		return RECORDER_ERROR_INVALID_OPERATION;
	} else if (pc->cb_info == NULL) {
		return RECORDER_ERROR_INVALID_OPERATION;
	}

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	_client_callback_destroy(pc->cb_info);
	g_free(pc);
	pc = NULL;
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_prepare(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_PREPARE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);

	LOGD("ret : 0x%x", ret);

	return ret;
}


int recorder_unprepare(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNPREPARE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_start(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_START;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_pause(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_PAUSE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_commit(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_COMMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_cancel(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_CANCEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_video_resolution(recorder_h recorder, int width, int height)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_VIDEO_RESOLUTION;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send2(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, width,
								INT, height);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_video_resolution(recorder_h recorder, int *width, int *height)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (!width || !height) {
		LOGE("NULL pointer width = [%p], height = [%p]", width, height);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_VIDEO_RESOLUTION;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_width;
	int get_height;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_width, pc->cb_info->recvMsg);
		mmsvc_recorder_msg_get(get_height, pc->cb_info->recvMsg);
		*width = get_width;
		*height = get_height;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_video_resolution(recorder_h recorder,
                                                recorder_supported_video_resolution_cb foreach_cb, void *user_data)
{
	if( recorder == NULL || foreach_cb == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_FOREACH_SUPPORTED_VIDEO_RESOLUTION;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION] = foreach_cb;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_audio_level(recorder_h recorder, double *level)
{
	if (recorder == NULL || level == NULL) {
		LOGE("NULL pointer %p %p", recorder, level);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_AUDIO_LEVEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_level;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_level, pc->cb_info->recvMsg);
		*level = get_level;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_filename(recorder_h recorder,  const char *filename)
{
	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (filename == NULL) {
		LOGE("filename is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_FILENAME;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle, sock_fd, pc->cb_info, ret, STRING, filename);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_filename(recorder_h recorder,  char **filename)
{
	if (recorder == NULL) {
		LOGE("handle is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (filename == NULL) {
		LOGE("filename is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_FILENAME;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	char get_filename[MMSVC_RECORDER_MSG_MAX_LENGTH] = {0,};

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get_string(get_filename, pc->cb_info->recvMsg);
		*filename = strdup(get_filename);
	}
	LOGD("ret : 0x%x, filename : %s", ret, *filename);
	return ret;
}


int recorder_set_file_format(recorder_h recorder, recorder_file_format_e format)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (format < RECORDER_FILE_FORMAT_3GP || format > RECORDER_FILE_FORMAT_OGG) {
		LOGE("invalid format %d", format);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_FILE_FORMAT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_format = (int)format;

	LOGD("ENTER, set_format : %d", set_format);

	mmsvc_recorder_msg_send1(api, pc->remote_handle, sock_fd, pc->cb_info, ret, INT, set_format);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_file_format(recorder_h recorder, recorder_file_format_e *format)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (format == NULL) {
		LOGE("NULL pointer data");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_FILE_FORMAT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_format;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_format, pc->cb_info->recvMsg);
		LOGD("get_fileformat : %d", get_format);
		*format = (recorder_file_format_e)get_format;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void* user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_STATE_CHANGED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_STATE_CHANGE] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_STATE_CHANGE] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_state_changed_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_STATE_CHANGED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_interrupted_cb(recorder_h recorder, recorder_interrupted_cb callback, void *user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_INTERRUPTED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_INTERRUPTED] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_INTERRUPTED] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_interrupted_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_INTERRUPTED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_audio_stream_cb(recorder_h recorder, recorder_audio_stream_cb callback, void* user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_AUDIO_STREAM_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_AUDIO_STREAM] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_AUDIO_STREAM] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_audio_stream_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_AUDIO_STREAM_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_error_cb(recorder_h recorder, recorder_error_cb callback, void *user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_ERROR_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_ERROR] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_ERROR] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_error_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_ERROR_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_recording_status_cb(recorder_h recorder, recorder_recording_status_cb callback, void* user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_RECORDING_STATUS_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_RECORDING_STATUS] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_RECORDING_STATUS] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_recording_status_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_RECORDING_STATUS_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_recording_limit_reached_cb(recorder_h recorder, recorder_recording_limit_reached_cb callback, void* user_data)
{
	if( recorder == NULL || callback == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_RECORDING_LIMIT_REACHED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = callback;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_unset_recording_limit_reached_cb(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_UNSET_RECORDING_LIMIT_REACHED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_file_format(recorder_h recorder, recorder_supported_file_format_cb foreach_cb, void *user_data)
{
	if( recorder == NULL || foreach_cb == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_FOREACH_SUPPORTED_FILE_FORMAT;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT] = foreach_cb;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_size_limit(recorder_h recorder, int kbyte)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_SIZE_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, kbyte);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_time_limit(recorder_h recorder, int second)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_TIME_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, second);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_audio_device(recorder_h recorder, recorder_audio_device_e device)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_AUDIO_DEVICE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_device = (int)device;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, set_device);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_audio_encoder(recorder_h recorder, recorder_audio_codec_e codec)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (codec != RECORDER_AUDIO_CODEC_DISABLE &&
	    (codec < RECORDER_AUDIO_CODEC_AMR || codec > RECORDER_AUDIO_CODEC_PCM)) {
		LOGE("invalid parameter : codec %d", codec);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_AUDIO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_codec = (int)codec;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, set_codec);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_audio_encoder(recorder_h recorder, recorder_audio_codec_e *codec)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (codec == NULL) {
		LOGE("codec is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_AUDIO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_codec, pc->cb_info->recvMsg);
		*codec = (recorder_audio_codec_e)get_codec;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_video_encoder(recorder_h recorder, recorder_video_codec_e codec)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (codec < RECORDER_VIDEO_CODEC_H263 || codec > RECORDER_VIDEO_CODEC_THEORA) {
		LOGE("invalid codec %d", codec);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_SET_VIDEO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_codec = (int)codec;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, set_codec);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_get_video_encoder(recorder_h recorder, recorder_video_codec_e *codec)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (codec == NULL) {
		LOGE("codec is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_GET_VIDEO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_codec, pc->cb_info->recvMsg);
		*codec = (recorder_audio_codec_e)get_codec;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_audio_samplerate(recorder_h recorder, int samplerate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (samplerate < 1) {
		LOGE("invalid samplerate %d", samplerate);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_AUDIO_SAMPLERATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER, samplerate : %d", samplerate);

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, samplerate);
	LOGD("ret : 0x%x, samplerate : %d", ret, samplerate);
	return ret;
}


int recorder_attr_set_audio_encoder_bitrate(recorder_h recorder, int bitrate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (bitrate < 1) {
		LOGE("invalid bitrate %d", bitrate);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_AUDIO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, bitrate);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_video_encoder_bitrate(recorder_h recorder, int bitrate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_VIDEO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, bitrate);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_size_limit(recorder_h recorder, int *kbyte)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (kbyte == NULL) {
		LOGE("NULL pointer kbyte");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_SIZE_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_kbyte;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_kbyte, pc->cb_info->recvMsg);
		*kbyte = get_kbyte;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_time_limit(recorder_h recorder, int *second)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (second == NULL) {
		LOGE("NULL pointer second");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_TIME_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_second;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_second, pc->cb_info->recvMsg);
		*second = get_second;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_audio_device(recorder_h recorder, recorder_audio_device_e *device)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (device == NULL) {
		LOGE("NULL pointer device");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_AUDIO_DEVICE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_device;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_device, pc->cb_info->recvMsg);
		*device = (recorder_audio_device_e)get_device;
	}

	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_audio_samplerate(recorder_h recorder, int *samplerate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (samplerate == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_AUDIO_SAMPLERATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_samplerate;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_samplerate, pc->cb_info->recvMsg);
		*samplerate = get_samplerate;
	}
	LOGD("ret : 0x%x, get_samplerate : %d", ret, get_samplerate);
	return ret;
}


int recorder_attr_get_audio_encoder_bitrate(recorder_h recorder, int *bitrate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (bitrate == NULL) {
		LOGE("NULL pointer");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_AUDIO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_bitrate;
	pc->cb_info->activating[api] = 0;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_bitrate, pc->cb_info->recvMsg);
		*bitrate = get_bitrate;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_video_encoder_bitrate(recorder_h recorder, int *bitrate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (bitrate == NULL) {
		LOGE("NULL pointer");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_VIDEO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_bitrate;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_bitrate, pc->cb_info->recvMsg);
		*bitrate = get_bitrate;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_audio_encoder(recorder_h recorder, recorder_supported_audio_encoder_cb foreach_cb, void *user_data)
{
	if( recorder == NULL || foreach_cb == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_FOREACH_SUPPORTED_AUDIO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER] = foreach_cb;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_video_encoder(recorder_h recorder, recorder_supported_video_encoder_cb foreach_cb, void *user_data)
{
	if( recorder == NULL || foreach_cb == NULL){
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_FOREACH_SUPPORTED_VIDEO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER] = foreach_cb;
	pc->cb_info->user_data[MMSVC_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER] = user_data;

	mmsvc_recorder_msg_send(api, pc->remote_handle, sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_mute(recorder_h recorder, bool enable)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_MUTE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_enable = (int)enable;
	pc->cb_info->activating[api] = 0;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, set_enable);
	LOGD("ret : 0x%x", ret);
	return ret;
}


bool recorder_attr_is_muted(recorder_h recorder)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return false;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_IS_MUTED;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_recording_motion_rate(recorder_h recorder, double rate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_RECORDING_MOTION_RATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								DOUBLE, rate);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_recording_motion_rate(recorder_h recorder, double *rate)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (rate == NULL) {
		LOGE("rate is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_RECORDING_MOTION_RATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_rate;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_rate, pc->cb_info->recvMsg);
		*rate = get_rate;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_audio_channel(recorder_h recorder, int channel_count)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (channel_count < 1) {
		LOGE("invalid channel %d", channel_count);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_AUDIO_CHANNEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, channel_count);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_get_audio_channel(recorder_h recorder, int *channel_count)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (channel_count == NULL) {
		LOGE("channel_count is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_AUDIO_CHANNEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_channel_count;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_channel_count, pc->cb_info->recvMsg);
		*channel_count = get_channel_count;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_attr_set_orientation_tag(recorder_h recorder, recorder_rotation_e orientation)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (orientation > RECORDER_ROTATION_270) {
		LOGE("invalid orientation %d", orientation);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_SET_ORIENTATION_TAG;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_orientation = (int)orientation;

	LOGD("ENTER");

	mmsvc_recorder_msg_send1(api, pc->remote_handle,
								sock_fd,
								pc->cb_info,
								ret,
								INT, set_orientation);
	LOGD("ret : 0x%x", ret);
	return ret;
}


int  recorder_attr_get_orientation_tag(recorder_h recorder, recorder_rotation_e *orientation)
{
	if (recorder == NULL) {
		LOGE("NULL pointer handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	if (orientation == NULL) {
		LOGE("orientation is NULL");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;
	mmsvc_recorder_api_e api = MMSVC_RECORDER_API_ATTR_GET_ORIENTATION_TAG;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_orientation;

	LOGD("ENTER");

	mmsvc_recorder_msg_send(api, pc->remote_handle,	sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		mmsvc_recorder_msg_get(get_orientation, pc->cb_info->recvMsg);
		*orientation = (recorder_rotation_e)get_orientation;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}
