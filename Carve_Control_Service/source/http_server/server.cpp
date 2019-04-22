//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "../source/CSpdLog.h"
#include "utils/msp_errors.h"
#include <boost/asio/socket_base.hpp>
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("server::") + std::string(__FUNCTION__)).c_str()) 
#endif

namespace http {
	namespace server3 {

		server::server(const std::string& address, const std::string& port,
			const std::string& doc_root, std::size_t thread_pool_size)
			: thread_pool_size_(thread_pool_size),
			signals_(io_service_),
			acceptor_(io_service_),
			new_connection_(),
			request_handler_(doc_root),
			m_str_local_ip(address),
			m_str_port(port)
		{
		}

		/************************************
		* Method:    run
		* Brief:  以io_service::run为线程函数，创建若干个线程来读取端口中数据，并等待所有线程结束
		* Access:    public 
		* Returns:   void
		* Qualifier:
		************************************/
		void server::run()
		{
			try
			{
				// Create a pool of threads to run all of the io_services.
				std::vector<boost::shared_ptr<boost::thread> > threads;
				for (std::size_t i = 0; i < thread_pool_size_; ++i)
				{
					boost::shared_ptr<boost::thread> thread(new boost::thread(
						boost::bind(&boost::asio::io_service::run, &io_service_)));
					threads.push_back(thread);
				}

				// Wait for all threads in the pool to exit.
				for (std::size_t i = 0; i < threads.size(); ++i)
				{
					threads[i]->join(); //线程中断点，即线程可以在此处被停止
				}
			}
			catch (boost::thread_interrupted& )//捕获线程中断异常，此异常为空异常的
			{
				LWarn("thread interrupted");
			}

		}

		void server::start_accept()
		{
			LTrace("server::start_accept");
			new_connection_.reset(new connection(io_service_, request_handler_));
			acceptor_.async_accept(new_connection_->socket(),
				boost::bind(&server::handle_accept, shared_from_this(),
				boost::asio::placeholders::error));
		}

		void server::handle_accept(const boost::system::error_code& e)
		{
			LTrace("server::handle_accept");
			if (!e)
			{//未出错
				new_connection_->start_asyn_operate();
			}
			else
			{//出错了
				LError("has error, reason:{}, err code:{}", e.message().c_str(), e.value());
			}
			start_accept();
		}

		void server::handle_stop()
		{
			LTrace("server::handle_stop");
			io_service_.stop();
		}

		/************************************
		* Method:    init
		* Brief:  //监听退出信号，并注册handle_stop，监听指定端口，最后启动accept
		* Access:    public 
		* Returns:   int 0:成功；非0：错误码
		* Qualifier:
		*Parameter: std::string & str_err_reason -[in/out]  
		************************************/
		int server::init(std::string& str_err_reason)
		{
			LTrace("server::init");
			try
			{
				// Register to handle the signals that indicate when the server should exit.
				// It is safe to register for the same signal multiple times in a program,
				// provided all registration for the specified signal is made through Asio.
				signals_.add(SIGINT);
				signals_.add(SIGTERM);
#if defined(SIGQUIT)
				signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
				signals_.async_wait(boost::bind(&server::handle_stop, shared_from_this()));
				// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
				boost::asio::ip::tcp::resolver resolver(io_service_);
				boost::asio::ip::tcp::resolver::query query(m_str_local_ip, m_str_port);
				boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
				boost::system::error_code err;
				acceptor_.open(endpoint.protocol(), err);
				if (err)
				{
					LError("fail to open, local ip:{}, port:{}, reason:{}", m_str_local_ip, m_str_port, err.message());
					str_err_reason = "fail to open, local ip:" + m_str_local_ip + ", port:" + m_str_port + ",reason:" + err.message();
					return err.value();
				}

				acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				acceptor_.bind(endpoint, err);
				if (err)
				{
					LError("fail to bind, local ip:{}, port:{}, reason:{}", m_str_local_ip, m_str_port, err.message());
					str_err_reason = "fail to open, local ip:" + m_str_local_ip + ", port:" + m_str_port + ",reason:" + err.message();
					return err.value();
				}

				acceptor_.listen(boost::asio::socket_base::max_connections, err);
				if (err)
				{
					LError("fail to listen, local ip:{}, port:{}, reason:{}", m_str_local_ip, m_str_port, err.message());
					str_err_reason = "fail to open, local ip:" + m_str_local_ip + ", port:" + m_str_port + ",reason:" + err.message();
					return err.value();
				}

				start_accept();
				return MSP_SUCCESS;
			}
			catch (std::exception& e)
			{
				str_err_reason = std::string(e.what());
				LError("Has exception:{}", str_err_reason.c_str());
				return MSP_ERROR_EXCEPTION;
			}
		}

		server::~server()
		{//by minglu
			LTrace("server::~server");
//			stop();
			if (acceptor_.is_open())
			{
				acceptor_.close();
			}
		}

// 		void server::stop()
// 		{//by minglu
// 			// Wait for all threads in the pool to exit.
// 			for (std::size_t i = 0; i < threads_vec_.size(); ++i)
// 			{
// 				if (threads_vec_[i]->joinable())
// 				{
// 					threads_vec_[i]->interrupt();
// 				}
// 
// 			}
// 		}

	} // namespace server3
} // namespace http
