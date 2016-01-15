/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __TIZEN_MEDIA_RECORDER_DOC_H__
#define __TIZEN_MEDIA_RECORDER_DOC_H__


/**
 * @file recorder_doc.h
 * @brief This file contains high level documentation of the recorder API.
 */


/**
 * @ingroup CAPI_MEDIA_FRAMEWORK
 * @defgroup CAPI_MEDIA_RECORDER_MODULE Recorder
 * @brief The @ref CAPI_MEDIA_RECORDER_MODULE API provides functions for audio and video recording.
 *
 * @section CAPI_MEDIA_RECORDER_MODULE_HEADER Required Header
 *   \#include <recorder.h>
 *
 *
 * @section CAPI_MEDIA_RECORDER_MODULE_OVERVIEW Overview
 * The Recorder API provides functions to control the recording of a multimedia content. Simple audio and audio/video are supported.
 * Recording operations operate as a state machine, described below.
 *
 * In addition, the API contains functions to configure the recording process, or find details about it, such as getting/setting the filename
 * for the recording, the file format and the video codec. Some of these interfaces are listed in the Recorder Attributes API.
 *
 * Additional functions allow registering notifications via callback functions for various state change events.
 *
 * @subsection CAPI_MEDIA_RECORDER_MODULE_LIFE_CYCLE_STATE_DIAGRAM State Diagram
 * @image html capi_media_recorder_state_diagram.png
 *
 * @subsection CAPI_MEDIA_RECORDER_MODULE_LIFE_CYCLE_STATE_TRANSITIONS State Transitions
 * <div><table class="doxtable" >
 * <tr>
 *    <th><B>FUNCTION</B></th>
 *    <th><B>PRE-STATE</B></th>
 *    <th><B>POST-STATE</B></th>
 *    <th><B>SYNC TYPE</B></th>
 * </tr>
 * <tr>
 *    <td> recorder_create_videorecorder()<br>recorder_create_audiorecorder() </td>
 *    <td> NONE </td>
 *    <td> CREATED </td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_destroy() </td>
 *    <td> CREATED </td>
 *    <td> NONE </td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_prepare() </td>
 *    <td> CREATED </td>
 *    <td> READY</td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_unprepare() </td>
 *    <td> READY </td>
 *    <td> CREATED </td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_start() </td>
 *    <td> READY / PAUSED </td>
 *    <td> RECORDING </td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_pause() </td>
 *    <td> RECORDING </td>
 *    <td> PAUSED </td>
 *    <td> ASYNC </td>
 * </tr>
 * <tr>
 *    <td> recorder_cancel() <br>recorder_commit() </td>
 *    <td> RECORDING / PAUSED </td>
 *    <td> READY </td>
 *    <td> ASYNC </td>
 * </tr>
 * </table>
 * </div>
 *
 * @subsection CAPI_MEDIA_RECORDER_MODULE_LIFE_CYCLE_CALLBACK_OPERATIONS  Callback(Event) Operations
 * The callback mechanism is used to notify the application about significant recorder events.
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b> REGISTER</b></th>
 *        <th><b> UNREGISTER</b></th>
 *        <th><b> CALLBACK</b></th>
 *        <th><b> DESCRIPTION</b></th>
 *     </tr>
 *     <tr>
 *        <td> recorder_set_recording_limit_reached_cb()</td>
 *        <td> recorder_unset_recording_limit_reached_cb()</td>
 *        <td> recorder_recording_limit_reached_cb()</td>
 *        <td> This callback is called when recording limitation error has occurred during recording.</td>
 *     </tr>
 *     <tr>
 *        <td> recorder_set_recording_status_cb()</td>
 *        <td> recorder_unset_recording_status_cb()</td>
 *        <td> recorder_recording_status_cb()</td>
 *        <td> This callback is used to notify the recording status.</td>
 *     </tr>
 *     <tr>
 *        <td> recorder_set_state_changed_cb()</td>
 *        <td> recorder_unset_state_changed_cb()</td>
 *        <td> recorder_state_changed_cb()</td>
 *        <td> This callback is used to notify the change of recorder's state.</td>
 *     </tr>
 * </table></div>
 *
 * @subsection CAPI_MEDIA_RECORDER_MODULE_FOREACH_OPERATIONS Foreach Operations
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>FOREACH</b></th>
 *        <th><b>CALLBACK</b></th>
 *        <th><b>DESCRIPTION</b></th>
 *     </tr>
 *     <tr>
 *        <td>recorder_foreach_supported_file_format()</td>
 *        <td>recorder_supported_file_format_cb()</td>
 *        <td>Supported file format </td>
 *     </tr>
 *     <tr>
 *        <td>recorder_foreach_supported_audio_encoder()</td>
 *        <td>recorder_supported_audio_encoder_cb()</td>
 *        <td>Supported audio encoder</td>
 *     </tr>
 *     <tr>
 *        <td>recorder_foreach_supported_video_encoder()</td>
 *        <td>recorder_supported_video_encoder_cb()</td>
 *        <td>Supported video encoder</td>
 *     </tr>
 *</table></div>
 *
 * @section CAPI_MEDIA_RECORDER_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/camera\n
 *  - http://tizen.org/feature/microphone
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 */


/**
 * @ingroup CAPI_MEDIA_RECORDER_MODULE
 * @defgroup CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE Attributes
 * @brief The @ref CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE API provides functions for getting and setting recorder attributes.
 *
 * @section CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE_HEADER Required Header
 *   \#include <recorder.h>
 *
 * @section CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE_OVERVIEW Overview
 * The Media Recorder API provides basic recorder attribute manipulators.
 *
 * The Recorder Attributes API provides functions to set and get basic recorder attributes:
 * <ul>
 *   <li>File size limit</li>
 *   <li>Recording time limit</li>
 *   <li>Audio recording device</li>
 *   <li>Audio sample rate</li>
 *   <li>Audio encoder bit rate</li>
 *   <li>Video encoder bit rate</li>
 * </ul>
 *
 * Each of these attributes have a get/set pair of functions. For example, recorder_attr_set_time_limit() and recorder_attr_get_time_limit().
 * For more detailed information and programming examples for this API, see the Multimedia Tutorial.
 *
 * @section CAPI_MEDIA_RECORDER_ATTRIBUTES_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/camera\n
 *  - http://tizen.org/feature/microphone
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 */

 /**
 * @ingroup CAPI_MEDIA_RECORDER_MODULE
 * @defgroup CAPI_MEDIA_RECORDER_CAPABILITY_MODULE Capability
 * @brief It provides capability information of the media recorder.
 * @section CAPI_MEDIA_RECORDER_CAPABILITY_MODULE_HEADER Required Header
 *   \#include <recorder.h>
 *
 * @section CAPI_MEDIA_RECORDER_CAPABILITY_MODULE_OVERVIEW  Overview
 * The Capability API allows you to retrieve the recorder capabilities mentioned below:
 * <ul>
 *   <li>Supported file formats</li>
 *   <li>Supported audio and video encoders</li>
 * </ul>
 * To get all supported file formats, call recorder_foreach_supported_file_format().
 * This function will internally invoke recorder_supported_file_format_cb() for each file format.
 *
 * @section CAPI_MEDIA_RECORDER_CAPABILITY_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/camera\n
 *  - http://tizen.org/feature/microphone
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 */

#endif /* __TIZEN_MEDIA_RECORDER_DOC_H__ */
