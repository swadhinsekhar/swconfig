#include "rpc_icall.h"
#include "ilog.h"
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "icall.h"

#define RPCD_FIFO_IN_PATH		"/tmp/iutil_in_rpcd"
#define RPCD_FIFO_OUT_PATH      "/tmp/iutil_out_rpcd"
#define RPCD_CMD_MAX_LEN		512
static bool isRpcdThreadRun = false;

static void * rpcd_thread_run(void * data);
static bool fifo_create(char * path);

int rpcd_start(void)
{
    pthread_t thread;
    int ret;

    if (isRpcdThreadRun==false){
        ret = pthread_create(&thread, NULL, rpcd_thread_run, NULL);
        if (ret != 0){
            ilog_error("rpcd_thread_run thread create fail");
            return -EPERM;
        }
        isRpcdThreadRun = true;
        ilog_info("pthread_create success");
    }
    return 0;
}

int rpcd_stop(void)
{
    isRpcdThreadRun = false;
    return 0;
}

void * rpcd_thread_run(void * data)
{
    bool bret = false;
    long call_ret = 0;
    int rfp_in = 0;
    int rfp_out = 0;
    int readNum = 0;
    char rBuf[RPCD_CMD_MAX_LEN+1];
    char retStr[RPCD_CMD_MAX_LEN+1];

    ilog_info("rpcd_thread_run started");

    bret = fifo_create(RPCD_FIFO_IN_PATH);
    if (!bret){
        goto run_exit;
    }

    bret = fifo_create(RPCD_FIFO_OUT_PATH);
    if (!bret){
        goto run_exit;
    }

    while (isRpcdThreadRun){
        rfp_in = open(RPCD_FIFO_IN_PATH, O_RDONLY);
        if (rfp_in<0){
            continue;
        }
        
        if ((readNum = read(rfp_in, rBuf, RPCD_CMD_MAX_LEN)) > 0){
            rBuf[readNum] = '\0';
            ilog_mask("InputCmd:%s", rBuf);
            bret = icall_parse_run(rBuf, &call_ret);
            sprintf(retStr, "Exec:%s  ReturnValue:0x%lx\n", (bret)?"True":"False", call_ret);
            ilog_mask("%s", retStr);

            rfp_out = open(RPCD_FIFO_OUT_PATH, O_WRONLY | O_NONBLOCK);
            if (rfp_out >=0 ){
                write(rfp_out, retStr, strlen(retStr));
                close(rfp_out);
                rfp_out = 0;
            }

            close(rfp_in);
            rfp_in = 0;
        }
    }

run_exit:
    close(rfp_in);
    close(rfp_out);
    ilog_info("rpcd_thread_run exit");

    return NULL;
}


bool fifo_create(char * path)
{
    int res = -1;
    
    if (!path){
        ilog_error("input param path is NULL");
        return false;
    }

    if (access(path, F_OK) == -1){
        /* 管道文件不存在, 创建命名管道 */
        res = mkfifo(path, 0777);
        if (res != 0){
            ilog_error("Could not create fifo %s\n", path);
            return false;
        }
    }

    return true;
}

