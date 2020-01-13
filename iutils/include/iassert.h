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





// �жϱ��ʽֵ�� ��ȷ״̬������״̬��ӡ
#define EX_INFO(expr, ...)			EXPR(expr, , true,  __VA_ARGS__)
// �жϱ��ʽֵ�� ����״̬��ӡ����ӡ������
#define EX_CHECK(expr, ...)			EXPR(expr, ilog_trace(), false, __VA_ARGS__)
// �жϱ��ʽֵ�� ����״̬��ӡ����ӡ��������*������ֹ����(Ĭ�ϲ���ֹ)
#define EX_ASSERT(expr, ...)		EXPR(expr, ilog_trace(), false,  __VA_ARGS__)
// �жϱ��ʽֵ�� ����״̬��ӡ����ӡ�����������ҷ���
#define EX_RET(expr, outs, ...)		EXPR(expr, ilog_trace();return outs, false,  __VA_ARGS__)



#endif
