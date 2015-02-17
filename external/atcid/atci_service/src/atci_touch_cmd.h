#ifndef __ATCI_TOUCH_CMD_H__
#define __ATCI_TOUCH_CMD_H__

typedef struct {
    int action;
    int x;
    int y;
} TP_POINT;


int touch_cmd_handler(char* cmdline, ATOP_t at_op, char* response);   


#endif

