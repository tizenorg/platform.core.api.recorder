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

static void _client_user_callback(recorder_cb_info_s *cb_info, char *recv_msg, muse_recorder_event_e event)
{
	if (recv_msg == NULL || event >= MUSE_RECORDER_EVENT_TYPE_NUM) {
		LOGE("invalid parameter - recorder msg %p, event %d", recv_msg, event);
		return;
	}

	LOGD("get recorder msg %s, event %d", recv_msg, event);

	if (cb_info->user_cb[event] == NULL) {
		LOGW("user callback for event %d is not set", event);
		return;
	}

	switch (event) {
	case MUSE_RECORDER_EVENT_TYPE_STATE_CHANGE:
		{
			int previous = 0;
			int current = 0;
			int by_policy = 0;

			muse_recorder_msg_get(previous, recv_msg);
			muse_recorder_msg_get(current, recv_msg);
			muse_recorder_msg_get(by_policy, recv_msg);

			((recorder_state_changed_cb)cb_info->user_cb[event])((recorder_state_e)previous,
				(recorder_state_e)current,
				(bool)by_policy,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_RECORDING_LIMITED:
		{
			int type = 0;

			muse_recorder_msg_get(type, recv_msg);

			((recorder_recording_limit_reached_cb)cb_info->user_cb[event])((recorder_recording_limit_type_e)type,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_RECORDING_STATUS:
		{
			int64_t cb_elapsed_time = 0;
			int64_t cb_file_size = 0;

			muse_recorder_msg_get(cb_elapsed_time, recv_msg);
			muse_recorder_msg_get(cb_file_size, recv_msg);

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

			muse_recorder_msg_get(policy, recv_msg);
			muse_recorder_msg_get(previous, recv_msg);
			muse_recorder_msg_get(current, recv_msg);

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

			muse_recorder_msg_get(tbm_key, recv_msg);
			if (tbm_key == 0) {
				LOGE("invalid key");
				break;
			}

			if (!_recorder_import_tbm_key(cb_info->bufmgr, tbm_key, &bo, &bo_handle)) {
				LOGE("tbm key %d import failed", tbm_key);
				break;
			}

			muse_recorder_msg_get(size, recv_msg);
			muse_recorder_msg_get(format, recv_msg);
			muse_recorder_msg_get(channel, recv_msg);
			muse_recorder_msg_get(timestamp, recv_msg);

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

			muse_recorder_msg_get(error, recv_msg);
			muse_recorder_msg_get(current_state, recv_msg);

			((recorder_error_cb)cb_info->user_cb[event])((recorder_error_e)error,
				(recorder_state_e)current_state,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_AUDIO_ENCODER:
		{
			int codec = 0;

			muse_recorder_msg_get(codec, recv_msg);

			((recorder_supported_audio_encoder_cb)cb_info->user_cb[event])((recorder_audio_codec_e)codec,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_FILE_FORMAT:
		{
			int format = 0;

			muse_recorder_msg_get(format, recv_msg);

			((recorder_supported_file_format_cb)cb_info->user_cb[event])((recorder_file_format_e)format,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_ENCODER:
		{
			int codec = 0;

			muse_recorder_msg_get(codec, recv_msg);

			((recorder_supported_video_encoder_cb)cb_info->user_cb[event])((recorder_video_codec_e)codec,
				cb_info->user_data[event]);
			break;
		}
	case MUSE_RECORDER_EVENT_TYPE_FOREACH_SUPPORTED_VIDEO_RESOLUTION:
		{
			int width = 0;
			int height = 0;

			muse_recorder_msg_get(width, recv_msg);
			muse_recorder_msg_get(height, recv_msg);

			((recorder_supported_video_resolution_cb)cb_info->user_cb[event])(width, height,
				cb_info->user_data[event]);
			break;
		}

	default:
			LOGE("Unknonw recorder event %d", event);
			break;
	}

	return;
}


static bool _recorder_idle_event_callback(void *data)
{
	recorder_cb_info_s *cb_info = NULL;
	recorder_idle_event_s *rec_idle_event = (recorder_idle_event_s *)data;

	if (rec_idle_event == NULL) {
		LOGE("rec_idle_event is NULL");
		return false;
	}

	/* lock event */
	g_mutex_lock(&rec_idle_event->event_mutex);

	cb_info = rec_idle_event->cb_info;
	if (cb_info == NULL) {
		LOGW("recorder cb_info is NULL. event %d", rec_idle_event->event);
		goto IDLE_EVENT_CALLBACK_DONE;
	}

	/* remove event from list */
	g_mutex_lock(&cb_info->idle_event_mutex);
	if (cb_info->idle_event_list)
		cb_info->idle_event_list = g_list_remove(cb_info->idle_event_list, (gpointer)rec_idle_event);

	/*LOGD("remove recorder idle event %p, %p", rec_idle_event, cb_info->idle_event_list);*/
	g_mutex_unlock(&cb_info->idle_event_mutex);

	/* user callback */
	_client_user_callback(rec_idle_event->cb_info, rec_idle_event->recv_msg, rec_idle_event->event);

	/* send signal for waiting thread */
	g_cond_signal(&cb_info->idle_event_cond);

IDLE_EVENT_CALLBACK_DONE:
	/* unlock and release event */
	g_mutex_unlock(&rec_idle_event->event_mutex);
	g_mutex_clear(&rec_idle_event->event_mutex);

	free(rec_idle_event);
	rec_idle_event = NULL;

	return false;
}


static void _recorder_remove_idle_event_all(recorder_cb_info_s *cb_info)
{
	recorder_idle_event_s *rec_idle_event = NULL;
	gboolean ret = TRUE;
	GList *list = NULL;
	gint64 end_time = 0;

	if (cb_info == NULL) {
		LOGE("cb_info is NULL");
		return;
	}

	g_mutex_lock(&cb_info->idle_event_mutex);

	if (cb_info->idle_event_list == NULL) {
		LOGD("No idle event is remained.");
	} else {
		list = cb_info->idle_event_list;

		while (list) {
			rec_idle_event = list->data;
			list = g_list_next(list);

			if (!rec_idle_event) {
				LOGW("Fail to remove idle event. The event is NULL");
			} else {
				if (g_mutex_trylock(&rec_idle_event->event_mutex)) {
					ret = g_idle_remove_by_data(rec_idle_event);

					LOGD("remove idle event [%p], ret[%d]", rec_idle_event, ret);

					if (ret == FALSE) {
						rec_idle_event->cb_info = NULL;
						LOGW("idle callback for event %p will be called later", rec_idle_event);
					}

					cb_info->idle_event_list = g_list_remove(cb_info->idle_event_list, (gpointer)rec_idle_event);

					g_mutex_unlock(&rec_idle_event->event_mutex);

					if (ret == TRUE) {
						g_mutex_clear(&rec_idle_event->event_mutex);

						free(rec_idle_event);
						rec_idle_event = NULL;

						LOGD("remove idle event done");
					}
				} else {
					LOGW("event lock failed. it's being called...");

					end_time = g_get_monotonic_time() + G_TIME_SPAN_MILLISECOND * 100;

					if (g_cond_wait_until(&cb_info->idle_event_cond, &cb_info->idle_event_mutex, end_time))
						LOGW("signal received");
					else
						LOGW("timeout");
				}
			}
		}

		g_list_free(cb_info->idle_event_list);
		cb_info->idle_event_list = NULL;
	}

	g_mutex_unlock(&cb_info->idle_event_mutex);

	return;
}


static void *_recorder_msg_handler_func(gpointer data)
{
	int ret = 0;
	int api = 0;
	int event = 0;
	int event_class = 0;
	recorder_message_s *rec_msg = NULL;
	recorder_idle_event_s *rec_idle_event = NULL;
	recorder_cb_info_s *cb_info = (recorder_cb_info_s *)data;

	if (cb_info == NULL) {
		LOGE("cb_info NULL");
		return NULL;
	}

	LOGD("start");

	g_mutex_lock(&cb_info->msg_handler_mutex);

	while (g_atomic_int_get(&cb_info->msg_handler_running)) {
		if (g_queue_is_empty(cb_info->msg_queue)) {
			LOGD("signal wait...");
			g_cond_wait(&cb_info->msg_handler_cond, &cb_info->msg_handler_mutex);
			LOGD("signal received");

			if (g_atomic_int_get(&cb_info->msg_handler_running) == 0) {
				LOGD("stop event thread");
				break;
			}
		}

		rec_msg = (recorder_message_s *)g_queue_pop_head(cb_info->msg_queue);

		g_mutex_unlock(&cb_info->msg_handler_mutex);

		if (rec_msg == NULL) {
			LOGE("NULL message");
			g_mutex_lock(&cb_info->msg_handler_mutex);
			continue;
		}

		api = rec_msg->api;

		if (api < MUSE_RECORDER_API_MAX) {
			g_mutex_lock(&cb_info->api_mutex[api]);

			if (muse_recorder_msg_get(ret, rec_msg->recv_msg)) {
				cb_info->api_ret[api] = ret;
				cb_info->api_activating[api] = 1;

				LOGD("recorder api %d - return 0x%x", ret);

				g_cond_signal(&cb_info->api_cond[api]);
			} else {
				LOGE("failed to get ret for api %d, msg %s", rec_msg->api, rec_msg->recv_msg);
			}

			g_mutex_unlock(&cb_info->api_mutex[api]);
		} else if (api == MUSE_RECORDER_CB_EVENT) {
			event = -1;
			event_class = -1;

			if (!muse_recorder_msg_get(event, rec_msg->recv_msg) ||
			    !muse_recorder_msg_get(event_class, rec_msg->recv_msg)) {
				LOGE("failed to get recorder event %d, class %d", event, event_class);

				g_free(rec_msg);
				rec_msg = NULL;

				g_mutex_lock(&cb_info->msg_handler_mutex);
				continue;
			}

			switch (event_class) {
			case MUSE_RECORDER_EVENT_CLASS_THREAD_SUB:
				_client_user_callback(cb_info, rec_msg->recv_msg, event);
				break;
			case MUSE_RECORDER_EVENT_CLASS_THREAD_MAIN:
				rec_idle_event = (recorder_idle_event_s *)malloc(sizeof(recorder_idle_event_s));
				if (rec_idle_event == NULL) {
					LOGE("rec_idle_event alloc failed");
					break;
				}

				rec_idle_event->event = event;
				rec_idle_event->cb_info = cb_info;
				g_mutex_init(&rec_idle_event->event_mutex);
				memcpy(rec_idle_event->recv_msg, rec_msg->recv_msg, sizeof(rec_idle_event->recv_msg));

				LOGD("add recorder event[%d, %p] to IDLE", event, rec_idle_event);

				g_mutex_lock(&cb_info->idle_event_mutex);
				cb_info->idle_event_list = g_list_append(cb_info->idle_event_list, (gpointer)rec_idle_event);
				g_mutex_unlock(&cb_info->idle_event_mutex);

				g_idle_add_full(G_PRIORITY_DEFAULT,
					(GSourceFunc)_recorder_idle_event_callback,
					(gpointer)rec_idle_event,
					NULL);
				break;
			default:
				LOGE("unknown recorder event class %d", event_class);
				break;
			}
		} else {
			LOGE("unknown recorder api[%d] message", api);
		}

		free(rec_msg);
		rec_msg = NULL;

		g_mutex_lock(&cb_info->msg_handler_mutex);
	}

	/* remove remained event */
	while (!g_queue_is_empty(cb_info->msg_queue)) {
		rec_msg = (recorder_message_s *)g_queue_pop_head(cb_info->msg_queue);
		if (rec_msg) {
			LOGD("remove recorder message %p", rec_msg);
			free(rec_msg);
			rec_msg = NULL;
		} else {
			LOGW("NULL recorder message");
		}
	}

	g_mutex_unlock(&cb_info->msg_handler_mutex);

	LOGD("return");

	return NULL;
}


static void *_recorder_msg_recv_func(gpointer data)
{
	int i = 0;
	int ret = 0;
	int api = 0;
	int api_class = 0;
	int num_token = 0;
	int str_pos = 0;
	int prev_pos = 0;
	char *recv_msg = NULL;
	char **parse_str = NULL;
	recorder_cb_info_s *cb_info = (recorder_cb_info_s *)data;

	if (cb_info == NULL) {
		LOGE("cb_info NULL");
		return NULL;
	}

	LOGD("start");

	parse_str = (char **)malloc(sizeof(char *) * RECORDER_PARSE_STRING_SIZE);
	if (parse_str == NULL) {
		LOGE("parse_str malloc failed");
		return NULL;
	}

	for (i = 0 ; i < RECORDER_PARSE_STRING_SIZE ; i++) {
		parse_str[i] = (char *)malloc(sizeof(char) * MUSE_RECORDER_MSG_MAX_LENGTH);
		if (parse_str[i] == NULL) {
			LOGE("parse_str[%d] malloc failed", i);
			goto CB_HANDLER_EXIT;
		}
	}

	recv_msg = cb_info->recv_msg;

	while (g_atomic_int_get(&cb_info->msg_recv_running)) {
		ret = muse_core_ipc_recv_msg(cb_info->fd, recv_msg);
		if (ret <= 0)
			break;
		recv_msg[ret] = '\0';

		str_pos = 0;
		prev_pos = 0;
		num_token = 0;

		/*LOGD("recvMSg : %s, length : %d", recv_msg, ret);*/

		/* Need to split the combined entering msgs.
		    This module supports up to 200 combined msgs. */
		for (str_pos = 0; str_pos < ret; str_pos++) {
			if (recv_msg[str_pos] == '}') {
				memset(parse_str[num_token], 0x0, sizeof(char) * MUSE_RECORDER_MSG_MAX_LENGTH);
				strncpy(parse_str[num_token], recv_msg + prev_pos, str_pos - prev_pos + 1);
				LOGD("splitted msg : [%s], Index : %d", parse_str[num_token], num_token);
				prev_pos = str_pos+1;
				num_token++;
			}
		}

		/*LOGD("num_token : %d", num_token);*/

		/* Re-construct to the useful single msg. */
		for (i = 0; i < num_token; i++) {
			if (i >= RECORDER_PARSE_STRING_SIZE) {
				LOGE("invalid token index %d", i);
				break;
			}

			api = -1;
			api_class = -1;

			if (!muse_recorder_msg_get(api, parse_str[i])) {
				LOGE("failed to get recorder api");
				continue;
			}

			if (muse_recorder_msg_get(api_class, parse_str[i]))
				LOGD("recorder api_class[%d]", api_class);

			if (api_class == MUSE_RECORDER_API_CLASS_IMMEDIATE) {
				g_mutex_lock(&cb_info->api_mutex[api]);

				if (!muse_recorder_msg_get(ret, parse_str[i])) {
					LOGE("failed to get recorder ret");
					g_mutex_unlock(&cb_info->api_mutex[api]);
					continue;
				}

				cb_info->api_ret[api] = ret;
				cb_info->api_activating[api] = 1;

				if (api == MUSE_RECORDER_API_CREATE) {
					if (ret != RECORDER_ERROR_NONE) {
						g_atomic_int_set(&cb_info->msg_recv_running, 0);
						LOGE("recorder create error 0x%x. close client cb handler", ret);
					}
				} else if (api == MUSE_RECORDER_API_DESTROY) {
					if (ret == RECORDER_ERROR_NONE) {
						g_atomic_int_set(&cb_info->msg_recv_running, 0);
						LOGD("recorder destroy done. close client cb handler");
					}
				}

				g_cond_signal(&cb_info->api_cond[api]);
				g_mutex_unlock(&cb_info->api_mutex[api]);
			} else if (api_class == MUSE_RECORDER_API_CLASS_THREAD_SUB ||
					api == MUSE_RECORDER_CB_EVENT) {
				recorder_message_s *rec_msg = g_new0(recorder_message_s, 1);
				if (rec_msg == NULL) {
					LOGE("failed to alloc rec_msg");
					continue;
				}

				rec_msg->api = api;
				memcpy(rec_msg->recv_msg, parse_str[i], sizeof(rec_msg->recv_msg));

				LOGD("add recorder message to queue : api %d", api);

				g_mutex_lock(&cb_info->msg_handler_mutex);
				g_queue_push_tail(cb_info->msg_queue, (gpointer)rec_msg);
				g_cond_signal(&cb_info->msg_handler_cond);
				g_mutex_unlock(&cb_info->msg_handler_mutex);
			} else {
				LOGW("unknown recorder api %d and api_class %d", api, api_class);
			}
		}
	}

	LOGD("client cb exit");

CB_HANDLER_EXIT:
	if (parse_str) {
		for (i = 0 ; i < RECORDER_PARSE_STRING_SIZE ; i++) {
			if (parse_str[i]) {
				free(parse_str[i]);
				parse_str[i] = NULL;
			}
		}

		free(parse_str);
		parse_str = NULL;
	}

	return NULL;
}

static recorder_cb_info_s *_client_callback_new(gint sockfd)
{
	recorder_cb_info_s *cb_info = NULL;
	GCond *tmp_cond = NULL;
	GMutex *tmp_mutex = NULL;
	gint *tmp_activating = NULL;
	gint *tmp_ret = NULL;

	g_return_val_if_fail(sockfd > 0, NULL);

	cb_info = g_new0(recorder_cb_info_s, 1);
	if (cb_info == NULL) {
		LOGE("cb_info failed");
		goto ErrorExit;
	}

	g_mutex_init(&cb_info->msg_handler_mutex);
	g_cond_init(&cb_info->msg_handler_cond);
	g_mutex_init(&cb_info->idle_event_mutex);
	g_cond_init(&cb_info->idle_event_cond);

	tmp_cond = g_new0(GCond, MUSE_RECORDER_API_MAX);
	if (tmp_cond == NULL) {
		LOGE("tmp_cond failed");
		goto ErrorExit;
	}

	tmp_mutex = g_new0(GMutex, MUSE_RECORDER_API_MAX);
	if (tmp_mutex == NULL) {
		LOGE("tmp_mutex failed");
		goto ErrorExit;
	}

	tmp_activating = g_new0(gint, MUSE_RECORDER_API_MAX);
	if (tmp_activating == NULL) {
		LOGE("tmp_activating failed");
		goto ErrorExit;
	}

	tmp_ret = g_new0(gint, MUSE_RECORDER_API_MAX);
	if (tmp_ret == NULL) {
		LOGE("tmp_ret failed");
		goto ErrorExit;
	}

	cb_info->msg_queue = g_queue_new();
	if (cb_info->msg_queue == NULL) {
		LOGE("msg_queue new failed");
		goto ErrorExit;
	}

	g_atomic_int_set(&cb_info->msg_handler_running, 1);
	cb_info->msg_handler_thread = g_thread_try_new("recorder_msg_handler",
		_recorder_msg_handler_func,
		(gpointer)cb_info,
		NULL);
	if (cb_info->msg_handler_thread == NULL) {
		LOGE("message handler thread creation failed");
		goto ErrorExit;
	}

	cb_info->fd = sockfd;
	cb_info->api_cond = tmp_cond;
	cb_info->api_mutex = tmp_mutex;
	cb_info->api_activating = tmp_activating;
	cb_info->api_ret = tmp_ret;

	g_atomic_int_set(&cb_info->msg_recv_running, 1);
	cb_info->msg_recv_thread = g_thread_try_new("recorder_msg_recv",
		_recorder_msg_recv_func,
		(gpointer)cb_info,
		NULL);
	if (cb_info->msg_recv_thread == NULL) {
		LOGE("message receive thread creation failed");
		goto ErrorExit;
	}

	return cb_info;


ErrorExit:
	if (cb_info) {
		if (cb_info->msg_handler_thread) {
			g_mutex_lock(&cb_info->msg_handler_mutex);
			g_atomic_int_set(&cb_info->msg_handler_running, 0);
			g_cond_signal(&cb_info->msg_handler_cond);
			g_mutex_unlock(&cb_info->msg_handler_mutex);

			g_thread_join(cb_info->msg_handler_thread);
			g_thread_unref(cb_info->msg_handler_thread);
			cb_info->msg_handler_thread = NULL;
		}

		g_mutex_clear(&cb_info->msg_handler_mutex);
		g_cond_clear(&cb_info->msg_handler_cond);
		g_mutex_clear(&cb_info->idle_event_mutex);
		g_cond_clear(&cb_info->idle_event_cond);

		if (cb_info->msg_queue) {
			g_queue_free(cb_info->msg_queue);
			cb_info->msg_queue = NULL;
		}

		g_free(cb_info);
		cb_info = NULL;
	}

	if (tmp_cond) {
		g_free(tmp_cond);
		tmp_cond = NULL;
	}
	if (tmp_mutex) {
		g_free(tmp_mutex);
		tmp_mutex = NULL;
	}
	if (tmp_activating) {
		g_free(tmp_activating);
		tmp_activating = NULL;
	}
	if (tmp_ret) {
		g_free(tmp_ret);
		tmp_ret = NULL;
	}

	return NULL;
}

static int client_wait_for_cb_return(muse_recorder_api_e api, recorder_cb_info_s *cb_info, int time_out)
{
	int ret = RECORDER_ERROR_NONE;
	gint64 end_time;

	LOGD("Enter api : %d", api);

	g_mutex_lock(&(cb_info->api_mutex[api]));

	if (cb_info->api_activating[api] == 0) {
		end_time = g_get_monotonic_time() + time_out * G_TIME_SPAN_SECOND;
		if (g_cond_wait_until(&(cb_info->api_cond[api]), &(cb_info->api_mutex[api]), end_time)) {
			ret = cb_info->api_ret[api];
			cb_info->api_activating[api] = 0;

			LOGD("return value : 0x%x", ret);
		} else {
			ret = RECORDER_ERROR_INVALID_OPERATION;

			LOGE("api %d was TIMED OUT!", api);
		}
	} else {
		ret = cb_info->api_ret[api];
		cb_info->api_activating[api] = 0;

		LOGD("condition is already checked for the api[%d], return[0x%x]", api, ret);
	}

	g_mutex_unlock(&(cb_info->api_mutex[api]));

	return ret;
}

static void _client_callback_destroy(recorder_cb_info_s *cb_info)
{
	g_return_if_fail(cb_info != NULL);

	LOGD("MSG receive thread[%p] destroy", cb_info->msg_recv_thread);

	g_thread_join(cb_info->msg_recv_thread);
	g_thread_unref(cb_info->msg_recv_thread);
	cb_info->msg_recv_thread = NULL;

	LOGD("msg thread removed");

	g_mutex_lock(&cb_info->msg_handler_mutex);
	g_atomic_int_set(&cb_info->msg_handler_running, 0);
	g_cond_signal(&cb_info->msg_handler_cond);
	g_mutex_unlock(&cb_info->msg_handler_mutex);

	g_thread_join(cb_info->msg_handler_thread);
	g_thread_unref(cb_info->msg_handler_thread);
	cb_info->msg_handler_thread = NULL;

	g_queue_free(cb_info->msg_queue);
	cb_info->msg_queue = NULL;
	g_mutex_clear(&cb_info->msg_handler_mutex);
	g_cond_clear(&cb_info->msg_handler_cond);
	g_mutex_clear(&cb_info->idle_event_mutex);
	g_cond_clear(&cb_info->idle_event_cond);

	LOGD("event thread removed");

	if (cb_info->bufmgr) {
		tbm_bufmgr_deinit(cb_info->bufmgr);
		cb_info->bufmgr = NULL;
	}
	if (cb_info->api_cond) {
		g_free(cb_info->api_cond);
		cb_info->api_cond = NULL;
	}
	if (cb_info->api_mutex) {
		g_free(cb_info->api_mutex);
		cb_info->api_mutex = NULL;
	}
	if (cb_info->api_activating) {
		g_free(cb_info->api_activating);
		cb_info->api_activating = NULL;
	}
	if (cb_info->api_ret) {
		g_free(cb_info->api_ret);
		cb_info->api_ret = NULL;
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
		muse_recorder_msg_get_pointer(handle, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_state;

	LOGD("Enter, remote_handle : %x", pc->remote_handle);

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_state, pc->cb_info->recv_msg);
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
	int sock_fd = 0;

	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}

	sock_fd = pc->cb_info->fd;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		_recorder_remove_idle_event_all(pc->cb_info);
		_client_callback_destroy(pc->cb_info);
		g_free(pc);
		pc = NULL;
	}

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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_width;
	int get_height;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_width, pc->cb_info->recv_msg);
		muse_recorder_msg_get(get_height, pc->cb_info->recv_msg);
		*width = get_width;
		*height = get_height;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_video_resolution(recorder_h recorder,
	recorder_supported_video_resolution_cb foreach_cb, void *user_data)
{
	if (recorder == NULL || foreach_cb == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_VIDEO_RESOLUTION;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_level;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_double(get_level, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	char get_filename[MUSE_RECORDER_MSG_MAX_LENGTH] = {0,};

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_string(get_filename, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_format;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);

	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_format, pc->cb_info->recv_msg);
		LOGD("get_fileformat : %d", get_format);
		*format = (recorder_file_format_e)get_format;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void* user_data)
{
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_STATE_CHANGED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_INTERRUPTED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_AUDIO_STREAM_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_ERROR_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_RECORDING_STATUS_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_SET_RECORDING_LIMIT_REACHED_CB;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || foreach_cb == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_FILE_FORMAT;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_codec, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_codec;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_codec, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_kbyte;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_kbyte, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_second;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_second, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_device;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_device, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_samplerate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_samplerate, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_bitrate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_bitrate, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_bitrate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_bitrate, pc->cb_info->recv_msg);
		*bitrate = get_bitrate;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}


int recorder_foreach_supported_audio_encoder(recorder_h recorder, recorder_supported_audio_encoder_cb foreach_cb, void *user_data)
{
	if (recorder == NULL || foreach_cb == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_AUDIO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
	if (recorder == NULL || foreach_cb == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	int ret = RECORDER_ERROR_NONE;

	recorder_cli_s *pc = (recorder_cli_s *)recorder;
	muse_recorder_api_e api = MUSE_RECORDER_API_FOREACH_SUPPORTED_VIDEO_ENCODER;

	LOGD("Enter, handle :%x", pc->remote_handle);

	int sock_fd;
	if (pc->cb_info == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int set_enable = (int)enable;

	LOGD("ENTER");

	muse_recorder_msg_send1(api, sock_fd, pc->cb_info, ret, INT, set_enable);

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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	double get_rate;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get_double(get_rate, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_channel_count;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_channel_count, pc->cb_info->recv_msg);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
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
		LOGE("INVALID_PARAMETER(0x%08x)", RECORDER_ERROR_INVALID_PARAMETER);
		return RECORDER_ERROR_INVALID_PARAMETER;
	}
	sock_fd = pc->cb_info->fd;
	int get_orientation;

	LOGD("ENTER");

	muse_recorder_msg_send(api, sock_fd, pc->cb_info, ret);
	if (ret == RECORDER_ERROR_NONE) {
		muse_recorder_msg_get(get_orientation, pc->cb_info->recv_msg);
		*orientation = (recorder_rotation_e)get_orientation;
	}
	LOGD("ret : 0x%x", ret);
	return ret;
}
