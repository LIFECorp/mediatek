//#ifdef BACKUP_RESTORE_SERVICE
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "backuprestore.h"

#define LOG_TAG "Backup_Restore_Service"
#include <cutils/log.h>
#include <cutils/sockets.h>

#define SOCKET_PATH "backuprestore"
#define QBACKLOG 5 //the queue length in the connection pool.
#define CMD_LENGTH 2048 
#define MAX_TOKEN 3
#define REPLY_LEN 10
#define PATH_LENGTH 256

static int srv_create_sk();
static int backup(char** args , char reply[REPLY_LEN]);
static int restore(char** args, char reply[REPLY_LEN]);
static int do_exit(char** args, char reply[REPLY_LEN]);
static int excute(int socket, char* cmd);
static int parseCmd(char* cmd);
static int readx(int socket, void* buf, int count);
static int writex(int socket, const void* buf, int count);
static inline void err_sys(const char* errstring);
static inline void info_sys(const char* info);
static inline void debug_sys(const char* debuginfo);
static int backupAppData(const char* dirSrc, const char* dirDest);

typedef struct cmd_struct{
    const char* cmd_name;
    const int cmd_args_cnt;
    int (*excute)(char** args, char reply[REPLY_LEN]);
}cmd_struct;

static cmd_struct cmds[] = {
    {"Backup", 3, backup},
    {"Restore", 3, restore},
    {"Exit", 0, do_exit},
};

static char cmd[CMD_LENGTH];
static char* cmdArgs[MAX_TOKEN];

int main(const int argc, const char *argv[])
{
	int server_fd, client_fd;
	int read_size;
	short cmd_len;
	struct sockaddr addr;
	socklen_t alen;

	info_sys("backuprestore service start!");

	if ((server_fd = srv_create_sk()) < 0)
		err_sys("create socket server error");
	alen = sizeof(addr);

	info_sys("Server socket create successfully!");

	while(1){
		client_fd = accept(server_fd, (struct sockaddr*) &addr, &alen);
		if (client_fd < 0) {
			debug_sys("socket accept failed, please try again!");
			continue;
		}
		info_sys("before excute");
		while (1) {
			cmd_len = 0;
			if (readx(client_fd, &cmd_len, sizeof(cmd_len)) <= 0){
				debug_sys("cmd length read fail");
				break;
			}

			if (cmd_len < 1 || cmd_len > 2048){
				debug_sys("cmd length is not right");
				break;
			}

			if (readx(client_fd, &cmd, cmd_len) <= 0){
				debug_sys("read cmd fail");
				break;
			}
			cmd[cmd_len] = 0;
			if (excute(client_fd, cmd) < 0)
				break;
		}
		close(client_fd);
	}
	close(server_fd);
	info_sys("backuprestore service end!");
	return EXIT_SUCCESS;
}

static int srv_create_sk()
{
	int lsocket;
	info_sys("Get socket from Android env!");
	lsocket = android_get_control_socket(SOCKET_PATH);
	if (lsocket < 0){ 
		debug_sys("Failed to get socket from environment");
        return -1;
    }
	
	if (listen(lsocket, QBACKLOG)) {
		debug_sys("Listen on socket failed");
        return -1;
	}
	fcntl(lsocket, F_SETFD, FD_CLOEXEC);
	return lsocket;
}

int copy_file(char* spath,char* dpath)    
{
    int nbyte ;                        
    int pDir_s,pDir_d ;                
    char buf[BUFSIZ] ;                 
    struct stat file_stat ;           
    struct utimbuf mod_time;         
    stat(spath,&file_stat) ;         
    if((pDir_s=open(spath,0)) == -1)     
    {
        //exit(0);
        ALOGI("copy_file() pDir_s is NULL");
        return -1;
    }

    pDir_d=creat(dpath, file_stat.st_mode);                   
    while((nbyte = read(pDir_s,buf,BUFSIZ)) > 0)
    {
        if(write(pDir_d,buf,nbyte) != nbyte)
        {
            //exit(0);
            ALOGI("copy_file() write fail");
            return -1;
        }
    }
    mod_time.actime = file_stat.st_atime ;          
    mod_time.modtime = file_stat.st_mtime ;
    utime(dpath,&mod_time) ;
    chmod(dpath,file_stat.st_mode);                   
    ALOGI("copy_file() st_mode:%d", file_stat.st_mode);

    close(pDir_s) ;                                  
    close(pDir_d) ;
    return 0;
}

void cleardir(char *pdir )
{
    DIR *pDir=NULL;
    struct dirent *ent = NULL;
    char path[PATH_LENGTH]=""; 

    pDir = opendir(pdir);

    if (pDir != NULL) {
        while(NULL !=(ent=readdir(pDir)))
        {
            if(strcmp(ent->d_name,"..")==0||strcmp(ent->d_name,".")==0)
                continue ;

            strcpy(path,pdir);
            if(4 == ent->d_type)
            {
                strcat(path,"/");
                strcat(path,ent->d_name);
                cleardir(path);
            }
            else
            {
                strcat(path,"/");
                strcat(path,ent->d_name);
                remove(path);
            }
        }
        rmdir(pdir);
        closedir(pDir);
    }
}

int app_data_del(char *pathSrc)
{
    DIR *pDir=NULL;
    char path[PATH_LENGTH]=""; 
    struct dirent *ent = NULL;//目录属性 

    if((pDir = opendir(pathSrc)) == NULL)
    {
        return -1;
    }   

    while(NULL !=(ent=readdir(pDir)))
    {
        if(strcmp(ent->d_name,"..")==0||strcmp(ent->d_name,".")==0) //遇到子目录'.'或父母录标记'..'，继续
        continue ;

 	strcpy(path, pathSrc);
        if(4 == ent->d_type)//子目录
        {
             strcat(path,"/");
             strcat(path,ent->d_name);
             cleardir(path);
        }
        else
        {
            strcat(path,"/");
            strcat(path,ent->d_name);
            remove(path);
        }
    }

    if(pDir != NULL)
    {
       closedir(pDir);
    }

    return 0;
}

int change_folder_owner(char *pathSrc, uid_t userId , gid_t groupId)
{
    DIR *pDir=NULL;
    char path[PATH_LENGTH]=""; 
    struct dirent *ent = NULL;//目录属性 
    struct stat info;

    if((pDir = opendir(pathSrc)) == NULL)
    {
        return -1;
    }   

    while(NULL !=(ent=readdir(pDir)))
    {
        if(strcmp(ent->d_name,"..")==0||strcmp(ent->d_name,".")==0) //遇到子目录'.'或父母录标记'..'，继续
            continue ;

 	strcpy(path, pathSrc);
	stat(path, &info);  // Error check omitted
	struct passwd *pw = getpwuid(info.st_uid);
	if(strcmp(pw->pw_name, "system") == 0){
              info_sys("backuprestore uid  = system!");
		continue;
         }

        strcat(path,"/");
        strcat(path,ent->d_name);
        chown(path, userId, groupId);
        if(4 == ent->d_type)//子目录
        {
	     change_folder_owner(path, userId, groupId);     
	    
        }
    }

    chown(pathSrc, userId, groupId);

    if(pDir != NULL)
    {
       closedir(pDir);
    }

    return 0;
}

 void change_owner(char *path) {
    struct stat info;
	const char *filename = path;  
	stat(filename, &info);  // Error check omitted
	struct passwd *pwd = getpwuid(info.st_uid);
        change_folder_owner(path, pwd->pw_uid, pwd->pw_uid);
}


static int backup(char** args , char reply[REPLY_LEN])
{
    info_sys("call backup");
    ALOGI("srcpath: %s; destpath: %s", args[0], args[1]);
    app_data_del(args[1]); // delete the file but the parent folder
    int ret = backupAppData(args[0], args[1]);
    if(ret >= 0){
        struct stat selfstat;
        stat("/data/data/com.mediatek.backuprestore", &selfstat);
        struct passwd *pw = getpwuid(selfstat.st_uid);
        if (pw != NULL)
        {
            change_folder_owner(args[1], pw->pw_uid, pw->pw_gid);
        }
        return 1;
    } else {
	  cleardir(args[1]); //if copy fail,then delete the copied file and return -1
	  return -1;
	}
}


static int backupAppData(const char* dirSrc, const char* dirDest)
{
    int result = 0;
    DIR* dirSrc_p = NULL;
    struct dirent* dirent_p = NULL;
    char src[PATH_LENGTH] = "", dest[PATH_LENGTH] = "";
    struct stat tmpstat;
    struct stat selfstat;

    stat(dirSrc, &selfstat);
    ALOGI("backupAppData() dirSrc() %s,selfstat.stmode:%d ", dirSrc, selfstat.st_mode);
    if ((dirSrc_p = opendir(dirSrc)) == NULL)
    {
        ALOGI("backupAppData() opendir() %s is NULL", dirSrc);
        result = -1;
    }

    if (result >= 0 && stat(dirDest, &tmpstat) != 0)
    {
        result = mkdir(dirDest, selfstat.st_mode);
        chmod(dirDest,selfstat.st_mode);
        stat(dirDest, &selfstat);
        ALOGI("backupAppData() after chmod dirDest() %s, selfstat.stmode:%d ", dirDest, selfstat.st_mode);
    }

    while (result >= 0 &&
           ((dirent_p = readdir(dirSrc_p)) != NULL))
    {
        if ((strcmp(dirent_p->d_name, "..") == 0) ||
            (strcmp(dirent_p->d_name, ".") == 0))
        {
            continue;
        }

        strcpy(src, dirSrc);
        strcat(src, "/");
        strcat(src, dirent_p->d_name);

        strcpy(dest, dirDest);
        strcat(dest, "/");
        strcat(dest, dirent_p->d_name);

        if (dirent_p->d_type == DT_DIR)
        {
            struct stat dirStat;
            DIR* dir_p;

            result = stat(dest, &dirStat);
            ALOGI("backupAppData() folder stat %s:result:%d; errno: %s", dest, result, strerror(errno));
            if (result != 0) // folder does not exists
            {
                if ((result = stat(src, &dirStat)) == 0)
                {
                    if ((result = mkdir(dest, dirStat.st_mode)) == 0)
                    {
                        chmod (dest, dirStat.st_mode);
                        stat(dest, &dirStat);
                        ALOGI("backupAppData() after chmod dirDest() %s, selfstat.stmode:%d ", dest, dirStat.st_mode);
                    }
                    else
                    {
                        ALOGI("backupAppData() folder mkdir() %s fail, result:%d", dest, result);
                    }
                }
                else
                {
                    ALOGI("backupAppData() folder stat %s:result:%d", dirDest, result);
                    result = -1;
                }
            }

            if (result >= 0)
            {
                result = backupAppData(src, dest);
            }
            else
            {
                ALOGI("backupAppData() folder result is -1");
            }
        }
        else if ((dirent_p->d_type == DT_REG) ||
                 (dirent_p->d_type == DT_UNKNOWN))
        {
            struct stat fileStat;

            if (stat(dest, &fileStat) == 0) // file exist
            {
                if (remove(dest) != 0)
                {
                    ALOGI("backupAppData() remove file fail");
                    result = -1;
                }
            }
            else
            {
                if (stat(dirDest, &fileStat) != 0) // file not exist, use parent folder property
                {
                    ALOGI("backupAppData() stat file fail");
                    result = -1;
                }
            }

            if (result >= 0)
            {
                result = copy_file(src, dest);
            }
            else
            {
                ALOGI("backupAppData() copy file result fail");
            }
        }
    }

    if (dirSrc_p != NULL)
    {
        closedir(dirSrc_p);
    }

    ALOGI("backupAppData() dirSrc:%s,dirDest:%s,result:%d", dirSrc, dirDest, result);
    return result;
}

static int restoreAppData(const char* dirSrc, const char* dirDest)
{
    int result = 0;
    DIR* dirSrc_p = NULL;
    struct dirent* dirent_p = NULL;
    char src[PATH_LENGTH] = "", dest[PATH_LENGTH] = "";
    struct stat tmpstat;

    if ((dirSrc_p = opendir(dirSrc)) == NULL)
    {
        ALOGI("restoreAppData() opendir() %s is NULL", dirSrc);
        result = -1;
    }

    if (stat(dirDest, &tmpstat) != 0)
    {
        stat(dirSrc, &tmpstat);
        mkdir(dirDest, tmpstat.st_mode);
        chmod(dirDest, tmpstat.st_mode);
    }

    while (result >= 0 &&
           ((dirent_p = readdir(dirSrc_p)) != NULL))
    {
        if ((strcmp(dirent_p->d_name, "..") == 0) ||
            (strcmp(dirent_p->d_name, ".") == 0))
        {
            continue;
        }

        strcpy(src, dirSrc);
        strcat(src, "/");
        strcat(src, dirent_p->d_name);

        strcpy(dest, dirDest);
        strcat(dest, "/");
        strcat(dest, dirent_p->d_name);
        if (dirent_p->d_type == 4)
        {
        	if(strcmp(dirent_p->d_name,"lib") == 0)
        	{
        		 ALOGI("restoreAppData() this folder is lib");
        		continue;
        	}
            struct stat dirStat;
            DIR* dir_p;

            result = stat(dest, &dirStat);
            ALOGI("restoreAppData() folder stat %s:result:%d", dest, result);
            if (result != 0) // folder does not exists
            {
                if ((result = stat(dirDest, &dirStat)) == 0)
                {
                    if ((result = mkdir(dest, dirStat.st_mode)) == 0)
                    {
                        struct passwd *pw = getpwuid(dirStat.st_uid);
                        if (pw != NULL)
                        {
                            result = chown(dest, pw->pw_uid, pw->pw_gid);
                             stat(src, &dirStat);
                             chmod(dest, dirStat.st_mode);
                            ALOGI("restoreAppData() chown folder result:%d", result);
                        }
                        else
                        {
                            ALOGI("restoreAppData() folder pw is null");
                            result = -1;
                        }
                    }
                }
                else
                {
                    ALOGI("restoreAppData() folder stat %s:result:%d", dirDest, result);
                    result = -1;
                }
            }

            if (result >= 0)
            {
                result = restoreAppData(src, dest);
            }
            else
            {
                ALOGI("restoreAppData() folder result is -1");
            }
        }
        else if ((dirent_p->d_type == DT_REG) ||
                         (dirent_p->d_type == DT_UNKNOWN))
        {
            struct stat fileStat;

            if (stat(dest, &fileStat) == 0) // file exist
            {
                if (remove(dest) != 0)
                {
                    ALOGI("restoreAppData()  errno: %s",  strerror(errno));
                    result = -1;
                }
            }
            else
            {
                if (stat(dirDest, &fileStat) != 0) // file not exist, use parent folder property
                {
                    ALOGI("restoreAppData() stat file fail");
                    result = -1;
                }
            }

            if (result >= 0)
            {
                struct passwd *pw = getpwuid(fileStat.st_uid);
                result = copy_file(src, dest);
                if (pw != NULL)
                {
                    result = chown(dest, pw->pw_uid, pw->pw_gid);

                    stat(src, &fileStat);
                    chmod(dest, fileStat.st_mode);
                    ALOGI("restoreAppData() chown file result:%d", result);
                }
                else
                {
                    ALOGI("restoreAppData() copy file fail");
                    result = -1;
                }
            }
            else
            {
                ALOGI("restoreAppData() copy file result fail");
            }
        }

        src[0] = 0;
        dest[0] = 0;
    }
    if (dirSrc_p != NULL)
    {
        closedir(dirSrc_p);
    }
    ALOGI("restoreAppData() dirSrc:%s,dirDest:%s,result:%d", dirSrc, dirDest, result);
    return result;
}

static int restore(char** args, char reply[REPLY_LEN])
{
    ALOGI("restore() srcpath:%s,destpath:%s", args[0], args[1]);
	int ret = restoreAppData(args[0], args[1]);
	if(ret<0){
		cleardir(args[1]);
	}
    return (ret >= 0) ? 1 : -1;
}

static int do_exit(char** args, char reply[REPLY_LEN]){
    exit(EXIT_SUCCESS);
}

static int excute(int socket, char* cmd){
    int argsCnt = 0, i = 0;
    short ret = -1;
    char reply[REPLY_LEN];
    if ((argsCnt = parseCmd(cmd)) < 0)
        goto done;

    for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); ++i){
        if (!strcmp(cmds[i].cmd_name, cmdArgs[0])){
            if (cmds[i].cmd_args_cnt != argsCnt)
            {
                debug_sys("cmds args is not map");
                goto done;               
            }
            else
            {
                ret = cmds[i].excute(cmdArgs + 1, reply);
                break;
            }            
        }
    }           
done:   
    return writex(socket, (char*)&ret, 2);
}

static int parseCmd(char* cmd)
{
    int i = 0;
    cmdArgs[i++] = cmd;
    while(*cmd){
        if(*cmd == ' '){
            *cmd++ = '\0';
            if (i < MAX_TOKEN)
                cmdArgs[i++] = cmd;
            else {
                debug_sys("argr count error!");
                return -1;
            }
        }
        ++cmd;
    }
    return i;
}

static int readx(int socket, void* buf, int count)
{
    int num = 0, rx = 0;
    char* _buf = buf;
    if (count <= 0) return -1;
    
    while (num < count){
        rx = read(socket, _buf + num, count - num);
        if (rx < 0){
            if (errno == EINTR) continue;
            debug_sys("read error");
            return -1;
        }
        else if (!rx){
            debug_sys("read the end of file");
            return -1;
        }
        num += rx;
    }
    return 1;
}

static int writex(int socket, const void* buf, int count)
{
    int num = 0, tx = 0;
    char* _buf = buf;
    if (count <= 0)
        return -1;
    while (num < count){
        tx = write(socket, buf + tx, count - tx);
        if (tx < 0){            
            if (errno == EINTR) continue;
            debug_sys("write error");
            return -1;
        }
        num += tx;            
    }
    return 1;
}

static inline void err_sys(const char* errstring)
{
	ALOGE("%s(%s)\n", errstring, strerror(errno));
	exit(EXIT_FAILURE);
}

static inline void info_sys(const char* info)
{
	ALOGI("%s\n", info);
}

static inline void debug_sys(const char* debuginfo)
{
	ALOGD("%s(%s)\n", debuginfo, strerror(errno));    
}
