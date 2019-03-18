//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"
#include "deal_http_msg.h"
#include "../busin_log.h"
#include "utils/msp_errors.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("connection::") + std::string(__FUNCTION__)).c_str()) 
#endif

namespace http {
	namespace server3 {

		connection::connection(boost::asio::io_service& io_service,
			request_handler& handler)
			: strand_(io_service),
			socket_(io_service),
			request_handler_(handler),
			m_bIs_full_msg(false)
		{
			buffer_.fill('\0');
		}

		boost::asio::ip::tcp::socket& connection::socket()
		{
			return socket_;
		}

		void connection::start_asyn_operate()
		{
			//清空缓冲区
			buffer_.fill('\0');
			socket_.async_read_some(boost::asio::buffer(buffer_),
				strand_.wrap(
				boost::bind(&connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));
		}

		void connection::handle_read(const boost::system::error_code& e,
			std::size_t bytes_transferred) 
		{
			businlog_tracer_perf(connection::handle_read);
			if (!e)
			{//未出错
				int ret = 0;
				businlog_info("%s | line:%d, buffer:%s, buff size:%d, bytes_transferred:%d, buff len:%d"
					, __FUNCTION__, __LINE__, buffer_.data(), buffer_.size(), bytes_transferred, strlen(buffer_.data()));
				//TODO::数据过大则报错返回，因为栈空间有限，后面可以改为auto_buff
				if (m_str_http_message.size() + bytes_transferred > MAX_MESSAGE_LEN)
				{
					businlog_error("%s | http message is larger than MAX_MESSAGE_LEN:%d."
						, __CLASS_FUNCTION__, MAX_MESSAGE_LEN);
					//发送bad_request到客户端
					on_bad_request();
					return;
				}
				//将数据追加到后面
				m_str_http_message.append(buffer_.data(), bytes_transferred);
				businlog_info("%s | buff is:%s.", __CLASS_FUNCTION__, buffer_.data());
				businlog_info("%s | http_message is:%s.", __CLASS_FUNCTION__, m_str_http_message.data());
				std::string str_json_body;
				std::string str_err_reason;
				ret = get_http_body(m_str_http_message.data(), m_str_http_message.size(), str_json_body, str_err_reason);
				if (0 == ret)
				{
					request_handler_.handle_request(request_, str_json_body, reply_);
					boost::asio::async_write(socket_, reply_.to_buffers(),
						strand_.wrap(
						boost::bind(&connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error)));
				}
				else if (ret)
				{//解析http请求出错了

					//如果是因为消息不完整，则继续获取。而不用报错
					if (false == m_bIs_full_msg)
					{
						start_asyn_operate();
					}
					else 
					{
						//其他原因出错，则直接报错返回
						businlog_error("%s | bad request, client addr:%s, reason:%s."
							, __CLASS_FUNCTION__, socket_.remote_endpoint().address().to_string().c_str()
							, str_err_reason.c_str());

						reply_ = reply::stock_reply(reply::bad_request);
						boost::asio::async_write(socket_, reply_.to_buffers(),
							strand_.wrap(
							boost::bind(&connection::handle_write, shared_from_this(),
							boost::asio::placeholders::error)));
					}
				}

			}
			else
			{//出错了
				// If an error occurs then no new asynchronous operations are started. This
				// means that all shared_ptr references to the connection object will
				// disappear and the object will be destroyed automatically after this
				// handler returns. The connection class's destructor closes the socket.
				if (2 == e.value())
				{//读取到末尾了。如果未来无法获取整个http请求的body，可以在这里添加逻辑。
				   //因为此时肯定读取结束
				}
				
				businlog_error("%s | Has error, err code:%d, err reason:%s, client addr:%s."
					, __CLASS_FUNCTION__, e.value(), e.message().c_str(), socket_.remote_endpoint().address().to_string().c_str());
			}
		}

		void connection::handle_write(const boost::system::error_code& e)
		{
			businlog_tracer_perf(connection::handle_write);
			if (!e)
			{
				// Initiate graceful connection closure.
				boost::system::error_code ignored_ec;
				socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
				if (ignored_ec)
				{//此处尽管出错了，但是可能为正确的
					businlog_info("%s | fail to shutdown socket, err core:%d, err reason:%s, client addr:%s."
						, __CLASS_FUNCTION__, ignored_ec.value(), ignored_ec.message().c_str(), socket_.remote_endpoint().address().to_string().c_str());
					if (ignored_ec.value() != 10022)
					{//其他错误，即真的出错了
						businlog_error("%s | fail to shutdown socket, err core:%d, err reason:%s, client addr:%s."
							, __CLASS_FUNCTION__, ignored_ec.value(), ignored_ec.message().c_str(), socket_.remote_endpoint().address().to_string().c_str());
					}
					else
					{
						//为客户端关闭链接，是正确的。故不打印错误信息
					}
				}
				else
				{//未出错反而是不正确的。可能为客户端未主动关闭链接
					businlog_error("%s | The client did not actively close the link. This may make the Server have an new TIME_WAIT."
						, __CLASS_FUNCTION__);
				}
			}
			else
			{
				// No new asynchronous operations are started. This means that all shared_ptr
				// references to the connection object will disappear and the object will be
				// destroyed automatically after this handler returns. The connection class's
				// destructor closes the socket.
				businlog_error("%s | has error, err code:%d, err reason:%s, client addr:%s."
					, __CLASS_FUNCTION__, e.value(), e.message().c_str(), socket_.remote_endpoint().address().to_string().c_str());
			}
		}

		int connection::get_http_body(const char* buf, size_t len, std::string& str_json_body, std::string& str_err_reason)
		{
			businlog_tracer_perf(connection::get_http_body);
			m_http_msg_tool_obj.reset();

			//转换消息并获取是否为完整的http请求消息
			int ret = m_http_msg_tool_obj.parse_msg(HTTP_BOTH, buf, len, m_bIs_full_msg);
			if (ret)
			{
				businlog_error("%s | parse_msg failed, ret:%d, msg:%s", __CLASS_FUNCTION__, ret, buf);
				str_err_reason = "The message is not valid http";
				return ret;
			}
			if (false == m_bIs_full_msg)
			{
				str_err_reason = "The msg Server got is not full.";
				businlog_error("%s | Msg is not full, message:%s", __CLASS_FUNCTION__, buf);
				return MSP_ERROR_INVALID_DATA;
			}
			unsigned long body_len = 0;
			//获取消息体
//			str_json_body = m_http_msg_tool_obj.get_http_body(&body_len, &ret);
			ret = get_http_body(std::string(buf), str_json_body, str_err_reason);
			if (ret)
			{
				str_err_reason = "Invalid http body";
				businlog_error("%s | get_http_body failed, ret:%d, msg:%s", __CLASS_FUNCTION__, ret, buf);
				return ret;
			}
			businlog_info("%s | http body:%s", __CLASS_FUNCTION__, str_json_body.c_str());
			return MSP_SUCCESS;

		}

		int connection::get_http_body(const std::string& str_medley_http_message, std::string& str_json_body, std::string& str_err_reason)
		{
			//从拼凑的http消息中获取body
			//找到http头部结束标记
			int nHeader_end_idx = str_medley_http_message.find("\r\n\r\n");
			if (nHeader_end_idx < 0)
			{
				businlog_error("%s | Can not find end of http head.", __FUNCTION__);
				return MSP_ERROR_INVALID_DATA;
			}
			str_json_body = str_medley_http_message.substr(nHeader_end_idx + 4);
			return MSP_SUCCESS;
		}

		connection::~connection()
		{//by minglu
			businlog_tracer(connection::~connection);
			if (socket_.is_open())
			{
				boost::system::error_code ec;
				socket_.close(ec);
				if (ec)
				{//出错
					//获取一些信息
					std::string str_remote_addr = socket_.remote_endpoint().address().to_string();
					businlog_error("%s | fail to close socket, client addr:%s"
						, __CLASS_FUNCTION__, str_remote_addr.c_str());
				}
			}
		}

		void connection::on_bad_request()
		{
			reply_ = reply::stock_reply(reply::bad_request);
			boost::asio::async_write(socket_, reply_.to_buffers(),
				strand_.wrap(
				boost::bind(&connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error)));
			businlog_error("%s | bad request, client addr:%s."
				, __CLASS_FUNCTION__, socket_.remote_endpoint().address().to_string().c_str());
		}

	} // namespace server3
} // namespace http
