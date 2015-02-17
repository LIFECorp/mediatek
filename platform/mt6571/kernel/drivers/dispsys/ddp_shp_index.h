#include "ddp_drv.h"

DISPLAY_SHP_T shpindex =
{
    entry:
    {
        // 0
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x00,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x00,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 1
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x05,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x05,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 2
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x0A,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x0A,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 3
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x0F,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x0F,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 4
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x14,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x14,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 5
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x18,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x18,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 6
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x1C,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x1C,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 7
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x20,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x20,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 8
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x24,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x24,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 9
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x28,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x28,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 10
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x00,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x00,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
        ,

        // 11
        {
            0x00,   // MDP_SHP_CFG              INK_SEL         14030010[06:06]
            0x00,   // MDP_SHP_CFG              DEMO_SWAP       14030010[05:05]
            0x00,   // MDP_SHP_CFG              DEMO_ENABLE     14030010[04:04]
            0x00,   // MDP_SHP_CFG              RELAY_MODE      14030010[00:00]

            0xDC,   // MDP_SHP_CON_00           PROT            14030014[31:24]
            0x3F,   // MDP_SHP_CON_00           LIMIT           14030014[21:16]
            0x00,   // MDP_SHP_CON_00           HPF_GAIN        14030014[13:08]
            0x00,   // MDP_SHP_CON_00           BPF_GAIN        14030014[05:00]

            0x32,   // MDP_SHP_CON_01           DECAY_RATIO     14030018[21:16]
            0x0A,   // MDP_SHP_CON_01           SOFT_RATIO      14030018[11:08]
            0x64,   // MDP_SHP_CON_01           BOUND           14030018[06:00]
        }
    }
};
