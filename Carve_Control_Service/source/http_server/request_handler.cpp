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
#include "../source/CSpdLog.h"
#include "utils/msp_errors.h"
#include "utils/sutils.h"
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
			while ((const unsigned int)is.read(buf, sizeof(buf)).gcount() > 0)
			{
				rep.content.append(buf, (const unsigned int)is.gcount());
			}
				
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
			Json::Value json_result; //json��ʽ����Ӧ���
			if (!reader.parse(str_json_body, root))
			{//�Ƿ�json��
				ret = MSP_ERROR_INVALID_DATA;
				str_err_reason = std::string("The body in http is invalid json. body:") + str_json_body;
				rep = reply::construct_message(ret, "", str_err_reason);
				LError("body:{}, body is invalid json.", str_json_body.c_str());
				return;
			}
			std::string str_key = "command";
			if (!root.isMember(str_key))
			{
				ret = MSP_ERROR_INVALID_PARA;
				str_err_reason = std::string("body:") + str_json_body + std::string(", json without key:") + str_key;
				rep = reply::construct_message(ret, "", str_err_reason);
				LError("body:{} without key:{}", str_json_body.c_str(), str_key.c_str());
				return;
			}
			// Fill out the reply to be sent to the client.
			const std::string str_cmd = root[str_key].asString();
			//���ݲ�ͬ����������Ӧ
			if ("connect" == str_cmd)
			{
				//���ӵ�̻�
				ret = on_connect(root, json_result, str_err_reason);
			}
			else if ("disconnect" == str_cmd)
			{
				//�Ͽ���̻�
				ret = on_disconnect(root, json_result, str_err_reason);
			}
			else if ("query_one_machine_status" == str_cmd)
			{
				//��ѯ������̻�״̬
				ret = on_query_one_carve_status(root, json_result, str_err_reason);
			}
			else if ("query_all_machines_status" == str_cmd)
			{
				//��ѯ���е�̻�״̬
				ret = MSP_ERROR_NOT_IMPLEMENT;
			}
			else if("query_one_machine_info" == str_cmd)
			{
				//��ѯ��̨��̻���Ϣ
				ret = on_query_one_carve_info(root, json_result, str_err_reason);
			}
			else if("query_all_machines_info" == str_cmd)
			{
				//��ѯ���е�̻���Ϣ
				ret = on_query_all_carves_info(json_result, json_result, str_err_reason);
			}
			else if ("download_gcode_OK" == str_cmd)
			{
				//����G�����ļ�
				ret = on_upload_file(root, json_result, str_err_reason);
			}
			else if("start" == str_cmd)
			{
				//��ʼ��̨�豸���
				ret = on_start(root, json_result, str_err_reason);
			}
			else if("start_all" == str_cmd)
			{
				//��ʼȫ���豸���
				ret = on_start_all(root, json_result, str_err_reason);
			}
			else if("emergency_stop_one" == str_cmd)
			{
				//֪ͨһ̨��̻���ͣ
				ret = on_emergency_stop_one(root, json_result, str_err_reason);
			}
			else if("pause_one" == str_cmd)
			{
				//֪ͨһ̨��̻���ͣ
				ret = on_pause_one(root, json_result, str_err_reason);
			}
			else if("pause_all" == str_cmd)
			{
				//֪ͨȫ����̻���ͣ
				ret = on_pause_all(root, json_result, str_err_reason);
			}
			else if("emergency_stop_all" == str_cmd)
			{
				//֪ͨȫ����̻���ͣ
				ret = on_emergency_stop_all(root, json_result, str_err_reason);
			}
			else if("cancel_emergency_stop_one" == str_cmd)
			{
				//ȡ��һ̨��̻���ͣ
				ret = on_cancel_emergency_stop_one(root, json_result, str_err_reason);
			}
			else if("cancel_emergency_stop_all" == str_cmd)
			{
				//ȡ��ȫ����̻���ͣ
				ret = on_cancel_emergency_stop_all(root, json_result, str_err_reason);
			}
			else if("delete_gcode_OK" == str_cmd)
			{
				//ɾ��G����
				ret = on_delete_file(root, json_result, str_err_reason);
			}
			else if("adjust_speed" == str_cmd)
			{
				//������̻������ٶ�
				ret = on_adjust_speed(root, json_result, str_err_reason);
			}
			else
			{
				//����ֵ����
				ret = MSP_ERROR_INVALID_PARA;
				str_err_reason = str_key +  string(":") + str_cmd + string(" is invalid.");
			}
			//ͳһ��ӡ���󣬱�����©��ӡ����
			if (ret)
			{
				LError("err reason:{}, ret:{}", str_err_reason.c_str(), ret);
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
			if (!json_root.isMember(str_key))
			{
				LError("json:{}, without key:{}", json_root.toStyledString(), str_key);
				str_err_reason = "json:" + json_root.toStyledString() + ",without key:" + str_key;
				return MSP_ERROR_INVALID_PARA;
			}

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
				//δ��ֹ���ε���ż����ʧ�ܵ������ѭ��3�ξ�ʧ������Ϊʧ��
				for (int nloop_Num =0; nloop_Num<3; ++nloop_Num)
				{
					str_err_reason_for_debug = "";
					str_err_reason_for_user = "";
					ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
					if ( MSP_SUCCESS == ret)
					{
						break;
					}
					boost::this_thread::sleep(boost::posix_time::millisec(300));
				}

				//ret = CCarve_Manager::instance()->connect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
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
				if (ret)
				{//�е����豸�������ۼӴ�����Ϣ
					str_err_reason += string(". ") + str_err_reason_for_debug;
				}
			}//end for
			return MSP_SUCCESS;
		}
		int request_handler::on_disconnect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			string str_key = "carveInfo";
			if (!json_root.isMember(str_key))
			{
				LError("json:{}, without key:{}", json_root.toStyledString(), str_key);
				str_err_reason = "json:" + json_root.toStyledString() + ",without key:" + str_key;
				return MSP_ERROR_INVALID_PARA;
			}

			const Json::Value& json_arr_carveInfo = json_root[str_key];
			int ret = 0;

			//������Ϣ����
			for (int i = 0; i != json_arr_carveInfo.size(); ++i)
			{
				//��ȡ������̻��豸���
				const Json::Value& json_single_params = json_arr_carveInfo[i];
				std::string str_err_reason_for_debug;
				std::string str_err_reason_for_user;
				//�Ͽ���̻�
				ret = CCarve_Manager::instance()->disconnect_carve(json_single_params, str_err_reason_for_debug, str_err_reason_for_user);
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
				if (ret)
				{
					str_err_reason += string(". ") + str_err_reason_for_debug;
				}
			}//end for
			return MSP_SUCCESS;
		}

		int request_handler::on_query_one_carve_status(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//��ȡ��̻�״̬
			ECARVE_STATUS_TYPE eCarve_common_status = CARVE_STATUS_MIN;
			ret = CCarve_Manager::instance()->get_carve_status(json_root, eCarve_common_status, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			json_result["currentStatus"] = eCarve_common_status;
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
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
			json_result["worktime"] = (int)carve_info.nTotal_engraving_time;
			json_result["gno"] = carve_info.str_gCode_no;
			json_result["rowNo"] = carve_info.nCurrent_line_num;
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{
				str_err_reason =  str_err_reason_for_debug;
			}

			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}

		int request_handler::on_start(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//������̻�
			ret = CCarve_Manager::instance()->start_engraving(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}
		int request_handler::on_start_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//��ʼȫ�����
			ret = CCarve_Manager::instance()->start_all_engraving(json_root,json_result,str_err_reason_for_debug, str_err_reason_for_user);
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ��
			return MSP_SUCCESS;
		}

		int request_handler::on_upload_file(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//�ϴ��ļ�
			ret = CCarve_Manager::instance()->upload_1_file(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}
		
		int request_handler::on_delete_file(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//ɾ���ļ�  
			//δ��ֹ���ε���ż����ʧ�ܵ������ѭ��3�ξ�ʧ������Ϊʧ��
			for (int nloop_Num = 0; nloop_Num < 3; ++nloop_Num)
			{
				str_err_reason_for_debug = "";
				str_err_reason_for_user = "";
				ret = CCarve_Manager::instance()->delete_1_file(json_root, str_err_reason_for_debug, str_err_reason_for_user);
				if ( MSP_SUCCESS == ret)
				{
					break;
				}
				boost::this_thread::sleep(boost::posix_time::millisec(300));
			}
			
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}

		int request_handler::on_emergency_stop_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//��̻���ͣ
			ret = CCarve_Manager::instance()->emergency_stop_one(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}
		int request_handler::on_emergency_stop_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//ȫ����̻���ͣ
			ret = CCarve_Manager::instance()->emergency_stop_all(json_root,json_result,str_err_reason_for_debug, str_err_reason_for_user);
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//�ж������Ƿ�ɹ�
			if (ret)
			{
				LError("fail to emergency stop all, reason:{}, ret:{}", str_err_reason_for_debug, ret);
				return ret;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ��
			return MSP_SUCCESS;
		}
		int request_handler::on_cancel_emergency_stop_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//ȡ����̻���ͣ
			ret = CCarve_Manager::instance()->cancel_emergency_stop_one(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}

		int request_handler::on_cancel_emergency_stop_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//ȫ����̻���ͣ
			ret = CCarve_Manager::instance()->cancel_emergency_stop_all(json_root,json_result,str_err_reason_for_debug, str_err_reason_for_user);
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//�ж������Ƿ�ɹ�
			if (ret)
			{
				LError("fail to emergency stop all, reason:{}, ret:{}", str_err_reason_for_debug, ret);
				return ret;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ��
			return MSP_SUCCESS;
		}


		int request_handler::on_pause_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//��̻���ͣ
			ret = CCarve_Manager::instance()->pause_one(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}
		int request_handler::on_pause_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//ȫ����̻���ͣ
			ret = CCarve_Manager::instance()->pause_all(json_root,json_result,str_err_reason_for_debug, str_err_reason_for_user);
			if (ret)
			{//������
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ��
			return MSP_SUCCESS;
		}
	
		int request_handler::on_adjust_speed(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			//TODO: �˹�����ͣ���ӿڴ�ȷ��
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			//������̻������ٶ�
			ret = CCarve_Manager::instance()->adjust_speed(json_root, str_err_reason_for_debug, str_err_reason_for_user);
			//ע�⣺���۳ɰܣ���������
			//������
			json_result["ret"] = ret;
			json_result["errmsg"] = str_err_reason_for_debug;
			json_result["errmsg_for_user"] = sp::toutf8(str_err_reason_for_user);
			if (json_root.isMember(CCarve::ms_str_carve_id_key))
			{
				json_result[CCarve::ms_str_carve_id_key] = json_root[CCarve::ms_str_carve_id_key];
			}
			else
			{
				json_result[CCarve::ms_str_carve_id_key] = Json::Value();
			}
			if (ret)
			{
				str_err_reason =  str_err_reason_for_debug;
			}
			//ע�⣺����MSP_SUCCESS��ʾ�ɹ�ִ�У�����ִ�н����˵
			return MSP_SUCCESS;
		}

		int request_handler::on_query_all_carves_info(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason)
		{
			int ret = 0;
			std::string str_err_reason_for_debug;
			std::string str_err_reason_for_user;
			ret = CCarve_Manager::instance()->get_all_carves_info(json_root, json_result
				, str_err_reason_for_debug, str_err_reason_for_user);
			//�ж������Ƿ�ɹ�
			if (ret)
			{
				str_err_reason = str_err_reason_for_debug;
			}
			if (ret)
			{
				LError("fail to get all carves info, reason:{}, ret:{}", str_err_reason_for_debug, ret);
				return ret;
			}
			return ret;
		}



	} // namespace server3
} // namespace http
