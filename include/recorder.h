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

#ifndef __TIZEN_MULTIMEDIA_RECORDER_H__
#define __TIZEN_MULTIMEDIA_RECORDER_H__
#include <tizen.h>
#include <camera.h>
#include <audio_io.h>
#include <sound_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORDER_ERROR_CLASS        TIZEN_ERROR_RECORDER | 0x10

/**
 * @file recorder.h
 * @brief This file contains the Recorder API.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief The Media recorder handle.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct recorder_s *recorder_h;

/**
 * @brief Enumeration for error code of the media recorder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_ERROR_NONE                  = TIZEN_ERROR_NONE,                /**< Successful */
	RECORDER_ERROR_INVALID_PARAMETER     = TIZEN_ERROR_INVALID_PARAMETER,   /**< Invalid parameter */
	RECORDER_ERROR_INVALID_STATE         = RECORDER_ERROR_CLASS | 0x02,     /**< Invalid state */
	RECORDER_ERROR_OUT_OF_MEMORY         = TIZEN_ERROR_OUT_OF_MEMORY ,      /**< Out of memory */
	RECORDER_ERROR_DEVICE                = RECORDER_ERROR_CLASS | 0x04,     /**< Device error */
	RECORDER_ERROR_INVALID_OPERATION     = TIZEN_ERROR_INVALID_OPERATION,   /**< Internal error */
	RECORDER_ERROR_SOUND_POLICY          = RECORDER_ERROR_CLASS | 0x06,     /**< Blocked by Audio Session Manager (Deprecated since 3.0) */
	RECORDER_ERROR_SECURITY_RESTRICTED   = RECORDER_ERROR_CLASS | 0x07,     /**< Restricted by security system policy */
	RECORDER_ERROR_SOUND_POLICY_BY_CALL  = RECORDER_ERROR_CLASS | 0x08,     /**< Blocked by Audio Session Manager - CALL (Deprecated since 3.0) */
	RECORDER_ERROR_SOUND_POLICY_BY_ALARM = RECORDER_ERROR_CLASS | 0x09,     /**< Blocked by Audio Session Manager - ALARM (Deprecated since 3.0) */
	RECORDER_ERROR_ESD                   = RECORDER_ERROR_CLASS | 0x0a,     /**< ESD situation */
	RECORDER_ERROR_OUT_OF_STORAGE        = RECORDER_ERROR_CLASS | 0x0b,     /**< Out of storage */
	RECORDER_ERROR_PERMISSION_DENIED     = TIZEN_ERROR_PERMISSION_DENIED,   /**< The access to the resources can not be granted */
	RECORDER_ERROR_NOT_SUPPORTED         = TIZEN_ERROR_NOT_SUPPORTED,       /**< The feature is not supported */
	RECORDER_ERROR_RESOURCE_CONFLICT     = RECORDER_ERROR_CLASS | 0x0c,     /**< Blocked by resource conflict (Since 3.0) */
} recorder_error_e;

/**
 * @brief Enumeration for recorder states.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_STATE_NONE,      /**< Recorder is not created */
	RECORDER_STATE_CREATED,   /**< Recorder is created, but not prepared */
	RECORDER_STATE_READY,     /**< Recorder is ready to record \n In case of video recorder, preview display will be shown */
	RECORDER_STATE_RECORDING, /**< Recorder is recording media */
	RECORDER_STATE_PAUSED,    /**< Recorder is paused while recording media */
} recorder_state_e;

/**
 * @brief Enumeration for the recording limit.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_RECORDING_LIMIT_TIME,        /**< Time limit (second) of recording file */
	RECORDER_RECORDING_LIMIT_SIZE,        /**< Size limit (kilo bytes [KB]) of recording file */
	RECORDER_RECORDING_LIMIT_FREE_SPACE,  /**< No free space in storage */
} recorder_recording_limit_type_e;

/**
 * @brief Enumeration for the file container format.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_FILE_FORMAT_3GP,    /**< 3GP file format */
	RECORDER_FILE_FORMAT_MP4,    /**< MP4 file format */
	RECORDER_FILE_FORMAT_AMR,    /**< AMR file format */
	RECORDER_FILE_FORMAT_ADTS,   /**< ADTS file format */
	RECORDER_FILE_FORMAT_WAV,    /**< WAV file format */
	RECORDER_FILE_FORMAT_OGG,    /**< OGG file format */
	RECORDER_FILE_FORMAT_M2TS    /**< MPEG2-TransportStream file format (Since 3.0) */
} recorder_file_format_e;

/**
 * @brief Enumeration for the audio codec.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_AUDIO_CODEC_DISABLE = -1, /**< Disable audio track */
	RECORDER_AUDIO_CODEC_AMR = 0,      /**< AMR codec */
	RECORDER_AUDIO_CODEC_AAC,          /**< AAC codec */
	RECORDER_AUDIO_CODEC_VORBIS,       /**< Vorbis codec */
	RECORDER_AUDIO_CODEC_PCM           /**< PCM codec */
} recorder_audio_codec_e;

/**
 * @brief Enumeration for the video codec.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_VIDEO_CODEC_H263,    /**< H263 codec */
	RECORDER_VIDEO_CODEC_H264,    /**< H264 codec */
	RECORDER_VIDEO_CODEC_MPEG4,   /**< MPEG4 codec */
	RECORDER_VIDEO_CODEC_THEORA   /**< Theora codec */
} recorder_video_codec_e;

/**
 * @brief Enumeration for audio capture devices.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_AUDIO_DEVICE_MIC,	/**< Mic device */
	RECORDER_AUDIO_DEVICE_MODEM,	/**< Modem */
} recorder_audio_device_e;

/**
 * @brief Enumeration for the recorder rotation type.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_ROTATION_NONE, /**< No rotation */
	RECORDER_ROTATION_90,   /**< 90 degree rotation */
	RECORDER_ROTATION_180,  /**< 180 degree rotation */
	RECORDER_ROTATION_270,  /**< 270 degree rotation */
} recorder_rotation_e;

/**
 * @brief Enumeration for the recorder policy.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum {
	RECORDER_POLICY_NONE = 0,               /**< None */
	RECORDER_POLICY_SOUND,                  /**< Sound policy (Deprecated since 3.0) */
	RECORDER_POLICY_SOUND_BY_CALL,          /**< Sound policy by CALL (Deprecated since 3.0) */
	RECORDER_POLICY_SOUND_BY_ALARM,         /**< Sound policy by ALARM (Deprecated since 3.0) */
	RECORDER_POLICY_SECURITY,               /**< Security policy */
	RECORDER_POLICY_RESOURCE_CONFLICT       /**< Resource conflict (Since 3.0) */
} recorder_policy_e;

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Called when limitation error occurs while recording.
 * @details The callback function is possible to receive three types of limits: time, size and no-space.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks After being called, recording data is discarded and not written in the recording file. Also the state of recorder is not changed.
 * @param[in] type The imitation type
 * @param[in] user_data The user data passed from the callback registration function
 * @pre You have to register a callback using recorder_set_recording_limit_reached_cb().
 * @see recorder_set_recording_status_cb()
 * @see recorder_set_recording_limit_reached_cb()
 * @see recorder_unset_recording_limit_reached_cb()
 */
typedef void (*recorder_recording_limit_reached_cb)(recorder_recording_limit_type_e type, void *user_data);

/**
 * @brief Called to indicate the recording status.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This callback function is repeatedly invoked during the #RECORDER_STATE_RECORDING state.
 * @param[in] elapsed_time  The time of the recording (milliseconds)
 * @param[in] file_size     The size of the recording file (KB)
 * @param[in] user_data     The user data passed from the callback registration function
 * @pre recorder_start() will invoke this callback if you register it using recorder_set_recording_status_cb().
 * @see	recorder_set_recording_status_cb()
 * @see	recorder_unset_recording_status_cb()
 * @see	recorder_start()
 */
typedef void (*recorder_recording_status_cb)(unsigned long long elapsed_time, unsigned long long file_size, void *user_data);

/**
 * @brief Called when the record state is changed.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] previous	The previous state of the recorder
 * @param[in] current	The current state of the recorder
 * @param[in] by_policy     @c true if the state is changed by policy, otherwise @c false if the state is not changed
 * @param[in] user_data	The user data passed from the callback registration function
 * @pre This function is required to register a callback using recorder_set_state_changed_cb().
 * @see	recorder_set_state_changed_cb()
 * @see	recorder_prepare()
 * @see	recorder_unprepare()
 * @see	recorder_start()
 * @see	recorder_pause()
 * @see	recorder_commit()
 * @see	recorder_cancel()
 */
typedef void (*recorder_state_changed_cb)(recorder_state_e previous , recorder_state_e current , bool by_policy, void *user_data);

/**
 * @brief Called when the recorder is interrupted by a policy.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] policy        The policy that is interrupting the recorder
 * @param[in] previous      The previous state of the recorder
 * @param[in] current       The current state of the recorder
 * @param[in] user_data     The user data passed from the callback registration function
 * @see	recorder_set_interrupted_cb()
 */
typedef void (*recorder_interrupted_cb)(recorder_policy_e policy, recorder_state_e previous, recorder_state_e current, void *user_data);

/**
 * @brief Called when audio stream data was being delivered just before storing in the recorded file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The callback function holds the same buffer that will be recorded. \n
 *          So if the user changes the buffer, the result file will contain the buffer.
 * @remarks The callback is called via internal thread of Frameworks, therefore do not invoke UI API, recorder_unprepare(), recorder_commit() and recorder_cancel() in callback.
 * @param[in] stream The audio stream data
 * @param[in] size The size of the stream data
 * @param[in] format The audio format
 * @param[in] channel The number of the channel
 * @param[in] timestamp The timestamp of the stream buffer (in msec)
 * @param[in] user_data The user data passed from the callback registration function
 * @see recorder_set_audio_stream_cb()
 */
typedef void (*recorder_audio_stream_cb)(void* stream, int size, audio_sample_type_e format, int channel, unsigned int timestamp, void *user_data);

/**
 * @brief Called once for each supported video resolution.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] width         The video image width
 * @param[in] height        The video image height
 * @param[in] user_data     The user data passed from the foreach function
 * @return    @c true to continue with the next iteration of the loop, \n otherwise @c false to break out of the loop
 * @pre	recorder_foreach_supported_video_resolution() will invoke this callback.
 * @see	recorder_foreach_supported_video_resolution()
 */
typedef bool (*recorder_supported_video_resolution_cb)(int width, int height, void *user_data);

/**
 * @brief Called when the error occurred.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This callback informs about the critical error situation. \n
 *          When being invoked, user should release the resource and terminate the application. \n
 *          This error code will be reported.
 *          #RECORDER_ERROR_DEVICE \n
 *          #RECORDER_ERROR_INVALID_OPERATION \n
 *          #RECORDER_ERROR_OUT_OF_MEMORY.
 * @param[in] error          The error code
 * @param[in] current_state  The current state of the recorder
 * @param[in] user_data      The user data passed from the callback registration function
 * @pre	This callback function is invoked if you register this callback using recorder_set_error_cb().
 * @see	recorder_set_error_cb()
 * @see	recorder_unset_error_cb()
 */
typedef void (*recorder_error_cb)(recorder_error_e error, recorder_state_e current_state, void *user_data);

 /**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief Called iteratively to notify about the supported file formats.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] format   The format of recording files
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n otherwise @c false to break out of the loop
 * @pre recorder_foreach_supported_file_format() will invoke this callback.
 * @see	recorder_foreach_supported_file_format()
 */
typedef bool (*recorder_supported_file_format_cb)(recorder_file_format_e format, void *user_data);

/**
 * @brief Called iteratively to notify about the supported audio encoders.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] codec	The codec of audio encoder
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n otherwise @c false to break out of the loop
 * @pre recorder_foreach_supported_audio_encoder() will invoke this callback.
 * @see	recorder_foreach_supported_audio_encoder()
 */
typedef bool (*recorder_supported_audio_encoder_cb)(recorder_audio_codec_e codec, void *user_data);

/**
 * @brief Called iteratively to notify about the supported video encoders.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] codec	The codec of video encoder
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n otherwise @c false to break out of the loop
 * @pre recorder_foreach_supported_video_encoder() will invoke this callback.
 * @see	recorder_foreach_supported_video_encoder()
 */
typedef bool (*recorder_supported_video_encoder_cb)(recorder_video_codec_e codec, void *user_data);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Creates a recorder handle to record a video.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks You must release @a recorder using recorder_destroy(). \n
 * The @a camera handle also could be used for capturing images. \n
 * If the camera state was #CAMERA_STATE_CREATED, the preview format will be changed to the recommended preview format for recording.
 * @remarks The created recorder state will be different according to camera state : \n
 * #CAMERA_STATE_CREATED -> #RECORDER_STATE_CREATED\n
 * #CAMERA_STATE_PREVIEW -> #RECORDER_STATE_READY\n
 * #CAMERA_STATE_CAPTURED -> #RECORDER_STATE_READY
 * @param[in]   camera	The handle to the camera
 * @param[out]  recorder	A handle to the recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see camera_create()
 * @see camera_stop_preview()
 * @see recorder_destroy()
 */
int recorder_create_videorecorder(camera_h camera, recorder_h *recorder);

/**
 * @brief Creates a recorder handle to record an audio.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks You must release @a recorder using recorder_destroy().
 * @param[out]  recorder  A handle to the recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post The recorder state will be #RECORDER_STATE_CREATED.
 * @see recorder_destroy()
 */
int recorder_create_audiorecorder(recorder_h *recorder);


/**
 * @brief Destroys the recorder handle.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks The video recorder's camera handle is not released by this function.
 * @param[in]	recorder    The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre  The recorder state should be #RECORDER_STATE_CREATED.
 * @post The recorder state will be #RECORDER_STATE_NONE.
 * @see	camera_destroy()
 * @see	recorder_create_videorecorder()
 * @see	recorder_create_audiorecorder()
 */
int recorder_destroy(recorder_h recorder);

/**
 * @brief Prepares the media recorder for recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks Before calling the function, it is required to properly set audio encoder (recorder_set_audio_encoder()),
 *          video encoder(recorder_set_video_encoder()) and file format (recorder_set_file_format()).
 * @param[in]	recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_RESOURCE_CONFLICT Resource conflict error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre  The recorder state should be #RECORDER_STATE_CREATED by recorder_create_videorecorder(), recorder_create_audiorecorder() or recorder_unprepare().
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @post If recorder handle is created by recorder_create_videorecorder(), the camera state will be changed to #CAMERA_STATE_PREVIEW.
 * @see	recorder_create_videorecorder()
 * @see	recorder_create_audiorecorder()
 * @see	recorder_unprepare()
 * @see	recorder_set_audio_encoder()
 * @see	recorder_set_video_encoder()
 * @see	recorder_set_file_format()
 */
int recorder_prepare(recorder_h recorder);

/**
 * @brief Resets the media recorder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre  The recorder state should be #RECORDER_STATE_READY set by recorder_prepare(), recorder_cancel() or recorder_commit().
 * @post The recorder state will be #RECORDER_STATE_CREATED.
 * @post If the recorder handle is created by recorder_create_videorecorder(), camera state will be changed to #CAMERA_STATE_CREATED.
 * @see	recorder_prepare()
 * @see	recorder_cancel()
 * @see	recorder_commit()
 */
int recorder_unprepare(recorder_h recorder);

/**
 * @brief Starts the recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks If file path has been set to an existing file, this file is removed automatically and updated by new one. \n
 *          In the video recorder, some preview format does not support record mode. It will return #RECORDER_ERROR_INVALID_OPERATION error. \n
 *          You should use default preview format or #CAMERA_PIXEL_FORMAT_NV12 in the record mode. \n
 *          When you want to record audio or video file, you need to add privilege according to rules below additionally. \n
 *          If you want to save contents to internal storage, you should add mediastorage privilege. \n
 *          If you want to save contents to external storage, you should add externalstorage privilege. \n
 *          The filename should be set before this function is invoked.
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_READY by recorder_prepare() or #RECORDER_STATE_PAUSED by recorder_pause(). \n
 *      The filename should be set by recorder_set_filename().
 * @post The recorder state will be #RECORDER_STATE_RECORDING.
 * @see	recorder_pause()
 * @see	recorder_commit()
 * @see	recorder_cancel()
 * @see	recorder_set_audio_encoder()
 * @see	recorder_set_filename()
 * @see	recorder_set_file_format()
 * @see	recorder_recording_status_cb()
 * @see	recorder_set_filename()
 */
int recorder_start(recorder_h recorder);

/**
 * @brief Pauses the recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks Recording can be resumed with recorder_start().
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_RECORDING.
 * @post The recorder state will be #RECORDER_STATE_PAUSED.
 * @see recorder_pause()
 * @see recorder_commit()
 * @see recorder_cancel()
 */
int recorder_pause(recorder_h recorder);

/**
 * @brief Stops recording and saves the result.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks When you want to record audio or video file, you need to add privilege according to rules below additionally. \n
 *          If you want to save contents to internal storage, you should add mediastorage privilege. \n
 *          If you want to save contents to external storage, you should add externalstorage privilege.
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_RECORDING set by recorder_start() or #RECORDER_STATE_PAUSED by recorder_pause().
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @see recorder_pause()
 * @see recorder_cancel()
 * @see recorder_set_filename()
 * @see	recorder_start()
 */
int recorder_commit(recorder_h recorder);

/**
 * @brief Cancels the recording.
 * @details The recording data is discarded and not written in the recording file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/recorder
 * @remarks When you want to record audio or video file, you need to add privilege according to rules below additionally. \n
 *          If you want to save contents to internal storage, you should add mediastorage privilege. \n
 *          If you want to save contents to external storage, you should add externalstorage privilege.
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_RECORDING set by recorder_start() or #RECORDER_STATE_PAUSED by recorder_pause().
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @see recorder_pause()
 * @see recorder_commit()
 * @see recorder_cancel()
 * @see recorder_start()
 */
int recorder_cancel(recorder_h recorder);

/**
 * @brief Gets the recorder's current state.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder The handle to the media recorder
 * @param[out]	state  The current state of the recorder
 * @return  @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 */
int recorder_get_state(recorder_h recorder, recorder_state_e *state);

/**
 * @brief Gets the peak audio input level that was sampled since the last call to this function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks @c 0 dB indicates maximum input level, @c -300 dB indicates minimum input level.
 * @param[in]  recorder The handle to the media recorder
 * @param[out] dB  The audio input level in dB
 * @return  @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_RECORDING or #RECORDER_STATE_PAUSED.
 */
int recorder_get_audio_level(recorder_h recorder, double *dB);

/**
 * @brief Sets the file path to record.
 * @details This function sets file path which defines where newly recorded data should be stored.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks If the same file already exists in the file system, then old file will be overwritten.
 * @param[in]	recorder	The handle to the media recorder
 * @param[in]	path The recording file path
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_get_filename()
 */
int recorder_set_filename(recorder_h recorder, const char *path);

/**
 * @brief Gets the file path to record.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks You must release @a path using free().
 * @param[in]	recorder    The handle to the media recorder
 * @param[out]	path    The recording file path
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_filename()
 */
int recorder_get_filename(recorder_h recorder, char **path);

/**
 * @brief Sets the file format for recording media stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks Since 2.3.1, it could be returned #RECORDER_ERROR_INVALID_OPERATION \n
 *          when it's audio recorder and its state is #RECORDER_STATE_READY \n
 *          because of checking codec compatibility with current encoder.
 * @param[in] recorder The handle to the media recorder
 * @param[in] format   The media file format
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation (Since 2.3.1)
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY (for video recorder only).\n
 *      Since 2.3.1, this API also works for audio recorder when its state is #RECORDER_STATE_READY.
 * @see recorder_get_file_format()
 * @see recorder_foreach_supported_file_format()
 */
int recorder_set_file_format(recorder_h recorder, recorder_file_format_e format);


/**
 * @brief Gets the file format for recording media stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder The handle to the media recorder
 * @param[out] format   The media file format
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see recorder_set_file_format()
 * @see recorder_foreach_supported_file_format()
 */
int recorder_get_file_format(recorder_h recorder, recorder_file_format_e *format);


/**
 * @brief Sets the recorder's sound manager stream information.
 * @since_tizen 3.0
 * @remarks You can set sound stream information including audio routing.
 *          For more details, please refer to @ref CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @param[in]	recorder	The handle to the media recorder
 * @param[in]	stream_info	The sound manager info
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see #sound_stream_info_h
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 */
int recorder_set_sound_stream_info(recorder_h recorder, sound_stream_info_h stream_info);


 /**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief Retrieves all supported file formats by invoking a specific callback for each supported file format.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[in] callback The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_supported_file_format_cb() will be invoked.
 * @see recorder_get_file_format()
 * @see recorder_set_file_format()
 * @see recorder_supported_file_format_cb()
 */
int recorder_foreach_supported_file_format(recorder_h recorder, recorder_supported_file_format_cb callback, void *user_data);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Sets the audio codec for encoding an audio stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks You can get available audio encoders by using recorder_foreach_supported_audio_encoder(). \n
 *          If set to #RECORDER_AUDIO_CODEC_DISABLE, the audio track is not created in recording files.\n
 *          Since 2.3.1, it could be returned #RECORDER_ERROR_INVALID_OPERATION \n
 *          when it's audio recorder and its state is #RECORDER_STATE_READY \n
 *          because of checking codec compatibility with current file format.
 * @param[in] recorder The handle to the media recorder
 * @param[in] codec    The audio codec
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation (Since 2.3.1)
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_get_audio_encoder()
 * @see recorder_foreach_supported_audio_encoder()
 */
int recorder_set_audio_encoder(recorder_h recorder, recorder_audio_codec_e codec);

/**
 * @brief Gets the audio codec for encoding an audio stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder The handle to the media recorder
 * @param[out] codec   The audio codec
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_audio_encoder()
 * @see recorder_foreach_supported_audio_encoder()
 */
int recorder_get_audio_encoder(recorder_h recorder, recorder_audio_codec_e *codec);

 /**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief Retrieves all supported audio encoders by invoking a specific callback for each supported audio encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[in] callback	The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_supported_audio_encoder_cb() will be invoked.
 * @see	recorder_set_audio_encoder()
 * @see	recorder_get_audio_encoder()
 * @see	recorder_supported_audio_encoder_cb()
 */
int recorder_foreach_supported_audio_encoder(recorder_h recorder, recorder_supported_audio_encoder_cb callback, void *user_data);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Sets the resolution of the video recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This function should be called before recording (recorder_start()).
 * @param[in] recorder	The handle to the media recorder
 * @param[in] width	The preview width
 * @param[in] height	The preview height
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre    The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_start()
 * @see	recorder_get_video_resolution()
 * @see	recorder_foreach_supported_video_resolution()
 */
int recorder_set_video_resolution(recorder_h recorder, int width, int height);

/**
 * @brief Gets the resolution of the video recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder	The handle to the media recorder
 * @param[out] width	The video width
 * @param[out] height	The video height
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_video_resolution()
 * @see	recorder_foreach_supported_video_resolution()
 */
int recorder_get_video_resolution(recorder_h recorder, int *width, int *height);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief Retrieves all supported video resolutions by invoking callback function once for each supported video resolution.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder	The handle to the media recorder
 * @param[in] foreach_cb	The callback function to be invoked
 * @param[in] user_data	The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post	This function invokes recorder_supported_video_resolution_cb() repeatedly to retrieve each supported video resolution.
 * @see	recorder_set_video_resolution()
 * @see	recorder_get_video_resolution()
 * @see	recorder_supported_video_resolution_cb()
 */
int recorder_foreach_supported_video_resolution(recorder_h recorder, recorder_supported_video_resolution_cb foreach_cb, void *user_data);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Sets the video codec for encoding video stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks You can get available video encoders by using recorder_foreach_supported_video_encoder().
 * @param[in] recorder The handle to the media recorder
 * @param[in] codec    The video codec
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see recorder_get_video_encoder()
 * @see recorder_foreach_supported_video_encoder()
 */
int recorder_set_video_encoder(recorder_h recorder, recorder_video_codec_e codec);

/**
 * @brief Gets the video codec for encoding video stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder The handle to the media recorder
 * @param[out] codec   The video codec
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see recorder_set_video_encoder()
 * @see recorder_foreach_supported_video_encoder()
 */
int recorder_get_video_encoder(recorder_h recorder, recorder_video_codec_e *codec);

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief Retrieves all supported video encoders by invoking a specific callback for each supported video encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder	The handle to the media recorder
 * @param[in] callback	The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_supported_video_encoder_cb() will be invoked.
 * @see recorder_set_video_encoder()
 * @see recorder_get_video_encoder()
 * @see	recorder_supported_video_encoder_cb()
 */
int recorder_foreach_supported_video_encoder(recorder_h recorder, recorder_supported_video_encoder_cb callback, void *user_data);

 /**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief Registers the callback function that will be invoked when the recorder state changes.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder	The handle to the media recorder
 * @param[in] callback	The function pointer of user callback
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_state_changed_cb() will be invoked.
 * @see recorder_unset_state_changed_cb()
 * @see recorder_state_changed_cb()
 */
int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see recorder_set_state_changed_cb()
 */
int recorder_unset_state_changed_cb(recorder_h recorder);

/**
 * @brief Registers a callback function to be called when the media recorder is interrupted according to a policy.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder	The handle to the media recorder
 * @param[in] callback	  The callback function to register
 * @param[in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_unset_interrupted_cb()
 * @see	recorder_interrupted_cb()
 */
int recorder_set_interrupted_cb(recorder_h recorder, recorder_interrupted_cb callback,
	    void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_interrupted_cb()
 */
int recorder_unset_interrupted_cb(recorder_h recorder);

/**
 * @brief Registers a callback function to be called when audio stream data is being delivered.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This callback function holds the same buffer that will be recorded. \n
 *          Therefore if an user changes the buffer, the result file will have the buffer. \n
 * @remarks The callback is called via internal thread of Frameworks. Therefore do not invoke UI API, recorder_unprepare(), recorder_commit() and recorder_cancel() in callback.\n
 *          This callback function to be called in #RECORDER_STATE_RECORDING and #RECORDER_STATE_PAUSED state.
 *
 * @param[in] recorder    The handle to the recorder
 * @param[in] callback    The callback function to register
 * @param[in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre	The recorder state should be #RECORDER_STATE_READY or #RECORDER_STATE_CREATED.
 * @see	recorder_unset_audio_stream_cb()
 * @see	recorder_audio_stream_cb()
 */
int recorder_set_audio_stream_cb(recorder_h recorder, recorder_audio_stream_cb callback, void* user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see     recorder_set_audio_stream_cb()
 */
int recorder_unset_audio_stream_cb(recorder_h recorder);

/**
 * @brief Registers a callback function to be invoked when the recording information changes.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder   The handle to the media recorder
 * @param[in]  callback   The function pointer of user callback
 * @param[in]  user_data  The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_recording_status_cb() will be invoked.
 * @see	recorder_unset_recording_status_cb()
 * @see	recorder_recording_status_cb()
 */
int recorder_set_recording_status_cb(recorder_h recorder, recorder_recording_status_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder    The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_recording_status_cb()
 */
int recorder_unset_recording_status_cb(recorder_h recorder);

/**
 * @brief Registers the callback function to be run when reached the recording limit.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to media recorder
 * @param[in]	callback	The function pointer of user callback
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post  recorder_recording_limit_reached_cb() will be invoked.
 * @see	recorder_unset_recording_limit_reached_cb()
 * @see	recorder_attr_set_size_limit()
 * @see	recorder_attr_set_time_limit()
 * @see	recorder_recording_limit_reached_cb()
 */
int recorder_set_recording_limit_reached_cb(recorder_h recorder, recorder_recording_limit_reached_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder  The handle to the media recorder
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_recording_limit_reached_cb()
 */
int recorder_unset_recording_limit_reached_cb(recorder_h recorder);

/**
 * @brief Registers a callback function to be called when an asynchronous operation error occurred.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This callback informs critical error situation.\n
 *          When this callback is invoked, user should release the resource and terminate the application. \n
 *          These error codes will occur. \n
 *          #RECORDER_ERROR_DEVICE \n
 *          #RECORDER_ERROR_INVALID_OPERATION \n
 *          #RECORDER_ERROR_OUT_OF_MEMORY
 * @param[in]	recorder	The handle to the recorder
 * @param[in]	callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return  @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @post	This function will invoke recorder_error_cb() when an asynchronous operation error occur.
 * @see	recorder_unset_error_cb()
 * @see	recorder_error_cb()
 */
int recorder_set_error_cb(recorder_h recorder, recorder_error_cb callback, void *user_data);


/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to the recorder
 * @return  @c on success, otherwise a negative error value
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_set_error_cb()
 */
int recorder_unset_error_cb(recorder_h recorder);


/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE
 * @{
 */


/**
 * @brief Sets the maximum size of a recording file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks After reaching the limitation, the recording data is discarded and not written in the recording file.
 * @param[in] recorder The handle to the media recorder
 * @param[in] kbyte    The maximum size of the recording file(KB) \n
 *                     @c 0 means unlimited recording size.
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_attr_get_size_limit()
 * @see	recorder_attr_set_time_limit()
 */
int recorder_attr_set_size_limit(recorder_h recorder, int kbyte);

/**
 * @brief Gets the maximum size of a recording file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder The handle to the media recorder
 * @param[out] kbyte    The maximum size of recording file (KB) \n
 *                      @c 0 means unlimited recording size.
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_size_limit()
 * @see	recorder_attr_get_time_limit()
 */
int recorder_attr_get_size_limit(recorder_h recorder, int *kbyte);

/**
 * @brief Sets the time limit of a recording file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks After reaching the limitation, the recording data is discarded and not written in the recording file.
 * @param[in] recorder The handle to the media recorder
 * @param[in] second   The time limit of the recording file (in seconds) \n
 *                     @c 0 means unlimited recording size.
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_attr_get_time_limit()
 * @see	recorder_attr_set_size_limit()
 */
int recorder_attr_set_time_limit(recorder_h recorder, int second);


/**
 * @brief Gets the time limit of a recording file.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder  The handle to the media recorder
 * @param[out] second    The time limit of the recording file (in seconds) \n
 *                       @c 0 means unlimited recording time.
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_time_limit()
 * @see	recorder_attr_get_size_limit()
 */
int recorder_attr_get_time_limit(recorder_h recorder, int *second);

/**
 * @brief Sets the audio device for recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder The handle to the media recorder
 * @param[in] device   The type of an audio device
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY (for video recorder only).\n
 *      Since 2.3.1, this API also works for audio recorder when its state is #RECORDER_STATE_READY.
 * @see	recorder_attr_get_audio_device()
 */
int recorder_attr_set_audio_device(recorder_h recorder, recorder_audio_device_e device);

/**
 * @brief Gets the audio device for recording.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[out] device The type of an audio device
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_audio_device()
 */
int recorder_attr_get_audio_device(recorder_h recorder, recorder_audio_device_e *device);

/**
 * @brief Sets the sampling rate of an audio stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder    The handle to the media recorder
 * @param[in] samplerate The sample rate in Hertz
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY (for video recorder only).\n
 *      Since 2.3.1, this API also works for audio recorder when its state is #RECORDER_STATE_READY.
 * @see	recorder_attr_get_audio_samplerate()
 */
int recorder_attr_set_audio_samplerate(recorder_h recorder, int samplerate);

/**
 * @brief Gets the sampling rate of an audio stream.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder    The handle to the media recorder
 * @param[out] samplerate  The sample rate in Hertz
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_audio_samplerate()
 */
int recorder_attr_get_audio_samplerate(recorder_h recorder, int *samplerate);

/**
 * @brief Sets the bitrate of an audio encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[in] bitrate   The bitrate (for mms : 12200[bps], normal : 288000[bps])
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_attr_get_audio_encoder_bitrate()
 */
int recorder_attr_set_audio_encoder_bitrate(recorder_h recorder, int bitrate);

/**
 * @brief Sets the bitrate of a video encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[in] bitrate   The bitrate in bits per second
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_attr_get_video_encoder_bitrate()
 */
int recorder_attr_set_video_encoder_bitrate(recorder_h recorder, int bitrate);

/**
 * @brief Gets the bitrate of an audio encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder  The handle to the media recorder
 * @param[out] bitrate   The bitrate in bits per second
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_audio_encoder_bitrate()
 */
int recorder_attr_get_audio_encoder_bitrate(recorder_h recorder, int *bitrate);

/**
 * @brief Gets the bitrate of a video encoder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]  recorder  The handle to the media recorder
 * @param[out] bitrate   The bitrate in bits per second
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_audio_encoder_bitrate()
 */
int recorder_attr_get_video_encoder_bitrate(recorder_h recorder, int *bitrate);

/**
 * @brief Sets the mute state of a recorder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[in] enable The mute state
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_is_muted()
 */
int recorder_attr_set_mute(recorder_h recorder, bool enable);

/**
 * @brief Gets the mute state of a recorder.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder The handle to the media recorder
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @return  @c true if the recorder is not recording any sound,
 *          otherwise @c false if the recorder is recording
 * @exception #RECORDER_ERROR_NONE Successful
 * @exception #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @exception #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @exception #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_mute()
 */
bool recorder_attr_is_muted(recorder_h recorder);

/**
 * @brief Sets the recording motion rate.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This attribute is valid only in a video recorder. \n
 *          If the rate bigger than @c 0 and smaller than @c 1, video is recorded in a slow motion mode. \n
 *          If the rate bigger than @c 1, video is recorded in a fast motion mode (time lapse recording).
 * @remarks Audio data is not recorded. \n
 *          To reset slow motion recording, set the rate to @c 1.
 * @param[in] recorder The handle to the media recorder
 * @param[in] rate     The recording motion rate \n
 *                     It is computed with fps. (@c 0<rate<@c 1 for slow motion, @c 1<rate for fast motion(time lapse recording), @c 1 to reset).
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY.
 * @see	recorder_attr_get_recording_motion_rate()
 */
int recorder_attr_set_recording_motion_rate(recorder_h recorder , double rate);

/**
 * @brief Gets the recording motion rate.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This attribute is valid only in a video recorder. \n
 *          If the rate bigger than @c 0 and smaller than @c 1, video is recorded in a slow motion mode. \n
 *          If the rate bigger than @c 1, video is recorded in a fast motion mode (time lapse recording).
 * @remarks Audio data is not recorded. \n
 *          To reset slow motion recording, set the rate to @c 1.
 * @param[in]  recorder The handle to the media recorder
 * @param[out] rate     The recording motion rate \n
 *                      It is computed with fps.
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_recording_motion_rate()
 */
int recorder_attr_get_recording_motion_rate(recorder_h recorder , double *rate);

/**
 * @brief Sets the number of the audio channel.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks This attribute is applied only in RECORDER_STATE_CREATED state. \n
 *          For mono recording, setting channel to @c 1. \n
 *          For stereo recording, setting channel to @c 2.
 * @param[in] recorder       The handle to the media recorder
 * @param[in] channel_count  The number of the audio channel
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY (for video recorder only).\n
 *      Since 2.3.1, this API also works for audio recorder when its state is #RECORDER_STATE_READY.
 * @see	recorder_attr_get_audio_channel()
 */
int recorder_attr_set_audio_channel(recorder_h recorder, int channel_count);

/**
 * @brief Gets the number of the audio channel.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] recorder  The handle to the media recorder
 * @param[out] channel_count  The number of the audio channel
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_audio_channel()
 */
int recorder_attr_get_audio_channel(recorder_h recorder, int *channel_count);


/**
 * @brief Sets the video orientation in a video metadata tag.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to a media recorder
 * @param[in]	orientation	The information of the video orientation
 * @return @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_get_orientation_tag()
 */
int recorder_attr_set_orientation_tag(recorder_h recorder,  recorder_rotation_e orientation);

/**
 * @brief Gets the video orientation in a video metadata tag.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in]	recorder	The handle to a media recorder
 * @param[out]  orientation	The information of the video orientation
 * @return  @c 0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_PERMISSION_DENIED The access to the resources can not be granted
 * @retval #RECORDER_ERROR_NOT_SUPPORTED The feature is not supported
 * @see	recorder_attr_set_orientation_tag()
 */
int recorder_attr_get_orientation_tag(recorder_h recorder, recorder_rotation_e *orientation);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MULTIMEDIA_RECORDER_H__ */

