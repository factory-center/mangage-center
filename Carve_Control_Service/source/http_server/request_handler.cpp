//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <json/json.h>
#include "../busin_log.h"
#include "utils/msp_errors.h"
#include "../CCarve.h"
#include "../carve_manager.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("request_handler::") + std::string(__FUNCTION__)).c_str()) 
#endif

namespace http 
{
	namespace server3 
	{

		request_handler::request_handler(const std::string& doc_root)
			: doc_root_(doc_root)
		{
		}

		void request_handler::handle_request(const request& req, reply& rep)
		{
			printf("%s | line:%d\n", __FUNCTION__, __LINE__);
			// Decode url to path.
			std::string request_path;
			if (!url_decode(req.uri, request_path))
			{
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			// Request path must be absolute and not contain "..".
			if (request_path.empty() || request_path[0] != '/'
				|| request_path.find("..") != std::string::npos)
			{
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			// If path ends in slash (i.e. is a directory) then add "index.html".
			if (request_path[request_path.size() - 1] == '/')
			{
				request_path += "index.html";
			}

			// Determine the file extension.
			std::size_t last_slash_pos = request_path.find_last_of("/");
			std::size_t last_dot_pos = request_path.find_last_of(".");
			std::string extension;
			if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
			{
				extension = request_path.substr(last_dot_pos + 1);
			}

			// Open the file to send back.
			std::string full_path = doc_root_ + request_path;
			std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
			if (!is)
			{
				rep = reply::stock_reply(reply::not_found);
				return;
			}

			// Fill out the reply to be sent to the client.
			rep.status = reply::ok;
			char buf[512];
			while (is.read(buf, sizeof(buf)).gcount() > 0)
				rep.content.append(buf, is.gcount());
			rep.headers.resize(2);
			rep.headers[0].name = "Content-Length";
			rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
			rep.headers[1].name = "Content-Type";
			rep.headers[1].value = mime_types::extension_to_type(extension);
		}

		void request_handler::handle_request(const request& req, const std::string& str_json_body, reply& rep)
		{
			//如果为非法json，则返回出错返回
			//TODO::统一返回
			Json::Reader reader;
			Json::Value root;
			int ret = 0;
			std::string str_err_reason;
			Json::Value json_result;
			if (!reader.parse(str_json_body, root))
			{//非法json体
				ret = MSP_ERROR_INVALID_DATA;
				str_err_reason = std::string("The body in http is invalid json. body:") + str_json_body;
				rep = reply::construct_message(ret, "", str_err_reason);
				businlog_error("%s | body:%s, body is invalid json.", __CLASS_FUNCTION__, str_json_body.c_str());
				return;
			}
			std::string str_key = "command";
			if (!root.isMember(str_key))
			{
				ret = MSP_ERROR_INVALID_PARA;
				str_err_reason = std::string("body:") + str_json_body + std::string(", json without key:") + str_key;
				rep = reply::construct_message(ret, "", str_err_reason);
				businlog_error("%s | body:%s without key:%s", __CLASS_FUNCTION__, str_json_body.c_str(), str_key.c_str());
				return;
			}
			// Fill out the reply to be sent to the client.
			const std::string str_cmd = root[str_key].asString();
			//根据不同的命令来响应
			if ("connect" == str_cmd)
			{//连接雕刻机
				ret = on_connect(root, json_result, str_err_reason);
			}
			else if ("query_one_machine_status" == str_cmd)
			{
			}
			rep = reply::construct_message(ret, json_result.toStyledString(), str_err_reason);
		}
		bool request_handler::url_decode(const std::string& in, std::string& out)
		{
			out.clear();
			out.reserve(in.size());
			for (std::size_t i = 0; i < in.size(); ++i)
			{
				if (in[i] == '%')
				{
					if (i + 3 <= in.size())
					{
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));
						if (is >> std::hex >> value)
						{
							out += static_cast<char>(value);
							i += 2;
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else if (in[i] == '+')
				{
					out += ' ';
				}
				else
				{
					out += in[i];
				}
			}
			return true;
		}

		int request_handler::on_connect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//遍历信息数组
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//获取单个雕刻机信息
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//连接雕刻机
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//构造结果
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_err_reason_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
				if (json_single_params.isMember(CCarve::ms_str_carve_id_key))
				{
					json_single_resp[CCarve::ms_str_carve_id_key] = json_single_params[CCarve::ms_str_carve_id_key];
				}
				else
				{
					json_single_resp[CCarve::ms_str_carve_id_key] = Json::Value();
				}
				//将单个结果添加到结果数组中
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}

	} // namespace server3
} // namespace http
