/*
 * Copyright (C) 2017 Baptiste CLENET
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       OpenThread test application
 *
 * @author      Baptiste Clenet <bapclenet@gmail.com>
 */

#include <stdio.h>

uint8_t borderRouteChangeCnt = 0;
uint8_t borderRouterLC = 0;
uint8_t borderRouterRC = 0;
uint16_t myRloc = 0;
uint16_t nextHopRloc = 0;
uint8_t debugNumFreeBuffers = 0;

uint32_t addressMsgCnt = 0;
uint32_t joiningMsgCnt = 0;
uint32_t routingMsgCnt = 0;
uint32_t linkMsgCnt = 0;
uint32_t controlMsgCnt = 0;
uint32_t packetSuccessCnt = 0;
uint32_t packetFailCnt = 0;
uint32_t packetBusyChannelCnt = 0;
uint32_t broadcastCnt = 0;

int main(void)
{
    /* Run wpantund to interact with NCP */
    return 0;
}
