#ifndef _MTK_DEVICE_APC_SW_H
#define _MTK_DEVICE_APC_SW_H

/*
 * Define constants.
 */
#define DEVAPC_TAG              "DEVAPC"
#define DEVAPC_MAX_TIMEOUT      100

#define DEVAPC_ABORT_EMI        0x1


/*
 * Define enums.
 */
typedef struct {
    int             device_num;
    bool            forbidden;
    DEVAPC_ATTR     d0_attr;
    DEVAPC_ATTR     d1_attr;
    DEVAPC_ATTR     d2_attr;
} DEVICE_INFO;
 

/*
 * Define all devices' attribute.
 */  
#if 1
static DEVICE_INFO DEVAPC_Devices[] = {

    {0,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {1,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {2,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {3,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {4,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {5,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {6,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {7,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {8,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {9,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {10,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {11,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {12,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {13,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {14,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {15,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {16,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {17,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {18,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {19,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {20,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {21,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {22,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {23,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {24,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {25,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {26,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {27,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {28,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {29,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {30,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {31,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {32,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {33,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {34,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {35,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {36,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {37,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {38,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {39,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {40,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {41,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {42,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {43,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {44,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {45,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {46,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {47,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {48,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {49,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {50,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {51,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {52,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {53,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {54,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {55,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {56,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {57,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {58,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {59,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {60,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {61,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {62,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {63,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {-1,  FALSE, E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
};
#endif


/* Backup Design */
#if 0

/*
 * Define all devices' attribute.
 */  
static DEVICE_INFO DEVAPC_Devices[] = {
        {0,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {1,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {2,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {3,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {4,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {5,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {6,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {7,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {8,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {9,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {10,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {11,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {12,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {13,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {14,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {15,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {16,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {17,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {18,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {19,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {20,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {21,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {22,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {23,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {24,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {25,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {26,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {27,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {28,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {29,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {30,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {31,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {32,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {33,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {34,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {35,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {36,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {37,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {38,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {39,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {40,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {41,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {42,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {43,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {44,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {45,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {46,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {47,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {48,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {49,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {50,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {51,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {52,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {53,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {54,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {55,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {56,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {57,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {58,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {59,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    
        {60,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {61,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {62,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {63,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
        {-1,  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

};
#endif 

/* Original Design */
#if 0
/*
 * Define enums.
 */
typedef struct {
    const char      *device_name;
    bool            forbidden;
} DEVICE_INFO;


/*
 * Define all devices' attribute.
 */  
static DEVICE_INFO DEVAPC_Devices[] = {
    {"0",   FALSE},
    {"1",   FALSE},
    {"2",   FALSE},
    {"3",   FALSE},
    {"4",   FALSE},
    {"5",   FALSE},
    {"6",   FALSE},
    {"7",   FALSE},
    {"8",   FALSE},
    {"9",   FALSE},

    {"10",  FALSE},
    {"11",  FALSE},
    {"12",  FALSE},
    {"13",  FALSE},
    {"14",  FALSE},
    {"15",  FALSE},
    {"16",  FALSE},
    {"17",  FALSE},
    {"18",  FALSE},
    {"19",  FALSE},

    {"20",  FALSE},
    {"21",  FALSE},
    {"22",  FALSE},
    {"23",  FALSE},
    {"24",  FALSE},
    {"25",  FALSE},
    {"26",  FALSE},
    {"27",  FALSE},
    {"28",  FALSE},
    {"29",  FALSE},

    {"30",  FALSE},
    {"31",  FALSE},
    {"32",  FALSE},
    {"33",  FALSE},
    {"34",  FALSE},
    {"35",  FALSE},
    {"36",  FALSE},
    {"37",  FALSE},
    {"38",  FALSE},
    {"39",  FALSE},

    {"40",  FALSE},
    {"41",  FALSE},
    {"42",  FALSE},
    {"43",  FALSE},
    {"44",  FALSE},
    {"45",  FALSE},
    {"46",  FALSE},
    {"47",  FALSE},
    {"48",  FALSE},
    {"49",  FALSE},

    {"50",  FALSE},
    {"51",  FALSE},
    {"52",  FALSE},
    {"53",  FALSE},
    {"54",  FALSE},
    {"55",  FALSE},
    {"56",  FALSE},
    {"57",  FALSE},
    {"58",  FALSE},
    {"59",  FALSE},

    {"60",  FALSE},
    {"61",  FALSE},
    {"62",  FALSE},
    {"63",  FALSE},

    {NULL,  FALSE},
};
#endif 

#endif
