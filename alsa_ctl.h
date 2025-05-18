#ifndef __ALSA_CTL__
#define __ALSA_CTL__

#include "define.h"
#include <alsa/asoundlib.h>

#define HW_PLAY_DEVICE "plughw:0,0"
#define WAV_RECOVER_DROP_SIZE 128 //wav 파일 drop 발생시 얼마만큼 뒤로 갈것인지

typedef enum
{
    ALSA_START_FLAG,
    ALSA_PAUSE_FLAG,
    ALSA_ABORT_FLAG,
    ALSA_FLAGS_MAX,
}ALSA_FLAGS;

typedef struct __attribute__((__packed__))
{
    char chunk_id[4];
    uint32_t chunk_size;
    char chunk_format[4];
    char sub_chunk1_id[4];
    uint32_t sub_chunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_per_sample;
    char sub_chunk2_id[4];
    uint32_t sub_chunk2_size;
}wav_format_packet_header_t;

typedef union
{
    wav_format_packet_header_t packet;
    uint8_t buf[sizeof(wav_format_packet_header_t)];
}wav_file_header_u;

typedef struct 
{
    int format;
    int channels;
    int rate;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *pcm_hw_params;
    char file_route[256];
}wav_info_t;

typedef struct
{
    int alsa_flag[ALSA_FLAGS_MAX];
    char file_route[256];
    sem_t sem;
}alsa_ctl_t;

int start_playing_wav();
int restart_playing_wav();
int pause_playing_wav();
int continue_playing_wav();
int abort_playing_wav();

int set_file_route(char *file_route);
int get_file_route(char *file_route, int file_route_size);
void start_wav_playing_thread();
#endif