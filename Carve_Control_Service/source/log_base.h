/*=============================================================================================================*/
/** 
 * @file	log_base.h
 * @brief	日志类的声明文件
 * 
 * 这个日志类是一个简单易用的的日志类，用户可以的使用在绝大多数C/C++应用程序中。为用户提供了非常方便的策略
 * 搭配方案，用户可以使用现有提供的多种策略模式，也可以自己扩展策略，包含IO策略、配置读取策略、锁定策略等。
 * 主要功能：
 *		1.  分等级记录信息：关键、信息、警告、错误、调试信息、性能信息；
 *		2.  多种输出方式：文件、控制台、调试器、MessageBox，如果在界面程序中使用控制台输出，会自动分配一个控制台；
 *		3.  提供线程进程级安全，支持多线程、多进程使用一个日志；
 *		4.  支持从注册表和配置文件读取配置信息，支持指定自定义的section名；
 *		5.  支持高精度的性能统计功能，支持函数调用时间的自动统计；
 *		6.  支持Windows、Linux、Solaris操作系统，支持VC6/7/8、GCC编译器；
 *		7.  支持主题和主题过滤功能，为了实现这个功能，需要派生子类并重载subject()函数；
 *		8.  应用了singleton模式，可以以单实例使用，同时支持非单体的使用；
 *		9.  支持用户自定义的互斥锁定方案：无锁定、线程锁定、进程锁定（Windows）等；
 *		10. 支持用户自定义配置读取方案，目前支持注册表和ini格式的配置文件；
 *		11. 支持用户自行实现日志的IO方法，目前提供的仅有基于文件的IO实现，用户可以考虑TCP方式、管道方式等；
 *		12. 支持当配置文件发生改变时，重新读取配置的功能，可以实时调整记录级别、输出方式；
 *		14. 提供了多种样式可供选择，包括时间、主题、等级、线程ID、进程ID、自动回车，可以组合；
 *		15. 不依赖于其他头文件，直接拷贝然后include就可以使用；
 *		16. 基于接口的模块间桥接机制，使得日志的代码更新不需要重新编译牵连的模块，多个模块之间可以
 *			实现安全的日志串联共享；
 *		17. 支持日志自动备份机制，支持日志大小、数量的限制，当日志数量达到一定数量（缺省20）时，
 *			会自动删除最早备份的文件；
 *
 * 用法举例：
 *      1. 如果需要一个不需要锁定和配置功能的文件日志类，可以如下用法：
 *			// 需要单体模式
 *			typedef Log_Singleton<Log_IO_FILE, Log_Mutex_Null, Log_Cfg_T<Log_Cfg_Heap> > XXX_Logger;
 *			// 不要单体模式：
 *			typedef Log_Impl_T<Log_IO_FILE, Log_Win32_Mutex, Log_Cfg_T<Log_Cfg_Heap> > XXX_Logger;
 *
 *		2. 如果需要一个在Win32下进程锁定和注册表、ini文件配置功能的文件日志类，可以如下：
 *			typedef Log_Singleton<Log_IO_FILE, Log_Win32_Process_Mutex, Log_Cfg_T<Log_Cfg_Heap_Reg> > XXX_Logger;
 *
 *		3. 用户可以从Log_Impl_T派生，实现自己需要的功能：
 *			class XXX_Log : public Log_Impl_T<IO, MUTEX, CFG> { ... }
 *			可以派生实现的函数在Log_Impl_T类中；
 *
 *		4. 单体类在使用的时候有多个open方法
 *			open(const CFG & cfg, const char * cfg_file = 0, const char * key = 0);
 *			open(const char* cfg_file, const char* key = 0,  const CFG* cfg_p = 0, LOG_HANDLE hmod = 0, void* rev = 0);
 *			open(const CFG & cfg, Log_IO * io_impl = 0, void * rev = 0);
 *			open(const char* io_lib,   const char* cfg_file, const char* key, const CFG* cfg_p, LOG_HANDLE hmod, void* rev);
 *			其中第一个函数是最常用的方法，此时用户需要传入一个配置文件的路径。
 *
 *		5. 简化的单体声明方式：
 *		   (1) LOG_DECL_SINGLETON_EX(Log_IO_FILE, Log_Mutex_Null, Log_Cfg_Heap_Reg, XXX_Logger)
 *             这个宏声明了一个使用文件IO，使用进程锁定，使用注册表、ini读取的日志单体
 *		   (2) LOG_DECL_SINGLETON_EX(Log_IO_FILE, Log_Mutex_Process, ISP_Log_Cfg_Heap, ISP_Logger)
 *			   这个宏声明了一个单体日志，使用文件方式、进程锁、能够读取cfg文件的配置类，该日志类的名字为ISP_Logger
 *		   (3) LOG_DECL_SIMPLE(Log_IO_FILE, Log_Mutex_Null, BASE_Logger)
 *			   这是一个更加简化的声明方式
 *
 *		6. 举一个实际的例子，用户需要一个名字为 My_Logger 的单体日志，步骤如下：
 *			// 在一个公用的头文件里面声明单体
 *			LOG_DECL_SINGLETON_EX(Log_IO_FILE, Log_Win32_Process_Mutex, Log_Cfg_Heap_Reg, My_Logger);
 *			...
 *			// 在程序初始化时初始化日志
 *			My_Logger_Cfg default_cfg("my.log", "My logging information");
 *			My_Logger::open("my.cfg", "logger", &default_cfg);
 *			...
 *			// 下面就可以正常使用了
 *			My_Logger::instance()->log_error("This is a log msg, line %d.", __LINE__);
 *			...
 *			// 最后程序退出时别忘了关闭日志
 *			My_Logger::close();
 *
 *		7. 如果用户临时使用一下(不想读取配置文件或注册表，也不想使用同步)的话，其实类似一个文件，可以如下方法：
 *			// 先声明类型
 *			typedef Log_Impl_T<Log_IO_FILE> My_Logger;
 *			// 需要使用的时候
 *			My_Logger log("c:\\test.log", "Test Log");
 *			log.log_error("This is a error line.");
 *
 *		8. 如果用户临时使用，需要读取配置文件或注册表，可以如下方法：
 *			// 先声明类型，注意这个配置的类型可以支持ini文件和注册表
 *          typedef Log_Cfg_T<Log_Cfg_Heap_Reg> My_Logger_Cfg;
 *			typedef Log_Impl_T<Log_IO_FILE, Log_Thread_Mutex, My_Logger_Cfg> My_Logger;
 *			// 需要使用的时候
 *			My_Logger_Cfg cfg("c:\\test.log", "Test Log");
 *			cfg.read_config("logger", "my_logger.cfg");
 *			My_Logger log;
 *			log.open(&cfg);
 *			log.log_error("This is a error line.");
 *			log.close();
 *
 *		9. 最简单的使用方法就是把本头文件最后的那些宏定义拷贝到自己的头文件中，改成自己的名
 *		   字，直接就可以使用了。
 *
 * 注意事项：
 *		1.  在单体方式使用本日志类时，如果在一个解决方案中存在多个静态库工程，请注意为你的每个静态库
 *			中的日志单体指定一个唯一的名字，静态库之间通过log_io接口进行连接；
 *			举个例子：
 *			LOG_DECL_SINGLETON_EX(io, lock, cfg, name) 其中的name就是你需要取得名字。
 */

#ifndef __BASELOGGER_H__
#define __BASELOGGER_H__

// disable vc6 warning
#if _MSC_VER < 1300
# pragma warning(disable:4786)
#endif /* _MSC_VER < 1300 */

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#ifdef WIN32
#  if !defined (_WIN32_WINNT)
#   define _WIN32_WINNT 0x0400
#  endif // _WIN32_WINNT
# include <io.h>
# include <WinSock2.h>
# include <Windows.h>
#else /* Linux */
# include <sys/time.h>
# include <stdarg.h>
# include <pthread.h>
#endif /* WIN32 */

#if _MSC_VER > 1000
# pragma once
#endif /* _MSC_VER > 1000 */

/* cdecl */
#ifdef WIN32
# define LOG_CDECL __cdecl
#else
# define LOG_CDECL
#endif

/* 句柄类型的定义 */
typedef void *					LOG_HANDLE;
typedef std::pair				<std::string, std::string> LOG_STR_PAIR;
typedef std::vector				<LOG_STR_PAIR> LOG_STRPAIR_ARR;

/* Invalid handle */
#define LOG_INVALID_HANDLE		((LOG_HANDLE)-1)

/* size_t */
#ifndef _SIZE_T_DEFINED
# ifdef  _WIN64
   typedef unsigned __int64		size_t;
# else
   typedef unsigned int			size_t;
# endif
# define _SIZE_T_DEFINED
#endif /* _SIZE_T_DEFINED */

/* time_t */
#ifndef _TIME_T_DEFINED
# ifdef  _WIN64
   typedef __int64 time_t;     /* time value */
# else
#  ifndef _AIX
    typedef long    time_t;     /* time value */
#  endif
# endif
# define _TIME_T_DEFINED
#endif /* _TIME_T_DEFINED */

/* 是否打开日志的宏 */
#define BASE_LOG_ENABLE			1

/* 最大写入信息长度 */
#ifdef _AIX
 const int LOG_MAXMSGLEN		= 4 * 1024;
#else
 const int LOG_MAXMSGLEN		= 80 * 1024;
#endif

/* 最大日志文件长度 */
const int LOG_MAXFILELEN		= 10 * 1024 * 1024;
/* 最大日志数量 */
const int LOG_MAXCOUNT			= 20;
/* 缺省日志文件 */
#define LOG_DEFFILE				"server.log"
/* 缺省标题 */
#define LOG_DEFTITLE			"Running Information"
/* 检查配置变化的间隔时间(秒) */
const int LOG_CFGCHECK_TIME		= 10;
/* 配置文件缺省的键名称 */
#define LOG_DEFKEY				"logger"


/* 日志记录的级别 */
enum LOG_LEVEL
{
	lgl_none		= 0,		/* 不输出 */
	lgl_crit		= 1,		/* 关键信息 */
	lgl_error		= 2,		/* 错误信息 */
	lgl_warning		= 4,		/* 警告信息 */
	lgl_low			= lgl_warning | lgl_error | lgl_crit,	/* 组合类型 - 少量信息 */
	lgl_info		= 8,		/* 一般信息 */
	lgl_normal		= lgl_low | lgl_info,					/* 组合类型 - 一般信息 */
	lgl_verbose		= 16,		/* 不重要的信息 */
	lgl_debug		= 32,		/* 调试信息，信息量狂大 */
	lgl_detail		= lgl_normal | lgl_verbose | lgl_debug,	/* 组合类型 - 大量信息 */
	lgl_perf		= 64,		/* 性能信息，信息量大 */
	lgl_trace		= 128,		/* 调用信息 */
	lgl_spec		= 256,		/* 特殊级别，用于特别用途 */
	lgl_all			= -1,		/* 所有信息 */
};

/* 日志记录的输出方式 */
enum LOG_OUTPUT
{
	lgo_none		= 0,		/* 不输出 */
	lgo_file		= 1,		/* 文件方式输出 */
	lgo_console		= 2,		/* 输出到控制台 */
	lgo_debugger	= 4,		/* 输出到调试器 */
	lgo_msgbox		= 8,		/* 输出到信息框 */
};

/* 记录日志的样式 */
enum LOG_STYLE
{
	lgs_default		= 0x00,		/* 缺省样式 */
	lgs_time_hms	= 0x01,		/* 简单的时间 */
	lgs_time		= 0x02,		/* 包含时间 */
	lgs_level		= 0x04,		/* 包含错误等级 */
	lgs_threadid	= 0x08,		/* 包含线程ID */
	lgs_crlf		= 0x10,		/* 包含回车 */
	lgs_subject		= 0x20,		/* 包含主题 */
	lgs_precessid	= 0x40,		/* 包含进程ID */
	lgs_none		= 0x100,	/* 不包含信息 */
	lgs_all			= 0x0FF,	/* 所有信息 */

	lgs_short		= lgs_time_hms | lgs_level | lgs_threadid | lgs_crlf | lgs_subject,
	lgs_long		= lgs_time | lgs_level | lgs_threadid | lgs_precessid | lgs_crlf | lgs_subject,
};

/* 日志的打开模式 */
enum LOG_MODE
{
	lgm_text		= 0,		/* 文本追加模式 */
	lgm_binary		= 1,		/* 二进制追加模式 */
	lgm_text_ow		= 2,		/* 文本覆盖模式 */
	lgm_binary_ow	= 3,		/* 二进制覆盖模式 */
};

/* 日志的宽窄字符设置 */
enum LOG_CHARSET
{
	lgc_auto		= 0,		/* 自动 */
	lgc_mbcs		= 1,		/* 窄字符 */
	lgc_wchar		= 2,		/* 宽字符 */
};

/** 
 * @class	Log_Cfg_Heap
 * 
 * @brief	配置读取类
 * 
 *  这里定义了一个配置读取和写入的封装外观
 *  在实例化的时候用户可以指定一个读取的实现方式
 * 
 * @author	
 * @date	2005-10-28
 * 
 */
struct Log_Cfg_Heap
{
	int open(const char* /* section */, const char* /* file */ = 0)
		{ return 0; }
	int close(void)
		{ return 0; }
	int get_string_value(const char* /* key */, std::string & /* value */, const char* /* sub */ = 0)
		{ return 0; }
	int set_string_value(const char* /* key */, const char* /* value */, const char* /* sub */ = 0)
		{ return 0; }
	int get_string_arr  (const char* /* key */, std::vector<std::string> & /* arr */)
		{ return 0; }
};

/** 
 * @class	 Log_Cfg
 * 
 * @brief	配置接口
 * 
 *  这里定义了一个配置接口，用于模块间的传递
 * 
 * @author	
 * @date	2005-10-28
 * 
 */
struct Log_Cfg
{
	virtual const char * file_name		(void) const = 0;
	virtual const char * title			(void) const = 0;
	virtual const char * cfg_file		(void) const = 0;
	virtual const char * locale			(void) const = 0;
	virtual size_t		 max_size		(void) const = 0;
	virtual bool		 overwrite		(void) const = 0;
	virtual int			 output			(void) const = 0;
	virtual int			 level			(void) const = 0;
	virtual int			 style			(void) const = 0;
	virtual int			 filter			(void) const = 0;
	virtual bool		 flush_always	(void) const = 0;
	virtual int			 max_count		(void) const = 0;
	virtual int			 cache_size		(void) const = 0;
	virtual int			 mode			(void) const = 0;
	virtual int          perf_thres     (void) const = 0;
	virtual bool		 auto_name		(void) const = 0;
	virtual bool		 cf_asyn		(void) const = 0;
	virtual bool		 has_head		(void) const = 0;
	virtual bool		 thr_vip		(void) const = 0;
	virtual void *		 module			(void) const = 0;
	virtual int			 console_level	(void) const = 0;
	virtual LOG_CHARSET	 charset		(void) const = 0;
	virtual time_t		 start_time		(void) const = 0;
	virtual unsigned int start_tick		(void) const = 0;
	virtual const char * log_server		(void) const = 0;
};

/* 日志的配置信息 */
template <class CFG_HEAP = Log_Cfg_Heap, typename BASE = Log_Cfg>
class Log_Cfg_T
	: public BASE
{
public:
	Log_Cfg_T(const char* file = LOG_DEFFILE, const char* ttl = LOG_DEFTITLE, int md = lgm_text)
		: max_size_		(LOG_MAXFILELEN)
		, max_count_	(20)
		, overwrite_	(0)
		, output_		(lgo_file)
		, level_		(lgl_low)
		, style_		(lgs_all)
		, filter_		(-1)
		, flush_always_	(false)
		, has_head_		(true)
		, synch_time_	(false)
		, module_		(0)
		, cfg_mtime_	(0)
		, section_		("logger")
		, mode_			(md)
		, encrypt_		(0)
		, locale_		(".ACP")
		, console_level_(level_)
		, charset_		(lgc_auto)
		, start_time_	(0)
		, start_tick_	(0)
		, perf_thres_	(0)
		, auto_name_	(false)
		, cf_asyn_		(false)
		, cache_size_	(0)
		, thr_vip_		(false)
		{
		file_name(file);
		title(ttl);
		if ( md == lgm_binary || md == lgm_binary_ow )
			style_ = lgs_none;
		}
	Log_Cfg_T(const BASE * cfg)
		{
		this->set(cfg);
		}

	Log_Cfg_T & operator = (const Log_Cfg_T & cfg)
		{
		max_size_	  = cfg.max_size_;
		max_count_	  = cfg.max_count_;
		overwrite_	  = cfg.overwrite_;
		output_		  = cfg.output_;
		level_		  = cfg.level_;
		style_		  = cfg.style_;
		filter_		  = cfg.filter_;
		module_		  = cfg.module_;
		file_name_	  = cfg.file_name_;
		title_		  = cfg.title_;
		flush_always_ = cfg.flush_always_;
		has_head_	  = cfg.has_head_;
		synch_time_	  = cfg.synch_time_;
		cfg_file_	  = cfg.cfg_file_;
		section_	  = cfg.section_;
		cfg_mtime_	  = cfg.cfg_mtime_;
		mode_		  = cfg.mode_;
		encrypt_	  = cfg.encrypt_;
		locale_		  = cfg.locale_;
		console_level_= cfg.console_level_;
		charset_	  = cfg.charset_;
		start_time_	  = cfg.start_time_;
		start_tick_	  = cfg.start_tick_;
		perf_thres_	  = cfg.perf_thres_;
		auto_name_	  = cfg.auto_name_;
		cf_asyn_	  = cfg.cf_asyn_;
		cache_size_	  = cfg.cache_size_;
		log_server_	  = cfg.log_server_;
		thr_vip_	  = cfg.thr_vip_;
		includes_.assign(cfg.includes_.begin(), cfg.includes_.end());
		excludes_.assign(cfg.excludes_.begin(), cfg.excludes_.end());
		return *this;
		}

	const char* file_name(void) const
		{ return file_name_.c_str(); }
	void file_name(const char* fn)
		{ if ( fn != 0 ) file_name_ = fn; }

	const char* title(void) const
		{ return title_.c_str(); }
	void title(const char* tl)
		{ if ( tl ) title_ = tl; }

	size_t max_size(void) const
		{ return max_size_; }
	void max_size(size_t size)
		{ max_size_ = size; }

	bool overwrite(void) const
		{ return overwrite_; }
	void overwrite(bool ow)
		{ overwrite_ = ow; }

	bool thr_vip(void) const
		{ return thr_vip_; }
	void thr_vip(bool vip)
		{ thr_vip_ = vip; }

	int output(void) const
		{ return output_; }
	void output(int op)
		{ output_ = op; }

	int level(void) const
		{ return level_; }
	void level(int lvl)
		{ console_level_ = level_ = lvl; }

	int style(void) const
		{ return style_; }
	void style(int stl)
		{ style_ = stl; }

	int filter(void) const
		{ return filter_; }
	void filter(int flt)
		{ filter_ = flt; }

	bool flush_always(void) const
		{ return flush_always_; }
	void flush_always(bool fa)
		{ flush_always_ = fa; }

	bool synch_time(void) const
		{ return synch_time_; }
	void synch_time(bool st)
		{ synch_time_ = st; }

	void * module(void) const
		{ return module_; }
	void module(void* mod)
		{ module_ = mod; }

	int max_count(void) const
		{ return max_count_; }
	void max_count(int maxc)
		{ if ( maxc > 0 ) max_count_ = maxc; }

	int mode(void) const
		{ return mode_; }
	void mode(int md)
		{ mode_ = md; }

	bool has_head(void) const
		{ return has_head_;	}
	void has_head(bool hhd)
		{ has_head_ = hhd; }

	const char * locale(void) const
		{ return locale_.c_str(); }
	void locale(const char * lc)
		{ if ( lc ) locale_ = lc; }

	int encrypt(void) const
		{ return encrypt_;	}
	void encrypt(int enc)
		{ encrypt_ = enc;	}

	int console_level(void) const
		{ return console_level_; }
	void console_level(int cl)
		{ console_level_ = cl; }

	LOG_CHARSET charset(void) const
		{ return charset_; }
	void charset(LOG_CHARSET cs)
		{ charset_ = cs; }

	time_t start_time(void) const
		{ return start_time_; }
	void start_time(time_t tm)
		{ start_time_ = tm; }
	unsigned int start_tick(void) const
		{ return start_tick_; }
	void start_tick(unsigned int tick)
		{ start_tick_ = tick; }

	const char* cfg_file(void) const
		{ return cfg_file_.c_str(); }

	int perf_thres(void) const
		{ return perf_thres_; }
	void perf_thres(int pth)
		{ perf_thres_ = pth; }
	
	bool auto_name(void) const
		{ return auto_name_; }
	void auto_name(bool an)
		{ auto_name_ = an; }

	bool cf_asyn(void) const
		{ return cf_asyn_; }
	void cf_asyn(bool cfa)
		{ cf_asyn_ = cfa;	}

	const char * log_server(void) const
		{ return log_server_.c_str();}

	void log_server(const char *svr)
		{ log_server_ = svr;}

	int cache_size(void) const
		{ return cache_size_; }
	void cache_size(int cs)
		{ cache_size_ = cs; }

	// 判断内容是否应该输出
	bool is_output(const char* msg, const char * sub) const
		{
		// 排除的优先
		std::vector<std::string>::const_iterator pi = excludes_.begin();
		for ( ; pi != excludes_.end(); ++pi )
			{
			if ( sub && (*pi).compare(sub) == 0 )
				return false;
			if ( strstr(msg, (*pi).c_str()) )
				return false;
			}

		// 再判断过滤的
		pi = includes_.begin();
		for ( ; pi != includes_.end(); ++pi )
			{
			if ( sub && (*pi).compare(sub) == 0 )
				return true;
			if ( strstr(msg, (*pi).c_str()) )
				return true;
			}

		return includes_.empty() ? true : false;
		}

	// 从一个接口获得接口的属性
	void set(const BASE * bs)
		{
		if ( bs == 0 )
			return ;
		max_size_		= bs->max_size();
		max_count_		= bs->max_count();
		overwrite_		= bs->overwrite();
		output_			= bs->output();
		level_			= bs->level();
		style_			= bs->style();
		filter_			= bs->filter();
		module_			= bs->module();
		flush_always_	= bs->flush_always();
		mode_			= bs->mode();
		has_head_		= bs->has_head();
		locale_			= bs->locale();
		console_level_  = bs->console_level();
		charset_		= bs->charset();
		file_name		 (bs->file_name());
		title			 (bs->title());
		encrypt_		= 0;
		cfg_mtime_		= 0;
		start_time_		= bs->start_time();
		start_tick_		= bs->start_tick();
		auto_name_		= bs->auto_name();
		cf_asyn_		= bs->cf_asyn();
		cache_size_		= bs->cache_size();
		thr_vip_		= bs->thr_vip();
		}

	// 添加或者减少level
	int level_add(int lvl)
		{
		return level_ |= lvl;
		}
	int level_remove(int lvl)
		{
		return level_ &= ~lvl;
		}
	// 添加或减少输出
	int output_add(int out)
		{
		return output_ |= out;
		}
	int output_remove(int out)
		{
		return output_ &= ~out;
		}

	// 读取配置信息
	int read_config (const char* key = 0, const char* cfg_file = 0);
	// 写入配置信息
	int write_config(const char* key = 0, const char* cfg_file = 0);
	// 判断配置信息是否改变了
	bool config_changed(void);

private:
	/// 日志文件名称
	std::string file_name_;
	/// 标题
	std::string title_;
	/// 日志最大尺寸
	size_t		max_size_;
	/// 日志文件的最大数量
	int			max_count_;
	/// 日志输出方式
	int			output_;
	/// 日志记录过滤器 
	int			level_;
	/// 日志的样式
	int			style_;
	/// 模块输出过滤器
	int			filter_;
	/// 是否覆盖（备份否）旧的文件
	bool		overwrite_;
	/// 是否每条信息都flush
	bool		flush_always_;
	/// 是否包含头
	bool		has_head_;
	/// 是否需要对时
	bool		synch_time_;
	bool		thr_vip_;
	/// 模块句柄
	void *		module_;
	/// 包含的字符串
	std::vector<std::string> includes_;
	/// 排除的字符串
	std::vector<std::string> excludes_;
	/// 配置文件名
	std::string cfg_file_;
	/// section名
	std::string section_;
	/// 日志配置文件的最后修改日期
	time_t		cfg_mtime_;
	/// 日志的打开模式
	int			mode_;
	/// 输出信息是否加密
	int			encrypt_;
	/// 位置信息
	std::string locale_;
	/// 控制台输出时的等级
	int			console_level_;
	/// 宽窄字符
	LOG_CHARSET charset_;
	/// 开始时间
	time_t		start_time_;
	unsigned int start_tick_;
	// 性能门限
	int			perf_thres_;
	// 文件存在时是否重新生成	
	bool		auto_name_;
	// 是否异步检测配置文件
	bool		cf_asyn_;
	int			cache_size_;
	// log服务器地址
	std::string	log_server_;
};


/** 
 * @class	 Log_Mutex
 * 
 * @brief	日志的锁定接口
 * 
 *  多线程方式下使用的日志需要用户自定义锁定方式
 * 
 * @author	
 * @date	2005-10-28
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
struct Log_Mutex
{
	virtual int open(const char* name = 0) = 0;
	virtual int close(bool call_delelte = false) = 0;
	virtual int acquire(int time_out = -1) = 0;
	virtual int release(void) = 0;
	virtual const char* name(void) = 0;
};

/** 
 * @class	 Log_IO
 * 
 * @brief	日志的I/O读取接口
 * 
 *  这个接口定义了日志的输出方式，
 *  可以自行实现为多种方式，例如文件方式、socket方式、内存方式等等
 * 
 * @author	
 * @date	2005-10-28
 * 
 * @see		Log_IO_File
 */
struct Log_IO
{
	// 初始化/逆初始化
	virtual int open(const char * fn, Log_Mutex * lock = 0, int mode = 0, void * rev = 0) = 0;
	virtual int close(void) = 0;

	// 写一条记录，文本方式
	virtual int write(const char * msg) = 0;
	virtual int write(const wchar_t * msg) = 0;
	// 二进制方式写入
	virtual int write(const void * data, int size) = 0;

	// 清空文件
	virtual int empty(void) = 0;
	// 从缓存刷新日志
	virtual void flush(void) = 0;
	// 获得当前尺寸
	virtual size_t size(void) = 0;

	/// 备份旧的文件
	virtual int backup(const char* bname = 0) = 0;
	/// 日志的句柄
	virtual LOG_HANDLE handle(void) = 0;

	/* 日志锁 */
	virtual Log_Mutex * mutex(void) = 0;
	virtual void mutex(Log_Mutex * lock) = 0;
	/* 日志配置 */
	virtual Log_Cfg * config(void) = 0;
	virtual void config(Log_Cfg * cfg) = 0;
	/* 是否自行释放 */
	virtual bool donot_delete(void) = 0;
};

/** 
 * @class	 Log_Mutex_Null
 * 
 * @brief	一个空的日志锁
 * 
 *  如果不需要锁定，请使用这个类
 * 
 * @author	
 * @date	2005-10-11
 * 
 */
class Log_Mutex_Null
	: public Log_Mutex
{
public:
	int open(const char* /* name */ = 0)
		{ return 0; }
	int close(bool call_delete = false)
		{ if ( call_delete ) delete this; return 0; }
	int acquire(int /* time_out */ = -1)
		{ return 0; }
	int release(void)
		{ return 0; }
	const char* name(void)
		{ return 0; }
	Log_Mutex_Null(const char* /* name */ = 0)
		{ }
};

/*
 * 在Win32下实现的进程锁
 */
class Log_Win32_Process_Mutex
	: public Log_Mutex
{
public:
	int open(const char* name = 0);
	int close(bool call_delete = false);
	int acquire(int time_out = -1);
	int release(void);
	const char* name(void);

	Log_Win32_Process_Mutex(const char* name = 0, bool create = false);
	~Log_Win32_Process_Mutex(void);

private:
	// 互斥量句柄
	LOG_HANDLE mutex_;
	// 名称
	char name_[252];
};

/*
 * 在Win32和Linux、Solaris下实现的线程锁
 */
class Log_Thread_Mutex
	: public Log_Mutex
{
public:
	int open(const char* name = 0);
	int close(bool call_delete = false);
	int acquire(int time_out = -1);
	int release(void);

	const char* name(void)
		{ return 0; }

	Log_Thread_Mutex(bool create = false);
	~Log_Thread_Mutex(void);

private:
	// 互斥量句柄
	LOG_HANDLE mutex_;
};

/*
 * 在Linux、Solaris下实现的进程锁
 */
class Log_Unix_Process_Mutex
	: public Log_Thread_Mutex
{
public:
	int open(const char* name = 0);
	int close(bool call_delete = false);
	int acquire(int time_out = -1);
	int release(void);

	const char* name(void)
		{ return 0; }

	Log_Unix_Process_Mutex(const char* name = 0, bool create = false);
	~Log_Unix_Process_Mutex(void);
};

/*
 * 在Win32和Linux、Solaris下实现的进程锁
 */
#if defined(WIN32) || defined(_WINDOWS)
  typedef Log_Win32_Process_Mutex Log_Process_Mutex;
#else // Non Windows
  typedef Log_Unix_Process_Mutex Log_Process_Mutex;
#endif // defined(WIN32) || defined(_WINDOWS)

// 同步对象使用适配器
template <typename T>
class Log_Mutex_Guard
{
public:
	Log_Mutex_Guard(T * lock)
		: lock_(lock)
		{
		if ( lock_ )
			lock_->acquire();
		}
	Log_Mutex_Guard(T & lock)
		: lock_(&lock)
		{
		if ( lock_ )
			lock_->acquire();
		}
	~Log_Mutex_Guard(void)
		{
		if ( lock_ )
			lock_->release();
		}
private:
	T * lock_;
};

// 方便使用的宏
#define LOG_GUARD(mutex, lock)	Log_Mutex_Guard<mutex> _mon(lock);
#define LOG_GUARD_PROCESS(name)	Log_Process_Mutex _lock(name, true); Log_Mutex_Guard<Log_Process_Mutex> _mon(_lock);

/// 使用文件实现的IO类
class Log_IO_FILE
	: public Log_IO
{
public:
	/* 初始化/逆初始化 */
	virtual int open(const char * fn, Log_Mutex * lock = 0, int mode = 0, void * rev = 0);
	virtual int close(void);

#ifndef WIN32
	/* Linux环境下异步日志时，用于备份日志 */
    int copyfile(const char* dst_file, const char* src_file);
    int copyfile(FILE* dst_fp, const char* src_file);
#endif

	/* 写一条记录，文本方式 */
	virtual int write(const char * msg);
	virtual int write(const wchar_t * msg);
	/* 二进制方式写入 */
	virtual int write(const void * data, int size);

	/* 清空文件 */
	virtual int empty(void);
	/* 从缓存刷新日志 */
	virtual void flush(void);
	/* 获得当前尺寸 */
	virtual size_t size(void);

	/* 备份旧的文件 */
	virtual int backup(const char * bname = 0);
	/* 日志的句柄 */
	virtual LOG_HANDLE handle(void);
	/* 日志锁 */
	virtual Log_Mutex * mutex(void);
	virtual void mutex(Log_Mutex * lock);
	/* 日志配置 */
	virtual Log_Cfg * config(void);
	virtual void config(Log_Cfg * cfg);
	virtual bool donot_delete(void);

	/* constructor */
	Log_IO_FILE(void);
	virtual ~Log_IO_FILE(void);

private:
	// 文件指针
	FILE*			file_;
	// 文件名称
	std::string		fn_;
	// 日志锁
	Log_Mutex *		lock_;
	// 配置
	Log_Cfg *		cfg_;
};

/// 单体模板类
template <typename T>
class Base_Singleton_T
{
public:
	// 得到日志全局实例
	// 得到日志全局实例，如果该实例还没有创建，则创建之
	// 用户必须调用close_singleton销毁创建的实例
	static T* open_singleton(void)
		{
		if ( instance() == 0 )
			{
			instance() = new T;
			need_delelte() = true;
			}
		ref_count()++;
		return instance();
		}
	static T* instance(T* tm)
		{
		T* t = instance();
		close_singleton();
		// 外部传入的不需要delete
		need_delelte() = false;
		instance() = tm;
		return t;
		}
	// 如果用户创建了全局实例，必须调用这个函数
	static void close_singleton(void)
		{
		if ( --ref_count() > 0 )
			return ;
		if ( instance() && need_delelte() )
			{
			delete instance();
			instance() = 0;
			need_delelte() = false;
			}
		}
	static T* & instance(void)
		{
		static T* the_inst = 0;
		return the_inst;
		}
	static bool & need_delelte(void)
		{
		/// 单实例相关的成员
		static bool	delete_instance_ = false;
		return delete_instance_;
		}
	static int & ref_count(void)
		{
		static int reference_count_ = 0;
		return reference_count_;
		}
protected:
	Base_Singleton_T(void)
		{ }
};

/** 
 * @brief	根据生成一个唯一的类型
 * 
 *  这个类用于解决Log_Singleton模板如果声明的时候两个单体类型完全一样，
 *  就会使用同一个单体指针的问题
 * 
 * @author	
 * @date	2005-12-23
 */
template <typename T, typename E /* , E nontype_param */>
class Log_Uni_Type
	: public T
{
	operator T & (void)
		{ return *this; }
};

// 这个宏可以生成一个唯一的类型
# define LOG_UT_DECL(cls, name)										\
	class __LOG_TOK_##cls##_##name##__ { };							\
	typedef Log_Uni_Type<cls, __LOG_TOK_##cls##_##name##__> name;

/**
 * 一个CFG_HEAP，在
 *	Windows下能够读取注册表和ini文件
 *	Linux下能够读取配置文件
 */
class Log_Cfg_Heap_Reg
	: public Log_Cfg_Heap
{
public:
	// open / close
	int open(const char* section, const char* file = 0);
	int close(void);
	
	// value get /set
	int get_string_value(const char* key, std::string & value, const char* sub = 0);
	int enum_values(LOG_STRPAIR_ARR & arr, const char* sub = 0);
	int get_string_arr  (const char* key, std::vector<std::string> & arr);

	// construct
	Log_Cfg_Heap_Reg(void);
	virtual ~Log_Cfg_Heap_Reg(void);

protected:
	// root
	std::string root_;
	// file
	std::string file_;
	// handle
	LOG_HANDLE	handle_;
	// is ini file?
	bool		ini_;
};

#ifdef WIN32
#include <process.h>
class Log_Thread
{
public:
	int active();
	int join();

protected:
	virtual int svc() = 0;

public:
	static unsigned __stdcall _run(void *thrp)
	{ Log_Thread* thr = (Log_Thread*)thrp; return thr->svc(); }

protected:
	bool	stop_;
private:
	HANDLE	thr_hdl_;
};
#endif
//====================================================================================
// 能够定制锁定和输出策略的日志模板类
//====================================================================================
template <typename IO = Log_IO_FILE,
	typename LOCK = Log_Mutex_Null, typename CFG = Log_Cfg_T<> >
#ifdef WIN32
class Log_Impl_T
	: public Log_Thread
#else
class Log_Impl_T
#endif
{
public:
	typedef IO		io_impl;
	typedef CFG		cfg_impl;
	typedef LOCK	lock_impl;

	Log_Impl_T(void);
	Log_Impl_T(const char * file, const char * title = 0, bool has_head = false, io_impl * io = 0);
	virtual ~Log_Impl_T(void);

public:	
	// 初始化/逆初始化
	virtual int open (const cfg_impl * cfg = 0, io_impl * io = 0, void * rev = 0);
	virtual int close(void);

	// 几个最常用的函数
	void LOG_CDECL write_msg	(const char * fmt, ...);

	// 不包含模块信息的日志函数
	void LOG_CDECL log_crit		(const char * fmt, ...);
	void LOG_CDECL log_error	(const char * fmt, ...);
	void LOG_CDECL log_warn		(const char * fmt, ...);
	void LOG_CDECL log_info		(const char * fmt, ...);
	void LOG_CDECL log_verbose	(const char * fmt, ...);
	void LOG_CDECL log_debug	(const char * fmt, ...);
	void LOG_CDECL log_perf		(const char * fmt, ...);
	void LOG_CDECL log_trace	(const char * fmt, ...);
	void LOG_CDECL log_spec		(const char * fmt, ...);
	void  LOG_CDECL log_err_return_msg  (const std::ostringstream& oss_err_msg);
	// 包含块信息的日志函数
	void LOG_CDECL log_crit		(int sub, const char * fmt, ...);
	void LOG_CDECL log_error	(int sub, const char * fmt, ...);
	void LOG_CDECL log_warn		(int sub, const char * fmt, ...);
	void LOG_CDECL log_info		(int sub, const char * fmt, ...);
	void LOG_CDECL log_verbose	(int sub, const char * fmt, ...);
	void LOG_CDECL log_debug	(int sub, const char * fmt, ...);
	void LOG_CDECL log_perf		(int sub, const char * fmt, ...);
	void LOG_CDECL log_trace	(int sub, const char * fmt, ...);
	void LOG_CDECL log_spec		(int sub, const char * fmt, ...);

	// 包含主题信息的日志函数
	void LOG_CDECL log_crits	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_errors	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_warns	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_infos	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_verboses	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_debugs	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_perfs	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_traces	(const char * sub, const char * fmt, ...);
	void LOG_CDECL log_specs	(const char * sub, const char * fmt, ...);

	// 记录日志信息
	void LOG_CDECL log_msg		(int sub,		  LOG_LEVEL lvl, int style, const char * fmt, ...);
	void LOG_CDECL log_msg		(const char* sub, LOG_LEVEL lvl, int style, const char * fmt, ...);
	void LOG_CDECL log_msg		(const char* msg);
	void LOG_CDECL log_msg		(const wchar_t* msg);

	// 不包含模块信息的日志函数
	void LOG_CDECL write_msg	(const wchar_t * fmt, ...);
	void LOG_CDECL log_crit		(const wchar_t * fmt, ...);
	void LOG_CDECL log_error	(const wchar_t * fmt, ...);
	void LOG_CDECL log_warn		(const wchar_t * fmt, ...);
	void LOG_CDECL log_info		(const wchar_t * fmt, ...);
	void LOG_CDECL log_verbose	(const wchar_t * fmt, ...);
	void LOG_CDECL log_debug	(const wchar_t * fmt, ...);
	void LOG_CDECL log_perf		(const wchar_t * fmt, ...);
	void LOG_CDECL log_trace	(const wchar_t * fmt, ...);
	void LOG_CDECL log_spec		(const wchar_t * fmt, ...);

	// 包含块信息的日志函数
	void LOG_CDECL log_crit		(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_error	(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_warn		(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_info		(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_verbose	(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_debug	(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_perf		(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_trace	(int sub, const wchar_t * fmt, ...);
	void LOG_CDECL log_spec		(int sub, const wchar_t * fmt, ...);

	// 清空文件
	int empty(void);
	// 从缓存刷新日志
	void flush(void);
	// 是否被禁止
	bool enable(void) const;
	// 获得当前尺寸
	size_t size(void) const;

	// =======================================================
	// 外部需要使用的一些方法
public:
	// 配置信息
	cfg_impl & config(void)
		{ return config_; }
	// 返回IO接口
	Log_IO * log_io(void) const
		{ return log_io_; }

	// 锁定和解锁
	virtual int lock (int time_out = -1);
	virtual int unlock (void);

	// =======================================================
	// 可以由派生类进行重新实现的函数
public:
	/// 写一条记录(主要是内部调用)
	virtual void log_msg_i(const char* sub, LOG_LEVEL lvl, int style, const char* msg);
	virtual void log_msg_i(const char* sub, LOG_LEVEL lvl, int style, const wchar_t* msg);
	virtual void log_msg_i(LOG_LEVEL lvl, const char* msg);
	virtual void log_msg_i(LOG_LEVEL lvl, const wchar_t* msg);

	/// 二进制方式写入
	virtual int write(const void * data, int size);

	// 写一个分隔行
	virtual void write_bound(char sep = '-');

protected:
	/// 从一个主题ID获得主题（派生类需要重新实现该函数！）
	virtual const char * subject(int sub, char * buf, int len) const;
	/// 用户可以自定义等级信息的字符串
	virtual const char * level_str(int lvl) const;

	/// 加密函数
	virtual int encrypt(void * msg, int len);

	/// 是否需要输出某个主题的过滤函数
	virtual bool sub_output(int sub) const;
	virtual bool sub_output(const char * sub) const;

	// 写文件头尾
	virtual void write_head(bool new_file = true);
	virtual void write_tail(bool end_file = true);

	// 备份日志
	virtual int  back_file(void);

	// 获得当前时间，缺省实现是精确到秒的
	// 如果需要更加精确，请用派生类实现
	virtual const char * cur_time(char tmbuf[], bool no_path_char = false, bool sht_fmt = false) const;

	// 检查配置文件是否更改，如果更改，重新读取
	virtual int check_config(void);

#ifdef WIN32
	virtual int svc();
#endif
protected:
	/// 日志的配置信息
	cfg_impl		config_;
	/// 日志IO的具体实现类
	Log_IO *		log_io_;
	bool			call_delete_io_;
	/// 日志锁的具体实现
	Log_Mutex *		lock_;
	bool			call_delete_lock_;
	// 日志最后更新时间
	time_t			last_ltime_;
	// 日志是否打开了一个console
	bool			alloc_console_;
};

// =======================================================
// 性能计算的自动辅助类
// 构造函数开始计时，析构函数结束计时，并且打印性能信息
// =======================================================
template <class TIMER, class LOG, typename T = double>
class Log_Perf_Helper
{
public:
	Log_Perf_Helper(const char* name)
		: ext_nsec_ (0)
		, stopped_	(false)
		, log_lvl_	(lgl_perf)
		, mthres_	(0)
		{
		#if _MSC_VER >= 1400
		strcpy_s(name_, sizeof name_, name);
		#else
		strncpy(name_, name, sizeof(name_));
		#endif
		timer_.start(); msg_[0] = 0;
		}
	Log_Perf_Helper(const char* name, T & nsec) 
		: ext_nsec_ (&nsec)
		, stopped_	(false)
		, log_lvl_	(lgl_perf)
		, mthres_	(0)
		{
		#if _MSC_VER >= 1400
		strcpy_s(name_, sizeof name_, name);
		#else
		strncpy(name_, name, sizeof(name_));
		#endif
		timer_.start(); msg_[0] = 0;
		}
	Log_Perf_Helper(const char* name, LOG_LEVEL lvl, int msec_thres) 
		: ext_nsec_ (0)
		, stopped_	(false)
		, log_lvl_	(lvl)
		, mthres_	(msec_thres)
		{
		#if _MSC_VER >= 1400
		strcpy_s(name_, sizeof name_, name);
		#else
		strncpy(name_, name, sizeof(name_));
		#endif
		timer_.start(); msg_[0] = 0;
		}
	~Log_Perf_Helper(void)
		{
		if ( LOG::instance() )
			{
			double msec = stop() * 1e-6;
			// add by pingli at 2012-06-27
			if (mthres_ > 0 && mthres_ < msec) {
				char wmsg[LOG_MAXMSGLEN];
				wmsg[0] = 0;
#ifdef _WINDOWS
				sprintf_s(wmsg, LOG_MAXMSGLEN, "%s %s %.03f msec.", name_, msg_, msec);
#else
				sprintf(wmsg, "%s %s %.03f msec.", name_, msg_, msec);
#endif
				
				LOG::instance()->log_msg_i(log_lvl_, wmsg);
			}

			int pth = LOG::instance()->config().perf_thres();
			if ( pth > 0 && pth < msec )
				LOG::instance()->log_perf("%s %s %.03f msec.", name_, msg_, msec);
			}
		if ( ext_nsec_ )
			*ext_nsec_ = nanosec_;
		}
	const T & stop(void)
		{
		if ( stopped_ )
			return nanosec_;
		timer_.stop();
		timer_.elapsed_time(nanosec_);
		stopped_ = true;
		return nanosec_;
		}

	const char * msg(void) const
		{ return msg_; }
	const T & nanosec(void) const
		{ return nanosec_; }
	TIMER & timer(void)
		{ return timer_; }

	void log(const char* fmt, ...)
		{
		if ( LOG::instance() == 0 )
			return ;
		msg_[0] = 0;
		if ( LOG::instance()->config().output() == 0 )
			return ;
		if ( LOG::instance()->config().level() & log_lvl_ )
			{
			va_list va;
			va_start(va, fmt);
			#if _MSC_VER >= 1400
			vsprintf_s(msg_, sizeof msg_, fmt, va);
			#else
			vsnprintf(msg_, sizeof(msg_), fmt, va);
			#endif
			va_end(va);
			}
		}

public:
	TIMER			timer_;
	char			name_[60];
	char			msg_ [10*1024];
	T 				nanosec_;
	T *				ext_nsec_;
	bool			stopped_;

	// add by pingli at 2012-06-27
	LOG_LEVEL		log_lvl_;
	int				mthres_;
};
// ===============================================
// 提供了一个简单的计时类
// Windows系统下实现了精确的计时功能
// ===============================================
class Log_Timer
{
public:
	void start(void)
		{
		#if defined(WIN32)
			QueryPerformanceCounter(&timestart_);
		#else
			gettimeofday(&timestart_, 0);
		#endif
		}
	double stop (bool restart = false)
		{
		elapsed_time_ = 0;
		#ifdef WIN32
			LARGE_INTEGER cur;
			if ( QueryPerformanceCounter(&cur) )
				{
				elapsed_time_ = (((cur.QuadPart - timestart_.QuadPart) * 1e9 * 1.) / frequency_.QuadPart);
				if ( restart )
					timestart_.QuadPart = cur.QuadPart;
				}
			return elapsed_time_;
		#else
			struct timeval	cur;
			gettimeofday(&cur, 0);
			elapsed_time_ = ( cur.tv_sec - timestart_.tv_sec ) * 1e9
				+ ( cur.tv_usec - timestart_.tv_usec ) * 1e3;
			return elapsed_time_;
		#endif
		}
	// 单位纳秒
	double elapsed_time(void)
		{ return elapsed_time_; }
	double elapsed_time(double & et)
		{ return et = elapsed_time_; }

	Log_Timer(void)
		: elapsed_time_(0.)
		{
		#ifdef WIN32
		QueryPerformanceFrequency(&frequency_);
		#endif
		start();
		}
	~Log_Timer(void)
		{ }

private:
	// 高分辨率的计数
	#ifdef WIN32
	LARGE_INTEGER	frequency_;
	LARGE_INTEGER	timestart_;
	#else // linux
	struct timeval	timestart_;		
	#endif /* WIN32 */

	// 内部计数
	double elapsed_time_;
};

// ===============================================
// 提供了一个简单的函数调用跟踪类
// 利用构造函数和析构函数的特点
// ===============================================
template<class LOGGER>
class Log_Func_Tracer
{
public:
	Log_Func_Tracer (const char * sub)
		{
		if ( sub )
			sub_ = sub;

		if ( LOGGER::instance() )
			LOGGER::instance()->log_trace("%s | enter.", sub_.c_str());
		}
	Log_Func_Tracer(const char * sub, const char * fmt, ...)
		{
		if ( sub )
			sub_ = sub;

		if ( LOGGER::instance() )
			{
			char msg[1024];
			va_list va;
			va_start(va, fmt);
			#if _MSC_VER >= 1400
			vsprintf_s(msg, sizeof msg, fmt, va);
			#else
			vsnprintf(msg, sizeof(msg), fmt, va);
			#endif
			LOGGER::instance()->log_trace("%s | enter, %s", sub_.c_str(), msg);
			va_end(va);
			}
		}
	~Log_Func_Tracer(void)
		{
		if ( LOGGER::instance() )
			LOGGER::instance()->log_trace("%s | leave.", sub_.c_str());
		}
private:
	// 主题
	std::string sub_;
};

// ===============================================
// 提供一个安全的初始化类
// 利用析构函数的特点在析构函数里面调用逆初始化
// ===============================================
template <class T>
class Log_Auto_Close
{
public:
	Log_Auto_Close(void)
		{ }
	~Log_Auto_Close(void)
		{ T::close(); }
};

/** 
 * @class	Log_Singleton
 * @brief	日志类声明
 * @author	
 * @date	2004年5月21日
 * @see		
 * @par 备注：用户可以从这个类派生，重新实现GetSubjuct()函数，就能够实现主题功能
 */
template <typename IO = Log_IO_FILE, typename LOCK = Log_Mutex_Null, typename CFG = Log_Cfg_T<Log_Cfg_Heap_Reg> >
class Log_Singleton
	: public Base_Singleton_T< Log_Impl_T<IO, LOCK, CFG> >
{
public:
	// 基类的别名
	typedef Base_Singleton_T< Log_Impl_T<IO, LOCK, CFG> > the_parent;

	/** 
	 * @brief 	Log_Singleton::open
	 *  
	 *  用户最常用的初始化日志方法
	 *  从配置文件里面读取配置信息的初始化函数
	 *  
	 * @author	
	 * @date	2006-2-14
	 * @return	static int	- Return 0 in success, otherwise return error code.
	 * @param	const char * cfg_file	- [in] 指定的配置文件或注册表位置 
	 * @param	const char * key = 0	- [in] 日志使用的项名称，缺省为logger
	 * @param	const CFG  * dcfg = 0	- [in] 缺省配置信息
	 * @param	LOG_HANDLE hmod = -1	- [in] 当前模块的句柄，exe为0，dll需要传递，用于获取该模块的路径信息
	 * @param	Log_IO * io = 0			- [in] 传入的IO接口
	 * @param	void * rev = 0			- [in] 保留，用于传递给IO接口的初始化
	 * @see		
	 */
	static int open(const CFG & cfg, const char * cfg_file = 0, const char * key = 0);
	static int open(const char * cfg_file, const char * key = 0, const CFG * dcfg = 0, LOG_HANDLE hmod = LOG_INVALID_HANDLE, Log_IO * io = 0, void * rev = 0);

	/** 
	 * @brief 	Log_Singleton::open
	 *  
	 *  简化的日志初始化方法
	 *  读取配置信息并初始化，用户自己填充配置信息的初始化函数
	 *  如果用户传入了io_impl接口，说明用户自己想利用已有的io接口，该io接口可以携带锁和配置信息
	 *  
	 * @author	
	 * @date	2006-2-14
	 * @return	static int	- Return 0 in success, otherwise return error code.
	 * @param	const CFG & cfg			- [in] 日志配置
	 * @param	Log_IO * io_impl		- [in] 使用的io接口实现，如果传零，则在内部创建一个io实现
	 * @param	void * rev = 0			- [in] 保留，用于传递给IO接口的初始化
	 * @see		
	 */
	static int open(const CFG & cfg,  Log_IO * io_impl, void * rev = 0);
	static int open(Log_IO * io_impl, LOG_HANDLE mod = LOG_INVALID_HANDLE, void * rev = 0);

	/** 
	 * @brief 	open
	 *  
	 *  从一个IO实现库初始化一个全局的日志单体类
	 *  这个函数用于如果用户自己实现了一个IO接口，并置于一个dll中的情况
	 *  
	 * @author	
	 * @date	2006-2-14
	 * @return	static int	- Return 0 in success, otherwise return error code.
	 * @param	const char * io_lib		- [in] 用户指定Log_IO接口实现库，这个dll必须导出base_create_logio和base_destroy_logio接口
	 * @param	const char * cfg_file	- [in] 请参考上面的函数
	 * @see		Log_IO
	 */
	static int open(const char * io_lib, const char * cfg_file, const char * key, const CFG * cfg, LOG_HANDLE hmod, void * rev);

	/** 
	 * @brief 	close
	 *  
	 *  大家共用的逆初始化函数，进程结束时别忘了调用
	 *  
	 * @author	
	 * @date	2006-2-14
	 * @return	static int	- Return 0 in success, otherwise return error code.
	 * @see		
	 */
	static int close(void);
};

// =============================================================================
// 用来生成唯一类型日志单体的宏
// 希望用户使用这个宏达到简单实用的目的
// =============================================================================
#define LOG_DECL_SINGLETON(io, lock, cfg, name)					\
	LOG_UT_DECL(io, name##io);									\
	typedef cfg name##_Cfg;										\
	typedef Log_Singleton<name##io, lock, name##_Cfg> name;

#define LOG_DECL_SINGLETON_EX(io, lock, cfg, name)				\
	LOG_DECL_SINGLETON(io, lock, Log_Cfg_T<cfg>, name)

#define LOG_DECL_SIMPLE(io, lock, name)							\
	LOG_DECL_SINGLETON_EX(io, lock, Log_Cfg_Heap_Reg, name)

// =============================================================================
// 这里定义了一个使用文件，使用临界区锁定的日志类
//  typedef Log_Singleton<> BASE_Logger;
//  typedef Log_Cfg_T<Log_Cfg_Heap_Reg> BASE_Logger_Cfg;
// 以上的声明方法可以采用下面的宏代替
// 如果用户需要使用进程锁，请在外部定义BASE_LOG_PROCESS预编译宏
// =============================================================================
#ifdef BASE_LOG_PROCESS
LOG_DECL_SIMPLE(Log_IO_FILE, Log_Win32_Process_Mutex, BASE_Logger)
# pragma message("BASE_Log::compile with process mutex.")
#else  // default to use thread mutex
LOG_DECL_SIMPLE(Log_IO_FILE, Log_Thread_Mutex, BASE_Logger)
#endif // Log mutex

/* enable __VA_ARGS__? */
#if __STDC_VERSION__ >= 199901L || _MSC_VER >= 1400
# ifndef BASE_LOG_NOVAGS
#  define VA_ARGS_ENABLE		1
# endif /* BASE_LOG_NOVAGS */
#else	/* lower C99 standard */
#endif  /* __VA_ARGS__ */

/* 如果定义了这个宏，那么所有的日志都禁用 */
#ifndef BASE_LOG_DISABLE

/**
 * @brief	方便使用的宏
 *	在使用单实例模式的时候，这几个宏可以更简洁的使用日志功能
 *  注意：BASE_LOG_DISABLE宏可以禁用所有的日志功能
 */
#  define BASE_LOGINST()				BASE_Logger::instance()
#  define BASE_LOGFLUSH					if ( BASE_LOGINST() ) BASE_LOGINST()->flush
#  define BASE_LOGCRIT					if ( BASE_LOGINST() ) BASE_LOGINST()->log_crit
#  define BASE_LOGERR					if ( BASE_LOGINST() ) BASE_LOGINST()->log_error
#  define BASE_LOGWARN					if ( BASE_LOGINST() ) BASE_LOGINST()->log_warn
#  define BASE_LOGINFO					if ( BASE_LOGINST() ) BASE_LOGINST()->log_info
#  define BASE_LOGVERBOSE				if ( BASE_LOGINST() ) BASE_LOGINST()->log_verbose
#  define BASE_LOGDBG					if ( BASE_LOGINST() ) BASE_LOGINST()->log_debug
#  define BASE_LOGPERF					if ( BASE_LOGINST() ) BASE_LOGINST()->log_perf
#  define BASE_LOGTRACE					if ( BASE_LOGINST() ) BASE_LOGINST()->log_trace
#  define BASE_LOGWMSG					if ( BASE_LOGINST() ) BASE_LOGINST()->write_msg
   /* 支持主题功能的日志函数 */
#  define BASE_LOGCRITS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_crits
#  define BASE_LOGERRS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_errors
#  define BASE_LOGWARNS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_warns
#  define BASE_LOGINFOS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_infos
#  define BASE_LOGVBSS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_verboses
#  define BASE_LOGDBGS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_debugs
#  define BASE_LOGPERFS					if ( BASE_LOGINST() ) BASE_LOGINST()->log_perfs
#  define BASE_LOGTRACES				if ( BASE_LOGINST() ) BASE_LOGINST()->log_traces

// 能够自动统计性能并打印的，分别为仅打印主题、携带变参
# define BASE_LOGPERF_N(name)			Log_Perf_Helper<Log_Timer, BASE_Logger>		__ph__(#name);
# define BASE_LOGPERF_S(name,fmt)		Log_Perf_Helper<Log_Timer, BASE_Logger>		__ph__(#name); __ph__.log fmt;
# define BASE_LOGPERF_NSEC(name, nsec)	Log_Perf_Helper<Log_Timer, BASE_Logger>		__ph__(#name, nsec);
# define BASE_LOGPERF_STOP()														__ph__.stop()

// 跟踪函数调用的方法
# define BASE_TRACER					Log_Func_Tracer<BASE_Logger>				__ilt__
# ifdef VA_ARGS_ENABLE
#  define BASE_TRACERS(sub, fmt, ...)	Log_Func_Tracer<BASE_Logger>				__ilt__(sub, fmt, ##__VA_ARGS__);
# endif /* VA_ARGS_ENABLE */

// 初始化和逆初始化
# define BASELOG_OPEN					BASE_Logger::open
# define BASELOG_CLOSE					BASE_Logger::close
# define BASELOG_OPEN_AC				Log_Auto_Close<BASE_Logger> __lac__; BASE_Logger::open

#else  /* BASE_LOG_DISABLE */

# ifdef VA_ARGS_ENABLE
#  define BASE_LOGINST()				BASE_Logger::instance()
#  define BASE_LOGCRIT(...)
#  define BASE_LOGERR(...)
#  define BASE_LOGWARN(...)
#  define BASE_LOGINFO(...)
#  define BASE_LOGVERBOSE(...)
#  define BASE_LOGDBG(...)
#  define BASE_LOGPERF(...)
#  define BASE_LOGTRACE(...)
#  define BASE_LOGWMSG(...)
#  define BASE_LOGFLUSH()
#  define BASE_LOGPERF_N(name)
#  define BASE_LOGPERF_S(name,fmt)
#  define BASE_LOGPERF_NSEC(name, nsec)
#  define BASE_LOGPERF_STOP()
#  define BASE_TRACER(...)
#  define BASE_TRACERS(...)
#  define BASELOG_OPEN(...)
#  define BASELOG_CLOSE(...)
#  define BASELOG_OPEN_AC(...)
# else
#  pragma(message, "This compiler not support VA_ARGS! CANNOT disable the log macros.")
#  define BASE_LOGINST()				BASE_Logger::instance()
#  define BASE_LOGCRIT					__log_fake__
#  define BASE_LOGERR					__log_fake__
#  define BASE_LOGWARN					__log_fake__
#  define BASE_LOGINFO					__log_fake__
#  define BASE_LOGVERBOSE				__log_fake__
#  define BASE_LOGDBG					__log_fake__
#  define BASE_LOGPERF					__log_fake__
#  define BASE_LOGTRACE					__log_fake__
#  define BASE_LOGWMSG					__log_fake__
#  define BASE_LOGFLUSH					__log_fake__
#  define BASE_LOGPERF_N				__log_fake__
#  define BASE_LOGPERF_S				__log_fake__
#  define BASE_LOGPERF_NSEC				__log_fake__
#  define BASE_LOGPERF_STOP				__log_fake__
#  define BASE_TRACER					__log_fake__
#  define BASE_TRACERS					__log_fake__
#  define BASELOG_OPEN					__log_fake__
#  define BASELOG_CLOSE					__log_fake__
#  define BASELOG_OPEN_AC				__log_fake__
   inline void __log_fake__(...)		{ ; }
# endif /* C99 */

#endif /* BASE_LOG_DISABLE */

// 方便的宏组合
#define BASELOG_ERROR_ASSERT(exp, msg)			\
				if (!(exp)) { BASE_LOGERR msg; SP_ASSERTS(exp, msg); }
#define BASELOG_ASSERT_RETURN(exp, msg, ret)	\
				if (!(exp)) { BASE_LOGERR msg; SP_ASSERTS(exp, msg); return ret; }
#define BASELOG_ERROR_RETURN(exp, msg, ret)		\
				if (!(exp)) { BASE_LOGERR msg; return ret; }
#define BASELOG_FUNC_INFO(ret, succ, fail)		\
				if ( ret != 0 ) { BASE_LOGERR msg; }  else { BASE_LOGINFO msg; }
#define BASELOG_FUNC_VBS(ret, succ, fail)		\
				if ( ret == 0 ) { BASE_LOGVBS succ;	} else { BASE_LOGERR fail; }

// 定义外部实现的函数
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 创建日志实现的函数
const char fn_base_logio_create[] = "babse_logio_create";
int base_logio_create(Log_IO** io, void* res);
typedef int (* proc_base_logio_create)(Log_IO ** io, void * res);

// 销毁日志实现的函数
const char fn_base_logio_destroy[] = "base_logio_destroy";
int base_logio_destroy(Log_IO** io, void* res);
typedef int (* proc_base_logio_destroy)(Log_IO * io);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

// 一堆的实现代码放在了这个文件里面，基本上不需要看
#include "log_base.cpp"

#endif /* __BASELOGGER_H__ */
