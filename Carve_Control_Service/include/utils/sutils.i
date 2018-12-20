/** 
 * @file	sputils.i
 * @brief	sputils��ʵ��
 * 
 *  ����ļ���������Щ����Ҫ����̫��ͷ�ļ��Ĳ���
 *  ���磺�ַ���������ͨ�ò���
 * 
 * @author	
 * @version	1.0
 * @date	2007-4-23
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>2007-4-23	<td>	<td>Create this file</tr>
 * </table>
 * 
 */
#ifndef __SPUTILS_I__
#define __SPUTILS_I__

#include "sutils.h"
#include <time.h>
#include <errno.h>
#include <fstream>
#include <sstream>

#ifdef WIN32
# if !defined (_WIN32_WINNT)
#  define _WIN32_WINNT 0x0400
# endif // _WIN32_WINNT
# include <WinSock2.h>
# include <Windows.h>
# include <io.h>
# include <stdlib.h>
#else  // Linux
# include <sys/time.h>
# include <sys/types.h>
# include <dirent.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <string.h>
# include <stdlib.h>
#endif // OS

// SP_INLINE
#ifdef __cplusplus
# define SP_INLINE inline
#else	/* C */
# define SP_INLINE
#endif /* __cplusplus */

// assert
#ifndef IU_ASSERT
# ifdef SP_ASSERT
#  define IU_ASSERT		SP_ASSERT
# elif defined(ASSERT)
#  define IU_ASSERT		ASSERT
# else // null
#  define IU_ASSERT(x)
# endif
#endif /* IU_ASSERT */

#ifdef ACE_WIN32
#include "ace/High_Res_Timer.h"
#endif // ACE_WIN32

/*=============================================================================
 * �ַ�������
 *=============================================================================*/
namespace sp
{

SP_INLINE
bool is_quanjiao(const char* pstr)
{
	IU_ASSERT(pstr);
	if ( pstr == 0 )
		return false;
	if ( *pstr == 0 || *(pstr+1) == 0 )
		return false;
	return ((unsigned char)(*pstr) & 0x80) && ((unsigned char)(*pstr) != 0xFF);
}

SP_INLINE
int trim_str(char* pstr, const char trim_char /* = ' ' */, bool trim_gb /* = false */)
{
	const char* p = pstr;
	if ( 0 == p )
		return 0;

	// Get start and end position
	int start = 0, end = 0;
	while ( *p )
	{
		// ȫ��
		if ( trim_gb && is_quanjiao(p) )
		{
			if ( *((unsigned short *)p) == 0xa1a1 )
			{
				if ( end == 0 )
					start += 2;
			}
			else
			{
				end = (int) (p - pstr + 2);
			}
			p += 2;
			continue;
		}
		// ���
		if ( ( (unsigned char)*p < 0x20 || trim_char == *p ) )
		{
			if ( end == 0 )
				start ++;
		}
		else
			end = (int) (p - pstr + 1);
		p++;
	}

	// trim it
	end > 0 ? pstr[end] = 0 : end = (int) (p - pstr);
	if ( end == start )
		pstr[0] = 0;
	else if ( start > 0 )
		memmove(pstr, pstr+start, end - start + sizeof(char));

	return end - start;
}

SP_INLINE
int trim_str(std::string & str, const char trim_char /* = ' ' */, bool trim_gb /* = false */)
{
	const char* pstr = str.c_str();
	const char* p = pstr;

	// Get start and end position
	int start = 0, end = 0;
	while ( *p )
	{
		// ȫ��
		if ( trim_gb && is_quanjiao(p) )
		{
			if ( *((unsigned short *)p) == 0xa1a1 )
			{
				if ( end == 0 )
					start += 2;
			}
			else
			{
				end = (int) (p - str.c_str() + 2);
			}
			p += 2;
			continue;
		}
		// ���
		if ( ( (unsigned char)*p < 0x20 || trim_char == *p ) )
		{
			if ( end == 0 )
				start ++;
		}
		else
			end = (int) (p - str.c_str() + 1);
		p++;
	}

	// trim it
	if ( end > 0 )
		str.resize(end);
	else
		end = (int) (p - pstr);

	if ( end == start )
		str.clear();
	else if ( start > 0 )
		str = str.substr(start);

	return end - start;
}

SP_INLINE
int trim_str(wchar_t * pstr, const wchar_t trim_char /* = L' ' */)
{
	const wchar_t* p = pstr;
	if ( 0 == p )
		return 0;

	// Get start and end position
	int start = 0, end = 0;
	while ( *p )
	{
		// ���
		if ( ( (unsigned short)*p < 0x20 || trim_char == *p ) )
		{
			if ( end == 0 )
				start ++;
		}
		else
			end = (int) (p - pstr + 1);
		p++;
	}

	// trim it
	end > 0 ? pstr[end] = 0 : end = (int) (p - pstr);
	if ( end == start )
		pstr[0] = 0;
	else if ( start > 0 )
		memmove(pstr, pstr+start, sizeof(wchar_t) * (end - start + sizeof(wchar_t)) );

	return end - start;
}

SP_INLINE
int trim_str(std::wstring & str, const wchar_t trim_char /* = L' ' */)
{
	const wchar_t* pstr = str.c_str();
	const wchar_t* p = pstr;

	// Get start and end position
	int start = 0, end = 0;
	while ( *p )
	{
		// ���
		if ( ( (unsigned short)*p < 0x20 || trim_char == *p ) )
		{
			if ( end == 0 )
				start ++;
		}
		else
			end = (int) (p - str.c_str() + 1);
		p++;
	}

	// trim it
	if ( end > 0 )
		str.resize(end);
	else
		end = (int) (p - pstr);

	if ( end == start )
		str.clear();
	else if ( start > 0 )
		str = str.substr(start);

	return end - start;
}

SP_INLINE
int trim_str(char * pstr, const char * strim, bool trim_gb /* = false */)
{
	int ret = 0;
	if ( strim == 0 )
		return trim_str(pstr, ' ', trim_gb);

	const char * p = strim;
	while ( *p )
		ret = trim_str(pstr, *p++, trim_gb);
	return ret;
}

SP_INLINE
int trim_str(std::string & str, const char * strim, bool trim_gb /* = false */)
{
	int ret = 0;
	if ( strim == 0 )
		return trim_str(str, ' ', trim_gb);

	const char * p = strim;
	while ( *p )
		ret = trim_str(str, *p++, trim_gb);
	return ret;
}

SP_INLINE
int trim_str(wchar_t * pstr, const wchar_t * strim)
{
	int ret = 0;
	if ( strim == 0 )
		return trim_str(pstr, L' ');

	const wchar_t * p = strim;
	while ( *p )
		ret = trim_str(pstr, *p++);
	return ret;
}

SP_INLINE
int trim_str(std::wstring & str, const wchar_t * strim)
{
	if ( strim == 0 )
		return trim_str(str, L' ');

	const wchar_t * p = strim;
	while ( *p )
		trim_str(str, *p++);
	
	return 0;
}

SP_INLINE
int str2ver(const char * str, int & ver, int & build)
{
	build = 0;
	ver   = 0;

	str_arr sa;
	split_str(str, sa, ".");
	int maj(0), mnr(0), rev(0);
	if ( sa.size() > 3 )
		build = strtoul(sa[3].c_str(), 0, 0);
	if ( sa.size() > 2 )
		rev   = strtoul(sa[2].c_str(), 0, 0);
	if ( sa.size() > 1 )
		mnr   = strtoul(sa[1].c_str(), 0, 0);
	if ( sa.size() > 0 )
		maj   = strtoul(sa[0].c_str(), 0, 0);

	ver = SP_VERMAKE(maj, mnr, rev);
	return 0;
}

SP_INLINE
std::string ver2str(int ver, int build)
{
	int maj = SP_VERMAJOR(ver);
	int mnr = SP_VERMINOR(ver);
	int rev = SP_VERREVISION(ver);

	char vers[64];
	sprintf(vers, "%d.%d.%d.%d", maj, mnr, rev, build);
	return vers;
}

template <typename char_t>
SP_INLINE
int search_strarr(const char_t *str, const char_t *arr[], int arr_size, bool case_sensitive /* = true */)
{
	int which = 0;
	for ( ; which < arr_size; ++which )
	{
		if ( case_sensitive )
		{
			if ( sp::strcmp(str, arr[which]) == 0 )
				break;
		}
		else
		{
			if ( sp::strcasecmp(str, arr[which]) == 0 )
				break;
		}
	}

	return which < arr_size ? which : -1;
}

template <typename char_t, int row, int col>
SP_INLINE
int search_strarr(const char_t * str, const char_t (&arr)[row][col], bool case_sensitive)
{
	int which = 0;
	for ( ; which < row; ++which )
	{
		if ( case_sensitive )
		{
			if ( sp::strcmp(str, arr[which]) == 0 )
				break;
		}
		else
		{
			if ( sp::strcasecmp(str, arr[which]) == 0 )
				break;
		}
	}

	return which < row ? which : -1;
}

template <typename char_t, template <typename T, typename A> class cont_t>
SP_INLINE
int search_strarr(const char_t * str, cont_t<std::basic_string<char_t>, std::allocator<std::basic_string<char_t> > > & cont, bool case_sensitive /* = true */)
{
	typename cont_t<std::basic_string<char_t>, std::allocator<std::basic_string<char_t> > >::iterator pi = cont.begin();
	for ( int ind = 0; pi != cont.end(); pi++, ind++ )
	{
		if ( case_sensitive && sp::strcmp(str, (*pi).c_str()) == 0 )
			return ind;
		if ( !case_sensitive && sp::strcasecmp(str, (*pi).c_str()) == 0 )
			return ind;
	}
	return -1;
}

template <class T>
SP_INLINE
int str_to_time(const char* strtm, T & hour, T & min, T & sec)
{
	IU_ASSERT(strtm);
	hour = min = sec = 0;

	std::vector<std::string> subs;
	split_str(strtm, subs, "/:-");

	if ( subs.size() > 0 )
		hour = (T)strtol(subs[0].c_str(), 0, 10);
	if ( subs.size() > 1 )
		min  = (T)strtol(subs[1].c_str(), 0, 10);
	if ( subs.size() > 2 )
		sec  = (T)strtol(subs[2].c_str(), 0, 10);
	return 0;
}

template <class T>
SP_INLINE
int str_to_time(const char * strtm, T & year, T & mon, T & day, T & hour, T & min, T & sec)
{
	IU_ASSERT(strtm);
	year = mon = day = hour = min = sec = 0;

	std::vector<std::string> subs;
	split_str(strtm, subs, "/:- ");

	if ( subs.size() > 0 )
		year = (T)strtol(subs[0].c_str(), 0, 10);
	if ( subs.size() > 1 )
		mon  = (T)strtol(subs[1].c_str(), 0, 10);
	if ( subs.size() > 2 )
		day  = (T)strtol(subs[2].c_str(), 0, 10);
	if ( subs.size() > 3 )
		hour = (T)strtol(subs[3].c_str(), 0, 10);
	if ( subs.size() > 4 )
		min  = (T)strtol(subs[4].c_str(), 0, 10);
	if ( subs.size() > 5 )
		sec  = (T)strtol(subs[5].c_str(), 0, 10);
	return 0;
}

SP_INLINE
int str_to_time(const char * strtm, time_t & tmv)
{
	unsigned short year(0), mon(0), day(0), hour(0), mnt(0), sec(0);
	str_to_time(strtm, year, mon, day, hour, mnt, sec);

	struct tm ts;
	memset(&ts, 0, sizeof ts);

	ts.tm_year	= get_val_in_range<unsigned short>(year - 1900, 0, 3000);
	ts.tm_mon	= get_val_in_range<unsigned short>(mon - 1, 0, 11);
	ts.tm_hour	= get_val_in_range<unsigned short>(hour, 0, 24);
	ts.tm_min	= get_val_in_range<unsigned short>(mnt,  0, 60);
	ts.tm_sec	= get_val_in_range<unsigned short>(sec,  0, 60);

 	tmv = mktime(&ts);
	return 0;
}

template <class T>
SP_INLINE
int time_to_ymdhms(time_t tmv, T & year, T & mon, T & day, T & hour, T & mnt, T & sec)
{
#ifdef WIN32
	struct tm * ptm = ::localtime(&tmv);
#else
	struct tm result;
	struct tm * ptm = ::localtime_r(&tmv, &result);
#endif
	if ( ptm == 0 )
		return -1;

	year = (T)ptm->tm_year + 1900;
	mon  = (T)ptm->tm_mon + 1;
	day  = (T)ptm->tm_mday;
	hour = (T)ptm->tm_hour;
	mnt  = (T)ptm->tm_min;
	sec	 = (T)ptm->tm_sec;
	return 0;
}

SP_INLINE
int time_to_str(time_t tmv, std::string & year, std::string & mon, std::string & day, std::string & hour, std::string & mnt, std::string & sec)
{
#ifdef WIN32
			struct tm * ptm = ::localtime(&tmv);
#else
			struct tm result;
			struct tm * ptm = ::localtime_r(&tmv, &result);
#endif

	//struct tm * ptm = ::localtime(&tmv);
	if ( ptm == 0 )
		return -1;

	char buf[60];
	sprintf(buf, "%d", ptm->tm_year + 1900);
	year = buf;
	sprintf(buf, "%d", ptm->tm_mon + 1);
	mon  = buf;
	sprintf(buf, "%d", ptm->tm_mday);
	day  = buf;
	sprintf(buf, "%d", ptm->tm_hour);
	hour = buf;
	sprintf(buf, "%d", ptm->tm_min);
	mnt  = buf;
	sprintf(buf, "%d", ptm->tm_sec);
	sec	 = buf;

	return 0;
}

SP_INLINE
const char * month_str(int mon)
{
	const char * month_table[] =
		{
		"", "Jan", "Feb", "Mar", "April", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"
		};
	if ( mon < 1 || mon > 12 )
		return 0;
	return month_table[mon];
}

SP_INLINE
std::string sec_to_dhms(double sec)
{
	const int sec_per_min = 60;
	const int sec_per_hour = 60 * 60;
	const int sec_per_day = sec_per_hour * 24;

	std::string str;
	int min = 0, hour = 0, day = 0, secs = 0;
	double seconds = sec + 1e-5;
	if ( seconds < 1e-5 )
		return str;

	if ( seconds > sec_per_day )
	{
		day = (int) (seconds / sec_per_day);
		seconds -= day * sec_per_day;
	}

	if ( seconds > sec_per_hour )
	{
		hour = (int) (seconds / sec_per_hour);
		seconds -= hour * sec_per_hour;
	}

	if ( seconds > sec_per_min )
	{
		min = (int) (seconds / sec_per_min);
		seconds -= min * sec_per_min;
	}

	secs = (int)(seconds + 0.5);

	if ( day > 1 )
		str += itostr(day, "%d days ");
	else if ( day > 0 )
		str += "1 day ";
	if ( hour > 1 )
		str += itostr(hour, "%d hours ");
	else if ( day > 0 || hour > 0 )
		str += itostr(hour, "%d hour ");
	if ( min > 1 )
		str += itostr(min, "%d mins ");
	else if ( day > 0 || hour > 0 || min > 0 )
		str += itostr(min, "%d min ");
	if ( secs > 1 )
		str += itostr((int)seconds, "%d secs");
	else
		str += ftostr(seconds, "%d sec");

	return str;
}

SP_INLINE
char * strsncpy (char *dst, const char *src, size_t maxcnt)
{
	register char *rdst = dst;
	register const char *rsrc = src;
	register size_t rmaxlen = maxcnt;

	if ( rmaxlen > 0 )
	{
		if ( rdst != rsrc )
		{
			*rdst = '\0';
			if ( rsrc != 0 )
				strncat (rdst, rsrc, --rmaxlen);
		}
		else
		{
			rdst += (rmaxlen - 1);
			*rdst = '\0';
		}
	}
	return dst;
}

SP_INLINE
wchar_t * strsncpy(wchar_t * dst, const wchar_t * src, size_t maxcnt)
{
	register wchar_t *rdst = dst;
	register const wchar_t *rsrc = src;
	register size_t rmaxlen = maxcnt;

	if ( rmaxlen > 0 )
	{
		if ( rdst != rsrc )
		{
			*rdst = L'\0';
			if ( rsrc != 0 )
				wcsncat (rdst, rsrc, --rmaxlen);
		}
		else
		{
			rdst += (rmaxlen - 1);
			*rdst = L'\0';
		}
	}
	return dst;
}

SP_INLINE
bool is_quote(char c)
{
	return c == '\'' || c == '\"';
}

SP_INLINE
bool is_quote(wchar_t c)
{
	return c == L'\'' || c == L'\"';
}


SP_INLINE
int split_str(const char* str, str_arr & subs_array, const char spliter[] /* = ",;" */, bool trim /* = true */,
				  bool ignore_blank /* = false */, bool supp_quote /* = false */, bool trim_quote /* = true */, bool supp_quanjiao /* = false */, char * longer_subs /* = NULL */, int longer_len /* = 0*/)
{
	char * subs = NULL;
	char subs_tmp[400];
	bool quote_start = false;
	char last_quote = 0;
	bool left_quote = false, right_quote = false;
	int  step = 1;
	int  subs_len = 0;

	if ( str[0] == 0 )
		return 0;

	subs = subs_tmp;
	subs_len = sizeof subs_tmp;
	if (longer_subs && longer_len)
	{
		subs = longer_subs;
		subs_len = longer_len;
	}

	for ( int i = 0, cursor = 0; ; i += step )
	{
		const char & c = str[i];

		// ֧��ȫ����
		step = 1;
		if ( supp_quanjiao && is_quanjiao(str + i) )
		{
			step = 2;
			continue ;
		}
		
		// �������
		if ( supp_quote )
		{
			if ( is_quote(c) )
			{
				if ( quote_start == false )
				{
					quote_start = true;
					left_quote  = true;
					last_quote  = c;
					continue;
				}
				else if ( c == last_quote )
				{
					quote_start = false;
					right_quote = true;
					last_quote  = 0;
				}
			}
		}

		// �ֶμ��
		if ( c == 0 ||
			( !quote_start && strchr(spliter, c) )
			)
		{
			subs[0] = 0;
			#if defined(WIN32) && _MSC_VER < 1300 // VC++ 6.0
			int len = min((i - cursor), subs_len - 1); // min((i - cursor), (sizeof subs) - 1);
			#else
			int len = std::min<int>(i - cursor, subs_len - 1); // std::min<int>(i - cursor, sizeof subs - 1);
			#endif
			if ( len > 0 )
			{
				if ( !supp_quote )
					strsncpy(subs, str + cursor, len + 1);
				else
				{
					if ( trim_quote && is_quote(str[cursor]) )
					{
						left_quote = true;
						if ( str[i-1] == str[cursor] )
							right_quote = true;

						strsncpy(subs, str + cursor + left_quote, len + 1 - left_quote - right_quote);
					}
					else
						strsncpy(subs, str + cursor, len + 1);

					left_quote = right_quote = false;
				}
			}
			cursor = i + 1;

			// trim it
			if ( trim || ignore_blank )
				trim_str(subs, ' ', supp_quanjiao);

			if ( !ignore_blank || subs[0] != 0 )
				subs_array.push_back(subs);
		}

		// end loop
		if ( str[i] == 0 )
			break;
	}

	return 0;
}
#ifndef __IPHONE_7_0
SP_INLINE
int split_str(const wchar_t * str, wstr_arr & subs_array, const wchar_t spliter[] /* = L",;" */, bool trim /* = true */,
				  bool ignore_blank /* = false */, bool supp_quote /* = false */, bool trim_quote /* = true */, wchar_t * longer_subs /* = NULL */, int longer_len /* = 0 */)
{
	wchar_t * subs;
	wchar_t subs_tmp[400];
	bool	quote_start = false;
	wchar_t	last_quote = 0;
	bool	left_quote = false, right_quote = false;
	size_t subs_len;

	if ( str[0] == 0 )
		return 0;

	subs = subs_tmp;
	subs_len = sizeof subs_tmp;
	if (longer_subs && longer_len)
	{
		subs = longer_subs;
		subs_len = longer_len;
	}

	for ( int i = 0, cursor = 0; ; i++ )
	{
		const wchar_t & c = str[i];

		// �������
		if ( supp_quote )
		{
			if ( c == L'\'' || c == L'\"' )
			{
				if ( quote_start == false )
				{
					quote_start = true;
					left_quote = true;
					last_quote  = c;
					continue;
				}
				else if ( c == last_quote )
				{
					quote_start = false;
					right_quote = true;
					last_quote  = 0;
				}
			}
		}

		// �ֶμ��
		if ( c == 0
			|| ( !quote_start && wcschr(spliter, c) )
			)
		{
			subs[0] = 0;
			#if defined(WIN32) && _MSC_VER < 1300 // VC++ 6.0
			size_t len = min((i - cursor), subs_len - 1);
			#else
			size_t len = std::min<int>(i - cursor, subs_len - 1);
			#endif
			if ( len > 0 )
			{
				if ( !supp_quote )
					strsncpy(subs, str + cursor, len + 1);
				else
				{
					if ( trim_quote && is_quote(str[cursor]) )
					{
						left_quote = true;
						if ( str[i-1] == str[cursor] )
							right_quote = true;

						strsncpy(subs, str + cursor + left_quote, len + 1 - left_quote - right_quote);
					}
					else
						strsncpy(subs, str + cursor, len + 1);
					left_quote = right_quote = false;
				}
			}
			cursor = i + 1;

			// trim it
			if ( trim || ignore_blank )
				trim_str(subs, L' ');

			if ( !ignore_blank || subs[0] != 0 )
				subs_array.push_back(subs);
		}

		// end loop
		if ( str[i] == 0 )
			break;
	}

	return 0;
}
#endif
    
#ifndef __IPHONE_7_0
SP_INLINE
int split_str(const char * str, wstr_arr & subs_array, const wchar_t spliter[] /* = L",;" */, bool trim /* = true */,
		bool ignore_blank /* = false */, bool supp_quote /* = false */, bool trim_quote /* = true */, wchar_t * longer_subs /* = NULL */, int longer_len /* = 0 */)
{
	return sp::split_str(sp::char2wchar(str).c_str(), subs_array, spliter, trim, ignore_blank, supp_quote, trim_quote, longer_subs, longer_len);
}

SP_INLINE
int split_str(const wchar_t * str, str_arr & subs_array, const char spliter[] /* = ",;:" */, bool trim /* = true */,
		bool ignore_blank /* = false */, bool supp_quote /* = false */, bool trim_quote /* = true */, char * longer_subs /* = NULL */, int longer_len /* = 0 */)
{
	return sp::split_str(sp::wchar2char(str).c_str(), subs_array, spliter, trim, ignore_blank, supp_quote, trim_quote, false, longer_subs, longer_len);
}
    
#endif

SP_INLINE
char * dequote_str(char * str)
{
	if ( str == 0 )
		return 0;
	
	const char head = str[0];
	if ( head != '\'' && head != '\"' )
		return str;

	int offset = 0;
	int len = (int) strlen(str);
	if ( str[len - 1] == head )
	{
		str[len - 1] = 0;
		offset = 1;
	}

	return str + offset;
}

SP_INLINE
wchar_t * dequote_str(wchar_t * str)
{
	if ( str == 0 )
		return 0;
	
	const wchar_t head = str[0];
	if ( head != L'\'' && head != L'\"' )
		return str;

	int offset = 0;
	int len = (int) wcslen(str);
	if ( str[len - 1] == head )
	{
		str[len - 1] = 0;
		offset = 1;
	}

	return str + offset;
}

template<typename char_t>
SP_INLINE
std::basic_string<char_t> quote_str(const char_t * cstr, char_t quote_chr /* = char_t('"') */)
{
	std::basic_string<char_t> str;
	if ( cstr == 0 )
		return str;

	str = cstr;
	if ( str.empty() )
	{
		str += quote_chr;
		str += quote_chr;
		return str;
	}

	if ( str.at(0) != quote_chr )
		str.insert(str.begin(), quote_chr);
	if ( str.at(str.length() - 1) != quote_chr )
		str.push_back(quote_chr);
	return str;
}

SP_INLINE
int get_attrib_and_value(const char* attr_val, std::string & attr, std::string & val, const char * split , bool trim /* = false */)
{
	int i = 0;
	char buf[1000];

	val.clear();

	for ( ; attr_val[i] && i < SP_COUNTOF(buf) - 1; i++ )
	{
		if ( strchr(split, attr_val[i]) != 0 )
			break;
		buf[i] = attr_val[i];
	}

	buf[i] = 0;
	if ( trim )
		trim_str(buf);
	attr = buf;

	if ( attr_val[i] != 0
		&& strchr(split, attr_val[i]) != 0 )
	{
		if ( trim )
		{
			strsncpy(buf, attr_val + i + 1, sizeof buf);
			trim_str(buf);
			val = dequote_str(buf);
		}
		else
			val = attr_val + i + 1;
	}

	return 0;
}
    
#ifndef __IPHONE_7_0
SP_INLINE
int get_attrib_and_value(const wchar_t* attr_val, std::wstring & attr, std::wstring & val, const wchar_t * split /* = L"=" */, bool trim /* = false */)
{
	int i = 0;
	wchar_t buf[1000];

	val.clear();

	for ( ; attr_val[i] && i < SP_COUNTOF(buf) - 1; i++ )
	{
		if ( wcschr(split, attr_val[i]) != 0 )
			break;
		buf[i] = attr_val[i];
	}
	
	buf[i] = 0;
	if ( trim )
		trim_str(buf);
	attr = buf;

	if ( attr_val[i] != 0
		&& wcschr(split, attr_val[i]) != 0 )
	{
		if ( trim )
		{
			strsncpy(buf, attr_val + i + 1, sizeof buf);
			trim_str(buf);
			val = dequote_str(buf);
		}
		else
			val = attr_val + i + 1;
	}

	return 0;
}
#endif

SP_INLINE
int str_to_attr_vals(const char* str, str_map & attrs, bool trim /* = false */,
			bool supp_quote /* = false */, const char * splt /* = " ;," */, const char * token /* = "=" */, char * longer_subs /* = 0 */, int longer_len /* = 0 */)
{
	str_arr arr;
	split_str(str, arr, splt, trim, true, supp_quote, true, false, longer_subs, longer_len);

	attrs.clear();
	for ( int i = 0; i < (int)(arr.size()); i++ )
	{
		std::string att, val;
		get_attrib_and_value(arr[i].c_str(), att, val, token, true);
		attrs.insert(sm_item(att, val));
	}

	return 0;
}

SP_INLINE
int str_to_attr_vals(const char* str, str_pair_arr & attrs, bool trim /* = false */,
			bool supp_quote /* = false */, const char * splt /* = " ;," */, const char * token /* = "=" */, char * longer_subs /* = 0 */, int longer_len /* = 0 */)
{
	str_arr arr;
	split_str(str, arr, splt, trim, true, supp_quote, true, false, longer_subs, longer_len);

	attrs.clear();
	for ( int i = 0; i < (int)(arr.size()); i++ )
	{
		std::string att, val;
		get_attrib_and_value(arr[i].c_str(), att, val, token, true);
		attrs.push_back(std::make_pair(att, val));
	}

	return 0;
}

/** 
 * @brief 	substr_interrog
 *  
 *  ��src_str�в����ַ���lpszFind��lpszFind�п��԰���ͨ���ַ���?��
 *  
 * @author	hjye
 * @date	2004-6-24
 * @return	int	- �ɹ�����ƥ��λ�ã����򷵻�-1
 * @param	const char* src_str	- [in] 
 * @param	const char* lpszFind	- [in] 
 * @param	int nStart = 0	- [in] nStartΪ��src_str�е���ʼ����λ��
 * @see		
 */
SP_INLINE
int substr_interrog(const char* src_str, const char* lpszFind, int nStart = 0)
{
	if ( src_str == 0 || lpszFind == 0 || nStart < 0 )
		return -1;

	int m = (int) strlen(src_str);
	int n = (int) strlen(lpszFind);

	if ( nStart + n > m )
		return -1;

	if ( n == 0 )
		return nStart;

	// KMP�㷨
	int* next = new int[n];
	// �õ������ַ�����next����
	{
		n--;
        int j, k;
		j = 0;
		k = -1;
		next[0] = -1;

		while ( j < n )
		{
			if ( k == -1 || lpszFind[k] == '?' || lpszFind[j] == lpszFind[k] )
			{
				j++;
				k++;
				next[j] = k;
			}
			else
				k = next[k];
		}
		
		n++;
	}

	int i = nStart, j = 0;
	while ( i < m && j < n )
	{
		if ( j == -1 || lpszFind[j] == '?' || src_str[i] == lpszFind[j] )
		{
			i++;
			j++;
		}
		else
			j = next[j];
	}

	delete []next;

	if ( j >= n )
		return i - n;
	else
		return -1;
}

SP_INLINE
bool strcmp_wildcard(const char* src_str, const char* match_str, bool match_case /* = true */)
{
	if ( src_str == 0 || match_str == 0 )
		return false;

	if ( match_str[0] == 0 )	//Is a empty string
	{
		if ( src_str[0] == 0 )
			return true;
		else
			return false;
	}

	int i = 0, j = 0;

	// ���ɱȽ�����ʱԴ�ַ���'szSource'
	char* szSource = new char[ (j = (int)strlen(src_str)+1) ];
	if ( match_case )
	{
		while( *(szSource+i) = *(src_str+i) ) i++;
	}
	else
	{
		//Lowercase 'src_str' to 'szSource'
		i = 0;
		while ( src_str[i] )
		{
			if ( src_str[i] >= 'A' && src_str[i] <= 'Z' )
				szSource[i] = src_str[i] - 'A' + 'a';
			else
				szSource[i] = src_str[i];
			i++;
		}
		szSource[i] = 0;
	}

	// ���ɱȽ�����ʱƥ���ַ���'szMatcher'
	char* szMatcher = new char[ strlen(match_str)+1 ];

	// ��match_str���������ġ�*������һ����*�����Ƶ�szMatcher��
	i = j = 0;
	while ( match_str[i] )
	{
		szMatcher[j++] = (!match_case) ?
			( (match_str[i] >= 'A' && match_str[i] <= 'Z') ?	// Lowercase match_str[i] to szMatcher[j]
				match_str[i] - 'A' + 'a' :
				match_str[i]
			) :
			match_str[i];	// Copy match_str[i] to szMatcher[j]
		// Merge '*'
		if ( match_str[i] == '*' )
			while ( match_str[++i] == '*' );
		else
			i++;
	}
	szMatcher[j] = 0;

	//��ʼ����ƥ����

	int nMatchOffset, nSourOffset;

	bool bIsMatched = true;
	nMatchOffset = nSourOffset = 0;
	while ( szMatcher[nMatchOffset] )
	{
		if ( szMatcher[nMatchOffset] == '*' )
		{
			if ( szMatcher[nMatchOffset+1] == 0 )
			{
				// szMatcher[nMatchOffset]�����һ���ַ�
				bIsMatched = true;
				break;
			}
			else
			{
				// szMatcher[nMatchOffset+1]ֻ����'?'����ͨ�ַ�
				int nSubOffset = nMatchOffset + 1;

				while ( szMatcher[nSubOffset] )
				{
					if ( szMatcher[nSubOffset] == '*' )
						break;
					nSubOffset++;
				}

				if ( strlen(szSource + nSourOffset) < size_t(nSubOffset-nMatchOffset-1) )
				{
					// Դ�ַ���ʣ�µĳ���С��ƥ�䴮ʣ��Ҫ�󳤶�
					bIsMatched = false; // �ж���ƥ��
					break;				// �˳�
				}

				if ( !szMatcher[nSubOffset] )	// nSubOffset is point to ender of 'szMatcher'
				{
					// ���ʣ�²����ַ��Ƿ�һһƥ��
					nSubOffset--;
					int nTempSourOffset = (int)strlen(szSource)-1;
					// �Ӻ���ǰ����ƥ��
					while ( szMatcher[nSubOffset] != '*' )
					{
						if ( szMatcher[nSubOffset] == '?' )
							;
						else
						{
							if ( szMatcher[nSubOffset] != szSource[nTempSourOffset] )
							{
								bIsMatched = false;
								break;
							}
						}
						nSubOffset--;
						nTempSourOffset--;
					}
					break;
				}
				else	// szMatcher[nSubOffset] == '*'
				{
					nSubOffset -= nMatchOffset;
					
					char* szTempFinder = new char[nSubOffset];
					nSubOffset--;
					memcpy(szTempFinder, szMatcher+nMatchOffset+1, nSubOffset);
					szTempFinder[nSubOffset] = 0;
					
					int nPos = substr_interrog(szSource+nSourOffset, szTempFinder, 0);
					delete []szTempFinder;
					
					if ( nPos != -1 )	// ��'szSource+nSourOffset'���ҵ�szTempFinder
					{
						nMatchOffset += nSubOffset;
						nSourOffset += (nPos+nSubOffset-1);
					}
					else
					{
						bIsMatched = false;
						break;
					}
				}
			}
		}	// end of "if(szMatcher[nMatchOffset] == '*')"
		else if ( szMatcher[nMatchOffset] == '?' )
		{
			if ( !szSource[nSourOffset] )
			{
				bIsMatched = false;
				break;
			}
			if ( !szMatcher[nMatchOffset+1] && szSource[nSourOffset+1] )
			{
				// ���szMatcher[nMatchOffset]�����һ���ַ���
				// ��szSource[nSourOffset]�������һ���ַ�
				bIsMatched = false;
				break;
			}
			nMatchOffset++;
			nSourOffset++;
		}
		else	// szMatcher[nMatchOffset]Ϊ�����ַ�
		{
			if ( szSource[nSourOffset] != szMatcher[nMatchOffset] )
			{
				bIsMatched = false;
				break;
			}
			if ( !szMatcher[nMatchOffset+1] && szSource[nSourOffset+1] )
			{
				bIsMatched = false;
				break;
			}
			nMatchOffset++;
			nSourOffset++;
		}
	}

	delete []szSource;
	delete []szMatcher;
	return bIsMatched;
}

SP_INLINE
int timecmp(int hour1, int min1, int sec1, int hour2, int min2, int sec2, int day1 /* = -1 */, int day2 /* = -1 */)
{
	// �ȱȽ���
	if ( day1 >= 0 && day2 >= 0 && day1 != day2 )
		return day1 - day2;
	if ( hour1 >= 0 && hour2 >= 0 && hour1 != hour2 )
		return hour1 - hour2;
	if ( min1 >= 0 && min2 >= 0 && min1 != min2 )
		return min1 - min2;
	return sec1 - sec2;
}

SP_INLINE
std::string & strlwr(std::string & str)
{
	size_t len = str.size();
	for ( size_t i = 0; i < len; i++ )
	{
		char & cc = str.at(i);
		if ( cc >= 'A' && cc <= 'Z' )
			cc += 'a' - 'A';
	}
	return str;
}

SP_INLINE
std::string & strupr(std::string & str)
{
	size_t len = str.size();
	for ( size_t i = 0; i < len; i++ )
	{
		char & cc = str.at(i);
		if ( cc >= 'a' && cc <= 'z' )
			cc += 'A' - 'a';
	}
	return str;
}

SP_INLINE
std::wstring & strlwr(std::wstring & str)
{
	size_t len = str.size();
	for ( size_t i = 0; i < len; i++ )
	{
		wchar_t & cc = str.at(i);
		if ( cc >= L'A' && cc <= L'Z' )
			cc += L'a' - L'A';
	}
	return str;
}

SP_INLINE
std::wstring & strupr(std::wstring & str)
{
	size_t len = str.size();
	for ( size_t i = 0; i < len; i++ )
	{
		wchar_t & cc = str.at(i);
		if ( cc >= L'a' && cc <= L'z' )
			cc += L'A' - L'a';
	}
	return str;
}

SP_INLINE
std::string str_replace(const char * str, const char * src, const char * dst, bool trim /* = false */)
{
	std::string str_res(str);
	std::string rep_src(src);
	std::string rep_dst(dst);
	std::string::size_type pos = 0;
	while( ( pos = str_res.find(src, pos) ) != std::string::npos )
	{
		str_res.replace(pos, rep_src.length(), dst);
		pos += rep_dst.length(); // pos += rep_src.length();
	}
	if ( trim )
		trim_str(str_res);
	return str_res;
}

SP_INLINE
std::wstring str_replace(const wchar_t * str, const wchar_t * src, const wchar_t * dst, bool trim /* = false */)
{
	std::wstring str_res(str);
	std::wstring rep_src(src);
	std::wstring rep_dst(dst);
	std::wstring::size_type pos = 0;
	while( ( pos = str_res.find(src, pos) ) != std::wstring::npos )
	{
		str_res.replace(pos, rep_src.length(), dst);
		pos += rep_dst.length(); // pos += rep_src.length();
	}
	if ( trim )
		trim_str(str_res);
	return str_res;
}

SP_INLINE
char * strchr(const char * str, int val)
{
	return (char*)::strchr(str, val);
}

SP_INLINE
wchar_t * strchr(const wchar_t * str, int val)
{
	return (wchar_t*)::wcschr(str, val);
}

// ���Դ�Сд�ıȽ�
SP_INLINE
int strcasecmp(const char* str1, const char * str2)
{
	#ifdef WIN32
	return ::stricmp(str1, str2);
	#else
	return ::strcasecmp(str1, str2);
	#endif
}

SP_INLINE
unsigned int towlower(unsigned int c)
{
	if ( c >= 'A' && c <= 'Z' )
		return  c + 'a' - 'A';
	return c;
}

SP_INLINE
int wcscasecmp (const wchar_t * str1, const wchar_t * str2)
{
	int ret = 0;

	for ( int i = 0; ; i++ )
	{
		const wchar_t & c1 = str1[i];
		const wchar_t & c2 = str2[i];
		
		ret = sp::towlower(c1) - sp::towlower(c2);
		if ( ret != 0 || c1 == 0 )
			return ret;
	}

	return 0;
}

SP_INLINE
int wcsncasecmp(const wchar_t * str1, const wchar_t * str2, size_t maxcount)
{
	int ret = 0;

	for ( size_t i = 0; i < maxcount; i++ )
	{
		const wchar_t & c1 = str1[i];
		const wchar_t & c2 = str2[i];
		
		ret = sp::towlower(c1) - sp::towlower(c2);
		if ( ret != 0 || c1 == 0 )
			return ret;
	}

	return 0;
}

/*
// ���Դ�Сд�ıȽ�
SP_INLINE
int strcasecmp(const wchar_t * str1, const wchar_t * str2)
{
	#ifdef WIN32
	return ::wcsicmp(str1, str2);
	#elif defined(__sun)
	return sp::wcscasecmp(str1, str2);
	#else // linux
	return ::wcscasecmp(str1, str2);
	#endif
}
*/
// ���Դ�Сд���޳��Ƚ�
SP_INLINE
int strncasecmp(const char *str1, const char *str2, size_t len)
{
	#ifdef WIN32
	return ::strnicmp(str1, str2, len);
	#else
	return ::strncasecmp(str1, str2, len);
	#endif
}

/*
// ���Դ�Сд���޳��Ƚ�
SP_INLINE
int strncasecmp(const wchar_t *str1, const wchar_t *str2, size_t len)
{
	#ifdef WIN32
	return ::wcsnicmp(str1, str2, len);
	#elif defined(__sun)
	return sp::wcsncasecmp(str1, str2, len);
	#else // linux
	return ::wcsncasecmp(str1, str2, len);
	#endif
}
*/
SP_INLINE
int strcasecmp(const std::string & str1, const std::string & str2)
{
	return strcasecmp(str1.c_str(), str2.c_str());
}

SP_INLINE
int strcasecmp(const std::wstring & str1, const std::wstring & str2)
{
	return strcasecmp(str1.c_str(), str2.c_str());
}

SP_INLINE
int strncasecmp(const std::string & str1, const std::string & str2, size_t len)
{
	return strncasecmp(str1.c_str(), str2.c_str(), len);
}

SP_INLINE
int strncasecmp(const std::wstring & str1, const std::wstring & str2, size_t len)
{
	return strncasecmp(str1.c_str(), str2.c_str(), len);
}

SP_INLINE
int strcmp(const char * str1, const char * str2)
{
	return ::strcmp(str1, str2);
}

SP_INLINE
int strcmp(const wchar_t * str1, const wchar_t * str2)
{
	return ::wcscmp(str1, str2);
}

SP_INLINE
int strncmp (const char * str1, const char * str2, size_t len)
{
	return ::strncmp(str1, str2, len);
}

SP_INLINE
int strncmp (const wchar_t * str1, const wchar_t * str2, size_t len)
{
	return ::wcsncmp(str1, str2, len);
}

SP_INLINE
size_t strlen(const char * str)
{
	return ::strlen(str);
}
SP_INLINE
size_t strlen(const wchar_t * str)
{
	return ::wcslen(str);
}

// mbstowcs and wcstombs
SP_INLINE
size_t mbstowcs(wchar_t *wcstr, const char *mbstr, size_t count)
{
	size_t cvts = ::mbstowcs(wcstr, mbstr, count);

#ifndef WIN32 // Non Win32
	if ( cvts == -1 )
	{
		size_t i = 0;
		for ( ; i < count; i++ )
		{
			if ( wcstr )
				wcstr[i] = mbstr[i];
			if ( mbstr[i] == 0 )
				break;
		}
		cvts = i - 1;
	}
#endif // LINUX
	return cvts;
}

SP_INLINE
size_t wcstombs(char *mbstr, const wchar_t *wcstr, size_t count)
{
	size_t cvts = ::wcstombs(mbstr, wcstr, count);

#ifndef WIN32 // Non Win32
	if ( cvts == -1 )
	{
		size_t i = 0;
		for ( ; i < count; i++ )
		{
			if ( mbstr )
				mbstr[i] = wcstr[i];
			if ( wcstr[i] == 0 )
				break;
		}
		cvts = i - 1;
	}
#endif // LINUX
	return cvts;
}

SP_INLINE
std::wstring char2wchar(const char * str, size_t * cvt_chrs /* = 0 */)
{
	if (str == NULL || *str == '\0')
		return L"";
		
	wchar_t dest_buf[20 * 1024];
	wchar_t * dest_ptr = dest_buf;

	size_t count = strlen(str) + 1;
	//size_t count = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, NULL, 0);
	if ( count >= sizeof dest_buf / sizeof dest_buf[0] )
		dest_ptr = new wchar_t [count];
	 dest_ptr[0] = 0;

#if _MSC_VER >= 1400
	int siw = (int)(std::max<>(sizeof dest_buf / sizeof dest_buf[0], count));
	//mbstowcs_s(cvt_chrs, dest_ptr, siw, str, count);
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, dest_ptr, siw);
#else	// not vc 8
	size_t cvts = mbstowcs(dest_buf, str, count + 1);
	if ( cvt_chrs )
		*cvt_chrs = cvts;
#endif	// not vc 8

	if ( dest_ptr == dest_buf )
		return dest_ptr;

	std::wstring out_str = dest_ptr;
	delete [] dest_ptr;
	return out_str;
}

/* 
 * convert wchar based string to char based string
 *  for local hacking purposes
 */
SP_INLINE
std::string wchar2char(const wchar_t * wstr, size_t * cvt_chrs /* = 0 */)
{
	if (wstr == NULL || *wstr == L'\0')
		return "";
	char dest_buf[20 * 1024];
	char * dest_ptr = dest_buf;

	size_t count = (wcslen(wstr) + 1) * sizeof wstr[0];
	//size_t count = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, wstr, -1, NULL, 0, NULL, FALSE);
	if ( count >= sizeof dest_buf )
		dest_ptr = new char [count];
	 dest_ptr[0] = 0;

#if _MSC_VER >= 1400
#pragma warning(push)
#pragma warning(disable : 4267)
	size_t sib = std::max<>(count, sizeof dest_buf);
	//wcstombs_s(cvt_chrs, dest_ptr, sib, wstr, count);
	::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, wstr, -1, dest_ptr, sib, NULL, FALSE);
#pragma warning(pop)
#else	// not vc 8
	size_t cvts = wcstombs(dest_ptr, wstr, count);
	if ( cvt_chrs )
		*cvt_chrs = cvts;
#endif	// not vc 8

	if ( dest_ptr == dest_buf )
		return dest_ptr;

	std::string out_str = dest_ptr;
	delete [] dest_ptr;
	return out_str;
}

SP_INLINE
std::string  tochar(const std::string & str)
{
	return str;
}
SP_INLINE
std::string  tochar(const std::wstring & str)
{
	return wchar2char(str.c_str());
}

SP_INLINE
std::wstring  towchar(const std::string & str)
{
	return char2wchar(str.c_str());
}
SP_INLINE
std::wstring  towchar(const std::wstring & str)
{
	return str;
}

SP_INLINE
std::string  toutf8(const std::string & str)
{
#ifdef WIN32
	return toutf8(towchar(str));
#else // linux
	return str;
#endif // OS
}
SP_INLINE
std::string  toutf8(const std::wstring & str)
{
#ifdef WIN32
	char dest[4000]; dest[0] = 0;
	char * dest_ptr = dest;
	int dest_cap = sizeof dest;
	if ( (int) str.size() > dest_cap / 2 )
	{
		dest_cap = (int) str.size() * 3;
		dest_ptr = new char[dest_cap];
	}

	int ret = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), dest_ptr, dest_cap, 0, 0);
	if ( ret >= 0 )
		dest_ptr[ret] = 0;
	if ( dest_ptr == dest )
		return dest_ptr;
	std::string utf_str = dest_ptr;
	delete [] dest_ptr;
	return utf_str;

#else // linux
	IU_ASSERT(0 && "Not implement yet.");
	return std::string("");
#endif // OS
}
SP_INLINE
std::wstring utf8towchar(const std::string & str)
{
#ifdef WIN32
	wchar_t dest[4000]; dest[0] = 0;
	wchar_t * dest_ptr = dest;
	int dest_cap = SP_COUNTOF(dest);
	bool is_new = false;
	if ( (int) str.size() > dest_cap )
	{
		is_new = true;
		dest_cap = (int) str.size();
		dest_ptr = new wchar_t[dest_cap];
	}

	int ret = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), dest_ptr, dest_cap);
	if ( ret >= 0 )
		dest_ptr[ret] = 0;

	std::wstring wstr = dest_ptr;
	if ( is_new )
	{
		delete [] dest_ptr;
	}
	else
	{
		return dest_ptr;
	}
	return wstr;

#else // linux
	IU_ASSERT(0 && "Not implement yet.");
	return std::wstring(L"");
#endif // OS
}
SP_INLINE
std::string  utf8tochar(const std::string & str)
{
#ifdef _WIN32
	return tochar(utf8towchar(str));
#else
	return str;
#endif 
}

SP_INLINE
int strtoi(const char * str)
{
	return (int)strtol(str, 0, 0);
}

SP_INLINE
int strtoi(const wchar_t * str)
{
	return (int)wcstol(str, 0, 0);
}

SP_INLINE
size_t strtosizet(const char * str)
{
	return strtol(str, 0, 0);
}

SP_INLINE
size_t strtosizet(const wchar_t * str)
{
	return wcstol(str, 0, 0);
}

SP_INLINE
unsigned int strtoui(const char * str)
{
	return (int)strtoul(str, 0, 0);
}

SP_INLINE
unsigned int strtoui(const wchar_t * str)
{
	return (int)wcstoul(str, 0, 0);
}

SP_INLINE
double strtod(const char * str)
{
	return ::strtod(str, 0);
}

SP_INLINE
double strtod(const wchar_t * str)
{
	return ::wcstod(str, 0);
}

SP_INLINE
float strtof(const char * str)
{
	double val = ::strtod(str, 0);
	return (float)val;
}

SP_INLINE
float strtof(const wchar_t * str)
{
	double val = ::wcstod(str, 0);
	return (float)val;
}

SP_INLINE
bool strtobool(const char * str, bool def /* = false */)
{
	if ( str == 0 )
		return def;
	if ( sp::strcasecmp(str, "true") == 0 )
		return true;
	if ( sp::strcasecmp(str, "false") == 0 )
		return false;
	if ( sp::strcmp(str, "0") == 0 )
		return false;
	if ( strtoi(str) != 0 )
		return true;
	return def;
}

SP_INLINE
bool strtobool(const wchar_t * str, bool def /* = false */)
{
	if ( str == 0 )
		return def;
	if ( sp::strcasecmp(str, L"true") == 0 )
		return true;
	if ( sp::strcasecmp(str, L"false") == 0 )
		return false;
	if ( sp::strcmp(str, L"0") == 0 )
		return false;
	if ( strtoi(str) != 0 )
		return true;
	return def;
}

SP_INLINE
bool strtoi(const char * str, int &int_val)
{
	char *endptr = NULL;
	int_val = ::strtol(str, &endptr, 0);
	return str != NULL && endptr != NULL && *endptr == '\0' && str != endptr;
}
SP_INLINE
bool strtosizet(const char * str, size_t &sz_val)
{
	char *endptr = NULL;
#if defined(_WIN32) && (_MSC_VER <= 1600)
#if defined(_WIN64)
	sz_val = ::_strtoui64(str, &endptr, 0);
#else
	sz_val = ::strtoul(str, &endptr, 0);
#endif
#else
	sz_val = ::strtoul(str, &endptr, 0);
#endif
	return str != NULL && endptr != NULL && *endptr == '\0' && str != endptr;
}
SP_INLINE
bool strtod(const char * str, double &d_val)
{
	char *endptr = NULL;
	d_val = ::strtod(str, &endptr);
	return str != NULL && endptr != NULL && *endptr == '\0' && str != endptr;
}
SP_INLINE
bool strtof(const char * str, float &f_val)
{
	char *endptr = NULL;
	f_val = (float)::strtod(str, &endptr);
	return str != NULL && endptr != NULL && *endptr == '\0' && str != endptr;
}
SP_INLINE
bool strtob(const char * str, bool & b_val)
{
	if(sp::strcasecmp(str, "true") == 0)
	{
		b_val = true;
		return true;
	}
	if(sp::strcasecmp(str, "false") == 0)
	{
		b_val = false;
		return true;
	}
	if(sp::strcasecmp(str, "1") == 0)
	{
		b_val = true;
		return true;
	}
	if(sp::strcasecmp(str, "0") == 0)
	{
		b_val = false;
		return true;
	}
	return false;
}
SP_INLINE
bool strtoui(const char * str, unsigned int &ui_val)
{
	char *endptr = NULL;
	ui_val = ::strtoul(str, &endptr, 0);
	return str != NULL && endptr != NULL && *endptr == '\0' && str != endptr;
}

SP_INLINE
const char * itostr(int val, std::string & str, const char * fmt)
{
	char buf[1024];
	sprintf(buf, fmt ? fmt : "%d", val);
	str = buf;
	return str.c_str();
}

SP_INLINE
const wchar_t * itostr(int val, std::wstring & str, const wchar_t * fmt)
{
	wchar_t buf[1024];
#ifdef WIN32
	swprintf(buf, fmt ? fmt : L"%d", val);
#else
    swprintf(buf, 1024, fmt ? fmt : L"%d", val);
#endif
	str = buf;
	return str.c_str();
}

SP_INLINE
std::string itostr(int val, const char * fmt /* = 0 */)
{
	char buf[1024];
	sprintf(buf, fmt ? fmt : "%d", val);
	return buf;
}

SP_INLINE
std::wstring itowstr(int val, const wchar_t * fmt /* = 0 */)
{
	wchar_t buf[1024];
#ifdef WIN32
	swprintf(buf, fmt ? fmt : L"%d", val);
#else
    swprintf(buf, 1024, fmt ? fmt : L"%d", val);
#endif
	return buf;
}

SP_INLINE
const char * ftostr(double val, std::string & str, const char * fmt)
{
	char buf[1024];
	sprintf(buf, fmt ? fmt : "%6.3f", val);
	str = buf;
	return str.c_str();
}

SP_INLINE
const wchar_t * ftostr(double val, std::wstring & str, const wchar_t * fmt)
{
	wchar_t buf[1024];
#ifdef WIN32
	swprintf(buf, fmt ? fmt : L"%6.3f", val);
#else
    swprintf(buf, 1024, fmt ? fmt : L"%6.3f", val);
#endif
	str = buf;
	return str.c_str();
}

SP_INLINE
std::string ftostr(double val, const char * fmt /* = 0 */)
{
	char buf[1024];
	sprintf(buf, fmt ? fmt : "%6.3f", val);
	return buf;
}

SP_INLINE
std::wstring ftowstr(double val, const wchar_t * fmt /* = 0 */)
{
	wchar_t buf[1024];
#ifdef WIN32
	swprintf(buf, fmt ? fmt : L"%6.3f", val);
#else
    swprintf(buf, 1024, fmt ? fmt : L"%6.3f", val);
#endif
	return buf;
}

SP_INLINE
const char* cur_time(char times[], bool no_path /* = false */, bool sht_fmt /* = false */, bool has_msec /* = true */, time_t start_time /* = 0 */, unsigned long start_tick /* = 0 */)
{
    times[0] = '\x0';
	/* Display operating system-style date and time. */
	time_t tmc, tmr;
	tmr = time(&tmc);

	char ms[8]; ms[0] = '\0';
#ifdef WIN32
	const DWORD max_sec = (DWORD)(0xFFFFFFFFU) / 1000;
	if ( tmc >= start_time && start_tick != 0 )
	{
		DWORD cost_tick = ::GetTickCount() - start_tick;
		tmr = start_time + (tmc - start_time) / max_sec + cost_tick / 1000;
		sprintf(ms, " %03d", cost_tick % 1000);
	}
	else
		sprintf(ms, " %03d", ::GetTickCount() % 1000);
#else	// linux
	struct timeval tv;
	gettimeofday(&tv, 0);
	sprintf(ms, " %03d", (tv.tv_usec / 1000) % 1000);
#endif	// end

#ifdef WIN32
			struct tm * ptm = ::localtime(&tmr);
#else
			struct tm result;
			struct tm * ptm = ::localtime_r(&tmr, &result);
#endif

	if ( no_path )
	{
		strftime(times, 128, "%Y-%m-%d_%H-%M-%S", ptm);
	}
	else
	{
		if ( sht_fmt )
			strftime(times, 128, "%H:%M:%S", ptm);
		else
			strftime(times, 128, "%y/%m/%d-%H:%M:%S", ptm);
		// has msec
		if ( has_msec )
			strcat(times, ms);
	}
	return times;
}

SP_INLINE
const wchar_t * cur_time(wchar_t times[], bool no_path /* = false */, bool sht_fmt /* = false */, bool has_msec /* = true */, time_t start_time /* = 0 */, unsigned long start_tick /* = 0 */)
{
	char times_mb[64];
	cur_time(times_mb, no_path, sht_fmt, has_msec);
	sp::strsncpy(times, sp::char2wchar(times_mb).c_str(), sizeof(times_mb));
	return times;
}

SP_INLINE
std::string  cur_time(bool no_path /* = false */, bool sht_fmt /* = false */, bool has_msec /* = true */, time_t start_time /* = 0 */, unsigned long start_tick /* = 0 */)
{
	char time_str[64];
	return cur_time(time_str, no_path, sht_fmt, has_msec);
}

SP_INLINE
std::wstring wcur_time(bool no_path, bool sht_fmt, bool has_msec /* = true */, time_t start_time /* = 0 */, unsigned long start_tick /* = 0 */)
{
	return sp::char2wchar(cur_time(no_path, sht_fmt, has_msec).c_str());
}

SP_INLINE
const char * strcopy(std::string & dest, const char * src)
	{
	dest = src;
	return dest.c_str();
	}
SP_INLINE
const wchar_t *  strcopy(std::wstring & dest, const char * src)
	{
	dest = sp::char2wchar(src);
	return dest.c_str();
	}
SP_INLINE
const char * strcopy(std::string & dest, const wchar_t * src)
	{
	dest = sp::wchar2char(src);
	return dest.c_str();
	}
SP_INLINE
const wchar_t *  strcopy(std::wstring & dest, const wchar_t * src)
	{
	dest = src;
	return dest.c_str();
	}

SP_INLINE
const char * trans_escape_char(std::string & str)
	{
	std::string::size_type pos = 0;
	std::string::size_type begin = 0;
	while ( (pos = str.find_first_of("\\", begin)) != std::string::npos )
		{
		char nchar = str.at(pos+1);
		if ( 'n' == nchar )
			str.replace(pos, 2, "\n");
		else if ( 'r' == nchar )
			str.replace(pos, 2, "\r");
		else if ( 't' == nchar )
			str.replace(pos, 2, "\t");
		else if ( '0' == nchar )
			str.replace(pos, 2, "\0");
		else if ( '\\' == nchar )
			str.replace(pos, 2, "\\");
		begin = pos + 2;
		}
	return str.c_str();
	}
SP_INLINE
const wchar_t * trans_escape_char(std::wstring & str)
	{
	std::wstring::size_type pos = 0;
	std::wstring::size_type begin = 0;
	while ( (pos = str.find_first_of(L"\\", begin)) != std::string::npos )
		{
		wchar_t nchar = str.at(pos+1);
		if ( L'n' == nchar )
			str.replace(pos, 2, L"\n");
		else if ( L'r' == nchar )
			str.replace(pos, 2, L"\r");
		else if ( L't' == nchar )
			str.replace(pos, 2, L"\t");
		else if ( L'0' == nchar )
			str.replace(pos, 2, L"\0");
		else if ( L'\\' == nchar )
			str.replace(pos, 2, L"\\");
		begin = pos + 2;
		}
	return str.c_str();
	}



	
} /* namespace sp string */

/*=============================================================================
 * �ļ�����
 *=============================================================================*/

// OS relative headers
#include <sys/stat.h>

#ifdef WIN32
#  include <windows.h>
#  pragma comment(lib, "version.lib")
#else /* LINUX */
#  include <unistd.h>
#  include <signal.h>
#  include <sys/param.h>
#  include <sys/types.h>
#endif /* WIN32 */

namespace sp
{

SP_INLINE
bool is_file_exist(const char* file)
{
#ifdef WIN32
	DWORD ret = ::GetFileAttributesA(file);
	if ( ret != 0xFFFFFFFF && !(ret & FILE_ATTRIBUTE_DIRECTORY) )
		return true;
#else
	struct stat fs = { 0 };
	if ( stat(file, &fs) == 0 && ( fs.st_mode & S_IFDIR ) == 0 )
		return true;
#endif /*WIN32*/
	return false;
}

SP_INLINE
bool is_file_exist(const wchar_t* file)
{
#ifdef WIN32
	DWORD ret = ::GetFileAttributesW(file);
	if ( ret != 0xFFFFFFFF && !(ret & FILE_ATTRIBUTE_DIRECTORY) )
		return true;
#else
	struct stat fs = { 0 };
	if ( stat(sp::wchar2char(file).c_str(), &fs) == 0 && ( fs.st_mode & S_IFDIR ) == 0 )
		return true;
#endif /*WIN32*/
	return false;
}

SP_INLINE
bool is_dir_exist(const char* dir)
{
#ifdef WIN32
	DWORD ret = ::GetFileAttributesA(dir);
	if ( ret != 0xFFFFFFFF && ( ret & FILE_ATTRIBUTE_DIRECTORY ) )
		return true;
#else
	struct stat fs = { 0 };
	if ( stat(dir, &fs) == 0 && ( fs.st_mode & S_IFDIR ) != 0 )
		return true;
#endif /*WIN32*/
	return false;
}

SP_INLINE
bool is_dir_exist(const wchar_t* dir)
{
#ifdef WIN32
	DWORD ret = ::GetFileAttributesW(dir);
	if ( ret != 0xFFFFFFFF && ( ret & FILE_ATTRIBUTE_DIRECTORY ) )
		return true;
#else
	struct stat fs = { 0 };
	if ( stat(sp::wchar2char(dir).c_str(), &fs) == 0 && ( fs.st_mode & S_IFDIR ) != 0 )
		return true;
#endif /*WIN32*/
	return false;
}

// �з�Ŀ¼��ΪĿ¼���ṹ
SP_INLINE
int path_to_dir_tree(const char * path, str_arr & dir_tree)
{
	if ( path == 0 || *path == 0 )
		return -1;

	int begin = 0;
	int len = (int)strlen(path);
	if ( len > 2 )
	{
		if ( path[0] == '/' )
			begin = 1;
		else if ( path[0] == '\\' && path[1] == '\\' )
			begin = 2;
		else if ( path[1] == ':' )
			begin = 3;
	}

	char dir_name[MAX_PATH]; memset(dir_name, 0, MAX_PATH);
	for ( int i = begin; ; i++ )
	{
		const char c = path[i];
		if ( c == '/'|| c == '\\' || c == '\0' )
		{
			strncpy(dir_name, path, i);
			dir_name[i] = 0;
			dir_tree.push_back(dir_name);
			if ( path[i+1] == 0 )
				break ;
		}
		if ( c == 0 )
			break ;
	}

	return 0;
}

// ����Ŀ¼
SP_INLINE
int create_directory(const char * dir, bool fail_if_exist /* = false */, bool recursive /* = true */)
{
	if ( is_dir_exist(dir) )
		return fail_if_exist ? -1 : 0;

	str_arr dirs;
	int ret = path_to_dir_tree(dir, dirs);
	if ( ret != 0 )
		return ret;

	if ( !recursive &&
		dirs.size() > 1 )
		return -1;

	for ( int i = 0; i < (int)dirs.size(); i++ )
	{
		const char * pdr = dirs[i].c_str();
		if ( is_dir_exist(pdr) )
			continue ;
#ifdef WIN32
		BOOL res = CreateDirectoryA(pdr, NULL);
		if ( res == FALSE )
		{
			ret = GetLastError();
			if(ret == 183 || ret == 80)
				continue;
			return ret;
		}

#else  // linux
		ret = mkdir(pdr, 0755);
		if (errno == 17 || ret == 17)
			continue;
		if ( ret != 0 )
			return errno;
#endif // WIN32
	}

	return 0;
}


SP_INLINE
int normalize_path(char path[MAX_PATH], const char path_split /* = SLASH_CHR */)
{
	int len = 0;
	if ( 0 == path )
		return 0;

	// �ж�\\server-ii�����
	int begin = 0;
	if ( path[0] == BSLASH_CHR && path[1] == BSLASH_CHR )
		begin = 2;

	// �����еĺ����ַ���ͬ���ַ���ת����ͬ
	char dest[MAX_PATH]; memset(dest, 0, MAX_PATH);
	bool slash_begin = false;
	bool need_copy = false;
	for ( int i = begin; ; i++ )
	{
		char & c = path[i];
		if ( c == SLASH_CHR || c == BSLASH_CHR )
		{
			c = path_split;
			if ( slash_begin )
			{
				need_copy = true;
				continue;
			}
			else if ( i > 0 )
				slash_begin = true;
		}
		else
			slash_begin = false;

		dest[len] = c;
		if ( c == '\0' )
			break;
		++len;
	}

	// �Ƿ���Ҫ�ٿ���һ�飿
	if ( len > 0 )
	{
		if ( dest[len-1] != path_split && is_dir_exist(dest) )
		{
			dest[len++] = path_split;
			dest[len]   = '\0';
			need_copy = true;
		}
	}
	if ( need_copy )
		strcpy(path + begin, dest);
	return len;
}

SP_INLINE
std::string normalize_path(const char * path, const char path_split /* = PATH_CHR */)
{
	char res_path[MAX_PATH]; memset(res_path, 0, MAX_PATH);
	strsncpy(res_path, path, SP_COUNTOF(res_path));
	normalize_path(res_path, path_split);
	return res_path;
}

SP_INLINE
const char * cat_path(char* path, const char* more, const char path_split /* = SLASH_CHR */)
{
	if ( 0 == path )
		return 0; 
	if ( more == 0 )
		return path;

	int len_path = (int) strlen(path);
	int len_more = (int) strlen(more);
	if ( len_path + len_more > MAX_PATH )
	{
		IU_ASSERT(0 && "catted path too long!!!");
		return 0;
	}

	char tmp[MAX_PATH]; memset(tmp, 0, MAX_PATH);
	len_path = normalize_path(path, path_split);
	strcpy(tmp, more);
	len_more = normalize_path(tmp, path_split);
	if ( len_path > 0 )
	{
		if ( path[len_path - 1] != path_split )
			strcat(path, path_split == SLASH_CHR ? SLASH_STR : BSLASH_STR);
		if ( tmp[0] == path_split )
			strcat(path, tmp + 1);
		else
			strcat(path, tmp);
	}
	else
		strcpy(path, tmp);

	return path;
}

SP_INLINE
std::string cat_path(const char * path, const char * more, const char path_split /* = PATH_CHR */)
{
	char new_path[MAX_PATH]; memset(new_path, 0, MAX_PATH);
	strsncpy(new_path, path, sizeof new_path);
	return cat_path(new_path, more, path_split);
}

SP_INLINE
char* get_bin_path(char path[MAX_PATH], IU_HANDLE hmod /* = 0 */)
{
	
	get_bin_pathname(path, hmod);
	return pathname_to_path(path, path);
}

SP_INLINE
std::string get_bin_path(IU_HANDLE hmod /* = 0 */)
{
	char path[MAX_PATH]; memset(path, 0, MAX_PATH);
	return get_bin_path(path, hmod);
}

SP_INLINE
char* get_bin_pathname(char pathname[MAX_PATH], IU_HANDLE hmod /* = 0 */)
{
	memset(pathname, 0, MAX_PATH);
#ifdef WIN32
	if ( hmod == INVALID_HANDLE_VALUE )
		hmod = mod_handle();
	::GetModuleFileNameA((HMODULE)hmod, pathname, MAX_PATH);
#elif defined(__sun) /* sun os */
	int ret = readlink("/proc/self/path/a.out", pathname, MAX_PATH);
	if ( ret == -1 || ret == 0 )
		fprintf(stderr, "sp::get_bin_pathname | readlink failed, ret = %d, errno = %d.", ret, errno);
#else /* Linux */
	if ( hmod == 0 || hmod == IU_INVALID_HANDLE)
    {
	    int ret = readlink("/proc/self/exe", pathname, MAX_PATH);
	    if ( ret == -1 || ret == 0 )
		    fprintf(stderr, "sp::get_bin_pathname | readlink failed, ret = %d, errno = %d.", ret, errno);
    }
    else
    {
        char command_line[MAX_PATH];
        sprintf(command_line, "awk '{print $6}' /proc/%d/maps | grep %s", getpid(), (const char*)hmod);
        FILE* fp = popen(command_line, "r");
        fgets(pathname, MAX_PATH, fp);
        pclose(fp);
    }
#endif /* NOT WIN32 */
	return pathname;
}

SP_INLINE
std::string get_bin_pathname(IU_HANDLE hmod /* = 0 */)
{
	char pathname[MAX_PATH]; memset(pathname, 0, MAX_PATH);
	return get_bin_pathname(pathname, hmod);
}

SP_INLINE
std::string get_file_strinfo(const char* psz_info, const char* file /* = 0 */)
{
	std::string sDetailInfo;

#ifdef WIN32
	char szFile[MAX_PATH]; memset(szFile, 0, MAX_PATH);
	DWORD dwData = 0, dwSize;

	UINT uSize;
	char* lpBuffer;
	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	// Get File Name
	if ( file == 0 )
		GetModuleFileNameA(0, szFile, MAX_PATH);
	else
		strcpy(szFile, file);

	// if file exist?
	if ( !is_file_exist(szFile) )
		return "";

	// GetVersionInfo
	dwSize = ::GetFileVersionInfoSizeA(szFile, &dwData);
	if ( dwSize ==  0 )
		return "";

	PBYTE pData = new BYTE[dwSize];
	GetFileVersionInfoA(szFile, 0, dwSize, pData);
	VerQueryValueA(pData, "\\VarFileInfo\\Translation", (PVOID*)&lpTranslate, &uSize);
	// Get info detail
	for( UINT i = 0; i < ( uSize / sizeof(LANGANDCODEPAGE) ); i++ )
	{
		char szSubBlock[128];
		wsprintfA(szSubBlock, 
			"\\StringFileInfo\\%04x%04x\\%s",
			lpTranslate[i].wLanguage, lpTranslate[i].wCodePage, psz_info);

		// Retrieve file description for language and code page "i". 
		BOOL bRet = VerQueryValueA(pData, szSubBlock, (void**)&lpBuffer, (UINT*)&dwSize);
		if ( bRet )
			sDetailInfo = lpBuffer;
	}
	delete [] pData;
#else
	//IU_ASSERT( 0 && "Not support.");
#endif
	return sDetailInfo;
}

SP_INLINE
char * pathname_to_path(char path[MAX_PATH], const char* path_name, const char path_split /* = PATH_CHR */)
{
	IU_ASSERT(path_name);
	if ( path != path_name )
		strcpy(path, path_name);
	normalize_path(path, path_split);

	char* p = strrchr(path, path_split);
	if ( 0 != p )
		*(p + 1) = '\0';
	else
		path[0] = 0;
	return path;
}

SP_INLINE
std::string pathname_to_path(const char* path_name, const char path_split /* = PATH_CHR */)
{
	char path[MAX_PATH]; memset(path, 0, MAX_PATH);
	return pathname_to_path(path, path_name, path_split);
}

SP_INLINE
char * pathname_to_name(char name[MAX_PATH], const char* path_name)
{
	char tmp[MAX_PATH]; memset(tmp, 0, MAX_PATH);
	IU_ASSERT(path_name);
	strcpy(tmp, path_name);
	normalize_path(tmp);

	char * p = strrchr(tmp, PATH_CHR);
	p == 0 ? p = tmp : p++;
	strcpy(name, p);
	return name;
}

SP_INLINE
std::string pathname_to_name(const char* path_name)
{
	char name[MAX_PATH]; memset(name, 0, MAX_PATH);
	return pathname_to_name(name, path_name);
}

SP_INLINE
char * pathname_to_noextname(char name[MAX_PATH], const char* path_name)
{
	pathname_to_name(name, path_name);
	char* p = strrchr(name, '.');
	if ( p != 0 )
		*p = 0;
	return name;
}

SP_INLINE
std::string pathname_to_noextname(const char* path_name)
{
	char nen[MAX_PATH]; memset(nen, 0, MAX_PATH);
	return pathname_to_noextname(nen, path_name);
}

SP_INLINE
char* pathname_to_extname(char ext[MAX_PATH], const char* path_name, bool no_dot /* = true */)
{
	IU_ASSERT(ext);
	char buf[MAX_PATH]; memset(buf, 0, MAX_PATH);

	char* p =  (char*)strrchr(path_name, '.');
	if ( p != 0 )
		strcpy(buf, no_dot ? p + 1 : p);
	if ( ext != 0 )
		return strcpy(ext, buf);
	if ( p == 0 )
		return 0;

	return no_dot ? p + 1 : p;
}

SP_INLINE
std::string pathname_to_extname(const char* path_name, bool no_dot /* = true */)
{
	char en[MAX_PATH]; memset(en, 0, MAX_PATH);
	return pathname_to_extname(en, path_name, no_dot);
}

SP_INLINE
int version_compare(const char * ver1, const char * ver2)
{
	if ( ver1 == ver2 )
		return 0;

	if ( ver1 == 0 )
		return -1;
	if ( ver2 == 0 )
		return 1;

	str_arr arr1, arr2;
	split_str(ver1, arr1, ".,", true, false, false);
	split_str(ver2, arr2, ".,", true, false, false);

	int major1 = 0, major2 = 0;
	int minor1 = 0, minor2 = 0;
	int rev1   = 0, rev2   = 0;
	int build1 = 0, build2 = 0;

	if ( arr1.size() > 0 )
		major1 = strtol(arr1[0].c_str(), 0, 0);
	if ( arr2.size() > 0 )
		major2 = strtol(arr2[0].c_str(), 0, 0);

	if ( arr1.size() > 1 )
		minor1 = strtol(arr1[1].c_str(), 0, 0);
	if ( arr2.size() > 1 )
		minor2 = strtol(arr2[1].c_str(), 0, 0);

	if ( arr1.size() > 2 )
		rev1   = strtol(arr1[2].c_str(), 0, 0);
	if ( arr2.size() > 2 )
		rev2   = strtol(arr2[2].c_str(), 0, 0);

	if ( arr1.size() > 3 )
		build1 = strtol(arr1[3].c_str(), 0, 0);
	if ( arr2.size() > 3 )
		build2 = strtol(arr2[3].c_str(), 0, 0);

	if ( major1 != major2 )
		return major1 - major2;
	else if ( minor1 != minor2 )
		return minor1 - minor2;
	else if ( rev1 != rev2 )
		return rev1 - rev2;

	return build1 - build2;
}

SP_INLINE
bool is_text_file(const char * file, int maxs /* = 1024 */)
{
	FILE* fp = fopen(file, "rb");
	if ( 0 == fp )
		return false;

	// ���������
	bool is_text = true;
	unsigned char cur;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);

	int ml = std::min<int>(len, maxs);
	for ( int i = 0; i< ml; i++ )
	{
		fread(&cur, 1, 1, fp);
		if ( cur >= 0x20 )
			continue;
		if ( cur != '\t'
			&& cur != '\r' && cur != '\n' )
		{
			is_text = false;
			break;
		}
	}
	fclose(fp);
	return is_text;
}

SP_INLINE
bool is_xml_file(const char * file)
{
	FILE* fp = fopen(file, "rb");
	if ( 0 == fp )
		return false;

	unsigned char head[16];
	
	bool is_xml = false;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	
	if(len<16)
	{
		fclose(fp);
		return false;
	}
	
	fread(head,1,16,fp);
	unsigned char* ml = &head[0];
	if(ml[0] == '\xEF'&& ml[1] == '\xBB' && ml[2] == '\xBF') /*utf-8*/
	{
		if(ml[3] == '\x3c'&& ml[4] == '\x3f' && ml[5] == '\x78'&& ml[6] == '\x6d'&& ml[7] == '\x6c')
			is_xml = true;
	}
	else if(ml[0] == '\xFE'&& ml[1] == '\xFF') /*utf-16*/
	{
		if(ml[2] == '\x00' && ml[3] == '\x3c'&& 
		   ml[4] == '\x00' && ml[5] == '\x3f'&& 
		   ml[6] == '\x00' && ml[7] == '\x78'&&
		   ml[8] == '\x00' && ml[9] == '\x6d'&&
		   ml[10]== '\x00' && ml[11]== '\x6c')
		   is_xml = true;
	}
	else if( ml[0] == '\xFF'&& ml[1] == '\xFE' )
	{
		if(ml[3] == '\x00' && ml[2] == '\x3c'&& 
		   ml[5] == '\x00' && ml[4] == '\x3f'&& 
		   ml[7] == '\x00' && ml[6] == '\x78'&&
		   ml[9] == '\x00' && ml[8] == '\x6d'&&
		   ml[11]== '\x00' && ml[10]== '\x6c')
		   is_xml = true;
	}
	/*no bom*/
	else if(ml[0] == '\x3c'&& ml[1] == '\x3f' && ml[2] == '\x78'&& ml[3] == '\x6d'&& ml[4] == '\x6c')
		is_xml = true;
	else if(ml[0] == '\x00' && ml[1] == '\x3c'&& 
		    ml[2] == '\x00' && ml[3] == '\x3f'&& 
		    ml[4] == '\x00' && ml[5] == '\x78'&&
		    ml[6] == '\x00' && ml[7] == '\x6d'&&
		    ml[8] == '\x00' && ml[9] == '\x6c')
		   is_xml = true;
	else if(ml[1] == '\x00' && ml[0] == '\x3c'&& 
		    ml[3] == '\x00' && ml[2] == '\x3f'&& 
		    ml[5] == '\x00' && ml[4] == '\x78'&&
		    ml[7] == '\x00' && ml[6] == '\x6d'&&
		    ml[9]== '\x00'  && ml[8] == '\x6c')
		   is_xml=true;
	fclose(fp);
	return is_xml;
}

SP_INLINE
bool is_utf8_file(const char * file)
{	
	FILE* fp = fopen(file, "rb");
	if ( 0 == fp )
		return false;

	char head[3];
	
	bool is_utf8 = false;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	
	if(len<3)
	{
		fclose(fp);
		return false;
	}
	
	fread(head,1,3,fp);
	if(head[0] == '\xEF'&& head[1] == '\xBB' && head[2] == '\xBF') /*utf-8*/
	{
		is_utf8 = true;
	}

	fclose(fp);
	return is_utf8;
}

SP_INLINE
bool is_abs_path(const char* path)
{
	return path[0] == '/'
		|| (path[0] == '\\' && path[1] == '\\')
		|| strchr(path, ':');
}

SP_INLINE
char * get_bin_name(char name[MAX_PATH], IU_HANDLE hmod /* = 0 */)
{
	char pathname[MAX_PATH]; memset(pathname, 0, MAX_PATH);
	get_bin_pathname(pathname, hmod);
	return pathname_to_name(name, pathname);
}

SP_INLINE
std::string get_bin_name(IU_HANDLE hmod /* = 0 */)
{
	char bin_name[MAX_PATH]; memset(bin_name, 0, MAX_PATH);
	return get_bin_name(bin_name, hmod);
}

SP_INLINE
int get_file_time(const char* file, time_t & tm_created, time_t & tm_modified)
{
	tm_created = 0;
	tm_modified = 0;

	if ( file != 0 )
	{
		struct stat si;
		int ret = ::stat(file, &si);
		if ( ret == 0 )
		{
			tm_modified = si.st_mtime;
			tm_created  = si.st_ctime;
			return 0;
		}
		return ret;
	}
	return 0;
}

SP_INLINE
time_t get_file_mtime(const char * file)
{
	time_t tmm = 0;
	time_t tmc = 0;
	get_file_time(file, tmc, tmm);
	return tmm;
}

SP_INLINE
time_t get_file_ctime(const char * file)
{
	time_t tmm = 0;
	time_t tmc = 0;
	get_file_time(file, tmc, tmm);
	return tmc;
}

SP_INLINE
int get_file_size(const char* file, unsigned int & size)
{
	struct stat si;
	size = 0;
	if ( file != 0 )
	{
		int ret = ::stat(file, &si);
		if ( ret == 0 )
			size = si.st_size;
		return ret;
	}
	return 0;
}

SP_INLINE
unsigned int get_file_size(const char* file)
{
	unsigned int size = 0;
	if ( get_file_size(file, size) == 0 )
		return size;
	return 0;
}

SP_INLINE
const char* install_path(const char* path /* = 0 */)
{
	static char ins_path[MAX_PATH] = "\0";
	if ( path )
		strsncpy(ins_path, path, MAX_PATH);
	return ins_path;
}

SP_INLINE
char* get_file_full_path(char path_name[], const char* file_name, const char* rel_path /* = "bin" */)
{
	IU_ASSERT(path_name);

	char fn[MAX_PATH]; memset(fn, 0, MAX_PATH);
	strcpy(path_name, file_name);

	if ( !is_abs_path(path_name) && !is_file_exist(path_name) )
	{
		strsncpy(fn, file_name, MAX_PATH);
		cat_path(get_bin_path(path_name), fn);
		if ( !is_file_exist(path_name) )
		{
			strcpy(path_name, install_path());
			cat_path(path_name, rel_path);
			cat_path(path_name, fn);
		}
	}
	return path_name;
}

SP_INLINE
std::string get_file_full_path(const char* file_name, const char* rel_path /* = "bin" */)
{
	char full_path[MAX_PATH]; memset(full_path, 0, MAX_PATH);
	return get_file_full_path(full_path, file_name, rel_path);
}

// ��õ�ǰģ��ľ��
#ifdef WIN32
#if _MSC_VER >= 1300    // for VC 7.0 and 8.0
# ifndef _delayimp_h
   extern "C" IMAGE_DOS_HEADER __ImageBase;
# endif
#endif // _MSC_VER >= 1300
#endif // WIN32

SP_INLINE
IU_HANDLE mod_handle(void)
{
#ifdef WIN32
# if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)
	MEMORY_BASIC_INFORMATION mbi;
	static int dummy;
	VirtualQuery( &dummy, &mbi, sizeof(mbi) );
	return reinterpret_cast<HMODULE>(mbi.AllocationBase);
# else    // VC 7.0 or later
	return reinterpret_cast<HMODULE>(&__ImageBase);
# endif
#else // LINUX
	return 0;
#endif // WIN32
}

SP_INLINE
std::string parent_path(const char * path)
{
	std::string cdir;
	if ( path == 0 || strcmp(path, ".") == 0 )
		cdir = get_bin_path();
	else
		cdir = pathname_to_path(path);

	int len = (int)cdir.length() - 1;
	if ( len <= 0 )
		return path;

	for ( int i = len - 1; i >= 0; i-- )
	{
		if ( cdir.at(i) == PATH_CHR )
		{
			cdir.resize(i);
			cdir += PATH_STR;
			break;
		}
	}
	return cdir;
}

SP_INLINE
int system(const char * cmd)
{
	return ::system(cmd);
}

SP_INLINE
int system(const wchar_t * cmd)
{
#ifdef WIN32
	return _wsystem(cmd);
#else  // Linux
	return ::system(wchar2char(cmd).c_str());
#endif // Linux 
}

SP_INLINE
const char* get_file_version(const char* file, char* version)
{
	version[0] = 0;
#ifdef WIN32
	std::string ver = get_file_strinfo(
		"FileVersion", file);
	IU_ASSERT(ver.length() < 30);
	
	char* p = version;
	for ( int i = 0; ; i++ )
	{
		char c = ver.c_str()[i];
		if ( c == '\x20' )
			continue;
		if ( c == ',' )
			c = '.';
		if ( ( *p++ = c ) == 0 )
			break;
	}
#endif
	return version;
}

SP_INLINE
std::string get_file_version(const char* file)
{
	char ver[200]; ver[0] = 0;
	return get_file_version(file, ver);
}

SP_INLINE
const char* get_prod_version(const char* file, char* ver)
{
	ver[0] = 0;
#ifdef WIN32
	std::string vers = get_file_strinfo(
		"ProductVersion", file);
	IU_ASSERT(vers.length() < 30);

	char* p = ver;
	for ( int i = 0; ; i++ )
	{
		char c = vers.c_str()[i];
		if ( c == '\x20' )
			continue;
		if ( c == ',' )
			c = '.';
		if ( ( *p++ = c ) == 0 )
			break;
	}
#endif
	return ver;
}

SP_INLINE
std::string get_prod_version(const char* file)
{
	char ver[200]; ver[0] = 0;
	return get_prod_version(file, ver);
}

// ԭ�����ڲ�ʹ��
SP_INLINE
bool match_string_i(char * pattern, const char * str)
{
	if ( !pattern || !str )
		return false;

	char * ps1 = pattern;
	char const * ps2 = str;

	while ( *ps1 != '\0' && *ps2 != '\0' )
	{
		switch ( *ps1 )
		{
		case '?':
			ps1++;
			ps2++;
			break;
		case '*':
			{
				// find existence of next block
				if ( 0 == *++ps1 )
					return true;

				char *ps = strchr(ps1, '*');
				char *pq = strchr(ps1, '?');
				if ( ps )
					*ps = '\0';
				if ( pq )
					*pq = '\0';

				char const * pstr = strstr(ps2, ps1);
				if ( ps )
					*ps = '*';
				if ( pq )
					*pq = '?';

				if ( pstr )
				{
					ps2 = pstr;
					return match_string_i(ps1, ps2);
				}
				return false;
				break;
			}
		default:
			if ( *ps1++ != *ps2++ )
				return false;
			break;
		}
	}
	// end ?
	if ( *ps1 == '\0' && *ps2 == '\0' )
		return true;
	else
		return false;
}

// ʹ��ͨ�������ƥ��
SP_INLINE
bool match_string(const char * pattern, const char * str)
{
	if ( !pattern || !str )
		return false;

	bool free_ppat = false;
	char pat[300]; pat[0] = 0;
	char * ppat = pat;
	size_t len = strlen(pattern);

	// malloc if pattern too long
	if ( sizeof pat < len )
	{
		free_ppat = true;
		ppat = new char[len];
	}
	strcpy(ppat, pattern);

	// call match safe
	bool ret = match_string_i(ppat, str);

	if ( free_ppat )
		delete [] ppat;

	return ret;
}

// ���һ��Ŀ¼�µ��ļ��б�
template<class LIST>
SP_INLINE
int get_file_list(const char* dir, LIST & flist, const char* filter /* = 0 */, bool recursive /* = true */, bool attr_hide/* = false*/)
{
	return get_file_list(dir, flist, filter, recursive, ff_proc_default());
}

template<class LIST, class FF_PROC>
SP_INLINE
int get_file_list(const char* dir, LIST & flist, const char* filter /* = 0 */, bool recursive /* = true */, FF_PROC fproc /* = 0 */, bool attr_hide/* = false*/)
{
	int ret = 0;
	char flt[MAX_PATH] = "*.*";

#ifdef WIN32
	intptr_t	find_handle;
	_finddata_t	find_data;
	char		find_file[MAX_PATH]; memset(find_file, 0, MAX_PATH);

	sprintf(find_file, "%s/%s", dir, flt);
	find_handle = _findfirst(find_file, &find_data);
	if ( -1 == find_handle )
		return 0;

	// get all file
	do
	{
		// skip . & ..
		if ( !strcmp(find_data.name, ".")
			|| !strcmp(find_data.name, "..") )
			continue;

		// hide file	
		if (!attr_hide && (find_data.attrib & _A_HIDDEN))
			continue ;
			
		// recursive find ?
		if ( find_data.attrib & _A_SUBDIR )
		{
			if ( recursive )
			{
				char find_path[MAX_PATH]; memset(find_path, 0, MAX_PATH);
				sprintf(find_path, "%s/%s", dir, find_data.name);
				normalize_path(find_path);
				ret = get_file_list(find_path, flist, filter, recursive, fproc);
				if ( ret != 0 )
					break;
			}
			continue;
		}

		// match mode
		if ( filter != 0 && !match_string(filter, find_data.name) )
			continue ;

		// push to list
		sprintf(find_file, "%s/%s", dir, find_data.name);
		normalize_path(find_file);
		if ( fproc(find_file) )
			flist.push_back(find_file);

	} while ( -1 != _findnext(find_handle, &find_data) );

	_findclose(find_handle);
	return ret;

#else  // Linux

	char find_file[MAX_PATH]; memset(find_file, 0, MAX_PATH);
	DIR	* pdr = opendir(dir);
	if ( 0 == pdr )
		return errno;
	
	for ( ; ; )
	{
		dirent *pde = readdir(pdr);
		if ( 0 == pde )
			break;

		// skip . & ..
		if ( !strcmp(pde->d_name, ".")
			|| !strcmp(pde->d_name, "..") )
			continue;
		
		sprintf(find_file, "%s/%s", dir, pde->d_name);
		normalize_path(find_file);
		
		struct stat	st;
		ret = stat(find_file, &st);
		if ( 0 != ret )
			break;
		
		if ( st.st_mode & S_IFDIR )
		{
			if ( recursive )
			{
				ret = get_file_list(find_file, flist, filter, recursive, fproc);
				if ( ret != 0 )
					break;
			}
			continue ;
		}
		else
		{
			if ( filter != 0 && !match_string(filter, pde->d_name) )
				continue;
		}

		if ( fproc(find_file) )
			flist.push_back(find_file);
	}

	closedir(pdr);
	return ret;

#endif /* #ifdef WIN32 */
}

// ���һ��Ŀ¼�µ�������Ŀ¼�б�
template<class LIST>
SP_INLINE
int get_dir_list (const char* dir, LIST & dlist, const char* filter /* = 0 */, bool recursive /* = true */, bool attr_hide /* = false*/)
{
	return get_dir_list(dir, dlist, filter, recursive, ff_proc_default());
}

template<class LIST, class FF_PROC>
SP_INLINE
int get_dir_list (const char* dir, LIST & dlist, const char* filter /* = 0 */, bool recursive /* = true */, FF_PROC fproc /* = 0 */, bool attr_hide /* = false*/)
{
	int ret = 0;
	char		flt[MAX_PATH] = "*.*";

#ifdef WIN32
	intptr_t	find_handle;
	_finddata_t	find_data;
	char		find_file[MAX_PATH]; memset(find_file, 0, MAX_PATH);

	sprintf(find_file, "%s/%s", dir, flt);
	find_handle = _findfirst(find_file, &find_data);
	if ( -1 == find_handle )
		return 0;

	// get all file
	dlist.push_back(dir);
	do
	{
		// skip . & ..
		if ( !strcmp(find_data.name, ".")
			|| !strcmp(find_data.name, "..") )
			continue;

		// hide file	
		if (!attr_hide && (find_data.attrib & _A_HIDDEN))
			continue ;
		
		// recursive find ?
		if ( ( find_data.attrib & _A_SUBDIR ) == 0 )
			continue ;
		
		// match mode
		if ( filter != 0 && !match_string(filter, find_data.name) )
			continue ;

		// push to list
		sprintf(find_file, "%s/%s", dir, find_data.name);
		normalize_path(find_file);
		if ( !recursive && fproc(find_file) )
			dlist.push_back(find_file);

		if ( recursive )
		{
			char find_path[MAX_PATH]; memset(find_path, 0, MAX_PATH);
			sprintf(find_path, "%s/%s", dir, find_data.name);
			normalize_path(find_path);
			ret = get_dir_list(find_path, dlist, filter, recursive, fproc);
			if ( ret != 0 )
				break;
		}
	} while ( -1 != _findnext(find_handle, &find_data) );

	_findclose(find_handle);
	return ret;

#else  // Linux

	char find_file[MAX_PATH]; memset(find_file, 0, MAX_PATH);
	DIR	* pdr = opendir(dir);
	if ( 0 == pdr )
		return errno;
	dlist.push_back(dir);

	for ( ; ; )
	{
		dirent *pde = readdir(pdr);
		if ( 0 == pde )
			break;

		// skip . & ..
		if ( !strcmp(pde->d_name, ".")
			|| !strcmp(pde->d_name, "..") )
			continue;

		if ( filter != 0 && !match_string(filter, pde->d_name ) )
			continue;

		sprintf(find_file, "%s/%s", dir, pde->d_name);
		normalize_path(find_file);

		struct stat	st;
		ret = stat(find_file, &st);
		if ( 0 != ret )
			break;

		if ( ( st.st_mode & S_IFDIR ) == 0 )
			continue ;

		if ( !recursive && fproc(find_file) )
			dlist.push_back(find_file);

		if ( recursive )
		{
			ret = get_dir_list(find_file, dlist, filter, recursive, fproc);
			if ( ret != 0 )
				break;
			continue ;
		}
	}

	closedir(pdr);
	return ret;

#endif /* #ifdef WIN32 */
}

SP_INLINE
int cat_audio_file(const char * file, const void * data, size_t bytes, bool write_head/* = true*/, bool fail_if_exist /* = false */)
{
	size_t	file_size, audio_head_len = 0, write_data, file_head_len = 0;
	FILE*	fp = NULL;

	if ( fail_if_exist && sp::is_file_exist(file) )
		return -1;
		
	/* ���û���������ݣ�ֱ�ӷ��� */
	if ( !data || bytes <= 0 )
	{
		return 0;
	}

	/* ��д����ʽ���ļ� */
	fp = fopen(file, "r+b");
	if ( fp == 0 )
		fp = fopen(file, "wb");
	if ( fp == 0 )
		return 2 /* ERROR_FILE_NOT_FOUND */;

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);


	/* �ж�����������ͷ���ݵĳ��ȣ������Ը���������ͷ������Ϊ�����ļ���ͷ���� */
	if ( memcmp(data, "RIFF", 4) == 0 )
	{
		if ( memcmp((char*)data + 44 - 8, "data", 4) == 0 )
			audio_head_len = 44;
		else if ( memcmp((char*)data + 58 - 8, "data", 4) == 0 )
			audio_head_len = 58;
	}

	/* ����ǵ�һ��д�����ݣ���ô��Ҫ��ͷд��ȥ */
	if ( file_size == 0 && audio_head_len > 0 )
	{
		fwrite((char*)data, audio_head_len, 1, fp);
		file_size += audio_head_len;
	}

	/* ����������ȥ������ͷ֮��ƴ�ӵ��ļ���ĩβ */
	fwrite((char*)data + audio_head_len, 
		bytes - audio_head_len, 1, fp);
	file_size += bytes - audio_head_len;

	// ����ļ���û��ͷ
	if ( write_head && audio_head_len == 0 && file_size >= 44 )
	{
		char file_head[58];
		memset(file_head, 0, sizeof file_head);

		fseek(fp, 0, SEEK_SET);
		size_t read_len = fread(file_head, 1, sizeof file_head, fp);

		if ( memcmp(file_head, "RIFF", 4) == 0 )
		{
			if ( memcmp((char*)file_head + 44 - 8, "data", 4) == 0 )
				file_head_len = 44;
			else if ( memcmp((char*)file_head + 58 - 8, "data", 4) == 0 )
				file_head_len = 58;
		}
		audio_head_len = file_head_len;
	}

	/* ����޸������ļ�������ͷ��Ϣ */
	if ( audio_head_len > 0 )
	{
		write_data = file_size - 8;
		fseek(fp, 4, SEEK_SET);
		fwrite(&write_data, sizeof(write_data), 1, fp);
		write_data = file_size - audio_head_len;
		fseek(fp, (int)audio_head_len - 4, SEEK_SET);
		fwrite(&write_data, sizeof(write_data), 1, fp);
		if ( audio_head_len == 58 )
		{
			fseek(fp, (int)audio_head_len - 12, SEEK_SET);
			fwrite(&write_data, sizeof(write_data), 1, fp);
		}
	}
	fclose(fp);

	return 0;
}

SP_INLINE
int read_text_file(const char * file, std::string & text)
{
	text.clear();

	std::ifstream fs(file);
	if ( fs )
	{
		unsigned int fsize = 0;
		sp::get_file_size(file, fsize);
		text.reserve(fsize);

		#ifdef WIN32
		fs.imbue(std::locale(""));
		#endif // WIN32
		while ( !fs.eof() )
		{
			char cnt[1000]; cnt[0] = 0;
			fs.get(cnt, sizeof cnt, -1);
			if ( fs.fail() )
				break;
			text += cnt;
		}
		return 0;
	}

	return last_error();
}

SP_INLINE
int read_text_file(const char * file, std::wstring & text)
{
	text.clear();

	std::wifstream fs(file);
	if ( fs )
	{
		unsigned int fsize = 0;
		sp::get_file_size(file, fsize);
		text.reserve(fsize);

		#ifdef WIN32
		fs.imbue(std::locale(".ACP"));
		#endif // WIN32
		while ( !fs.eof() )
		{
			wchar_t cnt[1000]; cnt[0] = 0;
			fs.get(cnt, sizeof cnt, -1);
			if ( fs.fail() )
				break;
			text += cnt;
		}
		return 0;
	}

	return last_error();
}

SP_INLINE
int read_bin_file (const char * file, void * data, size_t bytes, size_t * readed /* = 0 */)
{
	FILE * fp = fopen(file, "rb");
	if ( fp != 0 )
	{
		size_t rs = fread(data, 1, bytes, fp);
		if ( readed )
			*readed = rs;
		fclose(fp);
		return 0;
	}

	return last_error();
}

SP_INLINE
int write_text_file(const char * file, const std::string & text, bool fail_if_exist /* = false */, bool append /* = false */)
{
	std::ios::openmode mode = std::ios::out;
#ifdef WIN32
	if ( fail_if_exist )
		mode |= std::ios::_Noreplace;
#endif // WIN32
	if ( append )
		mode |= std::ios::app;

	std::ofstream fs(file, mode);
	if ( fs )
	{
		fs.write(text.c_str(), (std::streamsize)text.size());
		fs.close();
		return 0;
	}

	return last_error();
}

SP_INLINE
int write_text_file(const char * file, const std::wstring & text, bool fail_if_exist /* = false */, bool append /* = false */)
{
	std::ios::openmode mode = std::ios::out;
#ifdef WIN32
	if ( fail_if_exist )
		mode |= std::ios::_Noreplace;
#endif // WIN32
	if ( append )
		mode |= std::ios::app;

	std::wofstream fs(file, mode);
	if ( fs )
	{
		fs.write(text.c_str(), (std::streamsize)text.size());
		fs.close();
		return 0;
	}

	return last_error();
}

SP_INLINE
int write_bin_file(const char* file, const void * data, size_t bytes, size_t * written /* = 0 */, bool fail_if_exist /* = false */, bool append /* = false */)
{
	const char * mode = "wb";
	if ( fail_if_exist )
		mode = "r+b";
	if ( append )
		mode = "ab";

	FILE * fp = fopen(file, mode);
	if ( fp != 0 )
	{
		size_t ws = fwrite(data, 1, bytes, fp);
		if ( written )
			*written = ws;
		fclose(fp);
		return 0;
	}

	return last_error();
}

SP_INLINE
int last_error(void)
{
#if defined(WIN32)
	return ::GetLastError();
#else	// nonwin
	return errno;
#endif	// nonwin
}

// ��ý���ID
SP_INLINE
int getpid(void)
{
#if defined (WIN32)
	return ::GetCurrentProcessId ();
#elif defined (ACE_ACE_H)
	return ACE_OS::getpid();
#else
	return ::getpid ();
#endif /* WIN32 */
}

// ��õ�ǰ�߳�id
SP_INLINE
#ifndef __IPHONE_7_0
int thr_self(void)
#else
pthread_t thr_self(void)
#endif
{
#if defined (WIN32)
	return ::GetCurrentThreadId();
#elif defined (ACE_ACE_H)
	return ACE_OS::thr_self();
#else
	return ::pthread_self();
#endif /* WIN32 */
}

SP_INLINE
bool process_active (int pid)
{
#if defined (WIN32)
	// Create a handle for the given process id.
	HANDLE process_handle = ::OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (process_handle == INVALID_HANDLE_VALUE || process_handle == 0)
		return false;
	else
    {
      DWORD status;
      bool result = true;
      if ( ::GetExitCodeProcess (process_handle, &status) == 0
          || status != STILL_ACTIVE)
        result = false;

      ::CloseHandle (process_handle);
      return result;
    }
#elif defined (ACE_ACE_H)
	int retval = ACE_OS::kill (pid, 0);
	if (retval == 0)
		return true;
	else if (errno == ESRCH)
		return false;
	else
		return true;
#else
	return false;
#endif /* !ACE_WIN32 */
}

SP_INLINE
char* get_module_cfg(char path[MAX_PATH], const char* cfg_default, IU_HANDLE hmod)
{
	IU_HANDLE hdl = hmod;
	if ( hdl == IU_INVALID_HANDLE || hdl == 0 )
		hdl = sp::mod_handle();

	char dcfg[MAX_PATH]; memset(dcfg, 0, MAX_PATH);
	if ( cfg_default == 0 )
		cfg_default = get_bin_name(dcfg, hdl);

	if ( !is_file_exist(cfg_default) )
	{
		if ( is_file_exist(cat_path(get_bin_path(path, hdl), cfg_default)) )
			return path;
	}

	return strsncpy(path, cfg_default, MAX_PATH);
}

SP_INLINE
std::string get_module_cfg(const char * cfg_default, IU_HANDLE hmod)
{
	char path[MAX_PATH]; memset(path, 0, MAX_PATH);
	return get_module_cfg(path, cfg_default, hmod);
}

SP_INLINE
std::string get_cfg( const char * cfg_default )
{
	std::string cur_dir, cfg_path, cfg_name;
	int pos = 0;
	cur_dir = sp::get_bin_path(sp::mod_handle());

	cfg_path = cur_dir;
	
	if(!cfg_default)
	{
		cfg_name = get_bin_name(sp::mod_handle());
		pos = (int)cfg_name.rfind('.');
		if( pos > 0 )
		{
#ifdef _DEBUG
			if( cfg_name[pos-1] == 'd' )pos = pos - 1;
			if( pos > 0 )  cfg_name.assign(cfg_name,0,pos);
#else
			cfg_name.assign(cfg_name,0,pos);		
#endif
		}

		cfg_name += ".cfg";
	}
	else
	{
		cfg_name = cfg_default;
	}
	cfg_path += cfg_name;
	if( !is_file_exist(cfg_path.c_str()) )
	{
		cfg_path = parent_path(cur_dir.c_str());
		cfg_path += "conf/";
		cfg_path += cfg_name;
	}

	return cfg_path;
}
} /* namespace sp for file process */
#endif /* __SPUTILS_I__ */