#ifndef __BUSIN_LOG_H__
#define __BUSIN_LOG_H__

#include "log_base.h"
#include "utils/SP_Assert.h"
#include <sstream>
using std::ostringstream;
// ��־����
#ifdef WIN32
LOG_DECL_SINGLETON_EX(Log_IO_FILE, Log_Win32_Process_Mutex, Log_Cfg_Heap_Reg, busin_log);
#else
LOG_DECL_SINGLETON_EX(Log_IO_FILE, Log_Unix_Process_Mutex, Log_Cfg_Heap_Reg, busin_log);
#endif  /* WIN32 */

/**
* @brief	����ʹ�õĺ�
*	��ʹ�õ�ʵ��ģʽ��ʱ���⼸������Ը�����ʹ����־����
*/
#define businlog_inst						busin_log::instance()
#define businlog_crit						if ( businlog_inst ) businlog_inst->log_crit
#define businlog_error						if ( businlog_inst ) businlog_inst->log_error
#define businlog_warn						if ( businlog_inst ) businlog_inst->log_warn
#define businlog_info						if ( businlog_inst ) businlog_inst->log_info
#define businlog_verbose					if ( businlog_inst ) businlog_inst->log_verbose
#define businlog_dbg						if ( businlog_inst ) businlog_inst->log_debug
#define businlog_perf						if ( businlog_inst ) businlog_inst->log_perf
#define businlog_trace						if ( businlog_inst ) businlog_inst->log_trace
#define businlog_wmsg						if ( businlog_inst ) businlog_inst->write_msg
#define businlog_flush						if ( businlog_inst ) businlog_inst->flush
#define businlog_spec						if ( businlog_inst ) businlog_inst->log_spec

// �ܹ��Զ�ͳ�����ܲ���ӡ��
#define businlog_perf_n(name)				Log_Perf_Helper<Log_Timer, busin_log>		__ph__(#name);
#define businlog_perf_s(name,fmt)			Log_Perf_Helper<Log_Timer, busin_log>		__ph__(#name); __ph__.log fmt;
#define businlog_perf_nsec(name,nsec)		Log_Perf_Helper<Log_Timer, busin_log>		__ph__(#name, nsec);
#define businlog_perf_stop()														__ph__.stop()

// ���ٺ������õķ���
#define businlog_tracer(sub)				Log_Func_Tracer<busin_log>					__lt__(#sub);
#define businlog_tracer_s(sub,fmt,...)		Log_Func_Tracer<busin_log>					__lt__(#sub,fmt,##__VA_ARGS__);
#define businlog_tracer_perf(sub)			businlog_perf_n(sub); businlog_tracer(sub);

// ��ʼ�������ʼ��
#define businlog_open						busin_log::open
#define businlog_close						busin_log::close
#define businlog_open_ac					Log_Auto_Close<busin_log> __lac__; busin_log::open

// ������
typedef busin_log_Cfg						businlog_cfg;

// ============================================================================
// ����ĺ����
// ============================================================================

// ASSERTȻ���ӡ��־
#define businlog_error_assert(exp, msg)	if (!(exp)) { businlog_error msg; SP_ASSERTS(exp, msg); }
#define businlog_error_assert_return(exp, msg, ret)	\
	if (!(exp)) { businlog_error msg; SP_ASSERTS(exp, msg); return ret; }
#define businlog_error_assert_return_no(exp, msg)	\
	if (!(exp)) { businlog_error msg; SP_ASSERTS(exp, msg); return ; }
#define businlog_warn_assert(exp, msg)		if (!(exp)) { businlog_warn msg;  SP_ASSERTS(exp, msg); }
#define businlog_warn_assert_return(exp, msg, ret)	\
	if (!(exp)) { businlog_warn msg; SP_ASSERTS(exp, msg); return ret; }
#define businlog_warn_assert_return_no(exp, msg)	\
	if (!(exp)) { businlog_warn msg; SP_ASSERTS(exp, msg); return ; }

#define businlog_error_noassert(exp, msg)	if (!(exp)) { businlog_error msg; }
#define businlog_error_return(exp, msg, ret)	\
	if (!(exp)) { businlog_error msg; return ret; }
#define businlog_error_return_no(exp, msg)		\
	if (!(exp)) { businlog_error msg; return ; }
#define businlog_warn_noassert(exp, msg)	if (!(exp)) { businlog_warn msg; }
#define businlog_warn_return(exp, msg, ret)	\
	if (!(exp)) { businlog_warn msg; return ret; }
#define businlog_warn_return_no(exp, msg)	\
	if (!(exp)) { businlog_warn msg; return ; }
//���е�msg�ĸ�ʽΪxx<<yy<<zz<<...
#define businlog_error_return_err_reason(exp, msg, str_kernel_err_reason, ret)	\
	if (!(exp)) { std::ostringstream oss_err_msg; oss_err_msg << msg; \
	              if ( businlog_inst ) businlog_inst->log_err_return_msg(oss_err_msg); \
				  str_kernel_err_reason = oss_err_msg.str(); return ret; }
/************************************************************************/
/*    ���ܣ��ж����ʽ��Ϊ����ɶҲ��������֮����������з���Ա���û��Ĵ�����Ϣ������ 
      @exp -[in]֧�֣��������ı��ʽ
	  @msg_for_debug -[in]�����з���Ա���bug����Ϣ����ʽΪxx<<yy<<zz<<...
	  @str_err_reason_for_debug -[out] Ϊ�з���Ա�ṩ�Ĵ�����ʾ��Ϣ
	  @msg_for_user -[in] �����û����bug����Ϣ,xx<<yy<<zz<<...�����Ϊ���ģ�
	  @str_err_reason_for_user -[out] Ϊ�û��ṩ�Ĵ�����ʾ��Ϣ
*/
/************************************************************************/
#define businlog_error_return_debug_and_user_reason(exp, msg_for_debug, str_err_reason_for_debug, msg_for_user, str_err_reason_for_user, ret)	\
	if (!(exp)) {	\
		std::ostringstream oss_err_msg_for_debug; oss_err_msg_for_debug << msg_for_debug; \
		if ( businlog_inst ) businlog_inst->log_err_return_msg(oss_err_msg_for_debug); \
		str_err_reason_for_debug = oss_err_msg_for_debug.str(); \
		std::ostringstream oss_err_msg_for_user; \
		oss_err_msg_for_user << msg_for_user; \
		str_err_reason_for_user = oss_err_msg_for_user.str();\
		return ret; }
// ���ݷ���ֵ��ӡ��ͬ����Ϣ
#define businlog_func_ret(ret, msg)		if ( ret != 0 ) { businlog_error msg; } else { businlog_verbose msg; }
#define businlog_func_ret_info(ret, msg)	if ( ret != 0 ) { businlog_error msg; } else { businlog_info msg; }

// ���ݺ�������ֵ��ӡ��ͬ����Ϣ
#define businlog_func_verb(ret, succ_msg, fail_msg)	\
	if ( ret == 0 ) { businlog_verbose succ_msg; }	else { businlog_error fail_msg; }
#define businlog_func_info(ret, succ_msg, fail_msg)	\
	if ( ret == 0 ) { businlog_info succ_msg; } else { businlog_error fail_msg; }
#define businlog_func_verb_return(ret, succ_msg, fail_msg)	\
	if ( ret == 0 ) { businlog_verbose succ_msg; }	else { businlog_error fail_msg; return ret; }
#define businlog_func_info_return(ret, succ_msg, fail_msg)	\
	if ( ret == 0 ) { businlog_info succ_msg; } else { businlog_error fail_msg; return ret; }

// �жϱ��ʽ����ӡ��ͬ����Ϣ
#define businlog_func_expr(expr, succ_msg, fail_msg)	\
	if ( expr ) { businlog_info succ_msg; } else { businlog_error fail_msg; }

// У����������ӡ��־
#define businlog_verify_inst(func, inst)				\
	if ( !(inst) ) { businlog_error("%s | %s handle is NULL.", #func, #inst); return MSP_ERROR_NULL_HANDLE; }
#define businlog_verify_para(func, para)				\
	if ( !(para) ) { businlog_error("%s | para %s is NULL.", #func, #para); return MSP_ERROR_INVALID_PARA; }
#define businlog_verify_return(func, inst, ret, msg)	\
	if ( !(inst) ) { businlog_error("%s | %s is NULL, %s.", #func, #inst, msg); return ret; }

#define businlog_verify_rec(func)			businlog_verify_inst(func, rec)
#define businlog_verify_ep(func)			businlog_verify_inst(func, ep)

#endif /* __BUSIN_LOG_H__ */
