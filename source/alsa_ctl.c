#include "alsa_ctl.h"
#include "define.h"

// todo mixer로 sw 볼륨 조절해보기

audio_ctl_t audio_ctl = {
    0,
};
wav_info_t wav_info = {
    0,
};

#if 0
static int bit_swap(uint8_t *buf, int bit_size)
{
    uint8_t temp = 0;
    int loop1 = 0;
    int loop2 = bit_size;

    if(bit_size % 2 != 0)
    {
        return -1;
    }

    for(loop1 = 0; loop1 < bit_size / 2; loop1++)
    {
        temp = buf[--loop2];
        buf[loop2] = buf[loop1];
        buf[loop1] = temp;
    }
    
    return 1;
}
#endif

static int get_audio_flag(AUDIO_FLAGS flag)
{
    int ret = 0;
    sem_wait(&audio_ctl.sem);
    ret = audio_ctl.audio_flag[flag];
    sem_post(&audio_ctl.sem);
    return ret;
}

static void set_audio_flag(AUDIO_FLAGS flag, int flag_val)
{
    sem_wait(&audio_ctl.sem);
    audio_ctl.audio_flag[flag] = flag_val;
    sem_post(&audio_ctl.sem);
}

void set_audio_volume(int audio_val)
{
    sem_wait(&audio_ctl.sem);
    if (audio_val > 100)
    {
        audio_val = 100;
    }
    if (audio_val < 0)
    {
        audio_val = 0;
    }
    audio_ctl.volume = (float)audio_val / (100.0);
    sem_post(&audio_ctl.sem);
}

float get_audio_volume()
{
    float ret = 0;
    sem_wait(&audio_ctl.sem);
    ret = audio_ctl.volume;
    sem_post(&audio_ctl.sem);
    return ret;
}

static void printf_wav_info(wav_file_header_u *header)
{
    printf("================[wav info]===============\n");
    printf("[id] : %.4s\n", header->packet.chunk_id);
    printf("[chunk size] : %d\n", header->packet.chunk_size);
    printf("[format] : %.4s\n", header->packet.chunk_format);
    printf("[subchunk1 id] : %.4s\n", header->packet.sub_chunk1_id);
    printf("[subchunk1 size] : %d\n", header->packet.sub_chunk1_size);
    printf("[audio format] : %d\n", header->packet.audio_format);
    printf("[num channel] : %d\n", header->packet.num_channels);
    printf("[sample rate] : %d\n", header->packet.sample_rate);
    printf("[byte rate] : %d\n", header->packet.byte_rate);
    printf("[block align] : %d\n", header->packet.block_align);
    printf("[bits per sample] : %d\n", header->packet.bit_per_sample);
    printf("[sub chunk2 id] : %.4s\n", header->packet.sub_chunk2_id);
    printf("[sub chunk2 size] : %d\n", header->packet.sub_chunk2_size);
    printf("=========================================\n");
}

static int drop_pcm()
{
    int ret = 0;

    if ((ret = snd_pcm_drop(wav_info.pcm_handle)) < 0)
    {
        printr("fail to pcm drop : %s", snd_strerror(ret));
    }
    return ret;
}

static int close_pcm()
{
    int ret = 0;

    if ((ret = snd_pcm_close(wav_info.pcm_handle)) < 0)
    {
        printr("fail to pcm close : %s", snd_strerror(ret));
    }

    return ret;
}

static int set_hw_params()
{
    int ret = 0;
    int dir = 0;

    if ((ret = snd_pcm_hw_params_malloc(&wav_info.pcm_hw_params) < 0))
    {
        printr("pcm hw malloc fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params_any(wav_info.pcm_handle, wav_info.pcm_hw_params)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm hw any fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params_set_access(wav_info.pcm_handle, wav_info.pcm_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm hw access fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params_set_format(wav_info.pcm_handle, wav_info.pcm_hw_params, wav_info.format)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm hw format fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params_set_channels(wav_info.pcm_handle, wav_info.pcm_hw_params, wav_info.channels)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm hw channel fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params_set_rate_near(wav_info.pcm_handle, wav_info.pcm_hw_params, &wav_info.rate, &dir)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm hw set near fail : %s", snd_strerror(ret));
        return -1;
    }
    if ((ret = snd_pcm_hw_params(wav_info.pcm_handle, wav_info.pcm_hw_params)) < 0)
    {
        snd_pcm_hw_params_free(wav_info.pcm_hw_params);
        printr("pcm set hw params fail : %s", snd_strerror(ret));
        return -1;
    }

    snd_pcm_hw_params_free(wav_info.pcm_hw_params);
    return 1;
}
static void volume_control_u8(uint8_t *buf, size_t buf_size)
{
    int loop = 0;

    for (loop = 0; loop < buf_size; loop++)
    {
        buf[loop] *= get_audio_volume();
    }
}

static void volume_control_s16(int16_t *buf, size_t buf_size)
{
    int loop = 0;

    for (loop = 0; loop < buf_size / sizeof(uint16_t); loop++)
    {
        buf[loop] *= get_audio_volume();
    }
}

#define sizeof_s24 3
static void volume_control_s24(uint8_t *buf, size_t buf_size)
{
    int loop = 0;
    int32_t temp = 0;

    for (loop = 0; loop < buf_size / sizeof_s24; loop += sizeof_s24)
    {
        temp = (buf[loop + 2] << 24) + (buf[loop + 1] << 16) + (buf[loop] << 8);
        temp >>= 8; // 부호 확장
        temp *= get_audio_volume();

        buf[loop + 2] = (temp >> 16) & 0xff;
        buf[loop + 1] = (temp >> 8) & 0xff;
        buf[loop] = temp & 0xff;
    }
}

static void volume_control_s32(int32_t *buf, size_t buf_size)
{
    int loop = 0;

    for (loop = 0; loop < buf_size / sizeof(uint32_t); loop++)
    {
        buf[loop] *= get_audio_volume();
    }
}

static void volume_control(void *buf, size_t buf_size)
{
    switch (wav_info.format)
    {
    case SND_PCM_FORMAT_U8:
        volume_control_u8(buf, buf_size);
        break;

    case SND_PCM_FORMAT_S16_LE:
        volume_control_s16(buf, buf_size);
        break;

    case SND_PCM_FORMAT_S24_LE:
        volume_control_s24(buf, buf_size);
        break;

    case SND_PCM_FORMAT_S32_LE:
        volume_control_s32(buf, buf_size);
        break;

    default:
        break;
    }
}

static int play_pcm(uint8_t *buf, size_t buf_size)
{
    int ret = 0;
    int frame_size = 0;
    int frames = 0;

    if (get_audio_flag(ALSA_PAUSE_FLAG) == FLAG_ON)
    {
        return 1;
    }

    frame_size = (wav_info.channels * (snd_pcm_format_width(wav_info.format) / 8));
    frames = buf_size / frame_size;

    volume_control((void *)buf, buf_size);

    while (1)
    {
        if (get_audio_flag(ALSA_ABORT_FLAG) == FLAG_ON)
        {
            return 1;
        }
        else if (get_audio_flag(ALSA_PAUSE_FLAG) == FLAG_ON)
        {
            return 1;
        }
        if ((ret = snd_pcm_writei(wav_info.pcm_handle, buf, frames)) < 0)
        {
            if (ret == -EAGAIN) // 비동기 입력시 에러
            {
                continue;
            }
            else if (ret == -EPIPE) // 언더런(쓰기의 경우), 오버런(읽기의 경우)
            {
                printr("pcm write underrun : %s", snd_strerror(ret));
                snd_pcm_recover(wav_info.pcm_handle, ret, 0); // 드라이버 버그가 있는 듯? 리커버해도 복구 안됨
                return -1;
            }
            else if (ret == -ESTRPIPE) // 멈추거나 했을 경우 [snd_pcm_pause - 드라이버에 따라 구현 안됐을 수 있음]
            {
                printr("pcm write suspend : %s", snd_strerror(ret));
                snd_pcm_recover(wav_info.pcm_handle, ret, 0);
                return -1;
            }
            else
            {
                printr("pcm write : %s", snd_strerror(ret));
                snd_pcm_recover(wav_info.pcm_handle, ret, 0);
                return -1;
            }
        }
        else
        {
            return 1;
        }
    }
}

static int conversion_wav_file(wav_file_header_u *header)
{
    if (header->packet.audio_format == 1) // pcm 포멧일 때만
    {
        wav_info.channels = header->packet.num_channels;
        wav_info.rate = header->packet.sample_rate;
        switch (header->packet.bit_per_sample) // https://homspace.nl/samplerbox/WAVE%20File%20Format.htm Sample Points and Sample Frames 참조
        {
        case 8:
            wav_info.format = SND_PCM_FORMAT_U8;
            break;

        case 16:
            wav_info.format = SND_PCM_FORMAT_S16_LE;
            break;

        case 24:
            wav_info.format = SND_PCM_FORMAT_S24_LE;
            break;

        case 32:
            wav_info.format = SND_PCM_FORMAT_S32_LE;
            break;

        default:
            break;
        }

        return 1;
    }
    else
    {
        return -1;
    }
}

static int check_file_change()
{
    char temp_file_route[256] = {
        0,
    };

    if (get_file_route(temp_file_route, sizeof(temp_file_route)) < 0)
    {
        return -1;
    }

    if (strcmp(wav_info.file_route, temp_file_route) == 0)
    {
        return -1;
    }
    else
    {
        printg("파일 체크 성공 : %s", temp_file_route);
        strcpy(wav_info.file_route, temp_file_route);
        return 1;
    }
}

static int play_wav(int header_size, wav_file_header_u *header)
{
    int64_t play_size = 0;
    int pause_work_flag = 0;

    uint8_t data[4096] = {
        0,
    };
    int len = 0;

    if (conversion_wav_file(header) < 0)
    {
        printr("audio format is not pcm type...");
        return -1;
    }

    if (set_hw_params(&wav_info) < 0)
    {
        printr("hw parameter set fail");
        return -1;
    }

    if (system("clear") < 0)
    {
        printr("fail to clear : %s", strerror(errno));
        return -1;
    }
    printf_wav_info(header);
    printd("파라미터 설정 완료 [%s]", HW_PLAY_DEVICE);
    printd("헤더 크기 [%d], 재생 시작 [%s]", header_size, wav_info.file_route);

    while (1)
    {
        if (get_audio_flag(ALSA_ABORT_FLAG) == FLAG_ON)
        {
            break;
        }
        else if (get_audio_flag(ALSA_PAUSE_FLAG) == FLAG_ON)
        {
            if (pause_work_flag == FLAG_ON)
            {
                play_size -= sizeof(data) * WAV_RECOVER_DROP_SIZE; // drop시 버린 버퍼 복구, 파일 오프셋 뒤로 이동 ㄱㄱ
                play_size = (play_size < 0) ? 0 : play_size;
                lseek(wav_info.fd, header_size + play_size, SEEK_SET);

                drop_pcm(); //내부 버퍼에 저장된 PCM데이터를 모두 버림, 즉시 중단
                set_hw_params(&wav_info); // 하드웨어 파라미터 설정 안하면, drop 이후 bad file descriptor 오류 발생, 아마 드랍시 드라이버가 하드웨어 초기화 하는 듯
                pause_work_flag = FLAG_OFF;
            }
            continue;
        }

        len = read(wav_info.fd, data, sizeof(data));
        if (len < 0)
        {
            printr("fail to read : %s", strerror(errno));
        }
        else if (len < sizeof(data))
        {
            pause_work_flag = FLAG_ON;
            play_size += len;
            printd("audio 재생 끝");
            print_cordinate("18", "1", "재생중[%ld%%]", (play_size * 100) / (int64_t)header->packet.sub_chunk2_size);
            play_pcm(data, len);
            sleep(3);
            break;
        }
        else
        {
            pause_work_flag = FLAG_ON;
            play_size += len;
            print_cordinate("18", "1", "재생중[%ld%%]", (play_size * 100) / (int64_t)header->packet.sub_chunk2_size);
            play_pcm(data, len);
        }
    }

    close(wav_info.fd);
    return 1;
}

static int start_wav_conversion()
{
    uint64_t file_size = 0;
    wav_file_header_u header = {
        0,
    };

    int loop = 0;
    int conversion_flag = FLAG_OFF;
    int header_size = 0;

    if (check_file_change() < 0) // 파일이 같은지 체크
    {
        printd("file not change");
    }
    else
    {
        printd("file change");
    }

    if ((wav_info.fd = open(wav_info.file_route, O_RDONLY)) < 0)
    {
        printr("[%s]", wav_info.file_route);
        perror("fail to open wav");
        return -1;
    }

    if (read(wav_info.fd, &header, sizeof(header)) < 0)
    {
        printr("[%s]", wav_info.file_route);
        perror("fail to read file");
        return -1;
    }

    header_size += sizeof(header);

    for (loop = 0; loop < 1024; loop++)
    {
        if (memcmp(header.packet.sub_chunk2_id, "data", sizeof(header.packet.sub_chunk2_id)) != 0)
        {
            printd("expand header detected");
            header_size += sizeof(header.packet.sub_chunk2_id) + sizeof(header.packet.sub_chunk2_size) + header.packet.sub_chunk2_size;
            lseek(wav_info.fd, header.packet.sub_chunk2_size, SEEK_CUR);

            if (read(wav_info.fd, &header.packet.sub_chunk2_id, sizeof(header.packet.sub_chunk2_id)) < 0)
            {
                printr("[%s]", wav_info.file_route);
                perror("fail to read file");
                return -1;
            }
            if (read(wav_info.fd, &header.packet.sub_chunk2_size, sizeof(header.packet.sub_chunk2_size)) < 0)
            {
                printr("[%s]", wav_info.file_route);
                perror("fail to read file");
                return -1;
            }
        }
        else
        {
            printd("conversion complete");
            conversion_flag = FLAG_ON;
            break;
        }
    }

    if (conversion_flag == FLAG_ON)
    {
        play_wav(header_size, &header);
        return 1;
    }

    else
    {
        printr("변환 실패 : 필드 data를 찾을 수 없음");
        return -1;
    }
}

static int init_pcm()
{
    int ret = 0;

    audio_ctl.volume = 0.5;

    if ((ret = snd_pcm_open(&wav_info.pcm_handle, HW_PLAY_DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC)) < 0) // 논블록으로 열면 writei에서 언더런 발생, 이후 계속 언더런이 발생해서 음원 깨짐 (recover해도 깨짐)
    {
        printr("pcm open fail : %s", snd_strerror(ret));
        return -1;
    }

    if (sem_init(&audio_ctl.sem, 0, 1) < 0)
    {
        printr("fail to init sem : %s", strerror(errno));
        return -1;
    }

    printd("초기화 완료");
    return 1;
}

int start_playing_wav()
{
    set_audio_flag(ALSA_START_FLAG, FLAG_ON);
    set_audio_flag(ALSA_PAUSE_FLAG, FLAG_OFF);
    return 1;
}

int restart_playing_wav()
{
    abort_playing_wav();
    usleep(100 * 1000);
    start_playing_wav();
}

int pause_playing_wav()
{
    set_audio_flag(ALSA_PAUSE_FLAG, FLAG_ON);
    return 1;
}

int continue_playing_wav()
{
    set_audio_flag(ALSA_PAUSE_FLAG, FLAG_OFF);
    return 1;
}

int abort_playing_wav()
{
    set_audio_flag(ALSA_ABORT_FLAG, FLAG_ON);
    return 1;
}

int set_file_route(char *file_route)
{
    sem_wait(&audio_ctl.sem);
    strcpy(audio_ctl.file_route, file_route);
    sem_post(&audio_ctl.sem);
}

int get_file_route(char *file_route, int file_route_size)
{
    if (file_route_size < sizeof(audio_ctl.file_route))
    {
        printr("file size is too small");
        return -1;
    }

    sem_wait(&audio_ctl.sem);
    strcpy(file_route, audio_ctl.file_route);
    sem_post(&audio_ctl.sem);

    return 1;
}

void thread_wav_playing()
{
    init_pcm();

    while (1)
    {
        if (get_audio_flag(ALSA_START_FLAG) == FLAG_ON)
        {
            printd("시작");
            start_wav_conversion();
            drop_pcm();
            set_audio_flag(ALSA_START_FLAG, FLAG_OFF);
        }
        else if (get_audio_flag(ALSA_ABORT_FLAG) == FLAG_ON)
        {
            printd("중단");
            drop_pcm();
            set_audio_flag(ALSA_ABORT_FLAG, FLAG_OFF);
        }
        usleep(1 * 1000);
    }
}

void start_wav_playing_thread()
{
    pthread_t tid = 0;

    if (pthread_create(&tid, NULL, (void *)&thread_wav_playing, NULL) < 0)
    {
        printr("fail to create thread : %s", strerror(errno));
        return;
    }

    if (pthread_detach(tid) < 0)
    {
        printr("fail to detach thread : %s", strerror(errno));
        return;
    }
}