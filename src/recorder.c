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
#include <muse_recorder.h>
#include <muse_recorder_msg.h>
#include <muse_core_ipc.h>
#include <recorder_private.h>
#include <glib.h>
#include <muse_core.h>
#include <muse_core_msg_json.h>
#include <mm_camcorder_client.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_RECORDER"


static int _recorder_import_tbm_key(tbm_bufmgr bufmgr, unsigned int tbm_key, tbm_bo *bo, tbm_bo_handle *bo_handle)
{
	tbm_bo tmp_bo = NULL;
	tbm_bo_handle tmp_bo_handle = {NULL, };

	if (bufmgr == NULL || bo == NULL || bo_handle == NULL || tbm_key == 0) {
		LOGE("invalid parameter - bufmgr %p, bo %p, bo_handle %p, key %d",
		     bufmgr, bo, bo_handle, tbm_key);
		return false;
	}

	tmp_bo = tbm_bo_import(bufmgr, tbm_key);
	if (tmp_bo == NULL) {
		LOGE("bo import failed - bufmgr %p, key %d", bufmgr, tbm_key);
		return false;
	}

	tmp_bo_handle = tbm_bo_map(tmp_bo, TBM_DEVICE_CPU, TBM_OPTION_READ);
	if (tmp_bo_handle.ptr == NULL) {
		LOGE("bo map failed %p", tmp_bo);
		tbm_bo_unref(tmp_bo);
		tmp_bo = NULL;
		return false;
	}

	/* set bo and bo_handle */
	*bo = tmp_bo;
	*bo_handle = tmp_bo_handle;

	return true;
}

static void _recorder_release_imported_bo(tbm_bo *bo)
{
	if (bo == NULL || *bo == NULL) {
		LOGW("NULL bo");
		return;
	}

	tbm_bo_unmap(*bo);
	tbm_bo_unref(*bo);
	*bo = NULL;

	return;
}

static void _client_user_callback(callback_cb_info_s * cb_info, muse_recorder_event_e event)
{
	char *recvMsg = cb_info->recvMsg;
	LOGD("get event %d", event);

	switch (event) {
		case MUSE_RECORDER_EVENT_TYPE_STATE_CHANGE:
		{
			int previous = 0;
			int current = 0;
			int by_policy = 0;

			muse_recorder_msg_get(previous, recvMsg);
			muse_recorder_msg_get(current, recvMsg);
			muse_recorder_msg_get(by_policy, recvMsg);

			((recorder_state_changed_cb)cb_info->user_cb[event])((recorder_state_e)previous,
			                                                     (recorder_state_e)current,
			                                                     (bool)by_policy,
			                                                     cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_RECORDING_LIMITED:
		{
			int type = 0;

			muse_recorder_msg_get(type, recvMsg);

			((recorder_recording_limit_reached_cb)cb_info->user_cb[event])((recorder_recording_limit_type_e)type,
			                                                               cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_RECORDING_STATUS:
		{
			int64_t cb_elapsed_time = 0;
			int64_t cb_file_size = 0;

			muse_recorder_msg_get(cb_elapsed_time, recvMsg);
			muse_recorder_msg_get(cb_file_size, recvMsg);

			((recorder_recording_status_cb)cb_info->user_cb[event])((unsigned long long)cb_elapsed_time,
			                                                        (unsigned long long)cb_file_size,
			                                                        cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_INTERRUPTED:
		{
			int policy = 0;
			int previous = 0;
			int current = 0;

			muse_recorder_msg_get(policy, recvMsg);
			muse_recorder_msg_get(previous, recvMsg);
			muse_recorder_msg_get(current, recvMsg);

			((recorder_interrupted_cb)cb_info->user_cb[event])((recorder_policy_e)policy,
			                                                   (recorder_state_e)previous,
			                                                   (recorder_state_e)current,
			                                                   cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_AUDIO_STREAM:
		{
			int tbm_key = 0;
			int size = 0;
			int format = 0;
			int channel = 0;
			int timestamp = 0;
			tbm_bo bo = NULL;
			tbm_bo_handle bo_handle = {.ptr = NULL};

			muse_recorder_msg_get(tbm_key, recvMsg);
			if (tbm_key == 0) {
				LOGE("invalid key");
				break;
			}

			if (!_recorder_import_tbm_key(cb_info->bufmgr, tbm_key, &bo, &bo_handle)) {
				LOGE("tbm key %d import failed", tbm_key);
				break;
			}

			muse_recorder_msg_get(size, recvMsg);
			muse_recorder_msg_get(format, recvMsg);
			muse_recorder_msg_get(channel, recvMsg);
			muse_recorder_msg_get(timestamp, recvMsg);

			((recorder_audio_stream_cb)cb_info->user_cb[event])((void *)bo_handle.ptr,
			                                                    size,
			                                                    (audio_sample_type_e)format,
			                                                    channel,
			                                                    (unsigned int)timestamp,
			                                                    cb_info->user_data[event]);

			/* release imported bo */
			_recorder_release_imported_bo(&bo);

			/* return buffer */
			muse_recorder_msg_send1_no_return(MUSE_RECORDER_API_RETURN_BUFFER,
			                                  cb_info->fd, cb_info,
			                                  INT, tbm_key);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_ERROR:
		{
			int error = 0;
			int current_state = 0;

			muse_recorder_msg_get(error, recvMsg);
			muse_recorder_msg_get(current_state, recvMsg);

			((recorder_error_cb)cb_info->user_cb[event])((recorder_error_e)error,
			                                             (recorder_state_e)current_state,
			                                             cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER:
		{
			int codec = 0;

			muse_recorder_msg_get(codec, recvMsg);

			((recorder_supported_audio_encoder_cb)cb_info->user_cb[event])((recorder_audio_codec_e)codec,
			                                                               cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT:
		{
			int format = 0;

			muse_recorder_msg_get(format, recvMsg);

			((recorder_supported_file_format_cb)cb_info->user_cb[event])((recorder_file_format_e)format,
			                                                             cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER:
		{
			int codec = 0;

			muse_recorder_msg_get(codec, recvMsg);

			((recorder_supported_video_encoder_cb)cb_info->user_cb[event])((recorder_video_codec_e)codec,
			                                                               cb_info->user_data[event]);
			break;
		}
		case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION:
		{
			int width = 0;
			int height = 0;

			muse_recorder_msg_get(width, recvMsg);
			muse_recorder_msg_get(height, recvMsg);

			((recorder_supported_video_resolution_cb)cb_info->user_cb[event])(width, height,
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
	int ret = 0;
	int api = 0;
	int num_token = 0;
	int i = 0;
	int str_pos = 0;
	int prev_pos = 0;
	callback_cb_info_s *cb_info = data;
	char *recvMsg = NULL;
	char **parseStr = NULL;

	if (cb_info == NULL) {
		LOGE("cb_info NULL");
		return NULL;
	}

	parseStr = (char **)malloc(sizeof(char *) * RECORDER_PARSE_STRING_SIZE);
	if (parseStr == NULL) {
		LOGE("parseStr malloc failed");
		return NULL;
	}

	for (i = 0 ; i < RECORDER_PARSE_STRING_SIZE ; i++) {
		parseStr[i] = (char *)malloc(sizeof(char) * MUSE_RECORDER_MSG_MAX_LENGTH);
		if (parseStr[i] == NULL) {
			LOGE("parseStr[%d] malloc failed", i);
			goto CB_HANDLER_EXIT;
		}
	}

	recvMsg = cb_info->recvMsg;

	while (g_atomic_int_get(&cb_info->running)) {
		ret = muse_core_ipc_recv_msg(cb_info->fd, recvMsg);
		if (ret <= 0)
			break;
		recvMsg[ret] = '\0';

		str_pos = 0;
		prev_pos = 0;
		num_token = 0;

		LOGD("recvMSg : %s, length : %d", recvMsg, ret);

		/* Need to split the combined entering msgs.
		    This module supports up to 5 combined msgs. */
		for (str_pos = 0; str_pos < ret; str_pos++) {
			if(recvMsg[str_pos] == '}') {
				memset(parseStr[num_token], 0x0, sizeof(char) * MUSE_RECORDER_MSG_MAX_LENGTH);
				strncpy(&(parseStr[num_token][0]), recvMsg + prev_pos, str_pos - prev_pos + 1);
				LOGD("splitted msg : %s, Index : %d", &(parseStr[num_token][0]), num_token);
				prev_pos = str_pos+1;
				num_token++;
			}
		}

		LOGD("num_token : %d", num_token);

		/* Re-construct to the useful single msg. */
		for (i = 0 ; i < num_token ; i++) {
			if (i >= RECORDER_PARSE_STRING_SIZE) {
				LOGE("invalid index %d", i);
				break;
			}

			if (muse_recorder_msg_get(api, &(parseStr[i][0]))) {
				if(api < MUSE_RECORDER_API_MAX){
					LOGD("api : %d, wait ends.", api);

					g_mutex_lock(&(cb_info->pMutex[api]));

					/* The api msgs should be distinguished from the event msg. */
					memset(cb_info->recvApiMsg, 0, strlen(cb_info->recvApiMsg));
					strcpy(cb_info->recvApiMsg, &(parseStr[i][0]));

					LOGD("cb_info->recvApiMsg : %s", cb_info->recvApiMsg);

					cb_info->activating[api] = 1;

					if (api == MUSE_RECORDER_API_CREATE) {
						if (muse_recorder_msg_get(ret, cb_info->recvApiMsg)) {
							if (ret != RECORDER_ERROR_NONE) {
								g_atomic_int_set(&cb_info->running, 0);
								LOGE("recorder create error 0x%x. close client cb handler", ret);
							}
						} else {
							LOGE("failed to get api return");
						}
					} else if (api == MUSE_RECORDER_API_DESTROY) {
						if (muse_recorder_msg_get(ret, cb_info->recvApiMsg)) {
							if (ret == RECORDER_ERROR_NONE) {
								g_atomic_int_set(&cb_info->running, 0);
								LOGD("recorder destroy done. close client cb handler");
							}
						} else {
							LOGE("failed to get api return");
						}
					}

					g_cond_signal(&(cb_info->pCond[api]));
					g_mutex_unlock(&(cb_info->pMutex[api]));
				} else if (api == MUSE_RECORDER_CB_EVENT) {
					int event = -1;

					if (muse_recorder_msg_get(event, &(parseStr[i][0]))) {
						LOGD("go callback : %d", event);
						_client_user_callback(cb_info, event);
					}
				}
			}else{
				LOGD("muse_recorder_msg_get FAIL");
			}
		}
	}

	LOGD("client cb exit");

CB_HANDLER_EXIT:
	if (parseStr) {
		for (i = 0 ; i < RECORDER_PARSE_STRING_SIZE ; i++) {
			if (parseStr[i]) {
				free(parseStr[i]);
				parseStr[i] = NULL;
			}
		}

		free(parseStr);
		parseStr = NULL;
	}

	return NULL;
}

static callback_cb_info_s *_client_callback_new(gint sockfd)
{
	callback_cb_info_s *cb_info = NULL;
	GCond *recorder_cond = NULL;
	GMutex *recorder_mutex = NULL;
	gint *recorder_activ = NULL;
	g_return_val_if_fail(sockfd > 0, NULL);

	cb_info = g_new0(callback_cb_info_s, 1);
	if (cb_info == NULL) {
		LOGE("cb_info_s alloc failed");
		goto _ERR_RECORDER_EXIT;
	}

	recorder_cond = g_new0(GCond, MUSE_RECORDER_API_MAX);
	if (recorder_cond == NULL) {
		LOGE("recorder_cond alloc failed");
		goto _ERR_RECORDER_EXIT;
	}
	recorder_mutex = g_new0(GMutex, MUSE_RECORDER_API_MAX);
	if (recorder_mutex == NULL) {
		LOGE("recorder_mutex alloc failed");
		goto _ERR_RECORDER_EXIT;
	}
	recorder_activ = g_new0(gint, MUSE_RECORDER_API_MAX);
	if (recorder_activ == NULL) {
		LOGE("recorder_activ alloc failed");
		goto _ERR_RECORDER_EXIT;
	}

	g_atomic_int_set(&cb_info->running, 1);
	cb_info->fd = sockfd;
	cb_info->pCond = recorder_cond;
	cb_info->pMutex = recorder_mutex;
	cb_info->activating = recorder_activ;
	cb_info->thread = g_thread_try_new("callback_thread",
	                                   _client_cb_handler,
	                                   (gpointer)cb_info,
	                                   NULL);
	if (cb_info->thread == NULL) {
		LOGE("thread create failed");
		goto _ERR_RECORDER_EXIT;
	}

	return cb_info;

_ERR_RECORDER_EXIT:
	if (cb_info) {
		g_free(cb_info);
		cb_info = NULL;
	}
	if (recorder_cond) {
		g_free(recorder_cond);
		recorder_cond = NULL;
	}
	if (recorder_mutex) {
		g_free(recorder_mutex);
		recorder_mutex = NULL;
	}
	if (recorder_activ) {
		g_free(recorder_activ);
		recorder_activ = NULL;
	}

	return NULL;
}

static int client_wait_for_cb_return(muse_recorder_api_e api, callback_cb_info_s *cb_info, int time_out)
{
	int ret = RECORDER_ERROR_NONE;
	gint64 end_time;

	LOGD("Enter api : %d", api);
	g_mutex_lock(&(cb_info->pMutex[api]));

	end_time = g_get_monotonic_time() + time_out * G_TIME_SPAN_SECOND;
	if (cb_info->activating[api] != 1) {
		if (g_cond_wait_until(&(cb_info->pCond[api]), &(cb_info->pMutex[api]), end_time)) {
			LOGD("cb_info->recvApiMsg : %s", cb_info->recvApiMsg);
			if (!muse_recorder_msg_get(ret, cb_info->recvApiMsg)) {
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
		if (!muse_recorder_msg_get(ret, cb_info->recvApiMsg)) {
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

	LOGD("destroy thread %p", cb_info->thread);

	g_thread_join(cb_info->thread);
	g_thread_unref(cb_info->thread);

	if (cb_info->pCond) {
		g_free(cb_info->pCond);
		cb_info->pCond = NULL;
	}
	if (cb_info->pMutex) {
		g_free(cb_info->pMutex);
		cb_info->pMutex = NULL;
	}

	g_free(cb_info);
	cb_info = NULL;

	return;
}

static int _recorder_create_common(recorder_h *recorder, muse_recorder_type_e type, camera_h camera)
{
	int ret = RECORDER_ERROR_NONE;
	int destroy_ret = RECORDER_ERROR_NONE;
	int sock_fd = -1;
	char *send_msg = NULL;
	char *root_directory = NULL;
	intptr_t camera_handle = 0;
	intptr_t handle = 0;
	tbm_bufmgr bufmgr = NULL;
	recorder_cli_s *pc = NULL;

	LOGD("Enter - type %d", type);

	if (recorder == NULL) {
		LOGE("NULL pointer for recorder handle");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	if (type == MUSE_RECORDER_TYPE_VIDEO && camera == NULL) {
		LOGE("NULL pointer for camera handle on video recorder mode");
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	bufmgr = tbm_bufmgr_init(-1);
	if (bufmgr == NULL) {
		LOGE("get tbm bufmgr failed");
		return RECORDER_ERROR_INVALID_OPERATION;
	}

	pc = g_new0(recorder_cli_s, 1);
	if (pc == NULL) {
		ret = RECORDER_ERROR_OUT_OF_MEMORY;
		goto _ERR_RECORDER_EXIT;
	}

	sock_fd = muse_core_client_new();
	if (sock_fd < 0) {
		LOGE("muse_core_client_new failed - returned fd %d", sock_fd);
		ret = RECORDER_ERROR_INVALID_OPERATION;
		goto _ERR_RECORDER_EXIT;
	}

	if (type == MUSE_RECORDER_TYPE_AUDIO) {
		send_msg = muse_core_msg_json_factory_new(MUSE_RECORDER_API_CREATE,
		                                          MUSE_TYPE_INT, "module", MUSE_RECORDER,
		                                          MUSE_TYPE_INT, PARAM_RECORDER_TYPE, MUSE_RECORDER_TYPE_AUDIO,
		                                          MUSE_TYPE_INT, "pid", getpid(),
		                                          0);
	} else {
		camera_handle = (intptr_t)((camera_cli_s *)camera)->remote_handle;
		send_msg = muse_core_msg_json_factory_new(MUSE_RECORDER_API_CREATE,
		                                          MUSE_TYPE_INT, "module", MUSE_RECORDER,
		                                          MUSE_TYPE_INT, PARAM_RECORDER_TYPE, MUSE_RECORDER_TYPE_VIDEO,
		                                          MUSE_TYPE_POINTER, "camera_handle", camera_handle,
		                                          0);
	}

	LOGD("sock_fd : %d, msg : %s", sock_fd, send_msg);

	muse_core_ipc_send_msg(sock_fd, send_msg);
	muse_core_msg_json_factory_free(send_msg);

	pc->cb_info = _client_callback_new(sock_fd);
	if (pc->cb_info == NULL) {
		ret = RECORDER_ERROR_OUT_OF_MEMORY;
		goto _ERR_RECORDER_EXIT;
	}

	ret = client_wait_for_cb_return(MUSE_RECORDER_API_CREATE, pc->cb_info, CALLBACK_TIME_OUT);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_pointer(handle, pc->cb_info->recvMsg);
		if (handle == 0) {
			LOGE("Receiving Handle Failed!!");
			goto _ERR_RECORDER_EXIT;
		}

		if (mm_camcorder_client_get_root_directory(&root_directory) != MM_ERROR_NONE ||
		    root_directory == NULL) {
			LOGE("failed to get root directory of internal storage");
			ret = RECORDER_ERROR_INVALID_OPERATION;
			goto _ERR_RECORDER_AFTER_CREATE;
		}

		LOGD("set root directory [%s]", root_directory);

		muse_recorder_msg_send1(MUSE_RECORDER_API_ATTR_SET_ROOT_DIRECTORY,
		                        sock_fd, pc->cb_info, ret, STRING, root_directory);
		if (ret != RECORDER_ERROR_NONE) {
			LOGE("failed to set root directory %s", root_directory);
			ret = RECORDER_ERROR_INVALID_OPERATION;
			goto _ERR_RECORDER_AFTER_CREATE;
		}

		free(root_directory);
		root_directory = NULL;

		pc->remote_handle = handle;
		pc->cb_info->bufmgr = bufmgr;

		LOGD("recorder[type %d] %p create success : remote handle 0x%x",
		     type, pc, pc->remote_handle);

		*recorder = (recorder_h) pc;
	} else {
		goto _ERR_RECORDER_EXIT;
	}

	LOGD("done");

	return RECORDER_ERROR_NONE;

_ERR_RECORDER_AFTER_CREATE:
	muse_recorder_msg_send(MUSE_RECORDER_API_DESTROY, sock_fd, pc->cb_info, destroy_ret);
	LOGE("destroy return 0x%x", destroy_ret);

_ERR_RECORDER_EXIT:
	tbm_bufmgr_deinit(bufmgr);
	bufmgr = NULL;

	if (root_directory) {
		free(root_directory);
		root_directory = NULL;
	}

	if (pc) {
		if (pc->cb_info) {
			_client_callback_destroy(pc->cb_info);
			pc->cb_info = NULL;
		}
		g_free(pc);
		pc = NULL;
	}

	return ret;
}

int recorder_create_videorecorder(camera_h camera, recorder_h *recorder)
{
	return _recorder_create_common(recorder, MUSE_RECORDER_TYPE_VIDEO, camera);
}


int recorder_create_audiorecorder(recorder_h *recorder)
{
	return _recorder_create_common(recorder, MUSE_RECORDER_TYPE_AUDIO, NULL);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_STATE;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_state;

	LOGD("Enter, remote_handle : %x", pc->remote_handle);

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_state, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_DESTROY;
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

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_PREPARE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNPREPARE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_START;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_PAUSE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_COMMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_CANCEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_VIDEO_RESOLUTION;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send2(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_VIDEO_RESOLUTION;
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

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_width, pc->cb_info->recvMsg);
		muse_recorder_msg_get(get_height, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_VIDEO_RESOLUTION;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION] = foreach_cb;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_AUDIO_LEVEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_level;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_double(get_level, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_FILENAME;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api, sock_fd, pc->cb_info, ret, STRING, filename);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_FILENAME;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	char get_filename[MUSE_RECORDER_MSG_MAX_LENGTH] = {0,};

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_string(get_filename, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_FILE_FORMAT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_format = (int)format;

	LOGD("ENTER, set_format : %d", set_format);

	muse_recorder_msg_send1(api, sock_fd, pc->cb_info, ret, INT, set_format);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_FILE_FORMAT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_format;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_format, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_STATE_CHANGED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_STATE_CHANGE] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_STATE_CHANGE] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_STATE_CHANGED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_INTERRUPTED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_INTERRUPTED] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_INTERRUPTED] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_INTERRUPTED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_AUDIO_STREAM_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_AUDIO_STREAM] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_AUDIO_STREAM] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_AUDIO_STREAM_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_ERROR_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_ERROR] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_ERROR] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_ERROR_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_RECORDING_STATUS_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_RECORDING_STATUS] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_RECORDING_STATUS] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_RECORDING_STATUS_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_RECORDING_LIMIT_REACHED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = callback;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_RECORDING_LIMITED] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_UNSET_RECORDING_LIMIT_REACHED_CB;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_FILE_FORMAT;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT] = foreach_cb;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_SIZE_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_TIME_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_AUDIO_DEVICE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_device = (int)device;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_AUDIO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_codec = (int)codec;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_AUDIO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_codec, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_VIDEO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_codec = (int)codec;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_GET_VIDEO_ENCODER;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_codec, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_AUDIO_SAMPLERATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER, samplerate : %d", samplerate);

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_AUDIO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_VIDEO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_SIZE_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_kbyte;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_kbyte, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_TIME_LIMIT;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_second;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_second, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_AUDIO_DEVICE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_device;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_device, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_AUDIO_SAMPLERATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_samplerate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_samplerate, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_AUDIO_ENCODER_BITRATE;
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

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_bitrate, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_VIDEO_ENCODER_BITRATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_bitrate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_bitrate, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_AUDIO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER] = foreach_cb;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_VIDEO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	pc->cb_info->user_cb[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER] = foreach_cb;
	pc->cb_info->user_data[MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER] = user_data;

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_MUTE;
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

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_IS_MUTED;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_RECORDING_MOTION_RATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER - %.20lf", rate);

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_RECORDING_MOTION_RATE;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_rate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_double(get_rate, pc->cb_info->recvMsg);
		*rate = get_rate;
	}
	LOGD("ret : 0x%x - rate %.20lf", ret, *rate);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_AUDIO_CHANNEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_AUDIO_CHANNEL;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_channel_count;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_channel_count, pc->cb_info->recvMsg);
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_SET_ORIENTATION_TAG;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_orientation = (int)orientation;

	LOGD("ENTER");

	muse_recorder_msg_send1(api,
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
	muse_recorder_api_e api = MUSE_RECORDER_API_ATTR_GET_ORIENTATION_TAG;
	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)",RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_orientation;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_orientation, pc->cb_info->recvMsg);
		*orientation = (recorder_rotation_e)get_orientation;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}
