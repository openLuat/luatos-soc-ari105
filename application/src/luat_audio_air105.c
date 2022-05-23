/*
 * Copyright (c) 2022 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "luat_base.h"
#include "luat_msgbus.h"
#include "luat_fs.h"
#include <stdlib.h>
#include "luat_multimedia.h"
#include "app_interface.h"
#include "mp3_decode/minimp3.h"
#define LUAT_LOG_TAG "audio"
#include "luat_log.h"

#define MP3_FRAME_LEN 4 * 1152
static Audio_StreamStruct prvAudioStream;
static int32_t luat_audio_wav_decode(void *pData, void *pParam)
{
	Audio_StreamStruct *Stream = (Audio_StreamStruct *)pData;
	while ( (llist_num(&Stream->DataHead) < 3) && !Stream->IsStop)
	{
		Stream->FileDataBuffer.Pos = luat_fs_fread(Stream->FileDataBuffer.Data, Stream->FileDataBuffer.MaxLen, 1, Stream->fd);
		if (Stream->FileDataBuffer.Pos > 0)
		{
			if (!Stream->IsStop)
			{
				Audio_WriteRaw(Stream, Stream->FileDataBuffer.Data, Stream->FileDataBuffer.Pos);
			}
		}
		else
		{
			return 0;
		}
	}


	return 0;
}

static int32_t luat_audio_mp3_decode(void *pData, void *pParam)
{
	uint32_t pos;
	int read_len;
	int result;
	uint32_t is_not_end = 1;
	mp3dec_frame_info_t info;
	mp3dec_t *mp3_decoder = (mp3dec_t *)pParam;
	Audio_StreamStruct *Stream = (Audio_StreamStruct *)pData;
	Stream->AudioDataBuffer.Pos = 0;

	while ((llist_num(&Stream->DataHead) < 3) && is_not_end && !Stream->IsStop)
	{
		while (( Stream->AudioDataBuffer.Pos < (Stream->AudioDataBuffer.MaxLen >> 1) ) && is_not_end && !Stream->IsStop)
		{
			if (Stream->FileDataBuffer.Pos < MINIMP3_MAX_SAMPLES_PER_FRAME)
			{
				read_len = luat_fs_fread(Stream->FileDataBuffer.Data + Stream->FileDataBuffer.Pos, MINIMP3_MAX_SAMPLES_PER_FRAME, 1, Stream->fd);
				if (read_len > 0)
				{
					Stream->FileDataBuffer.Pos += read_len;
				}
				else
				{
					is_not_end = 0;
				}
			}
			pos = 0;
			do
			{
				memset(&info, 0, sizeof(info));

				result = mp3dec_decode_frame(mp3_decoder, Stream->FileDataBuffer.Data + pos, Stream->FileDataBuffer.Pos - pos,
						Stream->AudioDataBuffer.Data + Stream->AudioDataBuffer.Pos, &info);

				if (result > 0)
				{
					Stream->AudioDataBuffer.Pos += (result * info.channels * 2);
				}
				else
				{
					DBG("no mp3!");
				}
				pos += info.frame_bytes;
				if (Stream->AudioDataBuffer.Pos >= (Stream->AudioDataBuffer.MaxLen >> 1))
				{
					break;
				}
			} while ( ((Stream->FileDataBuffer.Pos - pos) >= (MINIMP3_MAX_SAMPLES_PER_FRAME * is_not_end + 1)) && !Stream->IsStop);
			OS_BufferRemove(&Stream->FileDataBuffer, pos);
		}
		if (!Stream->IsStop)
		{
			Audio_WriteRaw(Stream, Stream->AudioDataBuffer.Data, Stream->AudioDataBuffer.Pos);
		}
		Stream->AudioDataBuffer.Pos = 0;
	}

	return 0;
}

int32_t luat_audio_decode_run(void *pData, void *pParam)
{
	if (!prvAudioStream.IsStop)
	{
		prvAudioStream.Decoder(pData, pParam);
	}
	if (prvAudioStream.waitRequire)
	{
		prvAudioStream.waitRequire--;
	}
}

int32_t luat_audio_app_cb(void *pData, void *pParam)
{
    rtos_msg_t msg = {0};
	msg.handler = l_multimedia_raw_handler;
	msg.arg1 = (pParam == INVALID_HANDLE_VALUE)?MULTIMEDIA_CB_AUDIO_DONE:MULTIMEDIA_CB_AUDIO_NEED_DATA;
	msg.arg2 = prvAudioStream.pParam;
	luat_msgbus_put(&msg, 1);
	return 0;
}

int32_t luat_audio_play_cb(void *pData, void *pParam)
{
	if (pParam == INVALID_HANDLE_VALUE)
	{
	    rtos_msg_t msg = {0};
		msg.handler = l_multimedia_raw_handler;
		msg.arg1 = MULTIMEDIA_CB_AUDIO_DONE;
		msg.arg2 = prvAudioStream.pParam;
		luat_msgbus_put(&msg, 1);
	}
	else
	{
		if (!prvAudioStream.IsStop)
		{
			prvAudioStream.waitRequire++;
			ASSERT(prvAudioStream.waitRequire < 3);
			Core_ServiceRunUserAPIWithFile(luat_audio_decode_run, &prvAudioStream, prvAudioStream.CoderParam);
		}
	}
	return 0;
}

int luat_audio_start_raw(uint8_t multimedia_id, uint8_t audio_format, uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample, uint8_t is_signed)
{
	prvAudioStream.pParam = multimedia_id;
	if (prvAudioStream.fd) {
		prvAudioStream.CB = luat_audio_play_cb;
	} else {
		prvAudioStream.CB = luat_audio_app_cb;
	}

	prvAudioStream.BitDepth = bits_per_sample;
	prvAudioStream.BusType = AUSTREAM_BUS_DAC;
	prvAudioStream.BusID = 0;
	prvAudioStream.Format = audio_format;
	prvAudioStream.ChannelCount = num_channels;
	prvAudioStream.SampleRate = sample_rate;
	prvAudioStream.IsDataSigned = is_signed;
	return Audio_StartRaw(&prvAudioStream);
}

int luat_audio_write_raw(uint8_t multimedia_id, uint8_t *data, uint32_t len)
{
	if (len)
		return Audio_WriteRaw(&prvAudioStream, data, len);
	return -1;
}


int luat_audio_stop_raw(uint8_t multimedia_id)
{
	Audio_Stop(&prvAudioStream);
	OS_DeInitBuffer(&prvAudioStream.FileDataBuffer);
	OS_DeInitBuffer(&prvAudioStream.AudioDataBuffer);
	if (prvAudioStream.fd)
	{
		luat_fs_fclose(prvAudioStream.fd);
		prvAudioStream.fd = NULL;
	}
	if (prvAudioStream.CoderParam)
	{
		luat_heap_free(prvAudioStream.CoderParam);
		prvAudioStream.CoderParam = NULL;
	}
	prvAudioStream.IsStop = 0;
	prvAudioStream.IsPlaying = 0;
	prvAudioStream.waitRequire = 0;
	return ERROR_NONE;
}

int luat_audio_pause_raw(uint8_t multimedia_id, uint8_t is_pause)
{
	if (is_pause)
	{
		Audio_Pause(&prvAudioStream);
	}
	else
	{
		Audio_Resume(&prvAudioStream);
	}
	return ERROR_NONE;
}

int luat_audio_play_stop(uint8_t multimedia_id)
{
	prvAudioStream.IsStop = 1;
}

uint8_t luat_audio_is_finish(uint8_t multimedia_id)
{
	ASSERT(prvAudioStream.waitRequire < 4);
	return !prvAudioStream.waitRequire;
}

int luat_audio_play_file(uint8_t multimedia_id, const char *path)
{
	if (prvAudioStream.IsPlaying)
	{
		luat_audio_stop_raw(multimedia_id);
	}

	uint32_t jump, i;
	uint8_t temp[16];
	mp3dec_t *mp3_decoder;
	int result;
	int audio_format = MULTIMEDIA_DATA_TYPE_PCM;
	int num_channels;
	int sample_rate;
	int bits_per_sample = 16;
	uint32_t align;
	int is_signed = 1;
    size_t len;
	mp3dec_frame_info_t info;
	FILE *fd = luat_fs_fopen(path, "r");

	if (!fd)
	{
		return -1;
	}
	luat_fs_fread(temp, 12, 1, fd);
	if (!memcmp(temp, "ID3", 3))
	{
		jump = 0;
		for(i = 0; i < 4; i++)
		{
			jump <<= 7;
			jump |= temp[6 + i] & 0x7f;
		}
//		LLOGD("jump head %d", jump);
		luat_fs_fseek(fd, jump, SEEK_SET);
		mp3_decoder = luat_heap_malloc(sizeof(mp3dec_t));
		memset(mp3_decoder, 0, sizeof(mp3dec_t));
		mp3dec_init(mp3_decoder);
		OS_InitBuffer(&prvAudioStream.FileDataBuffer, MP3_FRAME_LEN);
		prvAudioStream.FileDataBuffer.Pos = luat_fs_fread(prvAudioStream.FileDataBuffer.Data, MP3_FRAME_LEN, 1, fd);
		result = mp3dec_decode_frame(mp3_decoder, prvAudioStream.FileDataBuffer.Data, prvAudioStream.FileDataBuffer.Pos, NULL, &info);
		if (result)
		{
			prvAudioStream.CoderParam = mp3_decoder;

			memset(mp3_decoder, 0, sizeof(mp3dec_t));
			num_channels = info.channels;
			sample_rate = info.hz;
			len = (num_channels * sample_rate >> 2);	//一次时间为1/16秒
			OS_ReInitBuffer(&prvAudioStream.AudioDataBuffer, len * 2);	//防止溢出，需要多一点空间
			prvAudioStream.Decoder = luat_audio_mp3_decode;
		}
		else
		{
			luat_heap_free(mp3_decoder);
		}
	}
	else if (!memcmp(temp, "RIFF", 4) || !memcmp(temp + 8, "WAVE", 4))
	{
		result = 0;
		prvAudioStream.CoderParam = NULL;
		OS_DeInitBuffer(&prvAudioStream.AudioDataBuffer);
		luat_fs_fread(temp, 8, 1, fd);
		if (!memcmp(temp, "fmt ", 4))
		{
			memcpy(&len, temp + 4, 4);
			OS_InitBuffer(&prvAudioStream.FileDataBuffer, len);
			luat_fs_fread(prvAudioStream.FileDataBuffer.Data, len, 1, fd);
			audio_format = prvAudioStream.FileDataBuffer.Data[0];
			num_channels = prvAudioStream.FileDataBuffer.Data[2];
			memcpy(&sample_rate, prvAudioStream.FileDataBuffer.Data + 4, 4);
			align = prvAudioStream.FileDataBuffer.Data[12];
			bits_per_sample = prvAudioStream.FileDataBuffer.Data[14];
			len = ((align * sample_rate) >> 3) & ~(3);	//一次时间为1/8秒
			OS_ReInitBuffer(&prvAudioStream.FileDataBuffer, len);
			luat_fs_fread(temp, 8, 1, fd);
			if (!memcmp(temp, "fact", 4))
			{
				memcpy(&len, temp + 4, 4);
				luat_fs_fseek(fd, len, SEEK_CUR);
				luat_fs_fread(temp, 8, 1, fd);
			}
			if (!memcmp(temp, "data", 4))
			{
				result = 1;
				prvAudioStream.Decoder = luat_audio_wav_decode;
			}
		}
	}
	else
	{
		result = 0;
	}
	if (result)
	{
		prvAudioStream.fd = fd;
		result = luat_audio_start_raw(multimedia_id, audio_format, num_channels, sample_rate, bits_per_sample, is_signed);
		if (!result)
		{
			LLOGD("decode %s ok,param,%d,%d,%d,%d,%d,%d,%u,%u", path,multimedia_id, audio_format, num_channels, sample_rate, bits_per_sample, is_signed, prvAudioStream.FileDataBuffer.MaxLen, prvAudioStream.AudioDataBuffer.MaxLen);
			prvAudioStream.IsPlaying = 1;
			prvAudioStream.pParam = multimedia_id;
			prvAudioStream.Decoder(&prvAudioStream, prvAudioStream.CoderParam);
			if (!llist_num(&prvAudioStream.DataHead))
			{
				prvAudioStream.fd = NULL;
			}
			else
			{
				return 0;
			}

		}
		else
		{
			prvAudioStream.fd = 0;
		}
	}
	luat_fs_fclose(fd);
	OS_DeInitBuffer(&prvAudioStream.FileDataBuffer);
	OS_DeInitBuffer(&prvAudioStream.AudioDataBuffer);
	if (prvAudioStream.CoderParam)
	{
		luat_heap_free(prvAudioStream.CoderParam);
		prvAudioStream.CoderParam = NULL;
	}
	return -1;
}
