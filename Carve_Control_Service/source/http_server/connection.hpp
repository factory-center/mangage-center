//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include <utils/autobuf.h>
#include "deal_http_msg.h"

namespace http {
	namespace server3 {
		const size_t MAX_MESSAGE_LEN = 8192;
		/// Represents a single connection from a client.
		class connection
			: public boost::enable_shared_from_this<connection>,
			private boost::noncopyable
		{
		public:
			/// Construct a connection with the given io_service.
			explicit connection(boost::asio::io_service& io_service,
				request_handler& handler);
			~connection();
			/// Get the socket associated with the connection.
			boost::asio::ip::tcp::socket& socket();

			/// Start the first asynchronous operation for the connection.
			void start_asyn_operate();
			//启动同步操作
			void start_sync_operate()
			{
				socket_.read_some(boost::asio::buffer(buffer_));
			}
			//TODO::是否需要新增一个析构函数用于关闭socket
		private:
			/// Handle completion of a read operation.
			void handle_read(const boost::system::error_code& e,
				std::size_t bytes_transferred);
			//从请求中获取http body
			int get_http_body(const char* buf, size_t len, std::string& str_json_body, std::string& str_err_reason);
			/// Handle completion of a write operation.
			void handle_write(const boost::system::error_code& e);
			//出错了，直接返回bad_request到服务端
			void on_bad_request();
			/// Strand to ensure the connection's handlers are not called concurrently.
			boost::asio::io_service::strand strand_;

			/// Socket for the connection.
			boost::asio::ip::tcp::socket socket_;

			/// The handler used to process the incoming request.
			request_handler& request_handler_;

			/// Buffer for incoming data.
			boost::array<char, MAX_MESSAGE_LEN> buffer_;

			/// The incoming request.
			request request_;

			/// The parser for the incoming request.
			request_parser request_parser_;

			/// The reply to be sent back to the client.
			reply reply_;
//			sp::auto_buf<sp::DEFAULT_BUFFER_SIZE, char> m_buffer_http_message;//合并tcp数据包后的数据
			std::string m_str_http_message;//合并tcp数据包后的数据
			bool m_bIs_full_msg; //是否为一个完整的请求消息
			deal_http_msg m_http_msg_tool_obj;
		};

		typedef boost::shared_ptr<connection> connection_ptr;

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP
