#include "stdlib.h"
#include "fcntl.h"
#include "errno.h"

#define LOG_TAG "wlanLoader"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

/*mtk80707, workaround ALPS01428007*/
static char current_cpu_perf_mode[PROPERTY_VALUE_MAX] = {0};
static const char CPU_PERF_MODE[]="persist.mtk_wifi.cpu_perf"; /*perf: performance mode*/

static void EnablePerf()
{
    char CmdStr[128];
    
    memset(CmdStr, 0, sizeof(CmdStr));
    ALOGD("enter -->%s\n, uid=%d, gid=%d", __func__, getuid(), getgid());
    
    if (property_get(CPU_PERF_MODE, current_cpu_perf_mode, NULL)) {
        /* Turn on cpu performance mode in conditional, workaround 11n WFA test plan 5.2.4 */
        if (strcmp(current_cpu_perf_mode, "6589") == 0) {
            ALOGD("System cmds to turn on cpu performance mode for 6589\n");
            sprintf(CmdStr, "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
            system(CmdStr);
            sprintf(CmdStr, "echo 0 > /sys/module/mt_hotplug_mechanism/parameters/g_enable");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu1/online");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu2/online");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu3/online");
            system(CmdStr);
            usleep(50000);
        }
        else if (strcmp(current_cpu_perf_mode, "8135") == 0) {
            ALOGD("System cmds to turn on cpu performance mode for 8135\n");
            sprintf(CmdStr, "echo bypass=1 > /data/data/hotplug/cmd");
            system(CmdStr);
            sprintf(CmdStr, "echo 0 > /sys/module/mt_hotplug_mechanism/parameters/g_enable");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu1/online");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu2/online");
            system(CmdStr);
            sprintf(CmdStr, "echo 1 > /sys/devices/system/cpu/cpu3/online");
            system(CmdStr);
            sprintf(CmdStr, "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
            system(CmdStr);
            sprintf(CmdStr, "echo performance > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor");
            system(CmdStr);
            usleep(50000);
        }
    }
}

int main(int argc, char *argv[])
{
    EnablePerf();
    return 0;
}
