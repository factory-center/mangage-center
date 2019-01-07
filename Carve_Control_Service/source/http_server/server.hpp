//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "request_handler.hpp"
#include <boost/thread.hpp>

namespace http {
	namespace server3 {

		/// The top-level class of the HTTP server.
		class server
			: private boost::noncopyable
			, public boost::enable_shared_from_this<server> //注意，这么写后不能在栈中构建server
		{
		public:
			/// Construct the server
			explicit server(const std::string& address, const std::string& port,
				const std::string& doc_root, std::size_t thread_pool_size);
			/// Run the server's io_service loop.
			void run();
			//监听退出信号，并注册handle_stop，监听指定端口，最后启动accept
			int init(std::string& str_err_reason);
			~server();
			const std::string& get_ip() const
			{
				return m_str_local_ip;
			}
			const std::string get_port() const
			{
				return m_str_port;
			}
		private:
			/// Initiate an asynchronous accept operation.
			void start_accept();
//			void stop();

			/// Handle completion of an asynchronous accept operation.
			void handle_accept(const boost::system::error_code& e);

			/// Handle a request to stop the server.
			void handle_stop();

			/// The number of threads that will call io_service::run().
			std::size_t thread_pool_size_;

			/// The io_service used to perform asynchronous operations.
			boost::asio::io_service io_service_;

			/// The signal_set is used to register for process termination notifications.
			boost::asio::signal_set signals_;

			/// Acceptor used to listen for incoming connections.
			boost::asio::ip::tcp::acceptor acceptor_;

			/// The next connection to be accepted.
			connection_ptr new_connection_;

			/// The handler for all incoming requests.
			request_handler request_handler_;
//			std::vector<boost::shared_ptr<boost::thread> > threads_vec_;
			std::string m_str_local_ip; //本机地址
			std::string m_str_port; //端口号
		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP
