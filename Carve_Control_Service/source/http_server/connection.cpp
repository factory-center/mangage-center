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
namespace http {
	namespace server3 {

		connection::connection(boost::asio::io_service& io_service,
			request_handler& handler)
			: strand_(io_service),
			socket_(io_service),
			request_handler_(handler)
		{
			buffer_.fill('\0');
		}

		boost::asio::ip::tcp::socket& connection::socket()
		{
			return socket_;
		}

		void connection::start_asyn_operate()
		{
			socket_.async_read_some(boost::asio::buffer(buffer_),
				strand_.wrap(
				boost::bind(&connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));
		}

		void connection::handle_read(const boost::system::error_code& e,
			std::size_t bytes_transferred)
		{
			printf("%s | line:%d\n", __FUNCTION__, __LINE__);
			if (!e)
			{
				int ret = 0;
				printf("%s | line:%d, buffer:%s, buff size:%d, bytes_transferred:%d, buff len:%d\n"
					, __FUNCTION__, __LINE__, buffer_.data(), buffer_.size(), bytes_transferred, strlen(buffer_.data()));
				printf("%s | request url:%s, method:%s\n", __FUNCTION__, this->request_.uri.c_str(), this->request_.method.c_str());
				std::string str_json_body;
				ret = deal_request(buffer_.data(), bytes_transferred, str_json_body);
				if (0 == ret)
				{
					request_handler_.handle_request(request_, str_json_body, reply_);
					boost::asio::async_write(socket_, reply_.to_buffers(),
						strand_.wrap(
						boost::bind(&connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error)));
				}
				else if (ret)
				{
					reply_ = reply::stock_reply(reply::bad_request);
					boost::asio::async_write(socket_, reply_.to_buffers(),
						strand_.wrap(
						boost::bind(&connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error)));
				}

			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		void connection::handle_write(const boost::system::error_code& e)
		{
			printf("%s | line:%d\n", __FUNCTION__, __LINE__);
			if (!e)
			{
				// Initiate graceful connection closure.
				boost::system::error_code ignored_ec;
				socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

		int connection::deal_request(const char* buf, size_t len, std::string& str_json_body)
		{
			deal_http_msg http_msg_tool_obj;
			http_msg_tool_obj.reset();
			bool bIs_full_msg = false;
			int ret = http_msg_tool_obj.parse_msg(HTTP_BOTH, buf, len, bIs_full_msg);
			if (ret)
			{
				printf("%s | parse_msg failed, ret:%d, msg:%s\n", __FUNCTION__, ret, buf);
				return ret;
			}
			if (false == bIs_full_msg)
			{
				printf("%s | Msg is not full\n", __FUNCTION__);
			}
			unsigned long body_len = 0;
			//获取消息体
			str_json_body = http_msg_tool_obj.get_http_body(&body_len, &ret);
			if (ret)
			{
				printf("%s | get_http_body failed, ret:%d, msg:%s", __FUNCTION__, ret, buf);
				return ret;
			}
			printf("%s | http body:%s\n", __FUNCTION__, str_json_body.c_str());
			return 0;

		}

	} // namespace server3
} // namespace http
