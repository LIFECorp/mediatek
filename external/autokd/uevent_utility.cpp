#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/rtc.h>
#include <sys/mman.h>
#include <utils/Log.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <pthread.h>
#include "autok.h"
#define UEVENT_MSG_LEN  1024
struct uevent {
    char *subsystem;
    int host_id;
    char *what;
};

static int open_uevent_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 64*1024; // XXX larger? udev uses 
    int on = 1;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if(s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));
    setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }

    return s;
}

static int parse_event(char *msg, struct uevent *uevent)
{
    uevent->subsystem = (char*)"";
    uevent->host_id = -1;
    uevent->what = (char*)"";
    
    /* currently ignoring SEQNUM */
    while(*msg) {
        if(!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            uevent->subsystem = (char*)calloc(strlen(msg)+1, sizeof(char));
            strcpy(uevent->subsystem, msg);
        } else if(!strncmp(msg, "HOST=", 5)) {
            msg += 5;
            uevent->host_id = atoi(msg);    
        } else if(!strncmp(msg, "WHAT=", 5)) {
            msg += 5;
            uevent->what = (char*)calloc(strlen(msg)+1, sizeof(char));
            strcpy(uevent->what, msg);    
        }

            /* advance to after the next \0 */
        while(*msg++)
            ;
    }
    
    if(!strncmp(uevent->subsystem, "mmc_host", 8))
        return 0;
    else
        return -1;
    
}

void* handle_device_fd(void *file_desc)
{   
    int fd = *((int*)file_desc);
    //printf("enter %s\n", __func__);
    for(;;) {
        char msg[UEVENT_MSG_LEN+2];
        char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
        struct iovec iov = {msg, sizeof(msg)};
        struct sockaddr_nl snl;
        struct msghdr hdr = {&snl, sizeof(snl), &iov, 1, cred_msg, sizeof(cred_msg), 0};

        ssize_t n = recvmsg(fd, &hdr, 0);
        if (n <= 0) {
            break;
        }
        if ((snl.nl_groups != 1) || (snl.nl_pid != 0)) {
            /* ignoring non-kernel netlink multicast message */
            continue;
        }
        struct cmsghdr * cmsg = CMSG_FIRSTHDR(&hdr);
        if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
            /* no sender credentials received, ignore message */
            continue;
        }
        struct ucred * cred = (struct ucred *)CMSG_DATA(cmsg);
        if (cred->uid != 0) {
            /* message from non-root user, ignore */
            continue;
        }
        if(n >= UEVENT_MSG_LEN) /* overflow -- discard */
            continue;

        msg[n] = '\0';
        msg[n+1] = '\0';

        struct uevent *uevent = (struct uevent*)malloc(sizeof(struct uevent));
        if(!parse_event(msg, uevent)){
            //printf("%s\n", msg);
            //printf("event { %s, %d, %s }\n", uevent->subsystem, uevent->host_id, uevent->what);
            pthread_exit((void *)uevent);
            return (void *)uevent;
        }
    }
    pthread_exit(NULL);
    return NULL;
}

int wait_sdio_uevent(int *id, const char *keyword)
{
    static int fd = 0;
    pthread_t uevent_tid;
    struct uevent *event = NULL;
    int ret = 0;
    if(fd <= 0){
        fd = open_uevent_socket();
        if (fd < 0) {
            printf("error!\n");
            return -1;
        }
    }
    
    while(1){
        ret = pthread_create(&uevent_tid, NULL, handle_device_fd, &fd);
        if(ret<0) { 
            printf("Thread Creation Failed\n");
            return 1; 
        }
        pthread_join(uevent_tid, (void **)&event);
        if(strncmp(event->what, keyword, strlen(keyword))==0){
            printf("test_event { %s, %d, %s }\n", event->subsystem, event->host_id, event->what);
            *id = event->host_id;
            free(event->subsystem);
            free(event->what);
            free(event);
            break;
        }
    }

    return 0;
}

