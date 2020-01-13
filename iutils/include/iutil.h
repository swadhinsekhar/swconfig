#ifndef IUTILS_H
#define IUTILS_H

#ifdef __cplusplus
extern "C" {
#endif


#include "itypes.h"
#include "ilog.h"
#include "iassert.h"
#include "ielf.h"
#include "icall.h"
#include "rpc_icall.h"
#include "ihash.h"
#include "inet.h"



uint64_t iutil_time_us(void);
uint64_t iutil_time_ns(void);
int iutil_system(char * fmt, ...);
char * iutil_trim(char * str);
int iutil_strcmp_fuzzy(const char * s1, const char * s2);
char * iutil_trace_format(void);
char * iutil_array_format(const uint8_t * ds, int num, int count);
char * iutil_mac_format(uint8_t mac[6], char * str);
bool iutil_mac_parse(char * str, uint8_t mac[6]);
char * iutil_kv_iterater(const char * str, char * key, char * value);
bool iutil_kv_lookup(char * str, char * key, char * value);
uint16_t iutil_checksum16(uint16_t *addr, int len);
void * iutil_packet_l3(char * buf, uint32_t len, uint16_t l3_type);
char * iutil_num_format(uint64_t num, char * str);
char * iutil_num_format_base(uint64_t num, char * str, uint32_t base);
bool iutil_address_check(void * addr);

#define PX_MAC		"%02x:%02x:%02x:%02x:%02x:%02x"
#define PS_MAC(mac) (unsigned char)((unsigned char *)mac)[0],(unsigned char)((unsigned char *)mac)[1],\
                    (unsigned char)((unsigned char *)mac)[2],(unsigned char)((unsigned char *)mac)[3],\
                    (unsigned char)((unsigned char *)mac)[4], (unsigned char)((unsigned char *)mac)[5]
#define PX_IP		"%d.%d.%d.%d"
#define PS_IP(ip)	(unsigned char)((unsigned char *)&(ip))[0],(unsigned char)((unsigned char *)&(ip))[1],\
                    (unsigned char)((unsigned char *)&(ip))[2],(unsigned char)((unsigned char *)&(ip))[3]



typedef struct
{
    uint64_t value;
    uint64_t mask;
    char * str;
}IBIT_DEF;

void iutil_bit_show(IBIT_DEF * ibit, int len, uint64_t data);


/* 统计结构体 */
typedef struct{
    uint64_t value_last;		// 上次统计的数据
    uint64_t time_last;			// 上次统计的时间  ns
    uint64_t value;				// 当前统计的数据
    uint64_t time;				// 当前统计的时间  ns
}istat;

void iutil_stat_update(istat * s, uint64_t value);		// 更新统计,使用内部时间
void iutil_stat_add(istat * s, uint64_t value);
void iutil_stat_update_in_time(istat * s, uint64_t value, uint64_t time);	// 更新统计,使用指定时间
uint64_t iutil_stat_incr(istat * s);				// 获取两次统计的数据增量
uint64_t iutil_stat_incr_time(istat * s);			// 获取两次统计的间隔时间
uint64_t iutil_stat_speed(istat * s);				// 获取速度
uint64_t iutil_stat_format(istat * s, char * str);	// 序列化 10000(+52)[15/s]
void iutil_stat_show(istat * s);

uint8_t iutil_bcd2bin(uint8_t val);
uint8_t iutil_bin2bcd(uint8_t val);

#ifdef __cplusplus
}
#endif


#endif