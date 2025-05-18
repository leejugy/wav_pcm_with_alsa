#include "menu_ctl.h"
#include "alsa_ctl.h"

int menu_num = 0;

static void print_menu_list()
{
    if(system("clear") < 0)
    {
        printr("fail to clear screen : %s", strerror(errno));
    }
    printf("[음악 메뉴]=====================\n");
    printf("1. 음악 재생\n");
    printf("2. 재시작\n");
    printf("3. 일시정지\n");
    printf("4. 계속\n");
    printf("5. 중단\n");
    printf("===============================\n");
}

static void play_music_menu()
{
    char temp_route[256] = {0, };
    int ret = 0;

    printf("음악 경로 입력 : ");
    memset(temp_route, 0, sizeof(temp_route));
    ret = scanf("%s", temp_route);
    (void) ret;
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
    
    default:
        break;
    }
}

static void thread_menu()
{
    char menu_num[256] = {0, };
    int ret = 0;

    while(1)
    {
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

    if(pthread_create(&tid, NULL, (void *)&thread_menu, NULL) < 0)
    {
        printr("fail to create thread : %s", strerror(errno));
        return;
    }

    if(pthread_detach(tid) < 0)
    {
        printr("fail to detach thread : %s", strerror(errno));
        return;
    }
}