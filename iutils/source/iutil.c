#include "iutil.h"
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "itypes.h"
#include "ilog.h"



void iutil_version(void)
{

	printf("------------[    iutil    ]------------\n");
    printf("Compile Date : %s\n", __DATE__);
    printf("Compile Time : %s\n", __TIME__);
}

int iutil_file_cat(char * path, char * buf, uint64_t num)
{
    if (path == NULL){
        return -1;
    }

    int read_num = 0;
    FILE * fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL){
        return -1;
    }

    read_num = fread(buf, 1, num, fp);
    fclose(fp);

    return read_num;
}

int iutil_file_echo(char * path, char * buf, uint64_t num)
{
    if (path == NULL) {
        return -1;
    }

    int read_num = 0;
    FILE * fp = NULL;
    fp = fopen(path, "a+");
    if (fp == NULL) {
        return -1;
    }

    read_num = fwrite(buf, 1, num, fp);
    fclose(fp);

    return read_num;
}


/*
 * 获取微秒为单位的时间
 */
uint64_t iutil_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

/*
* 获取纳秒为单位的时间
*/
uint64_t iutil_time_ns(void)
{
    struct timespec timens = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &timens);
    return timens.tv_nsec + timens.tv_sec * 1000000000;
}

int iutil_system(char * fmt, ...)
{
    int ret = -1;
    char * cmd = (char *)malloc(MAX_LINE + 1);
    if (cmd != NULL){
        va_list args;
        va_start(args, fmt);
        vsnprintf(cmd, MAX_LINE, fmt, args);
        va_end(args);
        ret = system(cmd);
        ilog_debug("[%s] %s ", (ret==0)?" OK ":"Fail", cmd);
        free(cmd);
    }
    return ret;
}

/*
 * tarce 调用栈信息
 * 备注：
 *     打印函数名称需添加编译选项 链接添加 -rdynamic
 *     返回字符串需要主动free
 */
char * iutil_trace_format(void)
{
    int i, tr_num;
    void * tr_buf[64];
    char ** tr_strs;
    char * buf = NULL;
    int buf_len = 0;

    tr_num = backtrace(tr_buf, 64);
    tr_strs = backtrace_symbols(tr_buf, tr_num);
    if (tr_strs == NULL || tr_num <= 0) {
        return NULL;
    }

    for (i = 0; i < tr_num; i++){
        buf_len += strlen(tr_strs[i]);
    }
    buf_len += tr_num + 1;

    buf = (char *)malloc(buf_len);
    if (buf == NULL){
        free(tr_strs);
    }

    buf[0] = '\0';
    for (i = 0; i < tr_num; i++){
        strcat(buf, tr_strs[i]);
        strcat(buf, "\n");
    }

    free(tr_strs);
    return buf;
}


/*
 *	格式化输出数组
 *	备注：
 *	    返回字符串需要主动free
 */
char * iutil_array_format(const uint8_t * ds, int num, int count)
{
    int i, j;
    char * buf = NULL;
    int buf_len = 0;
    int buf_used = 0;

    buf_len = (count * 4 + 17) * (num / count + 1);
    buf_len += 80;  //第一行地址信息显示
    buf = malloc(buf_len + 1);
    if (buf == NULL) {
        return NULL;
    }

    buf_used += snprintf(buf + buf_used, buf_len - buf_used, "[ data=0x%016X  num=%d  line=%d ]\n", (size_t)ds, num, count);

    for (i = 0; i < ceil((double)num / count); i++) {
        buf_used += snprintf(buf + buf_used, buf_len - buf_used, "0x%08X", i*count);
        for (j = 0; j < count; j++) {
            if ( i*count+j < num){
                buf_used += snprintf(buf + buf_used, buf_len - buf_used, " %02X", ds[i*count + j]);
            } else {
                buf_used += snprintf(buf + buf_used, buf_len - buf_used, "   ");
            }
            if (j==(count-1)/2){
                buf_used += snprintf(buf + buf_used, buf_len - buf_used, " ");
            }
        }

        buf_used += snprintf(buf + buf_used, buf_len - buf_used, "  ");

        buf_used += snprintf(buf + buf_used, buf_len - buf_used, "|");
        for (j = 0; j < count; j++) {
            
            if (i*count + j < num) {
                if (isprint(ds[i*count + j])) {
                    buf_used += snprintf(buf + buf_used, buf_len - buf_used, "%C", ds[i*count + j]);
                } else {
                    buf_used += snprintf(buf + buf_used, buf_len - buf_used, ".");
                }
            } else {
                buf_used += snprintf(buf + buf_used, buf_len - buf_used, " ");
            }
        }
        buf_used += snprintf(buf + buf_used, buf_len - buf_used, "|");

        buf_used += snprintf(buf + buf_used, buf_len - buf_used, "\n");

    }

    return buf;
}


char * iutil_mac_format(uint8_t mac[6], char * str)
{
    if (str == NULL){
        return NULL;
    }

    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)mac[0],
            (unsigned char)mac[1],
            (unsigned char)mac[2],
            (unsigned char)mac[3],
            (unsigned char)mac[4],
            (unsigned char)mac[5]);
    return str;
}


bool iutil_mac_parse(char * str, uint8_t mac[6])
{
    if (str == NULL) {
        return false;
    }
    int mac_d[6];
    if (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", mac_d, mac_d + 1, mac_d + 2, mac_d + 3, mac_d + 4, mac_d + 5) == 6) {
        mac[0] = mac_d[0]&0x00ff;
        mac[1] = mac_d[1]&0x00ff;
        mac[2] = mac_d[2]&0x00ff;
        mac[3] = mac_d[3]&0x00ff;
        mac[4] = mac_d[4]&0x00ff;
        mac[5] = mac_d[5]&0x00ff;

        return true;
    } else {
        return false;
    }

}


char * iutil_trim(char * str)
{
    char *start, *end;
    int len = strlen(str);
    int tlen = 0;
    //去掉两端的空格  
    start = str;			//指向首字符  
    end = str + len - 1;	//指向最后一个字符  
    while (*start && isspace(*start)){
        start++;			//如果是空格，首地址往前移一位，如果不是，则跳过该循环 
    }
    while (*end && isspace(*end)){
        *end-- = 0;			//如果是空格，末地址往前移一位，并赋结束符 
    }
    tlen = strlen(start);
    memmove(str, start, tlen);		//把首地址还给str 
    str[tlen] = '\0';
    return str;
}

/*
 * 不区分大小写，比较字符串
 */
int iutil_strcmp_fuzzy(const char * s1, const char * s2)
{
    if (s1 == NULL || s2 == NULL) {
        return -1;
    }

    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    size_t len = min(s1_len, s2_len);
    if (len == 0){
        return -1;
    }
    char ac = 'a' - 'A';
    char a1, a2;
    size_t i = 0;
    for (i = 0; i < len; i++) {
        a1 = (s1[i] >= 'a' && s1[i] <= 'z') ? (s1[i] - ac) : s1[i];
        a2 = (s2[i] >= 'a' && s2[i] <= 'z') ? (s2[i] - ac) : s2[i];
        if (a1 != a2) {
            return a1 - a2;
        }
    }

    return 0;
}

char * iutil_kv_iterater(const char * str, char * key, char * value)
{
    if (str == NULL || key == NULL || value == NULL){
        return NULL;
    }

    char csplit = '=';
    char * kv_split = NULL;
    char * kv_end = NULL;
    char * kv_end_t1 = NULL;
    char * kv_end_t2 = NULL;
    bool is_end = false;

    kv_split = strchr(str, csplit);
    if (kv_split != NULL){
        kv_end_t1 = strchr(kv_split, ',');
        kv_end_t2 = strchr(kv_split, ';');

        if(kv_end_t1 == NULL && kv_end_t2==NULL) {
            kv_end = kv_split + strlen(kv_split);
            is_end = true;
        } else if(kv_end_t1 == NULL && kv_end_t2 != NULL){
            kv_end = kv_end_t2;
        } else if(kv_end_t1 != NULL && kv_end_t2 == NULL) {
            kv_end = kv_end_t1;
        } else {
            kv_end = (kv_end_t1 < kv_end_t2) ? kv_end_t1 : kv_end_t2;
        }


        strncpy(key, str, kv_split - str);
        key[kv_split - str] = '\0';
        iutil_trim(key);

        strncpy(value, kv_split+1, kv_end - kv_split-1);
        value[kv_end - kv_split - 1] = '\0';
        iutil_trim(value);
        if (is_end){
            return kv_end;
        }
        return kv_end + 1;
    }

    return NULL;
}


bool iutil_kv_lookup(char * str, char * key, char * value)
{
    if (str == NULL || key == NULL){
        return false;
    }

    char key_tmp[MAX_NAME];
    char value_tmp[MAX_NAME];
    while ((str = iutil_kv_iterater(str, key_tmp, value_tmp)) != NULL){
        if (strncasecmp(key_tmp, key, strlen(key)) == 0){
            if (value){
                strcpy(value, value_tmp);
            }
            return true;
        }
    }

    return false;
}



uint16_t iutil_checksum16(uint16_t *addr, int len)
{
    int nleft = len;
    int sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= sizeof (uint16_t);
    }

    if (nleft == 1) {
        *(uint8_t *)(&answer) = *(uint8_t *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}


void * iutil_packet_l3(char * buf, uint32_t len, uint16_t l3_type)
{
    uint16_t * eth_type = (uint16_t *)(buf + 12);
    do {
        if (*eth_type == htons(l3_type)){
            return ((char *)eth_type) + 2;
        }else if (*eth_type == htons(0x8100)){
            eth_type = (uint16_t *)((char *)eth_type + 4);
        }else{
            return NULL;
        }
    } while ((char *)eth_type - buf <= (len - 2));
    return NULL;
}






char * iutil_num_format_base(uint64_t num, char * str, uint32_t base)
{
    char * dvs[] = { " ", "K", "M", "G", "T", "P", "E" };
    char ustr[256] = "";
    char mstr[256] = "";
    int i = 0;
    do {
        uint64_t u = num % base;
        if (u > 0) {
            sprintf(ustr, "%lu%s", u, dvs[i]);
            strcat(ustr, mstr);
            strcpy(mstr, ustr);
        }
        num = (num - u) / base;
        i++;
        if (i > ARRAY_SIZE(dvs)) {
            break;
        }
    } while (num > 0);
    strcpy(str, mstr);
    return str;
}

char * iutil_num_format(uint64_t num, char * str)
{
    return iutil_num_format_base(num, str, 1024);
}

/**************************************************************************
* 函数名称: iutil_bit_maskstring
* 功能描述: 将第pos位置bit设置为1，并输出字符串
*		   10  -- > 0x0200
* 输入参数: uint16_t pos
* 输入参数: char * mask_str
* ------------------------------------------------------------------------
* 修改日期			版本号     修改人          修改内容
* 2017年4月11日		v1.0      何兴诗          创建
**************************************************************************/
void iutil_bit_maskstring(uint16_t pos, char * mask_str)
{
    if (mask_str == NULL) {
        return;
    }
    int i;
    sprintf(mask_str, "0x%02x", 0x01 << ((pos) % 8));
    int ni = (pos) / 8;
    for (i = 0; i < ni; i++) {
        strcat(mask_str, "00");
    }
}


/* -------------------------[      统计        ]------------------------- */

// 更新统计,使用内部时间
void iutil_stat_update(istat * s, uint64_t value)
{
    EX_ASSERT(s);
    s->value_last = s->value;
    s->time_last = s->time;

    s->time = iutil_time_ns();
    s->value = value;
}

void iutil_stat_add(istat * s, uint64_t value)
{
    EX_ASSERT(s);
    iutil_stat_update(s, s->value + value);
}

// 更新统计,使用指定时间
void iutil_stat_update_in_time(istat * s, uint64_t value, uint64_t time)
{
    EX_ASSERT(s);
    s->time = time;
    s->value = value;
}

// 获取两次统计的数据增量
uint64_t iutil_stat_incr(istat * s)
{
    EX_ASSERT(s);
    return s->value - s->value_last;
}

// 获取两次统计的间隔时间
uint64_t iutil_stat_incr_time(istat * s)
{
    EX_ASSERT(s);
    return s->time - s->time_last;
}

// 获取速度
uint64_t iutil_stat_speed(istat * s)
{
    EX_ASSERT(s);
    if((s->time - s->time_last)==0) {
        return 0;
    }
    return (s->value - s->value_last) * 1000000000L / (s->time - s->time_last);
}

// 序列化 10000(+60|4000000us)[15/s]
uint64_t iutil_stat_format(istat * s, char * str)
{
    EX_ASSERT(s);
    EX_ASSERT(str);

    return sprintf(str, "%lu(+%lu|%lus)[%lu/s]", s->value, iutil_stat_incr(s), iutil_stat_incr_time(s)/1000000000, iutil_stat_speed(s));
}

void iutil_stat_show(istat * s)
{
    EX_ASSERT(s);
    char str[256];
    iutil_stat_format(s, str);
    printf("%s", str);
}


void iutil_stat_ut(void)
{
    int i = 0;
    istat stat;
    memset(&stat, 0x00, sizeof(istat));

    for (i = 0; i < 100; i++) {
        iutil_stat_add(&stat, 10000 * i);
        iutil_stat_show(&stat);
        printf("\n");
        usleep(10*1000);
    }
}


uint8_t iutil_bcd2bin(uint8_t val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}

//二进制转为BCD码 
uint8_t iutil_bin2bcd(uint8_t val)
{
    return ((val / 10) << 4) + val % 10;
}


void iutil_bit_show(IBIT_DEF * ibit, int len, uint64_t data)
{
    if(ibit == NULL) {
        return;
    }
    printf("data %d(0x%lx)\n", data, data);
    int i = 0;
    for(i = 0; i < len; i++) {
        if((data&(ibit[i].mask)) == (ibit[i].value)) {
            printf("%s\n", ibit[i].str);
        }
    }
}
