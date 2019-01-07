//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REPLY_HPP
#define HTTP_SERVER3_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "header.hpp"

namespace http {
	namespace server3 {

		/// A reply to be sent to a client.
		struct reply
		{
			/// The status of the reply.
			enum status_type
			{
				ok = 200,
				created = 201,
				accepted = 202,
				no_content = 204,
				multiple_choices = 300,
				moved_permanently = 301,
				moved_temporarily = 302,
				not_modified = 304,
				bad_request = 400,
				unauthorized = 401,
				forbidden = 403,
				not_found = 404,
				internal_server_error = 500,
				not_implemented = 501,
				bad_gateway = 502,
				service_unavailable = 503
			} status;

			/// The headers to be included in the reply.
			std::vector<header> headers;

			/// The content to be sent in the reply.
			std::string content;

			/// Convert the reply into a vector of buffers. The buffers do not own the
			/// underlying memory blocks, therefore the reply object must remain valid and
			/// not be changed until the write operation has completed.
			std::vector<boost::asio::const_buffer> to_buffers();
			/************************************
			* Method:    construct_message
			* Brief:  构造响应消息
			* Access:    public static 
			* Returns:   http::server3::reply
			* Qualifier:
			*Parameter: int nServer_ret -[in] 0:调用成功；非0：错误码 
			*Parameter: const std::string & str_json_result -[in] json串，出错时，其值为空 
			*Parameter: const std::string & str_err_reason -[in] 错误信息，成功时，其值为空
			************************************/
			static reply construct_message(int nServer_ret, const std::string& str_json_result, const std::string& str_err_reason);
			/// Get a stock reply.
			static reply stock_reply(status_type status);
		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REPLY_HPP
