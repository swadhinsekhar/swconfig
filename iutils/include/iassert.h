#ifndef HASSERT_H
#define HASSERT_H

#include "ilog.h"
#include <assert.h>


#define EXPR(expr, outs, show, ... )							  \
    {    														  \
        if ((expr)) {											  \
            if (show){											  \
                ilog_debug("[ OK ] "__VA_ARGS__);		          \
            }													  \
        } else { 												  \
            ilog_error("[Fail] ["#expr"] "__VA_ARGS__);  		  \
            outs; 												  \
        } 														  \
    }															  \





// 判断表达式值， 正确状态、错误状态打印
#define EX_INFO(expr, ...)			EXPR(expr, , true,  __VA_ARGS__)
// 判断表达式值， 错误状态打印，打印调用链
#define EX_CHECK(expr, ...)			EXPR(expr, ilog_trace(), false, __VA_ARGS__)
// 判断表达式值， 错误状态打印，打印调用链，*并且终止程序(默认不终止)
#define EX_ASSERT(expr, ...)		EXPR(expr, ilog_trace(), false,  __VA_ARGS__)
// 判断表达式值， 错误状态打印，打印调用链，并且返回
#define EX_RET(expr, outs, ...)		EXPR(expr, ilog_trace();return outs, false,  __VA_ARGS__)



#endif
