#ifndef __SMI_H__
#define __SMI_H__

#include "rtk_error.h"
#include "rtk_types.h"

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData);
rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData);

#endif /* __SMI_H__ */
