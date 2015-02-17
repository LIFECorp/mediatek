/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   
 *
 * Project:
 * --------
 *   
 *
 * Description:
 * ------------
 *   
 *
 * Author:
 * -------
 *   
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>

/* use nvram */
#include "CFG_Battery_Life_Time_Data_File.h"
#include "Custom_NvRam_LID.h"
#include "libnvram.h"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "battery_lifetime_data",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "battery_lifetime_data",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "battery_lifetime_data",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "battery_lifetime_data",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "battery_lifetime_data",__VA_ARGS__)
#define MAX_CHAR 100
#define MAX_LENGTH 1024

#define SYS_BAT_CYCLE "/sys/devices/platform/battery_meter/FG_Battery_Cycle"
#define SYS_BAT_MAX_VOL "/sys/devices/platform/battery_meter/FG_Max_Battery_Voltage"
#define SYS_BAT_MIN_VOL "/sys/devices/platform/battery_meter/FG_Min_Battery_Voltage"
#define SYS_BAT_MAX_CUR "/sys/devices/platform/battery_meter/FG_Max_Battery_Current"
#define SYS_BAT_MIN_CUR "/sys/devices/platform/battery_meter/FG_Min_Battery_Current"
#define SYS_BAT_MAX_TEMP "/sys/devices/platform/battery_meter/FG_Max_Battery_Temperature"
#define SYS_BAT_MIN_TEMP "/sys/devices/platform/battery_meter/FG_Min_Battery_Temperature"
#define SYS_BAT_AGING_FACTOR "/sys/devices/platform/battery_meter/FG_Aging_Factor"
#define BAT_RESET_FILE_PATH "/storage/sdcard0/nvram_need_reset_battery_lifetime_data"
#define NVRAM_INIT "nvram_init"

int write_nvram(unsigned char *ucNvRamData)
{
    F_ID nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    
    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    
    /* Sync with Nvram daemon ready */
    do {
        property_get(NVRAM_INIT, nvram_init_val, NULL);
        if(0 == strcmp(nvram_init_val, "Ready"))
            break;
        else {
            nvram_ready_retry ++;
            usleep(500000);
        }
    } while(nvram_ready_retry < 10);
    
    LOGD("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10){
        LOGE("Get NVRAM restore ready fails!\n");
        return -1;
    }
    
    /* Try Nvram first */
    nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_BATTERY_DATA_LID, &rec_size, &rec_num, ISWRITE);
    if(nvram_fd.iFileDesc >= 0){
        if(rec_num != 1){
            LOGE("Unexpected record num %d", rec_num);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        if(rec_size != sizeof(FILE_BATTERY_LIFE_TIME_DATA)){
            LOGE("Unexpected record size %d FILE_BATTERY_LIFE_TIME_DATA %d",
                    rec_size, sizeof(FILE_BATTERY_LIFE_TIME_DATA));
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        if(write(nvram_fd.iFileDesc, ucNvRamData, rec_num*rec_size) < 0){
            LOGE("Write NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        NVM_CloseFileDesc(nvram_fd);
    }
    else{
        LOGE("Open NVRAM fails errno %d\n", errno);
        return -1;        
    }
    
    return 0;
}

int read_nvram(unsigned char *ucNvRamData)
{
    F_ID nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    FILE_BATTERY_LIFE_TIME_DATA bat_nvram;
    
    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    
    /* Sync with Nvram daemon ready */
    do {
        property_get(NVRAM_INIT, nvram_init_val, NULL);
        if(0 == strcmp(nvram_init_val, "Ready"))
            break;
        else {
            nvram_ready_retry ++;
            usleep(500000);
        }
    } while(nvram_ready_retry < 10);
    
    LOGD("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10){
        LOGE("Get NVRAM restore ready fails!\n");
        return -1;
    }
    
    /* Try Nvram first */
    nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_BATTERY_DATA_LID, &rec_size, &rec_num, ISWRITE);
    if(nvram_fd.iFileDesc >= 0){
        if(rec_num != 1){
            LOGE("Unexpected record num %d", rec_num);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        if(rec_size != sizeof(FILE_BATTERY_LIFE_TIME_DATA)){
            LOGE("Unexpected record size %d FILE_BATTERY_LIFE_TIME_DATA %d",
                    rec_size, sizeof(FILE_BATTERY_LIFE_TIME_DATA));
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }

        if(read(nvram_fd.iFileDesc, &bat_nvram, rec_num*rec_size) < 0){
            LOGE("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        NVM_CloseFileDesc(nvram_fd);
    }
    else{
        LOGE("Open NVRAM fails errno %d\n", errno);
        return -1;        
    }
    
    memcpy(ucNvRamData, &bat_nvram, sizeof(FILE_BATTERY_LIFE_TIME_DATA));
    
    return 0;
}

int read_nvram_bat_info(unsigned char *ucNvRamData, int read_backup)
{
    F_ID nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    LT_BAT_LOG_INFO bat_info;
    
    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    off_t currpos;
    
    /* Sync with Nvram daemon ready */
    do 
    {
        property_get(NVRAM_INIT, nvram_init_val, NULL);
        if(0 == strcmp(nvram_init_val, "Ready"))
            break;
        else 
        {
            nvram_ready_retry ++;
            usleep(500000);
        }
    } while(nvram_ready_retry < 10);
    
    LOGD("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10)
    {
        LOGE("Get NVRAM restore ready fails!\n");
        return -1;
    }
    
    /* Try Nvram first */
    nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_BATTERY_DATA_LID, &rec_size, &rec_num, ISWRITE);
    if(nvram_fd.iFileDesc >= 0)
    {
        if(rec_num != 1)
        {
            LOGE("Unexpected record num %d", rec_num);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        if(rec_size != sizeof(FILE_BATTERY_LIFE_TIME_DATA))
        {
            LOGE("Unexpected record size %d FILE_BATTERY_LIFE_TIME_DATA %d", rec_size, sizeof(FILE_BATTERY_LIFE_TIME_DATA));
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }

        if(read_backup)
        {
            currpos = lseek(nvram_fd.iFileDesc, 0, SEEK_CUR);
            lseek(nvram_fd.iFileDesc, currpos + sizeof(FILE_BATTERY_LIFE_TIME_DATA) - sizeof(LT_BAT_LOG_INFO), SEEK_SET);
        }
        if(read(nvram_fd.iFileDesc, &bat_info, sizeof(LT_BAT_LOG_INFO)) < 0)
        {
            LOGE("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        NVM_CloseFileDesc(nvram_fd);
    }
    else
    {
        LOGE("Open NVRAM fails errno %d\n", errno);
        return -1;        
    }
    
    memcpy(ucNvRamData, &bat_info, sizeof(LT_BAT_LOG_INFO));
    
    return 0;
}

int write_nvram_bat_info(unsigned char *ucNvRamData, int write_backup)
{
    F_ID nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    char align_buffer[4096]={0}; 
    
    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    off_t currpos;
    
    /* Sync with Nvram daemon ready */
    do {
        property_get(NVRAM_INIT, nvram_init_val, NULL);
        if(0 == strcmp(nvram_init_val, "Ready"))
            break;
        else {
            nvram_ready_retry ++;
            usleep(500000);
        }
    } while(nvram_ready_retry < 10);
    
    LOGD("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10){
        LOGE("Get NVRAM restore ready fails!\n");
        return -1;
    }
    
    /* Try Nvram first */
    nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_BATTERY_DATA_LID, &rec_size, &rec_num, ISWRITE);
    if(nvram_fd.iFileDesc >= 0){
        if(rec_num != 1){
            LOGE("Unexpected record num %d", rec_num);
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }
        
        if(rec_size != sizeof(FILE_BATTERY_LIFE_TIME_DATA)){
            LOGE("Unexpected record size %d FILE_BATTERY_LIFE_TIME_DATA %d",
                    rec_size, sizeof(FILE_BATTERY_LIFE_TIME_DATA));
            NVM_CloseFileDesc(nvram_fd);
            return -1;
        }

        if(write_backup)
        {
            currpos = lseek(nvram_fd.iFileDesc, 0, SEEK_CUR);
            memcpy(&align_buffer[4096-sizeof(LT_BAT_LOG_INFO)], ucNvRamData, sizeof(LT_BAT_LOG_INFO));
            if (lseek(nvram_fd.iFileDesc, currpos + sizeof(FILE_BATTERY_LIFE_TIME_DATA) - 4096, SEEK_SET) < 0)
            {
                LOGE("Seek NVRAM fails errno %d\n", errno);
                NVM_CloseFileDesc(nvram_fd);
                return -1;
            }
                
            if (write(nvram_fd.iFileDesc, align_buffer, 4096) < 0)
            {
                LOGE("Write NVRAM fails errno %d\n", errno);
                NVM_CloseFileDesc(nvram_fd);
                return -1;
            }            
        }
        else
        {
            if (write(nvram_fd.iFileDesc, ucNvRamData, sizeof(LT_BAT_LOG_INFO)) < 0)
            {
                LOGE("Write NVRAM fails errno %d\n", errno);
                NVM_CloseFileDesc(nvram_fd);
                return -1;
            }            
        }
        
        NVM_CloseFileDesc(nvram_fd);
    }
    else{
        LOGE("Open NVRAM fails errno %d\n", errno);
        return -1;        
    }

    return 0;
}


int ReadBatInfo(char* path, int* value)
{
    char buffer[MAX_CHAR];
    FILE * pFile;
    
    pFile = fopen(path, "r");
    if(pFile == NULL)
    {
        LOGE("error opening file");
        return -1;
    }
    else
    {
        if(fgets(buffer, MAX_CHAR, pFile) == NULL)
        {
            LOGE("can not get the string from the file");
            return -1;
        }
    }
    *value = atoi(buffer);
    fclose(pFile);
    return 0;
}

int WriteBatInfo(char* path, int value)
{
    char buffer[MAX_CHAR];
    FILE * pFile;
    
    pFile = fopen(path, "w");
    if(pFile == NULL)
    {
        LOGE("error opening file");
        return -1;
    }
    else
    {
        fprintf(pFile, "%d", value);
    }
    fclose(pFile);
    return 0;
}

void InitDrvBatInfo(LT_BAT_LOG_INFO *pData)
{
    if (pData == NULL)
        return;

    WriteBatInfo(SYS_BAT_CYCLE, pData->battery_cycle);
    WriteBatInfo(SYS_BAT_MAX_VOL, pData->max_voltage);
    WriteBatInfo(SYS_BAT_MIN_VOL, pData->min_voltage);
    WriteBatInfo(SYS_BAT_MAX_CUR, pData->max_current);
    WriteBatInfo(SYS_BAT_MIN_CUR, pData->min_current);
    WriteBatInfo(SYS_BAT_MAX_TEMP, pData->max_temperature);
    WriteBatInfo(SYS_BAT_MIN_TEMP, pData->min_temperature);
    WriteBatInfo(SYS_BAT_AGING_FACTOR, pData->qmax_aging_factor);
}

int SyncBatInfo(LT_BAT_LOG_INFO *pData)
{
    int value, ret;
    int fgNeedSync = 0;

    if (pData == NULL)
        return 0;

    ret = ReadBatInfo(SYS_BAT_CYCLE, &value);
    if (ret >= 0)
    {
        if (value >= 0 && value != pData->battery_cycle && 
            (value > pData->battery_cycle && value - pData->battery_cycle < 3) )
        {
            fgNeedSync = 1;
            pData->battery_cycle = value;
        }
    }

    ret = ReadBatInfo(SYS_BAT_AGING_FACTOR, &value);
    if (ret >= 0)
    {
        if (value >=0 && value <= 100 && value != pData->qmax_aging_factor)
        {
            fgNeedSync = 1;
            pData->qmax_aging_factor = value;
        }
    }
    
    ret = ReadBatInfo(SYS_BAT_MAX_VOL, &value);
    if (ret >= 0)
    {
        if (value >= 0 && value > pData->max_voltage)
        {
            fgNeedSync = 1;
            pData->max_voltage = value;
        }
    }    

    ret = ReadBatInfo(SYS_BAT_MIN_VOL, &value);
    if (ret >= 0)
    {
        if (value >= 0 && value < pData->min_voltage)
        {
            fgNeedSync = 1;
            pData->min_voltage = value;
        }
    }  

    ret = ReadBatInfo(SYS_BAT_MAX_CUR, &value);
    if (ret >= 0)
    {
        if (value > pData->max_current)
        {
            fgNeedSync = 1;
            pData->max_current = value;
        }
    }    

    ret = ReadBatInfo(SYS_BAT_MIN_CUR, &value);
    if (ret >= 0)
    {
        if (value < pData->min_current)
        {
            fgNeedSync = 1;
            pData->min_current = value;
        }
    }  

    ret = ReadBatInfo(SYS_BAT_MAX_TEMP, &value);
    if (ret >= 0)
    {
        if (value > pData->max_temperature)
        {
            fgNeedSync = 1;
            pData->max_temperature = value;
        }
    }    

    ret = ReadBatInfo(SYS_BAT_MIN_TEMP, &value);
    if (ret >= 0)
    {
        if (value < pData->min_temperature)
        {
            fgNeedSync = 1;
            pData->min_temperature = value;
        }
    }  

    return fgNeedSync;
}

static unsigned long ComputeCheckSum(LT_BAT_LOG_INFO* pbat_info)
{
    unsigned long checkSum = 0;
    int flag = 1;
    int looptime = 0;
    int i;
    int *pTemp = (int*) pbat_info;
    
    looptime = (sizeof(LT_BAT_LOG_INFO) - sizeof(unsigned long))/sizeof(unsigned long); //exclude checksum.

    for (i = 0; i < looptime; i++)
    {
        if (flag)
        {
            checkSum ^= *pTemp;
            flag = 0;
        }
        else
        {
            checkSum += *pTemp;
            flag = 1;
        }
        pTemp++;
    }
    return checkSum;
}

int checkEmpty(LT_BAT_LOG_INFO* pData)
{
    LT_BAT_LOG_INFO zero = {0};
    
    if(0 == memcmp(pData, &zero, sizeof(LT_BAT_LOG_INFO)))
    {
        LOGD("Empty!");
        return 1;
    }
    else
        return 0;
}
    
void dumpBatteryData(FILE_BATTERY_LIFE_TIME_DATA* pData)
{
    LOGD("Dump:");
    LOGD("data: %d, %d, %d, %d, %d, %d, %d, %d %x\n", 
        pData->bat_info.battery_cycle, pData->bat_info.qmax_aging_factor, pData->bat_info.max_voltage
       ,pData->bat_info.min_voltage, pData->bat_info.max_current, pData->bat_info.min_current, 
       pData->bat_info.max_temperature, pData->bat_info.min_temperature,pData->bat_info.checksum);
    LOGD("backup data: %d, %d, %d, %d, %d, %d, %d, %d %x\n", 
        pData->bat_info_backup.battery_cycle, pData->bat_info_backup.qmax_aging_factor, pData->bat_info_backup.max_voltage
       ,pData->bat_info_backup.min_voltage, pData->bat_info_backup.max_current, pData->bat_info_backup.min_current, 
       pData->bat_info_backup.max_temperature, pData->bat_info_backup.min_temperature,pData->bat_info_backup.checksum);
}

int checkResetBatInfo(void)
{
    FILE * pFile;
    int fg_reset = 0;
    
    pFile = fopen(BAT_RESET_FILE_PATH, "r");
    if(pFile == NULL)
    {
        //LOGD("No reset file");
    }
    else
    {
        LOGD("Reset file is found and will reset battery lifetime data.");
        fg_reset = 1;
        fclose(pFile);
    }
    
    return fg_reset;
}

int main(int argc, char **argv)
{
    unsigned char ucNvRamData[sizeof(FILE_BATTERY_LIFE_TIME_DATA)] = {0};
    FILE_BATTERY_LIFE_TIME_DATA *pData = 0;
    int nvram_daemon_ready = 0;
    LT_BAT_LOG_INFO *pbat_info;
    LT_BAT_LOG_INFO *pbat_info_backup;
    unsigned long checkSum, checkSum_backup;
    
    LOGD("%s %d\n", __FILE__, __LINE__);
    
    while(1)
    {
        if(nvram_daemon_ready == 0)
        {
            /* Read Nvram data */
            if(read_nvram(ucNvRamData) < 0)
            {
                LOGE("Reading battery nvram data fails!\n");
                continue;
            }
            dumpBatteryData(ucNvRamData);

            pData = (FILE_BATTERY_LIFE_TIME_DATA*)ucNvRamData;
            pbat_info = &pData->bat_info;
            pbat_info_backup = &pData->bat_info_backup;

            checkSum = ComputeCheckSum(pbat_info);
            checkSum_backup = ComputeCheckSum(pbat_info_backup);

            LOGD("Check sum %x %x, source checksum %x %x", checkSum, checkSum_backup, pbat_info->checksum, pbat_info_backup->checksum);
            if ((checkSum != pbat_info->checksum && checkSum_backup != pbat_info_backup->checksum) ||
                (checkEmpty(pbat_info) && checkEmpty(pbat_info_backup)) ||
                (checkEmpty(pbat_info) && checkSum_backup != pbat_info_backup->checksum) ||
                (checkEmpty(pbat_info_backup) && checkSum != pbat_info->checksum) ||
                (checkResetBatInfo()== 1))
            {
                LOGD("Need initialize battery nvram data\n");
                pbat_info->battery_cycle = 0;
                pbat_info->qmax_aging_factor = 100;
                pbat_info->max_voltage = 0;
                pbat_info->min_voltage = 10000;
                pbat_info->max_current = 0;
                pbat_info->min_current = 0;
                pbat_info->max_temperature = -20;
                pbat_info->min_temperature = 120;
                pbat_info->checksum = ComputeCheckSum(pbat_info);
                memcpy(pbat_info_backup, pbat_info, sizeof(LT_BAT_LOG_INFO));

                #if 0
                if (write_nvram(ucNvRamData) < 0)
                {
                    LOGE("[init info] update battery nvram data fails!\n");
                }
                else
                {
                    LOGD("[init info]update battery nvram data succeed!\n");
                }
                #endif
                
                if (write_nvram_bat_info(pbat_info, 0) < 0)
                {
                    LOGE("[init info] update battery nvram data fails!\n");
                }
                else
                {
                    LOGD("[init info] update battery nvram data succeed!\n");
                }

                sleep(1);

                if (write_nvram_bat_info(pbat_info_backup, 1) < 0)
                {
                    LOGE("[init info] update battery backup data fails!\n");
                }
                else
                {
                    LOGD("[init info]update battery backup data succeed!\n");
                }                
								
                nvram_daemon_ready = 1;
                continue;;
            }
            else
            {            
                // correct bat_info or backup
                if (checkSum != pbat_info->checksum)
                {
                    LOGE("corrupted bat_info nvram data: %d, %d, %d, %d, %d, %d, %d, %d\n", pbat_info->battery_cycle, pbat_info->qmax_aging_factor, pbat_info->max_voltage
                        ,pbat_info->min_voltage, pbat_info->max_current, pbat_info->min_current, pbat_info->max_temperature, pbat_info->min_temperature);
                    
                    memcpy(pbat_info, pbat_info_backup, sizeof(LT_BAT_LOG_INFO));

                    if (write_nvram_bat_info(pbat_info, 0) < 0)
                    {
                        LOGE("[restore bat info]update battery nvram data fails!\n");
                    }
                    else
                    {
                        LOGD("[restore bat info]update battery nvram data succeed!\n");
                    }
                }
                else if (checkSum_backup != pbat_info_backup->checksum)
                {
                    LOGE("corrupted bat_info_backup nvram data: %d, %d, %d, %d, %d, %d, %d, %d\n", pbat_info_backup->battery_cycle, pbat_info_backup->qmax_aging_factor, pbat_info_backup->max_voltage
                        ,pbat_info_backup->min_voltage, pbat_info_backup->max_current, pbat_info_backup->min_current, pbat_info_backup->max_temperature, pbat_info_backup->min_temperature);
                    
                    memcpy(pbat_info_backup, pbat_info, sizeof(LT_BAT_LOG_INFO));
                    if (write_nvram_bat_info(pbat_info_backup, 1) < 0)
                    {
                        LOGE("[restore bat info]update battery nvram data fails!\n");
                    }
                    else
                    {
                        LOGD("[restore bat info]update battery nvram data succeed!\n");
                    }
                    
                }
            }

            LOGD("nvram data: %d, %d, %d, %d, %d, %d, %d, %d\n", pbat_info->battery_cycle, pbat_info->qmax_aging_factor, pbat_info->max_voltage
              ,pbat_info->min_voltage, pbat_info->max_current, pbat_info->min_current, pbat_info->max_temperature, pbat_info->min_temperature);
              
            // write to battery driver
            InitDrvBatInfo(pbat_info);
            nvram_daemon_ready = 1;
        }
        else
        {
            // polling sysfs, check update and write to nvram
            if (SyncBatInfo(pbat_info))
            {
                pbat_info->checksum = ComputeCheckSum(pbat_info);
                memcpy(pbat_info_backup, pbat_info, sizeof(LT_BAT_LOG_INFO));

                if (write_nvram_bat_info(pbat_info, 0) < 0)
                {
                    LOGE("[sync bat info] update battery nvram data fails!\n");
                }
                else
                {
                    LOGD("[sync bat info] update battery nvram data succeed!\n");
                }

                sleep(1);

                if (write_nvram_bat_info(pbat_info_backup, 1) < 0)
                {
                    LOGE("[sync bat info] update battery backup data fails!\n");
                }
                else
                {
                    LOGD("[sync bat info] update battery backup data succeed!\n");
                }        
            }            
            
            LOGD("battery data: %d, %d, %d, %d, %d, %d, %d, %d\n", pbat_info->battery_cycle, pbat_info->qmax_aging_factor, pbat_info->max_voltage
              ,pbat_info->min_voltage, pbat_info->max_current, pbat_info->min_current, pbat_info->max_temperature, pbat_info->min_temperature);

            /*
            LOGD("backup battery data: %d, %d, %d, %d, %d, %d, %d, %d\n", pbat_info_backup->battery_cycle, pbat_info_backup->qmax_aging_factor, pbat_info_backup->max_voltage
              ,pbat_info_backup->min_voltage, pbat_info_backup->max_current, pbat_info_backup->min_current, pbat_info_backup->max_temperature, pbat_info_backup->min_temperature);            
            */
        }
        sleep(1800);
    }

  return 0;
}

