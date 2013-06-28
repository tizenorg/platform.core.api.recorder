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
#define	__TIZEN_MULTIMEDIA_RECORDER_H__
#include <tizen.h>
#include <camera.h>
#include <audio_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORDER_ERROR_CLASS        TIZEN_ERROR_MULTIMEDIA_CLASS | 0x10


/**
 * @file recorder.h
 * @brief This file contains the Recorder API.
 */

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief The handle to media recorder
 */
typedef struct recorder_s *recorder_h;

/**
 * @brief  Enumerations of error code for the media recorder.
 */
typedef enum
{
	RECORDER_ERROR_NONE = TIZEN_ERROR_NONE,                                 /**< Successful */
	RECORDER_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,       /**< Invalid parameter */
	RECORDER_ERROR_INVALID_STATE = RECORDER_ERROR_CLASS | 0x02,             /**< Invalid state */
	RECORDER_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY ,              /**< Out of memory */
	RECORDER_ERROR_DEVICE = RECORDER_ERROR_CLASS | 0x04,                    /**< Device error */
	RECORDER_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,       /**< Internal error */
	RECORDER_ERROR_SOUND_POLICY = RECORDER_ERROR_CLASS | 0x06,              /**< Blocked by Audio Session Manager */
	RECORDER_ERROR_SECURITY_RESTRICTED = RECORDER_ERROR_CLASS | 0x07,       /**< Restricted by security system policy */
	RECORDER_ERROR_SOUND_POLICY_BY_CALL = RECORDER_ERROR_CLASS | 0x08,      /**< Blocked by Audio Session Manager - CALL */
	RECORDER_ERROR_SOUND_POLICY_BY_ALARM = RECORDER_ERROR_CLASS | 0x09,     /**< Blocked by Audio Session Manager - ALARM */
	RECORDER_ERROR_ESD = RECORDER_ERROR_CLASS | 0x0a,                       /**< ESD situation */
	RECORDER_ERROR_OUT_OF_STORAGE = RECORDER_ERROR_CLASS | 0x0b,            /**< Out of storage */
} recorder_error_e;

/**
 * @brief Enumerations for camera states.
 */
typedef enum
{
	RECORDER_STATE_NONE,				/**< Recorder is not created */
	RECORDER_STATE_CREATED,				/**< Recorder is created, but not prepared*/
	RECORDER_STATE_READY,			    /**< Recorder is ready to record\n  In case of video recorder, preview display will be shown */
	RECORDER_STATE_RECORDING,	        /**< Recorder is recording media*/
	RECORDER_STATE_PAUSED,			    /**< Recorder is paused while recording media*/
} recorder_state_e;

/**
 * @brief Enumerations of recording limitation.
 */
typedef enum
{
	RECORDER_RECORDING_LIMIT_TIME, 			/**< Time limit (second) of recording file */
	RECORDER_RECORDING_LIMIT_SIZE, 			/**< Size limit (kilo bytes [KB]) of recording file */
	RECORDER_RECORDING_LIMIT_FREE_SPACE, 	/**< No free space in storage  */
} recorder_recording_limit_type_e;

/**
 * @brief Enumerations of file container format.
 */
typedef enum
{
        RECORDER_FILE_FORMAT_3GP,                     /**< 3GP file format */
        RECORDER_FILE_FORMAT_MP4,                     /**< MP4 file format */
        RECORDER_FILE_FORMAT_AMR,                     /**< AMR file format */
        RECORDER_FILE_FORMAT_ADTS,                   /**< ADTS file format */
        RECORDER_FILE_FORMAT_WAV,                    /**< WAV file format */
        RECORDER_FILE_FORMAT_OGG,                    /**< OGG file format */
} recorder_file_format_e;


/**
 * @brief Enumerations of audio codec.
 */
typedef enum
{
	RECORDER_AUDIO_CODEC_DISABLE = -1, /**< Disable audio track */
	RECORDER_AUDIO_CODEC_AMR = 0,			/**< AMR codec */
	RECORDER_AUDIO_CODEC_AAC,			/**< AAC codec */
	RECORDER_AUDIO_CODEC_VORBIS,	/**< Vorbis codec */
	RECORDER_AUDIO_CODEC_PCM			/**< PCM codec */
} recorder_audio_codec_e;

/**
 * @brief Enumerations of video codec.
 */
typedef enum
{
	RECORDER_VIDEO_CODEC_H263,			/**< H263 codec */
	RECORDER_VIDEO_CODEC_H264,			/**< H264 codec */
	RECORDER_VIDEO_CODEC_MPEG4,		/**< MPEG4 codec */
	RECORDER_VIDEO_CODEC_THEORA		/**< Theora codec */
} recorder_video_codec_e;

/**
 * @brief Enumerations of audio capture devices.
 */
typedef enum
{
	RECORDER_AUDIO_DEVICE_MIC,	/**< Mic device */
	RECORDER_AUDIO_DEVICE_MODEM,	/**< Modem */
} recorder_audio_device_e;

/**
 * @brief Enumerations of the recorder rotation type.
 */
typedef enum
{
	RECORDER_ROTATION_NONE,	/**< No rotation */
	RECORDER_ROTATION_90,		/**< 90 degree rotation */
	RECORDER_ROTATION_180,	/**< 180 degree rotation */
	RECORDER_ROTATION_270,	/**< 270 degree rotation */
} recorder_rotation_e;

/**
 * @brief Enumerations of the recorder flip type.
 */
typedef enum
{
	RECORDER_FLIP_NONE, /**< No Flip */
	RECORDER_FLIP_HORIZONTAL, /**< Horizontal flip */
	RECORDER_FLIP_VERTICAL, /**< Vertical flip */
	RECORDER_FLIP_BOTH /**< Horizontal and vertical flip */
} recorder_flip_e;


/**
 * @brief Enumerations of the recorder policy.
 */
typedef enum
{
	RECORDER_POLICY_NONE = 0,       /**< None */
	RECORDER_POLICY_SOUND,          /**< Sound policy */
	RECORDER_POLICY_SOUND_BY_CALL,  /**< Sound policy by CALL */
	RECORDER_POLICY_SOUND_BY_ALARM, /**< Sound policy by ALARM */
	RECORDER_POLICY_SECURITY        /**< Security policy */
} recorder_policy_e;

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_MODULE
 * @{
 */

/**
 * @brief  The callback function called when limitation error occurs while recording.
 * @details The callback function is possible to receive three types of limits: time, size and no-space.
 * @remarks After called it, recording data is discarded and not written in recording file. Also the state of recorder is not changed.
 * @param[in] type		The limitation type
 * @param[in] user_data The user data passed from the callback registration function
 * @pre It is required to register a callback using recorder_set_recording_limit_reached_cb()
 * @see recorder_set_recording_status_cb()
 * @see recorder_set_recording_limit_reached_cb()
 * @see recorder_unset_recording_limit_reached_cb()
 */
typedef void (*recorder_recording_limit_reached_cb)(recorder_recording_limit_type_e type, void *user_data);


/**
 * @brief  Callback function to indicate recording status
 * @remarks This callback function is repeatedly invoked during #RECORDER_STATE_RECORDING state
 * @param[in] elapsed_time	The time of recording (milliseconds)
 * @param[in] file_size	The size of recording file (KB)
 * @param[in] user_data The user data passed from the callback registration function
 * @pre recorder_start() will cause this callback if you register this callback using recorder_set_recording_status_cb()
 * @see	recorder_set_recording_status_cb()
 * @see	recorder_unset_recording_status_cb()
 * @see	recorder_start()
 */
typedef void (*recorder_recording_status_cb)(unsigned long long elapsed_time, unsigned long long file_size, void *user_data);


/**
 * @brief  Called when the record state has changed.
 * @param[in] previous	The previous state of recorder
 * @param[in] current	The current state of recorder
 * @param[in] by_policy     @c true if the state is changed by policy, otherwise @c false
 * @param[in] user_data	The user data passed from the callback registration function
 * @pre This function is required to register a callback using recorder_set_state_changed_cb()
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
 * @brief	Called when the recorder interrupted by policy
 *
 * @param[in] policy     		The policy that interrupting the recorder
 * @param[in] previous      The previous state of the recorder
 * @param[in] current       The current state of the recorder
 * @param[in] user_data     The user data passed from the callback registration function
 * @see	recorder_set_interrupted_cb()
 */
typedef void (*recorder_interrupted_cb)(recorder_policy_e policy, recorder_state_e previous, recorder_state_e current, void *user_data);

/**
 * @brief Called when audio stream data was delivering just before storing in record file.
 * @remarks
 * The callback function holds the same buffer that will be recorded.\n
 * So if an user change the buffer, the result file will has the buffer.\n
 * The callback is called via internal thread of Frameworks. so don't be invoke UI API, recorder_unprepare(), recorder_commit() and recorder_cancel() in callback.
 *
 * @param[in] stream The audio stream data
 * @param[in] size The size of stream data
 * @param[in] format The audio format
 * @param[in] channel The number of channel
 * @param[in] timestamp The timestamp of stream buffer( in msec )
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @see recorder_set_audio_stream_cb()
 */
typedef void (*recorder_audio_stream_cb)(void* stream, int size, audio_sample_type_e format, int channel, unsigned int timestamp, void *user_data);

/**
 * @brief	Called when the error occurred.
 *
 * @remarks
 * This callback inform critical error situation.\n
 * When invoked this callback, user should release the resource and terminate application.\n
 * These error code will be occurred\n
 * #RECORDER_ERROR_DEVICE\n
 * #RECORDER_ERROR_INVALID_OPERATION\n
 * #RECORDER_ERROR_OUT_OF_MEMORY\n
 *
 * @param[in] error		The error code
 * @param[in] current_state	The current state of the recorder
 * @param[in] user_data		The user data passed from the callback registration function
 *
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
 * @brief Gets called iteratively to notify you of supported file formats. 
 * @param[in] format   The format of recording files
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop
 * @pre recorder_foreach_supported_file_format() will invoke this callback.
 * @see	recorder_foreach_supported_file_format()
 */
typedef bool (*recorder_supported_file_format_cb)(recorder_file_format_e format, void *user_data);


/**
 * @brief  Gets called iteratively to notify you of supported audio encoders.
 * @param[in] codec	The codec of audio encoder.
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop
 * @pre recorder_foreach_supported_audio_encoder() will invoke this callback.
 * @see	recorder_foreach_supported_audio_encoder()
 */
typedef bool (*recorder_supported_audio_encoder_cb)(recorder_audio_codec_e codec, void *user_data);


/**
 * @brief  Gets called iteratively to notify you of supported video encoders.
 * @param[in] codec	The codec of video encoder.
 * @param[in] user_data	The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop
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
 * @brief  Creates a recorder handle to record a video.
 * @param[in]   camera	The handle to camera
 * @param[out]  recorder	A handle to recorder
 * @remarks @a recorder must be released with recorder_destroy() by you.\n
 * The @a camera handle also could be used for capturing images.\n
 * If camera state was CAMERA_STATE_CREATED, the preview format will be changed to the recommended preview format for recording.\n
 * The created recorder state will be different according to camera state : \n
 * CAMERA_STATE_CREATED -> RECORDER_STATE_CREATED\n
 * CAMERA_STATE_PREVIEW -> RECORDER_STATE_READY\n
 * CAMERA_STATE_CAPTURED -> RECORDER_STATE_READY\n
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @see camera_create()
 * @see camera_stop_preview()
 * @see recorder_destroy()
 */
int recorder_create_videorecorder(camera_h camera, recorder_h *recorder);


/**
 * @brief  Creates a recorder handle to record an audio.
 * @remarks @a recorder must be released with recorder_destroy() by you 
 * @param[out]	recorder	A handle to  recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @post The recorder state will be #RECORDER_STATE_CREATED.
 * @see recorder_destroy()
 */
int recorder_create_audiorecorder(recorder_h *recorder);


/**
 * @brief  Destroys the recorder handle
 * @remarks	Video recorder's camera handle is not release by this function.
 * @param[in]	recorder    The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre  The recorder state should be #RECORDER_STATE_CREATED
 * @post The recorder state will be #RECORDER_STATE_NONE.
 * @see	camera_destroy()
 * @see	recorder_create_videorecorder()
 * @see	recorder_create_audiorecorder()
 */
int recorder_destroy(recorder_h recorder);


/**
 * @brief  Prepares the media recorder for recording
 * @remarks	Before calling the function, it is required to set audio encoder (recorder_set_audio_encoder()),
 * video encoder(recorder_set_video_encoder()), file format (recorder_set_file_format()) with proper value.
 * @param[in]	recorder	The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_SOUND_POLICY Sound policy error
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre  The recorder state should be #RECORDER_STATE_CREATED by recorder_create_videorecorder(), recorder_create_audiorecorder() or recorder_unprepare().
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @post	If recorder handle is created by recorder_create_videorecorder(), it's camera state will be changed to #CAMERA_STATE_PREVIEW.
 * @see	recorder_create_videorecorder()
 * @see	recorder_create_audiorecorder() 
 * @see	recorder_unprepare()
 * @see	recorder_set_audio_encoder()
 * @see 	recorder_set_video_encoder()
 * @see 	recorder_set_file_format()
 */
int recorder_prepare(recorder_h recorder);


/**
 * @brief  Reset the media recorder.
 * @param[in]	recorder    The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre  The recorder state should be #RECORDER_STATE_READY by recorder_prepare(), recorder_cancel(),or recorder_commit().
 * @post The recorder state will be #RECORDER_STATE_CREATED.
 * @post	If recorder handle is created by recorder_create_videorecorder(), it's camera state will be changed to #CAMERA_STATE_CREATED.
 * @see	recorder_prepare()
 * @see	recorder_cancel()
 * @see	recorder_commit()
 */
int recorder_unprepare(recorder_h recorder);


/**
 * @brief  Starts recording
 * @remarks If file path has been set to existing file, this file is removed automatically and updated by new one.\n
 * In video recorder, some preview format is not supporting record mode. It will return #RECORDER_ERROR_INVALID_OPERATION error.\n
 * You should use default preview format or #CAMERA_PIXEL_FORMAT_NV12 in record mode\n
 * @param[in]	recorder	The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_READY by recorder_prepare() or #RECORDER_STATE_PAUSED by recorder_pause()
 * @post The recorder state will be #RECORDER_STATE_RECORDING.
 * @see	recorder_pause()
 * @see 	recorder_commit()
 * @see 	recorder_cancel()
 * @see 	recorder_set_audio_encoder()
 * @see 	recorder_set_filename()
 * @see 	recorder_set_file_format()
 * @see	recorder_recording_status_cb()
 */
int recorder_start(recorder_h recorder);


/**
 * @brief  Pauses recording
 * @remarks	Recording can be resumed with recorder_start().
 * @param[in]	recorder	The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_RECORDING
 * @post The recorder state will be #RECORDER_STATE_PAUSED.
 * @see recorder_pause()
 * @see recorder_commit()
 * @see recorder_cancel()
 */
int recorder_pause(recorder_h recorder);


/**
 * @brief  Stops recording and saving the result
 * @param   [in]	recorder	The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_RECORDING by recorder_start() or #RECORDER_STATE_PAUSED by recorder_pause()
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @see recorder_pause()
 * @see recorder_cancel()
 * @see recorder_set_filename()
 * @see	recorder_start()
 */
int recorder_commit(recorder_h recorder);


/**
 * @brief  Cancels recording.
 * @detail The recording data is discarded and not written in recording file.
 * @param[in]	recorder    The handle to media recorder
 * @return	0 on success, otherwise a negative error value
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_RECORDING by recorder_start() or #RECORDER_STATE_PAUSED by recorder_pause()
 * @post The recorder state will be #RECORDER_STATE_READY.
 * @see recorder_pause()
 * @see recorder_commit()
 * @see recorder_cancel()
 * @see recorder_start()
 */
int recorder_cancel(recorder_h recorder);


/**
 * @brief Gets the recorder's current state.
 * @param[in]  recorder The handle to the recorder.
 * @param[out]	state  The current state of the recorder
 * @return  0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 */
int recorder_get_state(recorder_h recorder, recorder_state_e *state);

/**
 * @brief Gets the peak audio input level that was sampled since the last call to this function.
 * @remarks 0 dB indicates maximum input level, -300dB indicates minimum input level
 * @param[in]  recorder The handle to the recorder.
 * @param[out]	level  The audio input level in dB
 * @return  0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_RECORDING or #RECORDER_STATE_PAUSED
 */
int recorder_get_audio_level(recorder_h recorder, double *dB);

/**
 * @brief  Sets the file path to record
 * @details This function sets file path which defines where newly recorder data should be stored. 
 * @remarks	If there is already exists same file in file system, then old file will be overwritten.
 * @param[in]	recorder	The handle to media recorder
 * @param[in]	path The recording file path
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED by recorder_create() or recorder_unprepare()
 * @see	recorder_get_filename()
 */
int recorder_set_filename(recorder_h recorder, const char *path);


/**
 * @brief  Gets the file path to record
 * @remarks  @a path must be released with @c free() by you.
 * @param[in]	recorder    The handle to media recorder
 * @param[out]	path    The recording file path
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_set_filename()
 */
int recorder_get_filename(recorder_h recorder, char **path);


/**
 * @brief  Sets the file format for recording media stream
 * @param[in] recorder The handle to media recorder
 * @param[in] format   The media file format
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED
 * @see recorder_get_file_format()
 * @see recorder_foreach_supported_file_format()
 */
int recorder_set_file_format(recorder_h recorder, recorder_file_format_e format);


/**
 * @brief  Gets the file format for recording media stream
 * @param [in] recorder The handle to media recorder
 * @param [in] format   The media file format
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see recorder_set_file_format()
 * @see recorder_foreach_supported_file_format()
 */
int recorder_get_file_format(recorder_h recorder, recorder_file_format_e *format);


 /**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE
 * @{
 */

/**
 * @brief  Retrieves all supported file formats by invoking a specific callback for each supported file format
 * @param[in] recorder  The handle to media recorder
 * @param[in] callback The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_supported_file_format_cb() will be invoked
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
 * @brief  Sets the audio codec for encoding audio stream
 * @remarks You can get available audio encoders by using recorder_foreach_supported_audio_encoder().\n
 * if set to RECORDER_AUDIO_CODEC_DISABLE, audio track is not created in recording files.
 * @param[in] recorder The handle to media recorder
 * @param[in] codec    The audio codec
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see	recorder_get_audio_encoder()
 * @see recorder_foreach_supported_audio_encoder()
 */
int recorder_set_audio_encoder(recorder_h recorder, recorder_audio_codec_e codec);


/**
 * @brief  Gets the audio codec for encoding audio stream
 * @param [in] recorder The handle to media recorder
 * @param [out] codec   The audio codec
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
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
 * @brief  Retrieves all supported audio encoders by invoking a specific callback for each supported audio encoder.
 * @param[in] recorder  The handle to media recorder
 * @param[in] callback	The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_supported_audio_encoder_cb() will be invoked
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
 * @brief  Sets the video codec for encoding video stream.
 * @remarks You can get available video encoders by using recorder_foreach_supported_video_encoder().
 * @param[in] recorder The handle to media recorder
 * @param[in] codec    The video codec
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see recorder_get_video_encoder()
 * @see recorder_foreach_supported_video_encoder()
 */
int recorder_set_video_encoder(recorder_h recorder, recorder_video_codec_e codec);


/**
 * @brief  Gets the video codec for encoding video stream.
 * @param[in] recorder The handle to media recorder
 * @param[out] codec   The video codec
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
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
 * @brief  Retrieves all supported video encoders by invoking a specific callback for each supported video encoder.
 * @param[in] recorder	The handle to media recorder
 * @param[in] callback	The iteration callback
 * @param[in] user_data	The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_supported_video_encoder_cb() will be invoked
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
 * @brief  Registers the callback function that will be invoked when the recorder state changes.
 * @param[in] recorder	The handle to the recorder.
 * @param[in] callback	The function pointer of user callback
 * @param[in] user_data The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_state_changed_cb() will be invoked
 * @see recorder_unset_state_changed_cb()
 * @see recorder_state_changed_cb()
 */
int recorder_set_state_changed_cb(recorder_h recorder, recorder_state_changed_cb callback, void *user_data);


/**
 * @brief  Unregisters the callback function.
 * @param[in]  recorder The handle to the recorder.
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see recorder_set_state_changed_cb()
 */
int recorder_unset_state_changed_cb(recorder_h recorder);

/**
 * @brief	Registers a callback function to be called when recorder interrupted by policy.
 *
 * @param[in] recorder	The handle to the recorder
 * @param[in] callback	  The callback function to register
 * @param[in] user_data   The user data to be passed to the callback function
 *
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see recorder_unset_interrupted_cb()
 * @see	recorder_interrupted_cb()
 */
int recorder_set_interrupted_cb(recorder_h recorder, recorder_interrupted_cb callback,
	    void *user_data);

/**
 * @brief	Unregisters the callback function.
 *
 * @param[in]	recorder	The handle to the recorder
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see     recorder_set_interrupted_cb()
 */
int recorder_unset_interrupted_cb(recorder_h recorder);

/**
 * @brief	Registers a callback function to be called when audio stream data was delivering
 *
 * @remarks
 * This callback function holds the same buffer that will be recorded.\n
 * So if an user change the buffer, the result file will has the buffer.\n
 * The callback is called via internal thread of Frameworks. so don't be invoke UI API, recorder_unprepare(), recorder_commit() and recorder_cancel() in callback.\n
 * This callback function to be called in RECORDER_STATE_RECORDING and RECORDER_STATE_PAUSE state.
 *
 * @param[in] recorder	The handle to the recorder
 * @param[in] callback	  The callback function to register
 * @param[in] user_data   The user data to be passed to the callback function
 *
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre		The recorder state should be #RECORDER_STATE_READY or #RECORDER_STATE_CREATED.
 *
 * @see recorder_unset_audio_stream_cb()
 * @see recorder_audio_stream_cb()
 */
int recorder_set_audio_stream_cb(recorder_h recorder, recorder_audio_stream_cb callback, void* user_data);

/**
 * @brief	Unregisters the callback function.
 *
 * @param[in]	recorder	The handle to the recorder
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see     recorder_set_audio_stream_cb()
 */
int recorder_unset_audio_stream_cb(recorder_h recorder);


/**
 * @brief  Registers a callback function to be invoked when the recording information changes.
 * @param[in]	recorder    The handle to media recorder
 * @param[in]	callback	The function pointer of user callback
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_recording_status_cb() will be invoked
 * @see recorder_unset_recording_status_cb()
 * @see	recorder_recording_status_cb()
 */
int recorder_set_recording_status_cb(recorder_h recorder, recorder_recording_status_cb callback, void *user_data);


/**
 * @brief Unregisters the callback function.
 * @param[in]	recorder    The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see recorder_set_recording_status_cb()
 */
int recorder_unset_recording_status_cb(recorder_h recorder);


/**
 * @brief  Registers the callback function to run when reached recording limit.
 * @param[in]	recorder	The handle to media recorder
 * @param[in]	callback	The function pointer of user callback
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  recorder_recording_limit_reached_cb() will be invoked
 * @see recorder_unset_recording_limit_reached_cb()
 * @see recorder_attr_set_size_limit()
 * @see recorder_attr_set_time_limit()
 * @see	recorder_recording_limit_reached_cb()
 */
int recorder_set_recording_limit_reached_cb(recorder_h recorder, recorder_recording_limit_reached_cb callback, void *user_data);


/**
 * @brief  Unregisters the callback function.
 * @param[in]	recorder	The handle to media recorder
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_set_recording_limit_reached_cb()
 */
int recorder_unset_recording_limit_reached_cb(recorder_h recorder);


/**
 * @brief	Registers a callback function to be called when an asynchronous operation error occurred.
 *
 * @remarks
 * This callback inform critical error situation.\n
 * When invoked this callback, user should release the resource and terminate application.\n
 * These error code will be occurred\n
 * #RECORDER_ERROR_DEVICE\n
 * #RECORDER_ERROR_INVALID_OPERATION\n
 * #RECORDER_ERROR_OUT_OF_MEMORY\n
 *
 * @param[in]	recorder	The handle to the recorder
 * @param[in]	callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post	This function will invoke recorder_error_cb() when an asynchronous operation error occur.
 *
 * @see recorder_unset_error_cb()
 * @see recorder_error_cb()
 */
int recorder_set_error_cb(recorder_h recorder, recorder_error_cb callback, void *user_data);


/**
 * @brief	Unregisters the callback function.
 *
 * @param[in]	recorder	The handle to the recorder
 * @return	  0 on success, otherwise a negative error value.
 * @retval    #RECORDER_ERROR_NONE Successful
 * @retval    #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see recorder_set_error_cb()
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
 * @brief  Sets maximum size of recording file.
 * @remarks	After reached limitation, recording data is discarded and not written in recording file.
 * @param[in] recorder The handle to media recorder
 * @param[in] kbyte The maximum size of recording file(KB)\n @c 0 means unlimited recording size.
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see recorder_attr_get_size_limit()
 * @see recorder_attr_set_time_limit()
 */
int recorder_attr_set_size_limit(recorder_h recorder, int kbyte);


/**
 * @brief  Gets the maximum size of recording file.
 * @param[in] recorder The handle to media recorder
 * @param[out] kbyte   The maximum size of recording file (KB)\n @c 0 means unlimited recording size
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see recorder_attr_set_size_limit()
 * @see recorder_attr_get_time_limit()
 */
int recorder_attr_get_size_limit(recorder_h recorder, int *kbyte);


/**
 * @brief  Sets time limit of recording file.
 * @remarks	After reached limitation, recording data is discarded and not written in recording file.
 * @param[in] recorder The handle to media recorder
 * @param[in] second   The time limit of recording file (in seconds) \n @c 0 means unlimited recording size
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see recorder_attr_get_time_limit()
 * @see recorder_attr_set_size_limit()
 */
int recorder_attr_set_time_limit(recorder_h recorder, int second);


/**
 * @brief  Gets the time limit of recording file
 * @param[in] recorder  The handle to media recorder
 * @param[out] second   The time limit of recording file (in seconds)\n  @c 0 means unlimited recording time.
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_time_limit()
 * @see	recorder_attr_get_size_limit()
 */
int recorder_attr_get_time_limit(recorder_h recorder, int *second);


/**
 * @brief  Sets audio device for recording.
 * @param[in] recorder The handle to media recorder
 * @param[in] device   The type of audio device
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED
 * @see	recorder_attr_get_audio_device()
 */
int recorder_attr_set_audio_device(recorder_h recorder, recorder_audio_device_e device);


/**
 * @brief Gets the audio device for recording.
 * @param[in] recorder  The handle to media recorder
 * @param[out] device The type of audio device
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_audio_device()
 */
int recorder_attr_get_audio_device(recorder_h recorder, recorder_audio_device_e *device);


/**
 * @brief  Sets sampling rate of audio stream.
 * @param[in] recorder  The handle to media recorder
 * @param[in] samplerate The sample rate in Hertz
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED
 * @see	recorder_attr_get_audio_samplerate()
 */
int recorder_attr_set_audio_samplerate(recorder_h recorder, int samplerate);


/**
 * @brief  Gets the sampling rate of audio stream.
 * @param[in] recorder The handle to media recorder
 * @param[out] samplerate  The sample rate in Hertz
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_audio_samplerate()
 */
int recorder_attr_get_audio_samplerate(recorder_h recorder, int *samplerate);


/**
 * @brief  Sets the bitrate of audio encoder.
 * @param[in] recorder  The handle to media recorder
 * @param[in] bitrate   The bitrate (for mms : 12200[bps], normal : 288000[bps])
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see	recorder_attr_get_audio_encoder_bitrate()
 */
int recorder_attr_set_audio_encoder_bitrate(recorder_h recorder, int bitrate);


/**
 * @brief  Sets bitrate of video encoder.
 * @param[in] recorder  The handle to media recorder
 * @param[in] bitrate   The bitrate in bits per second
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #RECORDER_ERROR_INVALID_STATE Invalid state
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see	recorder_attr_get_video_encoder_bitrate()
 */
int recorder_attr_set_video_encoder_bitrate(recorder_h recorder, int bitrate);


/**
 * @brief  Gets the bitrate of audio encoder.
 * @param[in] recorder  The handle to media recorder
 * @param[out] bitrate  The bitrate in bits per second
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_audio_encoder_bitrate()
 */
int recorder_attr_get_audio_encoder_bitrate(recorder_h recorder, int *bitrate);


/**
 * @brief  Gets the bitrate of video encoder.
 * @param[in] recorder The handle to media recorder
 * @param[out] bitrate The bitrate in bits per second
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_audio_encoder_bitrate()
 */
int recorder_attr_get_video_encoder_bitrate(recorder_h recorder, int *bitrate);



/**
 * @brief  Sets the mute state of recorder
 * @param[in] recorder The handle to media recorder
 * @param[in] enable The mute state
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_is_muted()
 */
int recorder_attr_set_mute(recorder_h recorder, bool enable);

/**
 * @brief  Gets the mute state of recorder
 * @param[in] recorder The handle to media recorder
 * @return	true if the recorder is not recording any sound,\nelse false
 * @see	recorder_attr_set_mute()
 */
bool recorder_attr_is_muted(recorder_h recorder);

/**
 * @brief  Sets recording motion rate
 * @remarks
 * This attribute is valid only in video recorder.\n
 * If the rate bigger than 0 and smaller than 1, video is recorded slow motion mode.\n
 * If the rate bigger than 1, video is recorded fast motion mode(time lapse recording).
 * Audio data is not recorded.\n
 * To reset slow motion recording, setting rate to 1.
 * @param[in] recorder The handle to media recorder
 * @param[in] rate The recording motion rate. it is computed with fps. (0<rate<1 for slow motion, 1<rate for fast motion(time lapse recording), 1 to reset )
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre The recorder state must be #RECORDER_STATE_CREATED or #RECORDER_STATE_READY
 * @see	recorder_attr_get_recording_motion_rate()
 */
int recorder_attr_set_recording_motion_rate(recorder_h recorder , double rate);

/**
 * @brief  Gets the recording motion rate
 * @remarks
 * This attribute is valid only in video recorder.\n
 * If the rate bigger than 0 and smaller than 1, video is recorded slow motion mode.\n
 * If the rate bigger than 1, video is recorded fast motion mode(time lapse recording).
 * Audio data is not recorded.\n
 * To reset slow motion recording, setting rate to 1.
 * @param[in] recorder The handle to media recorder
 * @param[out] rate The recording motion rate. it is computed with fps.
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_recording_motion_rate()
 */
int recorder_attr_get_recording_motion_rate(recorder_h recorder , double *rate);

__attribute__ ((deprecated)) int recorder_attr_set_slow_motion_rate(recorder_h recorder , double rate);
__attribute__ ((deprecated)) int recorder_attr_get_slow_motion_rate(recorder_h recorder , double *rate);

/**
 * @brief  Sets the numer of audio channel.
 * @remarks This attribute is applied only in RECORDER_STATE_CREATED state.\n
 * For mono recording, setting channel to 1.\n
 * For stereo recording, setting channel to 2.
 * @param[in] recorder  The handle to media recorder
 * @param[in] channel_count The number of audio channel
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre The recorder state must be #RECORDER_STATE_CREATED
 * @see	recorder_attr_get_audio_channel()
 */
int recorder_attr_set_audio_channel(recorder_h recorder, int channel_count);

/**
 * @brief  Gets the numer of audio channel.
 * @param[in] recorder  The handle to media recorder
 * @param[out] channel_count The number of audio channel
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_audio_channel()
 */
int recorder_attr_get_audio_channel(recorder_h recorder, int *channel_count);

/**
 * @brief Sets the orientation of video recording data
 * @remarks
 * This attribute is valid only in video recorder.\n
 * This attribute is applied in video encoding.
 * @param[in] recorder  The handle to media recorder
 * @param[in] orientation The orientation of video recording data
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_get_recording_orientation()
 */
int recorder_attr_set_recording_orientation(recorder_h recorder, recorder_rotation_e orientation);

/**
 * @brief Gets the orientation of video recording data
 * @remarks
 * This attribute is valid only in video recorder.\n
 * @param[in] recorder  The handle to media recorder
 * @param[out] orientation The orientation of video recording data
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_recording_orientation()
 */
int recorder_attr_get_recording_orientation(recorder_h recorder, recorder_rotation_e *orientation);

/**
 * @brief Sets the flip of video recording data
 * @remarks
 * This attribute is valid only in video recorder.\n
 * This attribute is applied in video encoding.
 * @param[in] recorder  The handle to media recorder
 * @param[in] flip The flip of video recording data
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_get_recording_flip()
 */
int recorder_attr_set_recording_flip(recorder_h recorder, recorder_flip_e flip);

/**
 * @brief Gets the flip of video recording data
 * @remarks
 * This attribute is valid only in video recorder.\n
 * This attribute is applied in video encoding.
 * @param[in] recorder  The handle to media recorder
 * @param[out] flip The flip of video recording data
 * @return	0 on success, otherwise a negative error value.
 * @retval #RECORDER_ERROR_NONE Successful
 * @retval #RECORDER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	recorder_attr_set_recording_flip()
 */
int recorder_attr_get_recording_flip(recorder_h recorder, recorder_flip_e *flip);

/**
 * @brief Sets the camera orientation in video metadata tag.
 *
 * @param[in]	camera	The handle to the camera
 * @param[in]	orientation The information of the video orientation
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CAMERA_ERROR_NONE Successful
 * @retval      #CAMERA_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see	recorder_attr_get_orientation_tag()
 */
int recorder_attr_set_orientation_tag(recorder_h recorder,  recorder_rotation_e orientation);

/**
 * @brief Gets the camera orientation in video metadata tag.
 *
 * @param[in]	camera	The handle to the camera
 * @param[out]  orientation The information of the video orientation
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CAMERA_ERROR_NONE Successful
 * @retval      #CAMERA_ERROR_INVALID_PARAMETER Invalid parameter
 *
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

