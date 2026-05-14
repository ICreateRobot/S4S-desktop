#include "hw_sys.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_pin"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/
hw_sys_c hw_sys;


/********************
 * global functions
 *******************/


/*******************
 * class functions
 *******************/
hw_sys_c::hw_sys_c(void)
{}
hw_sys_c::~hw_sys_c(void)
{}

void hw_sys_c::version(uint8_t version[3])
{
    version[0] = VERSION_0;
    version[1] = VERSION_1;
    version[2] = VERSION_2;
}

String hw_sys_c::version(void)
{
    char    string_buffer[28] = {0};
    uint8_t version[]         = {VERSION_0, VERSION_1, VERSION_2};
    sprintf(string_buffer, "%d.%d.%d", version[0], version[1], version[2]);
    return String(string_buffer);
}

uint32_t hw_sys_c::tick_get(void)
{
    return zst_tick_elaps(last_tick);
}

void hw_sys_c::tick_reset(void)
{
    last_tick = zst_tick_get();
}
