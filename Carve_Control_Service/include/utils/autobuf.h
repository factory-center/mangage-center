/** 
* @file	autobuf.h
* @brief	能够自动伸展的buffer
* 
*  缺省可以使用栈内存，不够则使用堆内存，可以自动释放
* 
* @author	jdyu
* @version	1.0
* @date	2008/4/28
* 
* @see		
* 
* <b>History:</b><br>
* <table>
*  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
*  <tr> <td>1.0		<td>2008/4/28	<td>jdyu	<td>Create this file</tr>
* </table>
* 
*/
#ifndef __AUTOBUF_H__
#define __AUTOBUF_H__

#include <map>
#include <deque>
#include <algorithm>
#include <utils/SP_Assert.h>
namespace sp {

	// 缺省尺寸
	const unsigned int DEFAULT_BUFFER_SIZE = 4096;
	// 检查是否越界的看守
	const char	ab_buf_guard[] = "AutoBufferGuard";

	/** 
	* @class	auto_buf
	* @brief	自动管理的buffer类
	* 
	*  可以和下面的管理器配合使用
	* 
	* @author	jdyu
	* @date	2008/4/29
	*/
	template <unsigned int SIZE = DEFAULT_BUFFER_SIZE, typename TYPE = char>
	class auto_buf
	{
	public:
		auto_buf(size_t size = 0, TYPE * data = 0)
			: buf_real_		(0)
			, buf_size_		(0)
			, buf_cap_		(0)
			, high_water_	(0)
		{
			memset(buf_def_, 0, sizeof(buf_def_) / sizeof(buf_def_[0]));
			buf_real_ = buf_def_;
			buf_cap_  = sizeof buf_def_ / sizeof buf_def_[0];
			memcpy(buf_guard_, ab_buf_guard, sizeof ab_buf_guard);
			if ( size > 0 )
			{
				reserve(size);
				if ( data != 0 )
					set(data, size);
			}
		}
		~auto_buf(void)
		{
			wipe();
		}
		auto_buf(const auto_buf & that)
		{
			this->set(that.data(), that.size());
		}
		auto_buf & operator = (const auto_buf & that)
		{
			this->set(that.data(), that.size());
			return *this;
		}

		TYPE * data(void)
		{
			return buf_real_;
		}
		const TYPE * data(void) const
		{
			return buf_real_;
		}
		unsigned char * byte_data(void)
		{
			return (unsigned char*) data();
		}
		const char * str_data(void)
		{
			return (const char*) data();
		}
		short * short_data(void)
		{
			return (short*) data();
		}
		template <typename T>
		T * t_data(void)
		{
			return (T*) data();
		}

		size_t size(void) const
		{
			return buf_size_;
		}
		size_t capacity(void) const
		{
			return buf_cap_;
		}
		size_t size_bytes(void) const
		{
			return buf_size_ * sizeof(TYPE);
		}
		size_t capacity_bytes(void) const
		{
			return buf_cap_ * sizeof(TYPE);
		}
		bool empty(void) const
		{
			return buf_size_ == 0;
		}

		size_t high_water(void) const
		{
			return high_water_;
		}
		size_t high_water(size_t hw)
		{
			size_t lhw = high_water_;
			if ( hw > DEFAULT_BUFFER_SIZE )
				high_water_ = hw;
			return lhw;
		}

		int alloc(size_t cap)
		{
			resize(cap);
			return 0;
		}
		int free(bool wipe = false)
		{
			wipe ? this->wipe()	: resize(0);
			return 0;
		}
		void reset(void)
		{
			resize(0);
		}

		void reserve(size_t cap)
		{
			realloc(cap);
		}
		void resize(size_t new_size)
		{
			realloc(new_size);
			buf_size_ = new_size;
		}
		void realloc(size_t new_cap)
		{
			if ( high_water_ > 0 && capacity_bytes() > high_water_ )
				wipe();
			if ( new_cap > buf_cap_ )
			{
				TYPE * obuf = buf_real_;
				size_t nSize = new_cap * sizeof(TYPE) + sizeof ab_buf_guard;
				buf_real_ = (TYPE*) new char[nSize];
				SP_ASSERT(NULL != buf_real_);
				memset(buf_real_, 0, nSize);
				buf_cap_ = new_cap;

				memcpy(buf_real_ + new_cap * sizeof(TYPE), ab_buf_guard, sizeof ab_buf_guard);
				SP_ASSERT(buf_size_ <= buf_cap_);
				if ( buf_size_ > 0 )
					memcpy(buf_real_, obuf, buf_size_);
				if ( obuf != buf_def_ )
					delete [] obuf;
			}
		}
		void wipe(void)
		{
			SP_ASSERT(memcmp(buf_real_ + buf_cap_, ab_buf_guard, sizeof ab_buf_guard) == 0);
			if ( buf_real_ != buf_def_ )
				delete [] buf_real_;
			buf_real_ = buf_def_;
			buf_cap_  = sizeof buf_def_ / sizeof buf_def_[0];
			buf_size_ = 0;
		}

		void set(const TYPE * buf, size_t count)
		{
			resize(count);
			memcpy(buf_real_, buf, count * sizeof(TYPE));
		}
		void set(void * buf, size_t count)
		{
			set((const TYPE*)buf, count);
		}
		int get(TYPE * buf, size_t & count) const
		{
			int ret = 0;
			memcpy(buf, buf_real_, std::min(count, buf_size_) * sizeof(TYPE));
			if ( count < buf_size_ )
				ret = -1;
			count = std::min(count, buf_size_);
			return 0;
		}
		size_t append(const TYPE * data, size_t count)
		{
			if ( count + buf_size_ > buf_cap_ )
				realloc(count + buf_size_);
			memcpy(this->data() + buf_size_, data, count * sizeof(TYPE));
			buf_size_ += count;
			return buf_size_;
		}
		size_t append(void * data, size_t count)
		{
			return append((const TYPE *)data, count);
		}
		size_t fill(const TYPE & dat, size_t start = 0, size_t end = 0)
		{
			if ( end == 0 || end > buf_size_ )
				end = buf_size_;
			std::fill(buf_real_ + start, buf_real_ + buf_size_, dat);
		}

		TYPE & at(size_t index)
		{
			SP_ASSERT(index < buf_size_);
			return buf_real_[index];
		}
		TYPE & operator [] (size_t index)
		{
			return at(index);
		}

	private:
		// 缺省的栈缓冲区
		TYPE		buf_def_[SIZE];
		char		buf_guard_[sizeof ab_buf_guard];
		TYPE *		buf_real_;
		size_t		buf_size_;
		size_t		buf_cap_;
		// 最高的水位，如果尺寸大于这个值，那么每次resize的时候就会释放之
		size_t		high_water_;
	};

	/*循环缓冲区*/
	/************************************************************************/
	/*队空：front == rear                                                   */
	/*队满：rear+1 % n = front;                                             */
	/************************************************************************/
	template <size_t SIZE = DEFAULT_BUFFER_SIZE>
	class circle_buf
	{
	public:
		circle_buf()
			: base_  (new char[SIZE])
			, front_ (0)
			, rear_  (0)
			, capacity_(SIZE)
		{
		}
		~circle_buf()
		{
			if ( base_ )
				delete [] base_, base_ = NULL;
		}

	public:
		size_t front()
		{
			return front_;
		}
		size_t rear()
		{
			return rear_;
		}
		size_t capacity()
		{
			return capacity_;
		}
		size_t size()
		{
			return (rear_ - front_ + capacity_ ) % capacity_;
		}
		size_t space()
		{
			return (capacity_  - size());
		}
		bool empty()
		{
			return (front_ == rear_);
		}
		void clear()
		{
			memset(base_, 0, capacity_);
			front_ = rear_ = 0;
		}

		size_t write( const char* buf, size_t len )
		{
			len = len > space() ? space() : len;
			size_t tail_write = (capacity_ - rear_) % capacity_;
			tail_write = tail_write > len ? len : tail_write;
			memcpy(base_ + rear_, buf, tail_write);
			rear_ = (rear_ + tail_write) % capacity_;
			if ( tail_write < len )
			{
				memcpy(base_ + rear_, buf + tail_write, len - tail_write);
				rear_ = (rear_ + len - tail_write) % capacity_;
			}
			return len;
		}

		size_t read(char* buf, size_t len)
		{
			len = len > size() ? size() : len;
			size_t tail_read = (capacity_ - front_ ) % capacity_;
			tail_read = tail_read > len ? len : tail_read;
			memcpy(buf, base_ + front_, tail_read);
			front_ = (front_ + tail_read) % capacity_;
			if ( tail_read < len )
			{
				memcpy(buf+tail_read, base_ + front_, len - tail_read);
				front_ = (front_ + len - tail_read) % capacity_;
			}

			return len;
		}

		bool move_front(const size_t base_pos, size_t add_len)
		{
			if (add_len >= capacity_)
			{
				return false;
			}

			front_ = (base_pos + add_len)%capacity_;
			return true;
		}

		bool realloc(size_t size)
		{
			if (size <= capacity_)
			{
				return true;
			}

			char *temp = new char[size];
			if ( temp == NULL )
				return false;

			/*buf 当前为空 */
			if ( empty() )
			{
				delete base_;
				base_ = temp;
				front_ = rear_ = 0;
				capacity_ = size;
				return true;
			}

			size_t len = read(temp, this->size());
			front_ = 0;
			rear_ = len;
			capacity_ = size;
			delete base_;
			base_ = temp;
			return true;
		}

		void release()
		{
			if ( base_ )
				delete base_, base_ = NULL;

			front_ = rear_ = capacity_ = 0;
		}

	private:
		char *  base_;
		size_t  front_;
		size_t  rear_;
		size_t  capacity_;
	};
	/** 
	* @class	mutex_null
	* @brief	空锁
	* 
	*  达到虚锁定的功能
	* 
	* @author	jdyu
	* @date	2008/4/29
	*/
	class mutex_null
	{
	public:
		int acquire(void)
		{
			return 0;
		}
		int release(void)
		{
			return 0;
		}
	};

	/** 
	* @class	mutex_guard
	* @brief	自动锁定和解锁
	* @author	jdyu
	* @date	2008/4/29
	*/
	template<class MUTEX>
	class mutex_guard
	{
	public:
		mutex_guard(MUTEX & lock)
			: lock_	(&lock)
		{
			lock.acquire();
		}
		mutex_guard(MUTEX * lock)
			: lock_	(lock)
		{
			if ( lock_ )
				lock_->acquire();
		}
		~mutex_guard(void)
		{
			if ( lock_ )
				lock_->release();
		}

	private:
		// the lock
		MUTEX * lock_;
	};

	/** 
	* @class	auto_buf_mngr
	* @brief	自动内存管理器
	* 
	*  管理多个buffer，提供内存池的功能
	* 
	* @author	jdyu
	* @date	2008/4/29
	* 
	* @see		auto_buf, mutex_null
	*/
	template<class ATBUF = auto_buf<>,
	class MUTEX = mutex_null>
	class auto_buf_mngr
	{
	public:
		auto_buf_mngr(void)
			: high_water_	(0)
			, last_free_	(-1)
		{
		}
		auto_buf_mngr(size_t count, size_t cap)
			: high_water_	(0)
			, last_free_	(-1)
		{
			reserve(count, cap);
		}
		~auto_buf_mngr(void)
		{
			clear();
		}

		// 分配一个指定大小的缓冲区
		ATBUF * alloc(size_t cap = 0)
		{
			ATBUF * buf = 0;
			GUARD mon(bufs_lock_);

			// check last free
			if ( last_free_ != -1 )
			{
				ab_assist & as = auto_bufs_[last_free_];
				if ( as.buffer()->capacity() >= cap )
				{
					buf = as.alloc(cap);
					last_free_ = -1;
				}
			}

			size_t first_idle = -1;
			for ( size_t i = 0; i < auto_bufs_.size(); i++ )
			{
				ab_assist & as = auto_bufs_[i];
				if ( !as.busy() )
				{
					if ( first_idle == -1 )
						first_idle = i;

					if ( buf != 0 && last_free_ == -1 )
					{
						last_free_ = i;
						break;
					}

					if ( buf == 0 && as.buffer()->capacity() >= cap )
					{
						buf = as.alloc(cap);
						if ( last_free_ != -1 )
							break;
					}
				}
			}

			if ( buf == 0 && first_idle != -1 )
				buf = auto_bufs_[first_idle].alloc(cap);

			if ( buf == 0 )
			{
				auto_bufs_.push_back(ab_assist(cap, true));
				buf = auto_bufs_.back().buffer();
				bufs_map_.insert(std::make_pair(buf, auto_bufs_.size() - 1));
			}

			return buf;
		}
		int free(ATBUF * ab)
		{
			GUARD gd(bufs_lock_);
			typename bufs_map::iterator pi = bufs_map_.find(ab);
			if ( pi == bufs_map_.end() )
			{
				SP_ASSERT(0 && "cannot find the buffer to free!!!");
				return -1;
			}

			return do_free(pi->second);
		}
		int free(const void * ptr)
		{
			GUARD gd(bufs_lock_);

			for ( size_t i = 0; i < auto_bufs_.size(); i++ )
			{
				if ( auto_bufs_[i].buffer()->data() == ptr )
					return do_free(i);
			}

			return -1;
		}
		int free_all(void)
		{
			GUARD gd(bufs_lock_);
			std::for_each(auto_bufs_.begin(), auto_bufs_.end(), std::mem_fun_ref(&ATBUF::free));
			return 0;
		}
		int clear(void)
		{
			GUARD gd(bufs_lock_);
			for ( bufs_pi pi = auto_bufs_.begin(); pi != auto_bufs_.end(); pi++ )
			{
				ATBUF * buf = pi->buffer();
				buf->wipe();
				delete buf;
			}
			auto_bufs_.clear();
			bufs_map_.clear();
			return 0;
		}

		int reserve(size_t count, size_t cap)
		{
			GUARD gd(bufs_lock_);
			for ( size_t i = 0; i < count; i++ )
			{
				auto_bufs_.push_back(ab_assist(cap, false));
				bufs_map_.insert(
					std::make_pair(auto_bufs_.back().buffer(), auto_bufs_.size() - 1));
			}
			return 0;
		}
		int compact(void)
		{
			GUARD gd(bufs_lock_);
			bufs_pi pi = auto_bufs_.begin();
			for ( ; pi != auto_bufs_.end(); pi++ )
			{
				if ( !pi->busy() )
				{
					delete pi->buffer();
					delete *pi;
					pi = auto_bufs_.erase(pi);
				}
			}
			return 0;
		}

		size_t high_water(void) const
		{
			return high_water_;
		}
		size_t high_water(size_t hw)
		{
			size_t lhw = high_water_;
			if ( hw > DEFAULT_BUFFER_SIZE )
				high_water_ = hw;
			return lhw;
		}

		// 支持单体访问
		static auto_buf_mngr & instance(void)
		{
			static auto_buf_mngr the_ab_mngr;
			return the_ab_mngr;
		}

	protected:
		int do_free(size_t id)
		{
			SP_ASSERT(id < auto_bufs_.size());
			auto_bufs_[id].free();
			last_free_ = id;

			// 如果设置了水位，需要判断是否超过
			if ( high_water_ > 0 )
			{
				size_t total_size = 0;
				for ( size_t i = 0; i < auto_bufs_.size(); i++ )
					total_size += auto_bufs_[i].buffer()->capacity();
				if ( total_size > high_water_ )
					auto_bufs_[id].buffer()->wipe();
			}

			return 0;
		}

	private:
		// 缓冲区辅助
		struct ab_assist
		{
			ab_assist(size_t cap, bool alloc)
			{
				abuf_ = new ATBUF(cap);
				busy_ = alloc;
			}
			bool busy(void) const
			{
				return busy_;
			}
			void busy(bool bs)
			{
				busy_ = bs;
			}
			ATBUF * buffer(void)
			{
				return abuf_;
			}

			ATBUF * alloc(size_t cap)
			{
				SP_ASSERT(!busy_);
				busy_ = true;
				abuf_->alloc(cap);
				return abuf_;
			}
			void free(void)
			{
				busy_ = false;
				abuf_->resize(0);
			}

		private:
			ATBUF * abuf_;
			bool	busy_;
		};

		// 内存块容器
		typedef std::deque<ab_assist> bufs_list;
		typedef typename bufs_list::iterator bufs_pi;

		// Mutex Guard
		typedef mutex_guard<MUTEX>	GUARD;

		// 所有的内存块
		bufs_list					auto_bufs_;
		MUTEX						bufs_lock_;
		// 最高的水位，如果尺寸大于这个值，那么每次free的时候就会彻底释放该缓冲区
		size_t						high_water_;

		// 为优化速度而添加
		size_t						last_free_;
		// 映射表
		typedef std::map<ATBUF*, size_t> bufs_map;
		bufs_map					bufs_map_;
	};

} /* namespace sp */

#endif /* __AUTOBUF_H__ */
