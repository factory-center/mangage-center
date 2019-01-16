#include "deal_http_msg.h"
#include "http_parser.h"
#include "http_settings.h"
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "../busin_log.h"
#include "utils/msp_errors.h"
const char *default_http_response_ = "HTTP/1.1 200 OK\r\n";

deal_http_msg::deal_http_msg() 
	: http_parser_(boost::make_shared<http_parserc>())
	, already_init_(false)
{
}

deal_http_msg::~deal_http_msg()
{
}

std::string deal_http_msg::get_http_method( int *ret /*= NULL*/ )
{
	return http_method_;
}

std::string deal_http_msg::get_http_header( int *ret /*= NULL*/ )
{
	std::map<std::string, std::string>::iterator itor = http_header_.header_field_value.begin();
	std::string header_str = "";
	for( ;itor != http_header_.header_field_value.end();++itor )
	{
		header_str += itor->first + ":" + itor->second + "\r\n";
	} 

	return header_str;
}

std::string deal_http_msg::get_header_value(const std::string & header_name, int *ret /*= NULL*/ )
{
	const std::string str_lwr_header_name = boost::algorithm::to_lower_copy(header_name);

	std::map<std::string, std::string>::iterator itor = http_header_.header_field_value.find(header_name);

	if (itor != http_header_.header_field_value.end())
	{
		return itor->second;
	}

	return "";
}

std::string deal_http_msg::get_http_url( int *ret /*= NULL*/ )
{
	return http_url_;
}


std::string  deal_http_msg::get_http_body(unsigned long *val_len, int *ret /*= NULL*/ )
{
	*val_len = body_size_; 
	return http_body_;
}

std::string deal_http_msg::get_http_version( unsigned long *val_len, int *ret /*= NULL*/ )
{
	return http_version_;
}

int deal_http_msg::parse_msg(enum http_parser_type type, const void *msg, unsigned long msg_len, bool& bIs_full_msg )
{
	if(!already_init_)
	{
		http_parser_init(http_parser_.get(), type);
		already_init_ = true;
	}
	http_parser_->reserved = this;

	size_t nSize_parsed = 0;
	if (msg_len)
	{
		currently_parsing_eof_ = (msg_len == 0);
		//http_parser_settings *settings = (http_parser_settings *)&settings_;
		nSize_parsed = http_parser_execute(http_parser_.get(), &settings, (const char*)msg, msg_len);
		if (message_complete_)
		{
			bIs_full_msg = true;
		}
		//注意：由于如果为不完整的消息，上层会继续读取数据，故这里将下面代码屏蔽
		//在消息为完整的情况下，再判定转换长度与消息原来长度是否一致。如果消息不完整，则不必判定
// 		if (bIs_full_msg == true && nSize_parsed != msg_len)
// 		{
// 			businlog_error("%s | now Message is full, but fail to parser http message, parsed len:%d is not equal msg len:%d."
// 				, __FUNCTION__, nSize_parsed, msg_len);
// 			return MSP_ERROR_MSG_PARSE_ERROR;
// 		}
		if (!bIs_full_msg)
		{//消息不完整，则给出提示信息
			businlog_info("%s | http message is not full.", __FUNCTION__);
		}
	}
	return 0;
}

int deal_http_msg::reset()
{

	http_url_.clear();
	http_version_.clear();
	http_method_.clear();
	http_body_.clear();
	should_keep_alive_ = -1;
	http_header_.header_field_value.clear();
	http_header_.field_last.clear();
	http_header_.field.clear();

	last_header_element_ = NONE;
	message_begin_cb_called_ = -1;
	headers_complete_cb_called_ = -1;
	message_complete_cb_called_ = -1;
	message_complete_on_eof_ = -1;
	body_size_ = 0;
	body_is_final_ = 0;
	message_complete_ = false;
	currently_parsing_eof_ = -1;
	return 0;
}