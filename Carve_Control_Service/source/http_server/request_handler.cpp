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
			//���Ϊ�Ƿ�json���򷵻س�����
			//TODO::ͳһ����
			Json::Reader reader;
			Json::Value root;
			int ret = 0;
			std::string str_err_reason;
			Json::Value json_result;
			if (!reader.parse(str_json_body, root))
			{//�Ƿ�json��
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
			//���ݲ�ͬ����������Ӧ
			if ("connect" == str_cmd)
			{
				//���ӵ�̻�begin
				ret = http_on_connect(root, json_result, str_err_reason);
			}
			else if ("disconnect" == str_cmd)
			{
				//�Ͽ���̻�
				ret = http_disconnect(root, json_result, str_err_reason);
			}
			else if ("query_all_machines_status" == str_cmd)
			{
				//��ѯ���е�̻�״̬
				ret = http_query_all_machines_status(root, json_result, str_err_reason);
			}
			else if ("query_one_machine_status" == str_cmd)
			{
				//��ѯһ����̻�״̬
				ret = http_query_one_machine_status(root, json_result, str_err_reason);
			}
			else if ("download_gcode_OK" == str_cmd)
			{
				//֪ͨGCode�ļ��������
				ret = http_download_gcode_OK(root, json_result, str_err_reason);
			}
			else if ("emergency_stop_one" == str_cmd)
			{
				//֪ͨһ̨��̻���ͣ
				ret = http_emergency_stop_one(root, json_result, str_err_reason);
			}
			else if ("emergency_stop_all" == str_cmd)
			{
				//֪ͨȫ����̻���ͣ
				ret = http_emergency_stop_all(root, json_result, str_err_reason);
			}
			else if ("reboot_one" == str_cmd)
			{
				//֪ͨһ̨��̻�����
				ret = http_reboot_one(root, json_result, str_err_reason);
			}
			else if ("reboot_all" == str_cmd)
			{
				//֪ͨȫ����̻�����
				ret = http_reboot_all(root, json_result, str_err_reason);
			}
			else if ("adjust_speed" == str_cmd)
			{
				//������̻������ٶ�
				ret = http_adjust_speed(root, json_result, str_err_reason);
			}
			else if ("start" == str_cmd)
			{
				//��ʼ���
				ret = http_start(root, json_result, str_err_reason);
			}
			else if("query_one_machine_info" == str_cmd)
			{
				
			}
			else if("query_all_machine_info" == str_cmd)
			{

			}
			else
			{
				//����ֵ����
				ret = MSP_ERROR_INVALID_PARA;
				str_err_reason = str_key +  string(":") + str_cmd + string(" is invalid.");
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

		int request_handler::http_on_connect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			//һ�����߶��
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_query_all_machines_status(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_query_one_machine_status(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_download_gcode_OK(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//����G����
			ret = CCarve_Manager::instance()->download_gcode_OK(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);

			return MSP_SUCCESS;
		}
		int request_handler::http_emergency_stop_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			//��ȡ������̻���Ϣ
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//֪ͨһ̨��̻���ͣ
			ret = CCarve_Manager::instance()->emergency_stop_one(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);

			return MSP_SUCCESS;
		}
		int request_handler::http_emergency_stop_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//���ӵ�̻�
			ret = CCarve_Manager::instance()->emergency_stop_all(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			Json::Value json_single_resp;
			//������
			json_single_resp["ret"] = ret;
			json_single_resp["errmsg"] = str_err_reason_for_debug;
			json_single_resp["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);

			//�����������ӵ����������
			json_result["results"].append(json_single_resp);
			str_err_reason += string(". ") + str_err_reason_for_debug;

			return MSP_SUCCESS;
		}
		int request_handler::http_reboot_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_reboot_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_adjust_speed(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//���ӵ�̻�
				ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_disconnect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			//all �������
			string str_key = "carveInfo";
			businlog_error_return_err_reason(json_root.isMember(str_key), __CLASS_FUNCTION__ << " | json:" << json_root.toStyledString() << ", without key:" << str_key
				, str_err_reason, MSP_ERROR_INVALID_PARA);

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻���Ϣ
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//�Ͽ���̻�
				ret = CCarve_Manager::instance()->disconnect(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
				Json::Value json_single_resp;
				//������
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
				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				str_err_reason += string(". ") + str_err_reason_for_debug;
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::http_start(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//���ӵ�̻�
			ret = CCarve_Manager::instance()->start(json_root, str_err_reason_for_debug, str_err_reason_for_user);

			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			
			return MSP_SUCCESS;
		}

		int request_handler::on_query_one_carve_info(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//��ȡ��̻���Ϣ
			SCarve_Info carve_info;
			ret = CCarve_Manager::instance()->get_carve_info(json_root, carve_info, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			json_result["taskNo"] = carve_info.str_task_no;
			json_result["machine_ip"] = carve_info.str_machine_ip;
			json_result["currentStatus"] = carve_info.eCarve_status;
			json_result["worktime"] = carve_info.nTotal_engraving_time;
			json_result["gNo"] = carve_info.str_gCode_no;
			json_result["rowNo"] = carve_info.nCurrent_line_num;
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			str_err_reason =  str_err_reason_for_debug;
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}
	} // namespace server3
} // namespace http
