#ifndef __DDP_CMDQ_H__
#define __DDP_CMDQ_H__

#define CMDQ_BUFFER_NUM         8
#define CMDQ_BUFFER_SIZE        32*1024

#define CMDQ_ION_BUFFER_SIZE    CMDQ_BUFFER_SIZE*CMDQ_BUFFER_NUM

#define MAX_CMDQ_TASK_ID        256

#define CMDQ_THREAD_NUM         7
#define CMDQ_THREAD_LIST_LENGTH 10
#define CMDQ_TIMEOUT            27


typedef struct {
    int Owner;
    unsigned long VA;
    unsigned long MVA;
    unsigned int blocksize;
    unsigned long *blockTailAddr;
} cmdq_buff_t;


typedef struct {
    int cmdBufID;
    int cmdqThread;
} task_resource_t;


enum
{
    // CAM
    tIMGI   = 0,
    tIMGO   = 1,  // full size
    tIMG2O  = 2,  // samll size

    // MDP
    tRDMA0  = 3,
    tCAMIN  = 4,
    tSCL0   = 5,
    tTDSHP  = 6,
    tMDPSHP = 6,
    tWROT   = 7,

    tLSCI   = 8,
    tCMDQ   = 9,

    tTotal  = 10,
};


enum
{
    cbMDP = 0,
    cbISP = 1,
    cbMAX = 2,
};


typedef int (*CMDQ_TIMEOUT_PTR)(int);
typedef int (*CMDQ_RESET_PTR)(int);


void cmdqBufferTbl_init(unsigned long va_base, unsigned long mva_base);
int cmdqResource_required(void);
void cmdqResource_free(int taskID);
cmdq_buff_t * cmdqBufAddr(int taskID);
bool cmdqTaskAssigned(int taskID, unsigned int priority, unsigned int engineFlag, unsigned int blocksize);
void cmdqThreadComplete(int cmdqThread, int taskID);
void dumpMDPRegInfo(void);
void cmdqTerminated(void);
bool checkMdpEngineStatus(unsigned int engineFlag);
int cmdqRegisterCallback(int index, CMDQ_TIMEOUT_PTR pTimeoutFunc , CMDQ_RESET_PTR pResetFunc);

#endif
