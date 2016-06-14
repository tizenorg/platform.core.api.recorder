/*
 * recorder_testsuite
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jeongmo Yang <jm80.yang@samsung.com>
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
 *
 */

/*=======================================================================================
  |  INCLUDE FILES                                                                        |
  =======================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <sys/time.h>
#include <camera.h>
#include <recorder.h>
#include <dlog.h>
#include <Ecore.h>
#include <Elementary.h>
#include <sound_manager.h>

/*-----------------------------------------------------------------------
  |    GLOBAL VARIABLE DEFINITIONS:                                       |
  -----------------------------------------------------------------------*/
#define EXPORT_API __attribute__((__visibility__("default")))

#define PACKAGE "recorder_testsuite"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RECORDER_TESTSUITE"

GMainLoop *g_loop;
GIOChannel *stdin_channel;
int resolution_set;
int g_current_state;
int src_w, src_h;
bool isMultishot;
int  cam_info;
int recorder_state;
int multishot_num;
static GTimer *timer = NULL;
void *display;

/*-----------------------------------------------------------------------
  |    GLOBAL CONSTANT DEFINITIONS:                                       |
  -----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
  |    IMPORTED VARIABLE DECLARATIONS:                                    |
  -----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
  |    IMPORTED FUNCTION DECLARATIONS:                                    |
  -----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
  |    LOCAL #defines:                                                    |
  -----------------------------------------------------------------------*/


#define DISPLAY_W_320                       320                 /* for direct FB */
#define DISPLAY_H_240                       240                 /* for direct FB */


#define SRC_VIDEO_FRAME_RATE_15         15    /* video input frame rate */
#define SRC_VIDEO_FRAME_RATE_30         30    /* video input frame rate */
#define IMAGE_ENC_QUALITY               85    /* quality of jpeg */

#define MAX_FILE_SIZE_FOR_MMS           (250 * 1024)

#define EXT_JPEG                        "jpg"
#define EXT_MP4                         "mp4"
#define EXT_3GP                         "3gp"
#define EXT_AMR                         "amr"
#define EXT_MKV                         "mkv"

#define TARGET_FILENAME_PATH            "/home/owner/content/Sounds/"
#define IMAGE_CAPTURE_EXIF_PATH         TARGET_FILENAME_PATH"exif.raw"
#define TARGET_FILENAME_VIDEO           TARGET_FILENAME_PATH"test_rec_video.mp4"
#define TARGET_FILENAME_AUDIO           TARGET_FILENAME_PATH"test_rec_audio.m4a"
#define CAPTURE_FILENAME_LEN            256

#define AUDIO_SOURCE_SAMPLERATE_AAC     44100
#define AUDIO_SOURCE_SAMPLERATE_AMR     8000
#define AUDIO_SOURCE_CHANNEL_AAC        2
#define AUDIO_SOURCE_CHANNEL_AMR        1
#define VIDEO_ENCODE_BITRATE            40000000 /* bps */

#define CHECK_MM_ERROR(expr) \
	do {\
		int ret = 0; \
		ret = expr; \
		if (ret != 0) {\
			printf("[%s:%d] error code : %x \n", __func__, __LINE__, ret); \
			return; \
		} \
	} while (0)


#ifndef SAFE_FREE
#define SAFE_FREE(x)       if (x) { g_free(x); x = NULL; }
#endif


GTimeVal previous;
GTimeVal current;
GTimeVal res;
/* temp */

/**
 * Enumerations for command
 */
#define SENSOR_WHITEBALANCE_NUM     10
#define SENSOR_COLOR_TONE_NUM       31
#define SENSOR_FLIP_NUM         3
#define SENSOR_PROGRAM_MODE_NUM     15
#define SENSOR_FOCUS_NUM        6
#define SENSOR_INPUT_ROTATION       4
#define SENSOR_AF_SCAN_NUM      4
#define SENSOR_ISO_NUM          8
#define SENSOR_EXPOSURE_NUM     9
#define SENSOR_IMAGE_FORMAT     9


/*-----------------------------------------------------------------------
  |    LOCAL CONSTANT DEFINITIONS:                                        |
  -----------------------------------------------------------------------*/
enum {
	MODE_VIDEO_CAPTURE, /* recording and image capture mode */
	MODE_AUDIO,     /* audio recording*/
	MODE_NUM,
};

enum {
	MENU_STATE_MAIN,
	MENU_STATE_SETTING,
	MENU_STATE_NUM,
};

/*-----------------------------------------------------------------------
  |    LOCAL DATA TYPE DEFINITIONS:                   |
  -----------------------------------------------------------------------*/
typedef struct _cam_handle {
	camera_h camera;
	recorder_h recorder;
	int mode;                       /*video/audio(recording) mode */
	int menu_state;
	int fps;
	bool isMute;
	unsigned long long elapsed_time;
} cam_handle_t;

typedef struct _cam_xypair {
	const char *attr_subcat_x;
	const char *attr_subcat_y;
	int x;
	int y;
} cam_xypair_t;

typedef struct {
	int width[100];
	int height[100];
	int count;
} resolution_stack;

typedef struct {
	camera_attr_exposure_mode_e mode;
	int count;
} exposure_stack;

typedef struct {
	camera_attr_iso_e mode;
	int count;
} iso_stack;

typedef struct {
	camera_rotation_e mode;
	int count;
} camera_rotation_stack;

/*---------------------------------------------------------------------------
  |    LOCAL VARIABLE DEFINITIONS:                                            |
  ---------------------------------------------------------------------------*/
static cam_handle_t *hcamcorder ;

const char *wb[SENSOR_WHITEBALANCE_NUM] = {
	"None",
	"Auto",
	"Daylight",
	"Cloudy",
	"Fluoroscent",
	"Incandescent",
	"Shade",
	"Horizon",
	"Flash",
	"Custom",
};

const char *ct[SENSOR_COLOR_TONE_NUM] = {
	"NONE",
	"MONO",
	"SEPIA",
	"NEGATIVE",
	"BLUE",
	"GREEN",
	"AQUA",
	"VIOLET",
	"ORANGE",
	"GRAY",
	"RED",
	"ANTIQUE",
	"WARM",
	"PINK",
	"YELLOW",
	"PURPLE",
	"EMBOSS",
	"OUTLINE",
	"SOLARIZATION",
	"SKETCH",
	"WASHED",
	"VINTAGE_WARM",
	"VINTAGE_COLD",
	"POSTERIZATION",
	"CARTOON",
	"SELECTVE_COLOR_RED",
	"SELECTVE_COLOR_GREEN",
	"SELECTVE_COLOR_BLUE",
	"SELECTVE_COLOR_YELLOW",
	"SELECTVE_COLOR_RED_YELLOW",
	"GRAPHICS"
};

const char *flip[SENSOR_FLIP_NUM] = {
	"Horizontal",
	"Vertical",
	"Not flipped",
};

const char *program_mode[SENSOR_PROGRAM_MODE_NUM] = {
	"NORMAL",
	"PORTRAIT",
	"LANDSCAPE",
	"SPORTS",
	"PARTY_N_INDOOR",
	"BEACH_N_INDOOR",
	"SUNSET",
	"DUSK_N_DAWN",
	"FALL_COLOR",
	"NIGHT_SCENE",
	"FIREWORK",
	"TEXT",
	"SHOW_WINDOW",
	"CANDLE_LIGHT",
	"BACKLIGHT",
};

const char *focus_mode[SENSOR_FOCUS_NUM] = {
	"None",
	"Pan",
	"Auto",
	"Manual",
	"Touch Auto",
	"Continuous Auto",
};

const char *camera_rotation[SENSOR_INPUT_ROTATION] = {
	"None",
	"90",
	"180",
	"270",
};

const char *af_scan[SENSOR_AF_SCAN_NUM] = {
	"None",
	"Normal",
	"Macro mode",
	"Full mode",
};

const char *iso_mode[SENSOR_ISO_NUM] = {
	"ISO Auto",
	"ISO 50",
	"ISO 100",
	"ISO 200",
	"ISO 400",
	"ISO 800",
	"ISO 1600",
	"ISO 3200",
};

const char *exposure_mode[SENSOR_EXPOSURE_NUM] = {
	"AE off",
	"AE all mode",
	"AE center 1 mode",
	"AE center 2 mode",
	"AE center 3 mode",
	"AE spot 1 mode",
	"AE spot 2 mode",
	"AE custom 1 mode",
	"AE custom 2 mode",
};

const char *image_fmt[SENSOR_IMAGE_FORMAT] = {
	"NV12",
	"NV12T",
	"NV16",
	"NV21",
	"YUYV",
	"UYVY",
	"422P",
	"I420",
	"YV12",
};

const char *face_zoom_mode[] = {
	"Face Zoom OFF",
	"Face Zoom ON",
};

const char *display_mode[] = {
	"Default",
	"Primary Video ON and Secondary Video Full Screen",
	"Primary Video OFF and Secondary Video Full Screen",
};

const char *output_mode[] = {
	"Letter Box mode",
	"Original Size mode",
	"Full Screen mode",
	"Cropped Full Screen mode",
	"ROI mode",
};

const char *capture_sound[] = {
	"Default",
	"Extra 01",
	"Extra 02",
};

const char *rotate_mode[] = {
	"0",
	"90",
	"180",
	"270",
};

const char* strobe_mode[] = {
	"Strobe OFF",
	"Strobe ON",
	"Strobe Auto",
	"Strobe RedEyeReduction",
	"Strobe SlowSync",
	"Strobe FrontCurtain",
	"Strobe RearCurtain",
	"Strobe Permanent",
};

const char *detection_mode[2] = {
	"Face Detection OFF",
	"Face Detection ON",
};

const char *wdr_mode[] = {
	"WDR OFF",
	"WDR ON",
	"WDR AUTO",
};

const char *hdr_mode[] = {
	"HDR OFF",
	"HDR ON",
	"HDR ON and Original",
};

const char *ahs_mode[] = {
	"Anti-handshake OFF",
	"Anti-handshake ON",
	"Anti-handshake AUTO",
	"Anti-handshake MOVIE",
};

const char *vs_mode[] = {
	"Video-stabilization OFF",
	"Video-stabilization ON",
};

const char *visible_mode[] = {
	"Display OFF",
	"Display ON",
};


/*---------------------------------------------------------------------------
  |    LOCAL FUNCTION PROTOTYPES:                                             |
  ---------------------------------------------------------------------------*/
static void print_menu();
static gboolean cmd_input(GIOChannel *channel);
static gboolean init(int type);
static gboolean mode_change();


bool preview_resolution_cb(int width, int height, void *user_data)
{
	resolution_stack *data = (resolution_stack*)user_data;

	if (data == NULL) {
		printf("NULL data\n");
		return false;
	}

	data->width[data->count] = width;
	data->height[data->count] = height;

	printf("%d. %dx%d\n", data->count, width, height);

	data->count++;

	return true;
}

bool af_mode_foreach_cb(camera_attr_iso_e mode , void *user_data)
{
	printf("%d.%s\n", mode, af_scan[mode]);
	return true;
}

bool exposure_mode_cb(camera_attr_af_mode_e mode , void *user_data)
{
	exposure_stack *data = (exposure_stack*)user_data;

	if (data == NULL) {
		printf("NULL data\n");
		return false;
	}

	data->mode = mode;
	data->count++;

	printf("%d.%s\n", mode, exposure_mode[mode]);
	return true;
}

bool iso_mode_cb(camera_attr_iso_e mode , void *user_data)
{
	printf("%d.%s\n", mode, iso_mode[mode]);
	return true;
}

bool camera_rotation_cb(camera_rotation_e mode , void *user_data)
{
	camera_rotation_stack *data = (camera_rotation_stack*)user_data;

	if (data == NULL) {
		printf("NULL data\n");
		return false;
	}

	data->mode = mode;
	data->count++;

	printf("%d.%s\n", mode, camera_rotation[mode]);
	return true;
}

bool preview_format_cb(camera_pixel_format_e mode , void *user_data)
{
	printf("%d.%s\n", mode, image_fmt[mode]);
	return true;
}

bool white_balance_cb(camera_attr_whitebalance_e mode , void *user_data)
{
	printf("%d.%s\n", mode, wb[mode]);
	return true;
}

bool colortone_cb(camera_attr_effect_mode_e mode , void *user_data)
{
	printf("%d.%s\n", mode, ct[mode]);
	return true;
}

bool program_mode_cb(camera_attr_scene_mode_e mode , void *user_data)
{
	printf("%d.%s\n", mode, program_mode[mode]);
	return true;
}

bool strobe_mode_cb(camera_attr_flash_mode_e mode , void *user_data)
{
	printf("%d.%s\n", mode, strobe_mode[mode]);
	return true;
}

void _face_detected(camera_detected_face_s *faces, int count, void *user_data)
{
	printf("face detected!!\n");
	int i;
	for (i = 0 ; i < count ; i++)
		printf("%d) - %dx%d\n", faces[i].id, faces[i].x, faces[i].y);
}

void _state_changed_cb(recorder_state_e previous_state, recorder_state_e current_state, bool by_policy, void *user_data)
{
	printf("\n\tstate changed[by_policy:%d] : %d -> %d\n\n", by_policy, previous_state, current_state);
}

void _recording_status_cb(unsigned long long elapsed_time, unsigned long long file_size, void *user_data)
{
	static unsigned long long elapsed = -1;
	if (elapsed != elapsed_time / 1000) {
		unsigned long temp_time;
		unsigned long long hour, minute, second;
		elapsed = elapsed_time / 1000;
		temp_time = elapsed;
		hour = temp_time / 3600;
		temp_time = elapsed % 3600;
		minute = temp_time / 60;
		second = temp_time % 60;
		printf("\n\tCurrent Time - %lld:%lld:%lld, filesize %lld KB\n\n",
				hour, minute, second, file_size);
	}
}

void _recording_limit_reached_cb(recorder_recording_limit_type_e type, void *user_data)
{
	printf("\n\tRECORDING LIMIT REACHED [type: %d]\n\n", type);
}


static inline void flush_stdin()
{
	int ch;
	while ((ch = getchar()) != EOF && ch != '\n');
}


static void print_menu()
{
	switch (hcamcorder->menu_state)	{
	case MENU_STATE_MAIN:
		if (hcamcorder->mode == MODE_VIDEO_CAPTURE) {
			g_print("\n\t=======================================\n");
			if (recorder_state <= RECORDER_STATE_NONE) {
				g_print("\t   '1' Start Recording\n");
				g_print("\t   '2' Setting\n");
				g_print("\t   'b' back\n");
			} else if (recorder_state == RECORDER_STATE_RECORDING) {
				g_print("\t   'p' Pause Recording\n");
				g_print("\t   'c' Cancel\n");
				g_print("\t   's' Save\n");
			} else if (recorder_state == RECORDER_STATE_PAUSED) {
				g_print("\t   'r' Resume Recording\n");
				g_print("\t   'c' Cancel\n");
				g_print("\t   's' Save\n");
			}
			g_print("\t=======================================\n");
		} else if (hcamcorder->mode == MODE_AUDIO) {
			g_print("\n\t=======================================\n");
			g_print("\t   Audio Recording\n");
			g_print("\t=======================================\n");
			if (recorder_state <= RECORDER_STATE_NONE) {
				g_print("\t   '1' Start Recording\n");
				g_print("\t   'b' back\n");
			} else if (recorder_state == RECORDER_STATE_RECORDING) {
				g_print("\t   'p' Pause Recording\n");
				g_print("\t   'c' Cancel\n");
				g_print("\t   's' Save\n");
			} else if (recorder_state == RECORDER_STATE_PAUSED) {
				g_print("\t   'r' Resume Recording\n");
				g_print("\t   'c' Cancel\n");
				g_print("\t   's' Save\n");
			}
			g_print("\t=======================================\n");
		}
		break;

	case MENU_STATE_SETTING:
		g_print("\n\t=======================================\n");
		g_print("\t   Video Capture > Setting\n");
		g_print("\t=======================================\n");
		g_print("\t  >>>>>>>>>>>>>>>>>>>>>>>>>>>> [Camera]  \n");
		g_print("\t     '0' Preview resolution \n");
		g_print("\t     '2' Digital zoom level \n");
		g_print("\t     '3' Optical zoom level \n");
		g_print("\t     '4' AF mode \n");
		g_print("\t     '5' AF scan range \n");
		g_print("\t     '6' Exposure mode \n");
		g_print("\t     '7' Exposure value \n");
		g_print("\t     '8' F number \n");
		g_print("\t     '9' Shutter speed \n");
		g_print("\t     'i' ISO \n");
		g_print("\t     'r' Rotate camera input \n");
		g_print("\t     'f' Flip camera input \n");
		g_print("\t  >>>>>>>>>>>>>>>>>>>> [Display/Filter]\n");
		g_print("\t     'v' Visible \n");
		g_print("\t     'n' Display mode \n");
		g_print("\t     'o' Output mode \n");
		g_print("\t     'y' Rotate display \n");
		g_print("\t     'Y' Flip display \n");
		g_print("\t     'g' Brightness \n");
		g_print("\t     'c' Contrast \n");
		g_print("\t     's' Saturation \n");
		g_print("\t     'h' Hue \n");
		g_print("\t     'a' Sharpness \n");
		g_print("\t     'w' White balance \n");
		g_print("\t     't' Color tone \n");
		g_print("\t     'd' WDR \n");
		g_print("\t     'e' EV program mode \n");
		g_print("\t  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> [etc.]\n");
		g_print("\t     'z' Strobe (Flash) \n");
		g_print("\t     'l' Face detection \n");
		g_print("\t     'k' Anti-handshake \n");
		g_print("\t     'K' Video-stabilization \n");
		g_print("\t     'u' Touch AF area \n");;
		g_print("\t     'M' Camcorder Motion Rate setting \n");
		g_print("\t     'b' back\n");
		g_print("\t=======================================\n");
		break;

	default:
		LOGE("unknow menu state !!\n");
		break;
	}

	return;
}

static void main_menu(gchar buf)
{
	int err = 0;

	if (hcamcorder->mode == MODE_VIDEO_CAPTURE) {
		if (recorder_state == RECORDER_STATE_NONE) {
			switch (buf) {
			case '1': /* Start Recording */
				g_print("*Recording start!\n");
				hcamcorder->elapsed_time = 0;

				g_timer_reset(timer);
				err = recorder_start(hcamcorder->recorder);

				if (err != 0)
					LOGE("Rec start camcorder_record 0x%x", err);

				recorder_state = RECORDER_STATE_RECORDING;
				break;

			case '2': /* Setting */
				hcamcorder->menu_state = MENU_STATE_SETTING;
				break;

			case 'b': /* back */
				hcamcorder->menu_state = MENU_STATE_MAIN;
				mode_change();
				break;

			default:
				g_print("\t Invalid input \n");
				break;
			}
		} else if (recorder_state == RECORDER_STATE_RECORDING || recorder_state == RECORDER_STATE_PAUSED) {
			switch (buf) {
			case 'p': /* Pause Recording */
				g_print("*Pause!\n");
				err = recorder_pause(hcamcorder->recorder);

				if (err < 0)
					LOGE("Rec pause camcorder_pause  = %x", err);

				recorder_state = RECORDER_STATE_PAUSED;
				break;

			case 'r': /* Resume Recording */
				g_print("*Resume!\n");
				err = recorder_start(hcamcorder->recorder);
				if (err < 0)
					LOGE("Rec start camcorder_record  = %x", err);

				recorder_state = RECORDER_STATE_RECORDING;
				break;

			case 'c': /* Cancel */
				g_print("*Cancel Recording !\n");

				err = recorder_cancel(hcamcorder->recorder);

				if (err < 0)
					LOGE("Cancel recording camcorder_cancel  = %x", err);

				recorder_state = RECORDER_STATE_NONE;
				break;

			case 's': /* Save */
				g_print("*Save Recording!\n");
				g_timer_reset(timer);

				err = recorder_commit(hcamcorder->recorder);

				if (err < 0)
					LOGE("Save recording recorder_commit  = %x", err);

				recorder_state = RECORDER_STATE_NONE;
				break;

			default:
				g_print("\t Invalid input \n");
				break;
			} /* switch */
		} else {
			LOGE("Wrong camcorder state, check status!!");
		}
	} else if (hcamcorder->mode == MODE_AUDIO) {
		if (recorder_state == RECORDER_STATE_NONE) {
			switch (buf) {
			case '1': /*  Start Recording */
				g_print("*Recording start!\n");
				hcamcorder->elapsed_time = 0;
				g_timer_reset(timer);
				err = recorder_start(hcamcorder->recorder);

				if (err < 0)
					LOGE("Rec start camcorder_record  = %x", err);

				recorder_state = RECORDER_STATE_RECORDING;
				break;

			case 'b': /* back */
				hcamcorder->menu_state = MENU_STATE_MAIN;
				mode_change();
				break;

			default:
				g_print("\t Invalid input \n");
				break;
			}
		} else if (recorder_state == RECORDER_STATE_RECORDING || recorder_state == RECORDER_STATE_PAUSED) {
			switch (buf) {
			case 'p': /* Pause Recording */
				g_print("*Pause!\n");
				err = recorder_pause(hcamcorder->recorder);

				if (err < 0)
					LOGE("Rec pause camcorder_pause  = %x", err);

				recorder_state = RECORDER_STATE_PAUSED;
				break;

			case 'r': /* Resume Recording */
				g_print("*Resume!\n");
				err = recorder_start(hcamcorder->recorder);
				if (err < 0)
					LOGE("Rec start camcorder_record  = %x", err);

				recorder_state = RECORDER_STATE_RECORDING;
				break;

			case 'c': /* Cancel */
				g_print("*Cancel Recording !\n");
				err = recorder_cancel(hcamcorder->recorder);

				if (err < 0)
					LOGE("Cancel recording camcorder_cancel  = %x", err);

				recorder_state = RECORDER_STATE_NONE;
				break;

			case 's': /*  Save */
				g_print("*Save Recording!\n");
				g_timer_reset(timer);
				err = recorder_commit(hcamcorder->recorder);

				if (err < 0)
					LOGE("Save recording recorder_commit  = %x", err);

				recorder_state = RECORDER_STATE_NONE;
				break;

			default:
			g_print("\t Invalid input \n");
			break;
			}
		} else {
			LOGE("Wrong camcorder state, check status!!");
		}
	} else {
		g_print("\t Invalid mode, back to upper menu \n");
		hcamcorder->menu_state = MENU_STATE_MAIN;
		mode_change();
	}
}


static void setting_menu(gchar buf)
{
	gboolean bret = FALSE;
	int idx = 0;
	int min = 0;
	int max = 0;
	int value = 0;
	int err = 0;
	int x = 0, y = 0, width = 0, height = 0;

	if (hcamcorder->mode == MODE_VIDEO_CAPTURE) {
		switch (buf) {
		/* Camera setting */
		case '0':  /* Setting > Preview Resolution setting */
			g_print("*Select the preview resolution!\n");
			resolution_stack resolution_list;
			resolution_list.count = 0;

			recorder_foreach_supported_video_resolution(hcamcorder->recorder,
				preview_resolution_cb, &resolution_list);

			flush_stdin();
			err = scanf("%d", &idx);
			int result = 0;
			if (resolution_list.count > idx && idx >= 0) {
				printf("-----------------PREVIEW RESOLUTION (%dx%d)---------------------\n",
					resolution_list.width[idx], resolution_list.height[idx]);

				result = recorder_set_video_resolution(hcamcorder->recorder,
					resolution_list.width[idx], resolution_list.height[idx]);
			} else {
				printf("invalid input %d\n", idx);
				result = -1;
			}

			resolution_list.count = 0;

			if (result != 0)
				printf("FAIL\n");
			else
				printf("PASS\n");
			break;

		case '2': /* Setting > Digital zoom level */
			g_print("*Digital zoom level  !\n");

			camera_attr_get_zoom_range(hcamcorder->camera, &min, &max);
			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  Digital zoom level min %d - max %d\n", min, max);
				err = scanf("%d", &idx);
				bret = camera_attr_set_zoom(hcamcorder->camera, idx);
			}
			break;


		case '3': /* Setting > AF mode */

			g_print("*AF mode !\n");
			g_print("\t1. AF Start !\n");
			g_print("\t2. AF Stop !\n\n");

			flush_stdin();
			err = scanf("%d", &idx);
			switch (idx) {
			case 1:
				camera_start_focusing(hcamcorder->camera, 0);
				break;
			case 2:
				camera_cancel_focusing(hcamcorder->camera);
				break;
			default:
				g_print("Wrong Input[%d] !! \n", idx);
				break;
			}

			break;

		case '4': /* Setting > AF scan range */
			g_print("*AF scan range !\n");

			camera_attr_foreach_supported_af_mode(hcamcorder->camera,
				(camera_attr_supported_af_mode_cb)af_mode_foreach_cb, NULL);

			flush_stdin();
			err = scanf("%d", &idx);
			bret = camera_attr_set_af_mode(hcamcorder->camera, idx);
			break;

		case '5': /* Setting > Exposure mode */
			g_print("* Exposure mode!\n");

			camera_attr_foreach_supported_exposure_mode(hcamcorder->camera,
				(camera_attr_supported_exposure_mode_cb)exposure_mode_cb, NULL);

			flush_stdin();

			g_print("\n Select  Exposure mode \n");
			err = scanf("%d", &idx);
			bret = camera_attr_set_exposure_mode(hcamcorder->camera, idx);

			break;

		case '6':  /* Setting > Exposure value */
			camera_attr_get_exposure_range(hcamcorder->camera, &min, &max);
			if (min >= max) {
				g_print("Not supported !! \n");
			} else {

				flush_stdin();
				g_print("\n Select  Exposure mode min%d -max %d\n", min, max);
				err = scanf("%d", &idx);
				bret = camera_attr_set_exposure(hcamcorder->camera, idx);
			}
			break;

		case '7': /* Setting > F number */
			g_print("Not supported !! \n");
			break;

		case 'i': /* Setting > ISO */
			g_print("*ISO !\n");
			camera_attr_foreach_supported_iso(hcamcorder->camera, iso_mode_cb, NULL);

			flush_stdin();

			err = scanf("%d", &idx);
			bret =  camera_attr_set_iso(hcamcorder->camera, idx);
			break;

		case 'r': /* Setting > Rotate camera input when recording */
			g_print("*Rotate camera input\n");
			camera_attr_foreach_supported_stream_rotation(hcamcorder->camera, camera_rotation_cb, NULL);

			flush_stdin();
			err = scanf("%d", &idx);
			CHECK_MM_ERROR(camera_stop_preview(hcamcorder->camera));
			bret = camera_attr_set_stream_rotation(hcamcorder->camera, idx);
			CHECK_MM_ERROR(camera_start_preview(hcamcorder->camera));
			break;

		case 'f': /* Setting > Flip camera input */
			flush_stdin();
			g_print("*Flip camera input\n");
			g_print(" 0. Flip NONE\n");
			g_print(" 1. Flip HORIZONTAL\n");
			g_print(" 2. Flip VERTICAL\n");
			g_print(" 3. Flip BOTH\n");

			err = scanf("%d", &idx);

			CHECK_MM_ERROR(camera_stop_preview(hcamcorder->camera));
			camera_attr_set_stream_flip(hcamcorder->camera, idx);
			CHECK_MM_ERROR(camera_start_preview(hcamcorder->camera));
			break;

			/* Display / Filter setting */
		case 'v': /* Display visible */
			g_print("* Display visible setting !\n");

			g_print("\n Select Display visible \n");
			flush_stdin();
			int i;
			for (i = 0; i < 2; i++)
				g_print("\t %d. %s\n", i+1, visible_mode[i]);

			err = scanf("%d", &value);
			bret = camera_set_display_visible(hcamcorder->camera, idx-1);

			break;


		case 'o': /*  Setting > Display MODe */
			g_print("* Display mode!\n");

			flush_stdin();
			for (i = 1 ; i <= 2  ; i++)
				g_print("%d. %s\n", i, display_mode[i]);

			err = scanf("%d", &idx);
			bret =  camera_set_display_mode(hcamcorder->camera, idx-1);
			break;

		case 'y': /* Setting > Rotate Display */

			flush_stdin();
			g_print("\n Select Rotate mode\n");
			g_print("\t0. 0\n\t1. 90\n\t2. 180\n\t3. 270\n\n");
			err = scanf("%d", &idx);
			CHECK_MM_ERROR(camera_stop_preview(hcamcorder->camera));
			bret = camera_set_display_rotation(hcamcorder->camera, idx);
			CHECK_MM_ERROR(camera_start_preview(hcamcorder->camera));
			break;

		case 'Y': /* Setting > Flip Display */
			flush_stdin();
			g_print("\n Select Rotate mode\n");
			g_print("\t0. NONE\n\t1. HORIZONTAL\n\t2. VERTICAL\n\t3. BOTH\n\n");
			err = scanf("%d", &idx);
			bret = camera_set_display_flip(hcamcorder->camera, idx);
			break;

		case 'g': /* Setting > Brightness */
			g_print("*Brightness !\n");
			camera_attr_get_brightness_range(hcamcorder->camera, &min, &max);

			flush_stdin();
			g_print("\n Select  brightness min (%d) -max(%d)", min, max);
			err = scanf("%d", &idx);
			bret = camera_attr_set_brightness(hcamcorder->camera, idx);
			break;

		case 'c': /* Setting > Contrast */
			g_print("*Contrast !\n");
			camera_attr_get_contrast_range(hcamcorder->camera, &min, &max);

			flush_stdin();
			g_print("\n Select  Contrast min(%d)-max(%d)", min, max);
			err = scanf("%d", &idx);
			bret = camera_attr_set_contrast(hcamcorder->camera, idx);
			break;



		case 'w': /* Setting > White balance */
			g_print("*White balance !\n");

			flush_stdin();
			g_print("\n Select White balance \n");
			camera_attr_foreach_supported_whitebalance(hcamcorder->camera, white_balance_cb, NULL);
			err = scanf("%d", &idx);
			bret = camera_attr_set_whitebalance(hcamcorder->camera, idx);
			break;

		case 't': /* Setting > Color tone */
			g_print("*Color tone !\n");
			camera_attr_foreach_supported_effect(hcamcorder->camera, colortone_cb, NULL);

			g_print("\n Select Color tone \n");
			flush_stdin();
			err = scanf("%d", &idx);
			bret = camera_attr_set_effect(hcamcorder->camera, idx);

			break;

		case 'd': /* Setting > WDR */
			g_print("*WDR !\n");

			g_print("\n Select WDR Mode \n");
			flush_stdin();
			for (i = 0; i < 2; i++)
				g_print("\t %d. %s\n", i+1, wdr_mode[i]);

			err = scanf("%d", &idx);
			if (idx == 1)
				bret = camera_attr_enable_auto_contrast(hcamcorder->camera, 0);
			else if (idx == 2)
				bret = camera_attr_enable_auto_contrast(hcamcorder->camera, 1);

			break;

		case 'e': /* Setting > EV program mode */
			g_print("* EV program mode!\n");
			camera_attr_foreach_supported_scene_mode(hcamcorder->camera, program_mode_cb, NULL);

			g_print("\n Select EV program mode \n");
			flush_stdin();
			err = scanf("%d", &idx);
			bret = camera_attr_set_scene_mode(hcamcorder->camera, idx);

			break;

			/* ext. setting */
		case 'z': /* Setting > Strobe setting */
			g_print("*Strobe Mode\n");
			camera_attr_foreach_supported_flash_mode(hcamcorder->camera, strobe_mode_cb, NULL);
			g_print("\n Select Strobe Mode \n");
			flush_stdin();
			err = scanf("%d", &idx);
			bret = camera_attr_set_flash_mode(hcamcorder->camera, idx);

			break;

		case 'l': /* Setting > Face detection setting */
			/* hcamcorder->menu_state = MENU_STATE_SETTING_DETECTION; */
			g_print("* Face detect mode !\n");


			flush_stdin();
			for (i = 0; i < 2; i++)
				g_print("\t %d. %s \n", i+1, detection_mode[i]);

			err = scanf("%d", &idx);
			if (camera_is_supported_face_detection(hcamcorder->camera)) {
				if (idx >= 0 && idx < 2)
					bret = camera_start_face_detection(hcamcorder->camera, _face_detected, NULL);
			} else {
				g_print("face detection_not supported");
			}

			break;

		case 'k': /* Setting > Anti-handshake */
			g_print("*Anti-handshake !\n");

			g_print("\n Select Anti-handshake mode \n");
			flush_stdin();
			for (i = 0; i < 2; i++)
				g_print("\t %d. %s\n", i+1, ahs_mode[i]);

			err = scanf("%d", &idx);
			bret = camera_attr_enable_anti_shake(hcamcorder->camera, idx-1);

			break;

		case 'K': /* Setting > Video-stabilization */
			g_print("*Video-stabilization !\n");

			g_print("\n Select Video-stabilization mode \n");
			flush_stdin();
			for (i = 0; i < 2; i++)
				g_print("\t %d. %s\n", i+1, vs_mode[i]);

			err = scanf("%d", &idx);

			if (idx == 2) {
				g_print("\n Restart preview with NV12 and 720p resolution\n");

				err = camera_stop_preview(hcamcorder->camera);

				camera_set_preview_resolution(hcamcorder->camera, 1280, 720);
				camera_set_preview_format(hcamcorder->camera, CAMERA_PIXEL_FORMAT_NV12);
				camera_attr_enable_video_stabilization(hcamcorder->camera, idx-1);

				if (err == 0) {
					err = camera_start_preview(hcamcorder->camera);

					if (err != 0)
						g_print("\n Restart FAILED! %x\n", err);
				}
			}
			break;

		case 'u': /* Touch AF area */
			g_print("* Touch AF area !\n");

			flush_stdin();
			g_print("\n Input x,y,width,height \n");
			err = scanf("%d,%d,%d,%d", &x, &y, &width, &height);
			err = camera_attr_set_af_area(hcamcorder->camera, width, height);

			if (err != 0)
				g_print("Failed to set touch AF area.(%x)\n", err);
			else
				g_print("Succeed to set touch AF area.\n");

			break;


		case 'M':
			{
				float motion_rate = 0.0;

				flush_stdin();

				g_print("*Camcorder Motion Rate setting! (should be bigger than zero)\n");

				err = scanf("%f", &motion_rate);
				err = recorder_attr_set_recording_motion_rate(hcamcorder->recorder, motion_rate);
				if (err != 0)
					g_print("Failed to set Camcorder Motion Rate %f [err:0x%x]\n", motion_rate, err);
				else
					g_print("Succeed to set Motion Rate %f\n", motion_rate);
			}
			break;

		case 'b': /* back */
			hcamcorder->menu_state = MENU_STATE_MAIN;
			break;

		default:
			g_print("\t Invalid input \n");
			break;
		}
	} else {
		g_print("\t Invalid mode, back to upper menu \n");
		hcamcorder->menu_state = MENU_STATE_MAIN;
	}

	g_print("\t bret : 0x%x \n", bret);
}


/**
 * This function is to execute command.
 *
 * @param    channel [in]    1st parameter
 *
 * @return   This function returns TRUE/FALSE
 * @remark
 * @see
 */
static gboolean cmd_input(GIOChannel *channel)
{
	gchar *buf = NULL;
	gsize read_size;
	GError *g_error = NULL;

	LOGD("ENTER");

	g_io_channel_read_line(channel, &buf, &read_size, NULL, &g_error);
	if (g_error) {
		LOGD("g_io_channel_read_chars error");
		g_error_free(g_error);
		g_error = NULL;
	}

	if (buf) {
		g_strstrip(buf);

		LOGD("Menu Status : %d", hcamcorder->menu_state);
		switch (hcamcorder->menu_state) {
		case MENU_STATE_MAIN:
			main_menu(buf[0]);
			break;
		case MENU_STATE_SETTING:
			setting_menu(buf[0]);
			break;
		default:
			break;
		}

		g_free(buf);
		buf = NULL;

		print_menu();
	} else {
		LOGD("No read input");
	}

	return TRUE;
}

static gboolean init(int type)
{
	int err;

	if (!hcamcorder)
		return FALSE;

	if (!hcamcorder->recorder)
		return FALSE;

	/*================================================================================
	  Video capture mode
	 *=================================================================================*/
	if (type == MODE_VIDEO_CAPTURE) {
		err = recorder_set_file_format(hcamcorder->recorder, RECORDER_FILE_FORMAT_MP4);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_device(hcamcorder->recorder, RECORDER_AUDIO_DEVICE_MIC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_set_audio_encoder(hcamcorder->recorder, RECORDER_AUDIO_CODEC_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_set_video_encoder(hcamcorder->recorder, RECORDER_VIDEO_CODEC_MPEG4);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_video_encoder_bitrate(hcamcorder->recorder, VIDEO_ENCODE_BITRATE);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_samplerate(hcamcorder->recorder, AUDIO_SOURCE_SAMPLERATE_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_channel(hcamcorder->recorder, AUDIO_SOURCE_CHANNEL_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_set_filename(hcamcorder->recorder, TARGET_FILENAME_VIDEO);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
	} else if (type == MODE_AUDIO) {
	/*================================================================================
	  Audio mode
	 *=================================================================================*/
		err = recorder_set_file_format(hcamcorder->recorder, RECORDER_FILE_FORMAT_MP4);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_device(hcamcorder->recorder, RECORDER_AUDIO_DEVICE_MIC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_set_audio_encoder(hcamcorder->recorder, RECORDER_AUDIO_CODEC_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_samplerate(hcamcorder->recorder, AUDIO_SOURCE_SAMPLERATE_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_channel(hcamcorder->recorder, AUDIO_SOURCE_CHANNEL_AAC);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_set_filename(hcamcorder->recorder, TARGET_FILENAME_AUDIO);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_time_limit(hcamcorder->recorder, 360000);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
		err = recorder_attr_set_audio_encoder_bitrate(hcamcorder->recorder, 128000);
		if (err < 0) {
			LOGE("Init fail. (%x)", err);
			goto ERROR;
		}
	}

	recorder_set_state_changed_cb(hcamcorder->recorder, _state_changed_cb, NULL);
	recorder_set_recording_status_cb(hcamcorder->recorder, _recording_status_cb, NULL);
	recorder_set_recording_limit_reached_cb(hcamcorder->recorder, _recording_limit_reached_cb, NULL);

	LOGD("Init DONE.");

	return TRUE;

ERROR:
	LOGE("init failed.");
	return FALSE;
}

static gboolean init_handle()
{
	hcamcorder->mode = MODE_VIDEO_CAPTURE;  /* image(capture)/video(recording) mode */
	hcamcorder->menu_state = MENU_STATE_MAIN;
	hcamcorder->isMute = FALSE;
	hcamcorder->elapsed_time = 0;
	hcamcorder->fps = SRC_VIDEO_FRAME_RATE_15; /*SRC_VIDEO_FRAME_RATE_30;*/

	return TRUE;
}


static void _sound_stream_focus_state_changed_cb(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	g_print("focus changed : reason %d\n", reason_for_change);
	return;
}

/**
 * This function is to change camcorder mode.
 *
 * @param    type    [in]    image(capture)/video(recording) mode
 *
 * @return   This function returns TRUE/FALSE
 * @remark
 * @see      other functions
 */
static gboolean mode_change()
{
	int err = RECORDER_ERROR_NONE;
	int state = RECORDER_STATE_NONE;
	char media_type = '\0';
	bool check = FALSE;

	if (hcamcorder->recorder) {
		err = recorder_get_state(hcamcorder->recorder, (recorder_state_e*)&state);
		if (state != RECORDER_STATE_NONE) {
			if (state == RECORDER_STATE_RECORDING ||
					state == RECORDER_STATE_PAUSED) {
				LOGD("recorder_cancel");
				err = recorder_cancel(hcamcorder->recorder);
				if (err != RECORDER_ERROR_NONE) {
					LOGE("exit recorder_cancel failed 0x%x", err);
					return FALSE;
				}
			}

			err = recorder_get_state(hcamcorder->recorder, (recorder_state_e*)&state);
			if (state == RECORDER_STATE_READY) {
				LOGD("recorder_unprepare");
				recorder_unprepare(hcamcorder->recorder);
			}

			err = recorder_get_state(hcamcorder->recorder, (recorder_state_e*)&state);
			if (state == RECORDER_STATE_CREATED) {
				LOGD("recorder_destroy");
				err = recorder_destroy(hcamcorder->recorder);
				if (err == RECORDER_ERROR_NONE) {
					LOGD("recorder_destroy done");
					hcamcorder->recorder = NULL;
				}
			}
		}
	} else {
		LOGW("NULL handle");
	}

	init_handle();

	g_get_current_time(&previous);
	g_timer_reset(timer);

	while (!check) {
		g_print("\n\t=======================================\n");
		g_print("\t   RECORDER_TESTSUIT\n");
		g_print("\t=======================================\n");
		g_print("\t   '1' Video Capture - Rear Camera\n");
		g_print("\t   '2' Audio Recording - MIC\n");
		g_print("\t   '3' Audio Recording - MODEM\n");
		g_print("\t   'q' Exit\n");
		g_print("\t=======================================\n");

		g_print("\t  Enter the media type:\n\t");

		err = scanf("%c", &media_type);
		if (err == EOF) {
			g_print("\t!!!read input error!!!\n");
			continue;
		}

		LOGD("media type : %c", media_type);

		switch (media_type) {
		case '1':
			hcamcorder->mode = MODE_VIDEO_CAPTURE;

			LOGD("camera_create");

			err = camera_create(CAMERA_DEVICE_CAMERA0, &hcamcorder->camera);
			if (err != CAMERA_ERROR_NONE) {
				LOGE("camera create failed 0x%d", err);
				continue;
			}

			err = camera_set_display(hcamcorder->camera, CAMERA_DISPLAY_TYPE_OVERLAY, GET_DISPLAY(display));
			if (err != CAMERA_ERROR_NONE) {
				LOGE("set display failed 0x%d", err);
				camera_destroy(hcamcorder->camera);
				hcamcorder->camera = NULL;
				continue;
			}

			err = camera_start_preview(hcamcorder->camera);
			if (err != CAMERA_ERROR_NONE) {
				LOGE("start preview failed 0x%d", err);
				camera_destroy(hcamcorder->camera);
				hcamcorder->camera = NULL;
				continue;
			}

			err = recorder_create_videorecorder(hcamcorder->camera, &hcamcorder->recorder);
			if (err != RECORDER_ERROR_NONE) {
				LOGE("video recorder create failed 0x%d", err);
				camera_stop_preview(hcamcorder->camera);
				camera_destroy(hcamcorder->camera);
				hcamcorder->camera = NULL;
				continue;
			}

			check = TRUE;
			break;
		case '2':
			hcamcorder->mode = MODE_AUDIO;
			err = recorder_create_audiorecorder(&hcamcorder->recorder);
			if (err != RECORDER_ERROR_NONE) {
				LOGE("audio recorder create failed 0x%x", err);
				continue;
			}

			{
				sound_stream_info_h stream_info = NULL;

				sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, _sound_stream_focus_state_changed_cb, hcamcorder, &stream_info);
				if (stream_info) {
					recorder_set_sound_stream_info(hcamcorder->recorder, stream_info);
					sound_manager_destroy_stream_information(stream_info);
				}
			}

			err = recorder_attr_set_audio_device(hcamcorder->recorder, RECORDER_AUDIO_DEVICE_MIC);
			if (err != RECORDER_ERROR_NONE) {
				LOGE("set audio device failed 0x%x", err);
				recorder_destroy(hcamcorder->recorder);
				hcamcorder->recorder = NULL;
				continue;
			}

			check = TRUE;
			break;
		case '3':
			hcamcorder->mode = MODE_AUDIO;
			err = recorder_create_audiorecorder(&hcamcorder->recorder);
			if (err != RECORDER_ERROR_NONE) {
				LOGE("audio recorder create failed 0x%d", err);
				continue;
			}

			recorder_attr_set_audio_device(hcamcorder->recorder, RECORDER_AUDIO_DEVICE_MODEM);
			if (err != RECORDER_ERROR_NONE) {
				LOGE("set audio device failed 0x%d", err);
				recorder_destroy(hcamcorder->recorder);
				hcamcorder->recorder = NULL;
				continue;
			}

			check = TRUE;
			break;
		case 'q':
			g_print("\t Quit Camcorder Testsuite!!\n");
			hcamcorder->mode = -1;
			if (g_main_loop_is_running(g_loop))
				g_main_loop_quit(g_loop);

			return FALSE;
		default:
			g_print("\t Invalid media type(%c)\n", media_type);
			continue;
		}
	}

	g_timer_reset(timer);

	if (!init(hcamcorder->mode)) {
		LOGE("testsuite init() failed.");
		return -1;
	}

	g_timer_reset(timer);

	err = recorder_prepare(hcamcorder->recorder);

	LOGD("recorder_start()  : %12.6lfs", g_timer_elapsed(timer, NULL));

	if (err != RECORDER_ERROR_NONE) {
		LOGE("recorder_start  = %x", err);
		return -1;
	}

	g_get_current_time(&current);
	timersub(&current, &previous, &res);

	LOGD("Recorder Starting Time  : %ld.%lds", res.tv_sec, res.tv_usec);

	return TRUE;
}


/**
 * This function is the example main function for recorder API.
 *
 * @param
 *
 * @return   This function returns 0.
 * @remark
 * @see      other functions
 */
int main(int argc, char **argv)
{
	int bret;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	if (!g_thread_supported())
		g_thread_init(NULL);
#endif

	timer = g_timer_new();

	elm_init(argc, argv);

	LOGD("elm_init() : %12.6lfs", g_timer_elapsed(timer, NULL));

	hcamcorder = (cam_handle_t *) g_malloc0(sizeof(cam_handle_t));

	Evas_Object *eo = NULL;
	eo = elm_win_add(NULL, "VIDEO OVERLAY", ELM_WIN_BASIC);
	elm_win_title_set(eo, "VIDEO oVERLAY");
	evas_object_resize(eo, 200, 200);
	evas_object_show(eo);
	display = (void *)eo;

	recorder_state = RECORDER_STATE_NONE;

	g_timer_reset(timer);

	bret = mode_change();
	if (!bret)
		return bret;

	print_menu();

	g_loop = g_main_loop_new(NULL, FALSE);

	stdin_channel = g_io_channel_unix_new(fileno(stdin));/* read from stdin */
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)cmd_input, NULL);

	LOGD("RUN main loop");

	g_main_loop_run(g_loop);

	LOGD("STOP main loop");

	if (timer) {
		g_timer_stop(timer);
		g_timer_destroy(timer);
		timer = NULL;
	}
	/* g_print("\t Exit from the application.\n"); */
	g_free(hcamcorder);
	g_main_loop_unref(g_loop);
	g_io_channel_unref(stdin_channel);

	return bret;
}

/*EOF*/
