#include "alsa_ctl.h"
#include "menu_ctl.h"

int main()
{
    start_wav_playing_thread();
    start_menu_thread();
    while(1);
}