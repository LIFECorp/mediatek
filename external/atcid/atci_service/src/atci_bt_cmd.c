#include <sys/reboot.h>
#include <stdint.h>
#include <fcntl.h>

#include "atci_service.h"
#include "atcid_util.h"

static void addrToString(const unsigned char *addr, char *str){
    if(addr && str){
        sprintf(str, "%02X%02X%02X%02X%02X%02X", 
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    }
}

static void stringToAddr(const char *str, unsigned char *addr){
    char tmp[3] = {0,0,0};
    int i;
    if(str && addr){
        for(i = 0;i < 6;i++){
            tmp[0] = *(str++);
            tmp[1] = *(str++);
            addr[i] = (unsigned char)strtol(tmp, NULL, 16);
        }
    }
}

int
btad_cmd_process(
    char* cmdline,
    ATOP_t at_op,
    char* response
    )
{
    int ret = -1;

    if(at_op == AT_SET_OP) {
        int i, count;
        char c_addr[13];
        unsigned char addr[6];
        c_addr[12] = 0;
        for(i = 0, count = 0; count < 12 && cmdline[i] != '\0' ; i++) {
            if((cmdline[i] >= '0' && cmdline[i] <= '9')
            || (cmdline[i] >= 'A' && cmdline[i] <= 'F')
            || (cmdline[i] >= 'a' && cmdline[i] <= 'F')){
                c_addr[count] = cmdline[i];
                count++;
                ALOGD("%d : %c", count, cmdline[i]);
            }
        }
        if(count ==12)
            stringToAddr(c_addr, addr);
        if(count ==12 && writeBTAddr(addr) >= 0){
            ret = 0;
        }
        if(ret != 0){
            sprintf(response, "\r\n%cBLUETOOTH ADDRESS WRITE FAIL%c\r\n\r\nERROR\r\n", STX, ETX);
        }else{
            sprintf(response, "\r\n%cBLUETOOTH ADDRESS WRITE OK%c\r\n\r\nOK\r\n", STX, ETX);
        }   
    }else if(at_op == AT_READ_OP
            || at_op == AT_ACTION_OP) {
        char c_addr[13];
        unsigned char addr[6];
        if(readBTAddr(addr) < 0){
            sprintf(response, "\r\n%cBLUETOOTH ADDRESS READ FAIL%c\r\n\r\nERROR\r\n", STX, ETX);
        }else{
            addrToString(addr, c_addr);
//LGE_BT jaehun7.kim@lge.com 20120924 default address [s]
			if(strncmp(c_addr,"000000000000",12)==0){
              strncpy(c_addr,"0005c9000000",12);  
			}
//LGE_BT jaehun7.kim@lge.com 20120924 default address [e]
            sprintf(response, "\r\n%c%s%c\r\n\r\nOK\r\n", STX, c_addr, ETX);
            ret = 0;
        }
    }else if(at_op == AT_TEST_OP){
        sprintf(response, "\r\n%cAT%BTAD=[BD ADDR : 12 HEX nibble => 6 Bytes]%c\r\n\r\nOK\r\n", STX, ETX);
    }else{
        /* generate error message */
        sprintf(response, "\r\n\r\nNOT IMPLEMENTED\r\n\r\n");        
    }

    #if 0
    if(ret != 0) {
        /* generate error message */
        sprintf(response, "\r\n\r\nNOT IMPLEMENTED\r\n\r\n");
    }
    #endif

    return ret;
}

int
bt_btad_cmd_handler(
    char* cmdline,
    ATOP_t at_op,
    char* response
    )
{
    ALOGD("bt_btad_cmd_handler : %s", cmdline);
    return btad_cmd_process(cmdline, at_op, response);
}
