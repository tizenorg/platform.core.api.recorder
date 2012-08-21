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



#include <tet_api.h>
#include <media/recorder.h>

#define MY_ASSERT( fun , test , msg ) \
{\
	if( !test ) \
		dts_fail(fun , msg ); \
}		


static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_media_recorder_attr_get_audio_device_p(void);
static void utc_media_recorder_attr_get_audio_device_n(void);
static void utc_media_recorder_attr_get_audio_encoder_bitrate_p(void);
static void utc_media_recorder_attr_get_audio_encoder_bitrate_n(void);
static void utc_media_recorder_attr_get_audio_samplerate_p(void);
static void utc_media_recorder_attr_get_audio_samplerate_n(void);
static void utc_media_recorder_attr_get_size_limit_p(void);
static void utc_media_recorder_attr_get_size_limit_n(void);
static void utc_media_recorder_attr_get_time_limit_p(void);
static void utc_media_recorder_attr_get_time_limit_n(void);
static void utc_media_recorder_attr_get_video_encoder_bitrate_p(void);
static void utc_media_recorder_attr_get_video_encoder_bitrate_n(void);
static void utc_media_recorder_attr_set_audio_device_p(void);
static void utc_media_recorder_attr_set_audio_device_n(void);
static void utc_media_recorder_attr_set_audio_encoder_bitrate_p(void);
static void utc_media_recorder_attr_set_audio_encoder_bitrate_n(void);
static void utc_media_recorder_attr_set_audio_samplerate_p(void);
static void utc_media_recorder_attr_set_audio_samplerate_n(void);
static void utc_media_recorder_attr_set_size_limit_p(void);
static void utc_media_recorder_attr_set_size_limit_n(void);
static void utc_media_recorder_attr_set_time_limit_p(void);
static void utc_media_recorder_attr_set_time_limit_n(void);
static void utc_media_recorder_attr_set_video_encoder_bitrate_p(void);
static void utc_media_recorder_attr_set_video_encoder_bitrate_n(void); 
static void utc_media_recorder_get_audio_encoder_p(void);
static void utc_media_recorder_get_audio_encoder_n(void);
static void utc_media_recorder_get_file_format_p(void);
static void utc_media_recorder_get_file_format_n(void);
static void utc_media_recorder_get_video_encoder_p(void);
static void utc_media_recorder_get_video_encoder_n(void);
static void utc_media_recorder_set_audio_encoder_p(void);
static void utc_media_recorder_set_audio_encoder_n(void);
static void utc_media_recorder_set_file_format_p(void);
static void utc_media_recorder_set_file_format_n(void);
static void utc_media_recorder_set_filename_p(void);
static void utc_media_recorder_set_filename_n(void);
static void utc_media_recorder_set_video_encoder_p(void);
static void utc_media_recorder_set_video_encoder_n(void);

struct tet_testlist tet_testlist[] = { 
	{ utc_media_recorder_attr_get_audio_device_p , 1 },
	{ utc_media_recorder_attr_get_audio_device_n , 2 },
	{ utc_media_recorder_attr_get_audio_encoder_bitrate_p , 1 },
	{ utc_media_recorder_attr_get_audio_encoder_bitrate_n , 2 },
	{ utc_media_recorder_attr_get_audio_samplerate_p , 1 },
	{ utc_media_recorder_attr_get_audio_samplerate_n , 2 },
	{ utc_media_recorder_attr_get_size_limit_p , 1 },
	{ utc_media_recorder_attr_get_size_limit_n , 2 },
	{ utc_media_recorder_attr_get_time_limit_p , 1 },
	{ utc_media_recorder_attr_get_time_limit_n , 2 },
	{ utc_media_recorder_attr_get_video_encoder_bitrate_p , 1 },
	{ utc_media_recorder_attr_get_video_encoder_bitrate_n , 2 },
	{ utc_media_recorder_attr_set_audio_device_p , 1 },
	{ utc_media_recorder_attr_set_audio_device_n , 2 },
	{ utc_media_recorder_attr_set_audio_encoder_bitrate_p , 1 },
	{ utc_media_recorder_attr_set_audio_encoder_bitrate_n , 2 },
	{ utc_media_recorder_attr_set_audio_samplerate_p , 1 },
	{ utc_media_recorder_attr_set_audio_samplerate_n , 2 },
	{ utc_media_recorder_attr_set_size_limit_p , 1 },
	{ utc_media_recorder_attr_set_size_limit_n , 2 },
	{ utc_media_recorder_attr_set_time_limit_p , 1 },
	{ utc_media_recorder_attr_set_time_limit_n , 2 },
	{ utc_media_recorder_attr_set_video_encoder_bitrate_p , 1 },
	{ utc_media_recorder_attr_set_video_encoder_bitrate_n , 2 }, 
	{ utc_media_recorder_get_audio_encoder_p , 1 },
	{ utc_media_recorder_get_audio_encoder_n , 2 },
	{ utc_media_recorder_get_file_format_p , 1 },
	{ utc_media_recorder_get_file_format_n , 2 },
	{ utc_media_recorder_get_video_encoder_p , 1 },
	{ utc_media_recorder_get_video_encoder_n , 2 },
	{ utc_media_recorder_set_audio_encoder_p , 1 },
	{ utc_media_recorder_set_audio_encoder_n , 2 },
	{ utc_media_recorder_set_file_format_p , 1 },
	{ utc_media_recorder_set_file_format_n , 2 },
	{ utc_media_recorder_set_filename_p , 1 },
	{ utc_media_recorder_set_filename_n , 2 },
	{ utc_media_recorder_set_video_encoder_p , 1 },
	{ utc_media_recorder_set_video_encoder_n , 2 }, 
	{ NULL, 0 },
};

recorder_h recorder;

static void startup(void)
{
	/* start of TC */
	int ret;
	ret = recorder_create_audiorecorder(&recorder);	
	if( ret != 0 )
		dts_fail("recorder_create_audiorecorder", "Could not create audio recorder"); 	
}

static void cleanup(void)
{
	/* end of TC */
	recorder_destroy(recorder);	
}

static void utc_media_recorder_attr_get_audio_device_p(void)
{
	int ret;
	recorder_audio_device_e device;
	ret = recorder_attr_get_audio_device(recorder, &device);
	dts_check_eq("utc_media_recorder_attr_get_audio_device_p", ret , RECORDER_ERROR_NONE ,  "error get audio device");
}

static void utc_media_recorder_attr_get_audio_device_n(void)
{
	tet_printf("utc_recorder_attr_get_audio_device_negative\n");

	int ret;
	ret = recorder_attr_get_audio_device (recorder, NULL);
	dts_check_ne("utc_media_recorder_attr_get_audio_device_n", ret, RECORDER_ERROR_NONE,"recorder_attr_set_audio_device get by NULL should be fail");
}

static void utc_media_recorder_attr_get_audio_encoder_bitrate_p(void)
{
	int ret;
	int bitrate;
	ret = recorder_attr_get_audio_encoder_bitrate(recorder, &bitrate);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "fail audio encoder bitrate get");
}

static void utc_media_recorder_attr_get_audio_encoder_bitrate_n(void)
{
	int ret;
	ret = recorder_attr_get_audio_encoder_bitrate(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}

static void utc_media_recorder_attr_get_audio_samplerate_p(void)
{
	int ret;
	int samplerate;
	ret = recorder_attr_get_audio_samplerate(recorder, &samplerate);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "error get audio samplerate");

}

static void utc_media_recorder_attr_get_audio_samplerate_n(void)
{
	int ret;
	ret = recorder_attr_get_audio_samplerate(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}


static void utc_media_recorder_attr_get_size_limit_p(void)
{
	int ret;
	int size;
	ret = recorder_attr_get_size_limit(recorder, &size);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "error get limit size");
}

static void utc_media_recorder_attr_get_size_limit_n(void)
{
	int ret;
	ret = recorder_attr_get_size_limit(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}

static void utc_media_recorder_attr_get_time_limit_p(void)
{
	int ret;
	int value;
	ret = recorder_attr_get_time_limit(recorder, &value);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "error get limit time");

}

static void utc_media_recorder_attr_get_time_limit_n(void)
{
	int ret;
	ret = recorder_attr_get_time_limit(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");

}

static void utc_media_recorder_attr_get_video_encoder_bitrate_p(void)
{
	int ret;
	int value;
	ret = recorder_attr_get_video_encoder_bitrate(recorder, &value);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "error get video encoder bitrate");
}

static void utc_media_recorder_attr_get_video_encoder_bitrate_n(void)
{
	int ret;
	ret = recorder_attr_get_video_encoder_bitrate(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}

static void utc_media_recorder_attr_set_audio_device_p(void)
{
	int ret;
	ret = recorder_attr_set_audio_device(recorder, 1);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "error set audio device");

}

static void utc_media_recorder_attr_set_audio_device_n(void)
{
	int ret;
	ret = recorder_attr_set_audio_device(recorder, 10);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out of bound error is must occured");
}

static void utc_media_recorder_attr_set_audio_encoder_bitrate_p(void)
{
	int ret;
	ret = recorder_attr_set_audio_encoder_bitrate(recorder, 11111);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "can be set 11111");
}

static void utc_media_recorder_attr_set_audio_encoder_bitrate_n(void)
{
	int ret;
	ret = recorder_attr_set_audio_encoder_bitrate(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");

}

static void utc_media_recorder_attr_set_audio_samplerate_p(void)
{
	int ret;
	ret = recorder_attr_set_audio_samplerate(recorder, 11111);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "11111 is allow");

}

static void utc_media_recorder_attr_set_audio_samplerate_n(void)
{
	int ret;
	ret = recorder_attr_set_audio_samplerate(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");
}

static void utc_media_recorder_attr_set_size_limit_p(void)
{
	int ret;
	ret = recorder_attr_set_size_limit(recorder, 0);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "can be set 0");
}

static void utc_media_recorder_attr_set_size_limit_n(void)
{
	int ret;
	ret = recorder_attr_set_size_limit(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");
}

static void utc_media_recorder_attr_set_time_limit_p(void)
{
	int ret;
	ret = recorder_attr_set_time_limit(recorder, 11111);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "11111 is allow");

}

static void utc_media_recorder_attr_set_time_limit_n(void)
{
	int ret;
	ret = recorder_attr_set_time_limit(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");

}

static void utc_media_recorder_attr_set_video_encoder_bitrate_p(void)
{
	int ret;
	ret = recorder_attr_set_video_encoder_bitrate(recorder, 0);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE , "can be set 0");
}

static void utc_media_recorder_attr_set_video_encoder_bitrate_n(void)
{
	int ret;
	ret = recorder_attr_set_video_encoder_bitrate(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");

}

static void utc_media_recorder_get_audio_encoder_p(void)
{
	int ret;
	recorder_audio_codec_e value;
	ret = recorder_get_audio_encoder(recorder,&value);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_get_audio_encoder_n(void)
{
	int ret;
	ret = recorder_get_audio_encoder(recorder,NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}

static void utc_media_recorder_get_file_format_p(void)
{
	int ret;
	recorder_file_format_e value;
	ret = recorder_get_file_format(recorder,&value);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_get_file_format_n(void)
{
	int ret;
	ret = recorder_get_file_format(recorder,NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");

}

static void utc_media_recorder_get_video_encoder_p(void)
{
	int ret;
	recorder_video_codec_e value;
	ret = recorder_get_video_encoder(recorder,&value);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "error");
}

static void utc_media_recorder_get_video_encoder_n(void)
{
	int ret;
	ret = recorder_get_video_encoder(recorder,NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "out parameter must not be null");
}
 
static void utc_media_recorder_set_audio_encoder_p(void)
{
	int ret;
	ret = recorder_set_audio_encoder(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");
}

static void utc_media_recorder_set_audio_encoder_n(void)
{
	int ret;
	ret = recorder_set_audio_encoder(recorder, 0);
	dts_check_eq(__func__ , ret , RECORDER_ERROR_NONE, "0 is allow");
}

static void utc_media_recorder_set_file_format_p(void)
{
	int ret;
	ret = recorder_set_file_format(recorder, RECORDER_FILE_FORMAT_3GP);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "RECORDER_FILE_FORMAT_3GP is not supported");
}

static void utc_media_recorder_set_file_format_n(void)
{
	int ret;
	ret = recorder_set_file_format(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "RECORDER_FILE_FORMAT_INVALID is not allowed");
}

static void utc_media_recorder_set_filename_p(void)
{
	int ret;
	char *filename;
	ret = recorder_set_filename(recorder, "/mnt/nfs/test.test");
	MY_ASSERT(__func__, ret == 0 , "Fail recorder_set_filename");
	ret = recorder_get_filename(recorder, &filename);
	free(filename);
	MY_ASSERT(__func__, ret == 0 , "Fail recorder_get_filename");
	dts_pass(__func__, "PASS");	
}

static void utc_media_recorder_set_filename_n(void)
{
	int ret;
	ret = recorder_set_filename(recorder, NULL);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "NULL is not allowed");
}

static void utc_media_recorder_set_video_encoder_p(void)
{
	int ret;
	ret = recorder_set_video_encoder(recorder, RECORDER_VIDEO_CODEC_MPEG4);
	dts_check_eq(__func__, ret , RECORDER_ERROR_NONE, "RECORDER_VIDEO_CODEC_MPEG4 is not supported");
}

static void utc_media_recorder_set_video_encoder_n(void)
{
	int ret;
	ret = recorder_set_video_encoder(recorder, -1);
	dts_check_ne(__func__, ret , RECORDER_ERROR_NONE, "-1 is not allowed");
}

