#ifndef __PORT_KERNEL_H__
#define __PORT_KERNEL_H__

typedef struct _ccci_msg {
	union{
		u32 magic;	// For mail box magic number
		u32 addr;	// For stream start addr
		u32 data0;	// For ccci common data[0]
	};
	union{
		u32 id;	// For mail box message id
		u32 len;	// For stream len
		u32 data1;	// For ccci common data[1]
	};
	u32 channel;
	u32 reserved;
} ccci_msg_t; // __attribute__ ((packed));

typedef struct dump_debug_info {
	unsigned int type;
	char *name;
	unsigned int more_info;
	union {
		struct {
			char file_name[30];
			int line_num;
			unsigned int parameters[3];
		} assert;	
		struct {
			int err_code1;
    		int err_code2;
		}fatal_error;
		ccci_msg_t data;
		struct {
			unsigned char execution_unit[9]; // 8+1
			char file_name[30];
			int line_num;
			unsigned int parameters[3];
		}dsp_assert;
		struct {
			unsigned char execution_unit[9];
			unsigned int  code1;
		}dsp_exception;
		struct {
			unsigned char execution_unit[9];
			unsigned int  err_code[2];
		}dsp_fatal_err;
	};
	void *ext_mem;
	size_t ext_size;
	void *md_image;
	size_t md_size;
	void *platform_data;
	void (*platform_call)(void *data);
}DEBUG_INFO_T;

#if defined (CONFIG_MTK_AEE_FEATURE)
extern void aed_md_exception(const int *log, int log_size, const int *phy, int phy_size, const char* detail);
extern __weak void aee_kernel_warning_api(const char *file, const int line, const int db_opt, const char *module, const char *msg, ...);
#define DB_OPT_FTRACE		(1<<0)
#endif

#define CCCI_AED_DUMP_EX_MEM		(1<<0)
#define CCCI_AED_DUMP_MD_IMG_MEM	(1<<1)
#define CCCI_AED_DUMP_CCIF_REG		(1<<2)

#define MD_IMG_DUMP_SIZE				(1<<8)
#define DSP_IMG_DUMP_SIZE				(1<<9)

#define EE_BUF_LEN		(256)
#define AED_STR_LEN		(512)

enum { 
	MD_EX_TYPE_INVALID = 0, 
	MD_EX_TYPE_UNDEF = 1, 
	MD_EX_TYPE_SWI = 2,
	MD_EX_TYPE_PREF_ABT = 3, 
	MD_EX_TYPE_DATA_ABT = 4, 
	MD_EX_TYPE_ASSERT = 5,
	MD_EX_TYPE_FATALERR_TASK = 6, 
	MD_EX_TYPE_FATALERR_BUF = 7,
	MD_EX_TYPE_LOCKUP = 8, 
	MD_EX_TYPE_ASSERT_DUMP = 9,
	MD_EX_TYPE_ASSERT_FAIL = 10,
	DSP_EX_TYPE_ASSERT = 11,
	DSP_EX_TYPE_EXCEPTION = 12,
	DSP_EX_FATAL_ERROR = 13,
	NUM_EXCEPTION,
	
	MD_EX_TYPE_EMI_CHECK = 99,
};

enum {
	MD_EE_FLOW_START = 0,
	MD_EE_DUMP_ON_GOING,
	MD_STATE_UPDATE,
	MD_EE_MSG_GET,
	MD_EE_TIME_OUT_SET,
	MD_EE_OK_MSG_GET,
	MD_EE_FOUND_BY_ISR,
	MD_EE_FOUND_BY_TX,
	MD_EE_PENDING_TOO_LONG,
	MD_EE_SWINT_GET,
	
	MD_EE_INFO_OFFSET = 20,
	MD_EE_EXCP_OCCUR = 20,
	MD_EE_AP_MASK_I_BIT_TOO_LONG = 21,
};

enum {
	MD_EE_CASE_NORMAL = 0,
	MD_EE_CASE_ONLY_EX,
	MD_EE_CASE_ONLY_EX_OK,
	MD_EE_CASE_TX_TRG,
	MD_EE_CASE_ISR_TRG,
	MD_EE_CASE_NO_RESPONSE,
	MD_EE_CASE_AP_MASK_I_BIT_TOO_LONG,
	MD_EE_CASE_ONLY_SWINT,
	MD_EE_CASE_SWINT_MISSING,
};

typedef enum
{
    IPC_RPC_CPSVC_SECURE_ALGO_OP = 0x2001,
	IPC_RPC_GET_SECRO_OP		= 0x2002,
	IPC_RPC_GET_TDD_EINT_NUM_OP = 0x4001,
	IPC_RPC_GET_TDD_GPIO_NUM_OP = 0x4002,
	IPC_RPC_GET_TDD_ADC_NUM_OP  = 0x4003,
	IPC_RPC_GET_EMI_CLK_TYPE_OP = 0x4004,
	IPC_RPC_GET_EINT_ATTR_OP	= 0x4005,
	IPC_RPC_GET_RF_CLK_BUF	    = 0x4006,

	// not using
	IPC_RPC_GET_GPIO_VAL_OP	    = 0x8006,
	IPC_RPC_GET_ADC_VAL_OP	    = 0x8007,

    IPC_RPC_IT_OP               = 0x4321,  
}RPC_OP_ID;

struct rpc_pkt {
	unsigned int len;
	void *buf;
};

struct rpc_buffer {
	struct ccci_header header;
	u32 op_id;
	u32 para_num;
	u8  buffer[0];
};

#define CLKBUF_MAX_COUNT 4 // hardcode, becarefull with data size, should not exceed tmp_data in ccci_rpc_work_helper()
struct ccci_clkbuf_result {
	u16 CLKBuf_Count;
	u8 CLKBuf_Status[CLKBUF_MAX_COUNT];
}; // the total size should sync with tmp_data[] using in ccci_rpc_work_helper()

#define RPC_REQ_BUFFER_NUM       2 /* support 2 concurrently request*/
#define RPC_MAX_ARG_NUM          6 /* parameter number */
#define RPC_MAX_BUF_SIZE         2048 
#define RPC_API_RESP_ID          0xFFFF0000

#define FS_NO_ERROR										 0
#define FS_NO_OP										-1
#define	FS_PARAM_ERROR									-2
#define FS_NO_FEATURE									-3
#define FS_NO_MATCH									    -4
#define FS_FUNC_FAIL								    -5
#define FS_ERROR_RESERVED								-6
#define FS_MEM_OVERFLOW									-7

#define CCCI_SED_LEN_BYTES   16 
typedef struct {unsigned char sed[CCCI_SED_LEN_BYTES]; }sed_t;
#define SED_INITIALIZER { {[0 ... CCCI_SED_LEN_BYTES-1]=0}}

typedef int (*ccci_sys_cb_func_t)(int, int);
typedef struct{
	unsigned int		id;
	ccci_sys_cb_func_t	func;
}ccci_sys_cb_func_info_t;
#define MAX_KERN_API 10

#endif //__PORT_KERNEL_H__
