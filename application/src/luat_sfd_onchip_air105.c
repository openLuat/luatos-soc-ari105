#include "luat_base.h"
#include "luat_sfd.h"

// #ifdef LUAT_SFD_ONCHIP

#define LUAT_LOG_TAG "onchip"
#include "luat_log.h"

#include "app_interface.h"

extern const size_t script_luadb_start_addr;

int sfd_onchip_init (void* userdata) {
    sfd_onchip_t* onchip = (sfd_onchip_t*)userdata;
    if (onchip == NULL)
       return -1;
    onchip->addr = script_luadb_start_addr - 64*1024;
    return 0;
}

int sfd_onchip_status (void* userdata) {
    return 0;
}

int sfd_onchip_read (void* userdata, char* buff, size_t offset, size_t len) {
    int ret;
    sfd_onchip_t* onchip = (sfd_onchip_t*)userdata;
    if (onchip == NULL)
       return -1;
    mempcpy(buff, (char*)(offset + onchip->addr), len);
    return 0;
}

int sfd_onchip_write (void* userdata, const char* buff, size_t offset, size_t len) {
    int ret;
    sfd_onchip_t* onchip = (sfd_onchip_t*)userdata;
    if (onchip == NULL)
       return -1;
    ret = Flash_ProgramData(offset + onchip->addr, (uint32_t *)buff, len, 0);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}
int sfd_onchip_erase (void* userdata, size_t offset, size_t len) {
    int ret;
    sfd_onchip_t* onchip = (sfd_onchip_t*)userdata;
    if (onchip == NULL)
       return -1;
    ret = Flash_EraseSector(offset + onchip->addr, 0);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

int sfd_onchip_ioctl (void* userdata, size_t cmd, void* buff) {
    return 0;
}

// #endif
