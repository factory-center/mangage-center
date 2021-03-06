//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <json/json.h>
namespace http {
	namespace server3 {

		struct reply;
		struct request;

		/// The common handler for all incoming requests.
		class request_handler
			: private boost::noncopyable
		{
		public:
			/// Construct with a directory containing files to be served.
			explicit request_handler(const std::string& doc_root);

			/// Handle a request and produce a reply.
			void handle_request(const request& req, reply& rep);
			void handle_request(const request& req, const std::string& str_json_body, reply& rep);
		private:
			int on_connect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_disconnect(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_query_one_carve_status(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_query_one_carve_info(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_query_all_carves_info(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_start(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_start_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_upload_file(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_emergency_stop_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_emergency_stop_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_cancel_emergency_stop_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_cancel_emergency_stop_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_adjust_speed(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_delete_file(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_pause_one(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);
			int on_pause_all(const Json::Value& json_root, Json::Value& json_result, std::string& str_err_reason);

			
			/// The directory containing the files to be served.
			std::string doc_root_;

			/// Perform URL-decoding on a string. Returns false if the encoding was
			/// invalid.
			static bool url_decode(const std::string& in, std::string& out);
		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
