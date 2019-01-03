/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: http_settings.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/25
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/25	<td>minglu	<td>Create head file </tr>
* </table>
*****************************************************************/
#include "http_settings.h"
#include <boost/algorithm/string/case_conv.hpp>



size_t get_end_pos(const char *src, size_t maxlen)
{
	const char *p = NULL;

	p = (const char *)memchr(src, '\0', maxlen);
	if (p == NULL)
		return maxlen;

	return p - src;
}

size_t strlncat(char *dst, size_t dst_len, const char *src, size_t len)
{
	size_t src_len = 0;
	size_t ncpy = 0;

	src_len = get_end_pos(src, len);

	if (dst_len >= src_len) {
		memcpy(dst, src, src_len);
		dst[src_len] = '\0';
	}

	//assert(len > slen + dlen);
	return src_len ;
}

int message_begin_cb(http_parserc * hparser)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->message_begin_cb_called_ = 1;
	return 0;
}

int request_url_cb(http_parserc *hparser, const char *buf, size_t len)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->http_url_.append(buf,len);
	return 0;
}

int status_cb(http_parserc *hparser)
{
	return 0;
}

int header_field_cb(http_parserc *hparser, const char *buf, size_t len)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	std::string field_value = std::string(buf, len);
	boost::algorithm::to_lower(field_value);
	parser->http_header_.field.append(field_value);
	parser->http_header_.field_last = parser->http_header_.field;
	parser->last_header_element_ = FIELD;

	return 0;
}

int header_value_cb(http_parserc *hparser, const char *buf, size_t len)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->http_header_.field= parser->http_header_.field_last;

	std::string header_value(buf,len);
	std::map<std::string, std::string>::iterator iter = parser->http_header_.header_field_value.find(parser->http_header_.field);
	if(iter == parser->http_header_.header_field_value.end()){
		parser->http_header_.header_field_value.insert(std::pair<std::string, std::string>(parser->http_header_.field, header_value));
	}
	else{
		iter->second.append(header_value);
	}
	parser->http_header_.field.clear();
	parser->last_header_element_ = VALUE;
	return 0;
}

int headers_complete_cb(http_parserc *hparser)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->http_method_ = http_method_str(http_method(hparser->method));
	parser->http_major_ = hparser->http_major;
	parser->http_minor_ = hparser->http_minor;
	parser->headers_complete_cb_called_ = true;
	parser->should_keep_alive_ = http_should_keep_alive(hparser);
	return 0;
}

int body_cb(http_parserc *hparser, const char *buf, size_t len)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->body_size_ += len;
	parser->http_body_.append(buf, len);
	parser->body_is_final_ = http_body_is_final(hparser);
	return 0;
}

int message_complete_cb(http_parserc *hparser)
{
	deal_http_msg *parser = (deal_http_msg *)hparser->reserved;
	parser->message_complete_cb_called_ = true;
	parser->message_complete_ = true;
	return 0;
}

http_parser_settings settings =
{
	message_begin_cb,    //on_message_begin
	request_url_cb,      //on_url
	status_cb,           //on_status_complete
	header_field_cb,     //on_header_field
	header_value_cb,     //on_header_value
	headers_complete_cb, //on_headers_complete
	body_cb,             //on_body
	message_complete_cb  //on_message_complete
};
