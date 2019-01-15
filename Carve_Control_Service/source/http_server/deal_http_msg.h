/** 
* @file	deal_http_msg.h
* @brief   解析http消息
* 
* 
* 
* @author	fanli
* @version	1.0
* @date	2013年9月2日
* 
* @see		
* 
* @par 版本记录：
* <table border=1>
*  <tr> <th>版本	<th>日期			<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	<td>2013年9月2日	<td>fanli	<td>创建 </tr>
* </table>
*/

#ifndef __DEAL_HTTP_MSG_H__
#define __DEAL_HTTP_MSG_H__

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "http_parser.h"
#include <boost/smart_ptr/shared_ptr.hpp>

#define MAX_HEADERS 13
enum header_element{ NONE=0, FIELD, VALUE };

struct header
{
	std::string field;
	std::string field_last;
	std::map<std::string, std::string>	header_field_value;
};


class deal_http_msg
{
public:
	deal_http_msg();
	~deal_http_msg();

	std::string  get_http_method(int *ret = NULL);
	std::string  get_http_header(int *ret = NULL);
	std::string  get_header_value(const std::string & header_name, int *ret = NULL);
	std::string  get_http_url(int *ret = NULL);
	std::string  get_http_body(unsigned long *val_len, int *ret = NULL);
	std::string  get_http_version(unsigned long *val_len, int *ret = NULL);
	int parse_msg(enum http_parser_type type, const void *msg, unsigned long msg_len,  bool& bIs_full_msg);	
	int reset();
	boost::shared_ptr<http_parserc> http_parser_;
	std::string http_url_;
	std::string http_version_;
	std::string http_method_;
	std::string http_body_;
	int should_keep_alive_;
	header http_header_;


	header_element last_header_element_;
	int message_begin_cb_called_;
	int headers_complete_cb_called_;
	int message_complete_cb_called_;
	int message_complete_on_eof_;
	int body_is_final_; //是否为消息体的最后一块
	bool message_complete_;
	int currently_parsing_eof_;

	size_t body_size_;

	unsigned short http_major_;
	unsigned short http_minor_;

	// 键-值
	struct attri_value
	{
		std::string attri_;
		std::string val_;
	};
private:
		bool already_init_; //http parser是否已经初始化了
};

#endif