#include "menu_ctl.h"
#include "alsa_ctl.h"

int menu_num = 0;

static int printf_file(char *file_route, char *file_name)
{
    char dir_content[4096] = {
        0,
    };
    int count = 0;
    int enter_flag = FLAG_OFF;
    DIR *dir = NULL;
    struct dirent *dir_info = NULL;

    if ((dir = opendir(file_route)) == NULL)
    {
        printr("fail to print file : %s", strerror(errno));
        return -1;
    }

    while ((dir_info = readdir(dir)) != NULL)
    {
        if (strstr(dir_info->d_name, file_name) != 0)
        {
            sprintf(&dir_content[strlen(dir_content)], "%s ", dir_info->d_name);
            enter_flag = FLAG_ON;
            count++;
        }

        if (count % 6 == 5 && enter_flag == FLAG_ON)
        {
            count = 0;
            enter_flag = FLAG_OFF;
            sprintf(&dir_content[strlen(dir_content)], "\n");
        }
    }

    print_cordinate("2", "1", "%s", dir_content);
}

static void print_menu_list()
{
    if (system("clear") < 0)
    {
        printr("fail to clear screen : %s", strerror(errno));
    }
    print_cordinate("1", "1", "===============================");
    printf_file("/root", ".wav");
    print_cordinate("4", "1", "[음악 메뉴]=====================");
    print_cordinate("5", "1", "1. 음악 재생");
    print_cordinate("6", "1", "2. 재시작");
    print_cordinate("7", "1", "3. 일시정지");
    print_cordinate("8", "1", "4. 계속");
    print_cordinate("9", "1", "5. 중단");
    print_cordinate("10", "1", "6. 볼륨 설정");
    print_cordinate("11", "1", "===============================");
}

static void play_music_menu()
{
    char temp_route[256] = {
        0,
    };
    int ret = 0;

    printf("음악 경로 입력 : ");
    ret = scanf("%s", temp_route);
    (void)ret;
    set_file_route(temp_route);
    start_playing_wav();
}

static void restart_music_menu()
{
    restart_playing_wav();
}

static void pause_music_menu()
{
    pause_playing_wav();
}

static void continue_music_menu()
{
    continue_playing_wav();
}

static void abort_music_menu()
{
    abort_playing_wav();
}

static void volume_control_menu()
{
    char volume[256] = {
        0,
    };
    int ret = 0;

    printf("볼륨 입력(0 ~ 100) : ");
    ret = scanf("%s", volume);
    (void)ret;
    set_audio_volume(atoi(volume));
}

static void process_menu(int menu_num)
{
    switch (menu_num)
    {
    case 1:
        play_music_menu();
        break;
    case 2:
        restart_music_menu();
        break;
    case 3:
        pause_music_menu();
        break;
    case 4:
        continue_music_menu();
        break;
    case 5:
        abort_music_menu();
        break;
    case 6:
        volume_control_menu();
        break;

    default:
        break;
    }
}

static void thread_menu()
{
    char menu_num[256] = {
        0,
    };
    int ret = 0;

    while (1)
    {
        usleep(100 * 1000);
        print_menu_list();
        memset(menu_num, 0, sizeof(menu_num));
        ret = scanf("%s", menu_num);
        (void)ret;
        process_menu(atoi(menu_num));
    }
}

void start_menu_thread()
{
    pthread_t tid = 0;

    if (pthread_create(&tid, NULL, (void *)&thread_menu, NULL) < 0)
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