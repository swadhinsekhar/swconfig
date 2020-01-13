/*
 * Copyright c                  Realtek Semiconductor Corporation, 2006
 * All rights reserved.
 *
 * Program : Control smi connected RTL8366
 * Abstract :
 * Author : Yu-Mei Pan (ympan@realtek.com.cn)
 *  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtk_error.h"
#include <rtk_types.h>
#include <smi.h>

#define REG_PROC_PATH 		"/sys/devices/platform/ff540000.ethernet/mdio_bus/stmmac-0/stmmac-0:00/rtl_phy_reg"
#define REGVAL_PROC_PATH 	"/sys/devices/platform/ff540000.ethernet/mdio_bus/stmmac-0/stmmac-0:00/rtl_phy_regValue"

static void rtlglue_drvMutexLock(void)
{
	/* It is empty currently. Implement this function if Lock/Unlock function is
	 * needed */
	return;
}

static void rtlglue_drvMutexUnlock(void)
{
	/* It is empty currently. Implement this function if Lock/Unlock function is
	 * needed */
	return;
}

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
	int pData;

	if (mAddrs > 0xFFFF)
		return RT_ERR_INPUT;

	if (rData == NULL)
		return RT_ERR_NULL_POINTER;

	/* Lock */
	rtlglue_drvMutexLock();

	FILE *fd = fopen(REG_PROC_PATH, "w");
	if (!fd)
		return RT_ERR_SMI;
	fprintf(fd, "%d", mAddrs);
	fclose(fd);

	fd = fopen(REGVAL_PROC_PATH, "r");
	if (!fd)
		return RT_ERR_SMI;
	if (1 != fscanf(fd, "%d\n", &pData)) {
		fclose(fd);
		return RT_ERR_SMI;
	}
	*rData = pData;
	fclose(fd);

	/* Unlock */
	rtlglue_drvMutexUnlock();

	return RT_ERR_OK;
}

rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
	if (mAddrs > 0xFFFF)
		return RT_ERR_INPUT;

	if (rData > 0xFFFF)
		return RT_ERR_INPUT;

	/* Lock */
	rtlglue_drvMutexLock();

	FILE *fd = fopen(REG_PROC_PATH, "w");
	if (!fd)
		return RT_ERR_SMI;
	fprintf(fd, "%d", mAddrs);
	fclose(fd);

	fd = fopen(REGVAL_PROC_PATH, "w");
	if (!fd)
		return RT_ERR_SMI;
	fprintf(fd, "%d", rData);
	fclose(fd);

	/* Unlock */
	rtlglue_drvMutexUnlock();

	return RT_ERR_OK;
}
