#pragma once
//  (C) Copyright 2006-8 Anthony Williams
//  (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <Windows.h>
#include <exception>
//#include <iostream>
#include <chrono>
#include <assert.h>

namespace sm {

	class shared_mutex
	{
	private:
		struct state_data
		{
			unsigned shared_count:11,
			shared_waiting:11,
			exclusive:1,
			upgrade:1,
			exclusive_waiting:7,
			exclusive_waiting_blocked:1;

			friend bool operator==(state_data const& lhs,state_data const& rhs)
			{
				return *reinterpret_cast<unsigned const*>(&lhs)==*reinterpret_cast<unsigned const*>(&rhs);
			}
		};


		template<typename T>
		T interlocked_compare_exchange(T* target,T new_value,T comparand)
		{
			//static_assert(sizeof(T)==sizeof(long));
			long const res=InterlockedCompareExchange(reinterpret_cast<long*>(target),
				*reinterpret_cast<long*>(&new_value),
				*reinterpret_cast<long*>(&comparand));
			return *reinterpret_cast<T const*>(&res);
		}

		enum
		{
			unlock_sem = 0,
			exclusive_sem = 1
		};

		state_data state;
		HANDLE semaphores[2];
		HANDLE upgrade_sem;

		void release_waiters(state_data old_state)
		{
			if(old_state.exclusive_waiting)
			{
				ReleaseSemaphore(semaphores[exclusive_sem],1,0);
			}

			if(old_state.shared_waiting || old_state.exclusive_waiting)
			{
				ReleaseSemaphore(semaphores[unlock_sem],old_state.shared_waiting + (old_state.exclusive_waiting?1:0),0);
			}
		}
		void release_shared_waiters(state_data old_state)
		{
			if(old_state.shared_waiting || old_state.exclusive_waiting)
			{
				ReleaseSemaphore(semaphores[unlock_sem],old_state.shared_waiting + (old_state.exclusive_waiting?1:0),0);
			}
		}

		shared_mutex(const shared_mutex&) {}
		shared_mutex& operator=(const shared_mutex&) {}

	public:
		shared_mutex()
		{
			semaphores[unlock_sem]=CreateSemaphore(0,0,LONG_MAX,0);
			semaphores[exclusive_sem]=CreateSemaphore(0,0,LONG_MAX,0);
			if (!semaphores[exclusive_sem])
			{
				ReleaseSemaphore(semaphores[unlock_sem],LONG_MAX,0);
				throw std::exception("semaphore initialization failed");
			}
			upgrade_sem=CreateSemaphore(0,0,LONG_MAX,0);
			if (!upgrade_sem)
			{
				ReleaseSemaphore(semaphores[unlock_sem],LONG_MAX,0);
				ReleaseSemaphore(semaphores[exclusive_sem],LONG_MAX,0);
				throw std::exception("semaphore initialization failed");
			}
			state_data state_={0,0,0,0,0,0};
			state=state_;
		}

		~shared_mutex()
		{
			//std::cout << " close handles" << std::endl;
			CloseHandle(upgrade_sem);
			CloseHandle(semaphores[unlock_sem]);
			CloseHandle(semaphores[exclusive_sem]);
		}

		bool try_lock_shared()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				if(!new_state.exclusive && !new_state.exclusive_waiting_blocked)
				{
					++new_state.shared_count;
					if(!new_state.shared_count)
					{
						return false;
					}
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			return !(old_state.exclusive| old_state.exclusive_waiting_blocked);
		}

		void lock_shared()
		{
			try_lock_shared_until(std::chrono::steady_clock::now());
		}

		template <class Rep, class Period>
		bool try_lock_shared_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return try_lock_shared_until(std::chrono::steady_clock::now() + rel_time);
		}
		template <class Clock, class Duration>
		bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration>& t)
		{
			using namespace std;
			using namespace chrono;
			system_clock::time_point     s_now = system_clock::now();
			typename Clock::time_point  c_now = Clock::now();
			return try_lock_shared_until(s_now + ceil<system_clock::duration>(t - c_now));
		}
		template <class Duration>
		bool try_lock_shared_until(const std::chrono::time_point<std::chrono::system_clock, Duration>& t)
		{
			using namespace std::chrono;
			typedef time_point<std::chrono::system_clock, std::chrono::system_clock::duration> sys_tmpt;
			return try_lock_shared_until(sys_tmpt(std::chrono::ceil<std::chrono::system_clock::duration>(t.time_since_epoch())));
		}
		bool try_lock_shared_until(const std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration>& tp)
		{
			using namespace std;
			for(;;)
			{
				state_data old_state=state;
				for(;;)
				{
					state_data new_state=old_state;
					if(new_state.exclusive || new_state.exclusive_waiting_blocked)
					{
						++new_state.shared_waiting;
						if(!new_state.shared_waiting)
						{
							std::exception("shared_mutex: lock error");
						}
					}
					else
					{
						++new_state.shared_count;
						if(!new_state.shared_count)
						{
							std::exception("shared_mutex: lock error");
						}
					}

					state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
					if(current_state==old_state)
					{
						break;
					}
					old_state=current_state;
				}

				if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
				{
					return true;
				}

				chrono::system_clock::time_point n = chrono::system_clock::now();
				unsigned long res;
				if (tp>n) {

					chrono::milliseconds rel_time= std::chrono::duration_cast<std::chrono::milliseconds>(tp-n);
					res=WaitForSingleObjectEx(semaphores[unlock_sem],
						static_cast<unsigned long>(rel_time.count()), 0);
				} else {
					res=WAIT_TIMEOUT;
				}
				if(res==WAIT_TIMEOUT)
				{
					for(;;)
					{
						state_data new_state=old_state;
						if(new_state.exclusive || new_state.exclusive_waiting_blocked)
						{
							if(new_state.shared_waiting)
							{
								--new_state.shared_waiting;
							}
						}
						else
						{
							++new_state.shared_count;
							if(!new_state.shared_count)
							{
								return false;
							}
						}

						state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
						if(current_state==old_state)
						{
							break;
						}
						old_state=current_state;
					}

					if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
					{
						return true;
					}
					return false;
				}

				assert(res==0);
			}
		}


		void unlock_shared()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				bool const last_reader=!--new_state.shared_count;

				if(last_reader)
				{
					if(new_state.upgrade)
					{
						new_state.upgrade=false;
						new_state.exclusive=true;
					}
					else
					{
						if(new_state.exclusive_waiting)
						{
							--new_state.exclusive_waiting;
							new_state.exclusive_waiting_blocked=false;
						}
						new_state.shared_waiting=0;
					}
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					if(last_reader)
					{
						if(old_state.upgrade)
						{
							assert(ReleaseSemaphore(upgrade_sem,1,0)!=0);
						}
						else
						{
							release_waiters(old_state);
						}
					}
					break;
				}
				old_state=current_state;
			}
		}

		void lock()
		{
			try_lock_until(std::chrono::steady_clock::now());
		}

		bool try_lock()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				if(new_state.shared_count || new_state.exclusive)
				{
					return false;
				}
				else
				{
					new_state.exclusive=true;
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			return true;
		}

		template <class Rep, class Period>
		bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return try_lock_until(std::chrono::steady_clock::now() + rel_time);
		}
		template <class Clock, class Duration>
		bool try_lock_until(const std::chrono::time_point<Clock, Duration>& t)
		{
			using namespace std;
			using namespace chrono;
			system_clock::time_point     s_now = system_clock::now();
			typename Clock::time_point  c_now = Clock::now();
			return try_lock_until(s_now + ceil<system_clock::duration>(t - c_now));
		}
		template <class Duration>
		bool try_lock_until(const std::chrono::time_point<std::chrono::system_clock, Duration>& t)
		{
			using namespace std;
			using namespace chrono;
			typedef time_point<chrono::system_clock, chrono::system_clock::duration> sys_tmpt;
			//return try_lock_until(sys_tmpt(chrono::ceil<chrono::system_clock::duration>(t.time_since_epoch())));
			return try_lock_until(sys_tmpt(t.time_since_epoch()));
		}
		bool try_lock_until(const std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration>& tp)
		{
			using namespace std;
			for(;;)
			{
				state_data old_state=state;

				for(;;)
				{
					state_data new_state=old_state;
					if(new_state.shared_count || new_state.exclusive)
					{
						++new_state.exclusive_waiting;
						if(!new_state.exclusive_waiting)
						{
							std::exception("shared_mutex: lock error");
						}

						new_state.exclusive_waiting_blocked=true;
					}
					else
					{
						new_state.exclusive=true;
					}

					state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
					if(current_state==old_state)
					{
						break;
					}
					old_state=current_state;
				}

				if(!old_state.shared_count && !old_state.exclusive)
				{
					return true;
				}
#ifndef UNDER_CE
				const bool wait_all = true;
#else
				const bool wait_all = false;
#endif

				chrono::system_clock::time_point n = chrono::system_clock::now();
				unsigned long wait_res;
				if (tp>n) {
					chrono::milliseconds rel_time = chrono::duration_cast<chrono::milliseconds>(tp-chrono::system_clock::now());
					wait_res=WaitForMultipleObjectsEx(2,semaphores,wait_all,
						static_cast<unsigned long>(rel_time.count()), 0);
				} else {
					wait_res=WAIT_TIMEOUT;
				}
				if(wait_res==WAIT_TIMEOUT)
				{
					for(;;)
					{
						bool must_notify = false;
						state_data new_state=old_state;
						if(new_state.shared_count || new_state.exclusive)
						{
							if(new_state.exclusive_waiting)
							{
								if(!--new_state.exclusive_waiting)
								{
									new_state.exclusive_waiting_blocked=false;
									must_notify = true;
								}
							}
						}
						else
						{
							new_state.exclusive=true;
						}

						state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
						if (must_notify)
						{
							assert(ReleaseSemaphore(semaphores[unlock_sem],1,0)!=0);
						}
						if(current_state==old_state)
						{
							break;
						}
						old_state=current_state;
					}
					if(!old_state.shared_count && !old_state.exclusive)
					{
						return true;
					}
					return false;
				}
				assert(wait_res<2);
			}
		}


		void unlock()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				new_state.exclusive=false;
				if(new_state.exclusive_waiting)
				{
					--new_state.exclusive_waiting;
					new_state.exclusive_waiting_blocked=false;
				}
				new_state.shared_waiting=0;

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			release_waiters(old_state);
		}

		void lock_upgrade()
		{
			for(;;)
			{
				state_data old_state=state;
				for(;;)
				{
					state_data new_state=old_state;
					if(new_state.exclusive || new_state.exclusive_waiting_blocked || new_state.upgrade)
					{
						++new_state.shared_waiting;
						if(!new_state.shared_waiting)
						{
							std::exception("shared_mutex: lock error");
						}
					}
					else
					{
						++new_state.shared_count;
						if(!new_state.shared_count)
						{
							std::exception("shared_mutex: lock error");
						}
						new_state.upgrade=true;
					}

					state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
					if(current_state==old_state)
					{
						break;
					}
					old_state=current_state;
				}

				if(!(old_state.exclusive|| old_state.exclusive_waiting_blocked|| old_state.upgrade))
				{
					return;
				}

				WaitForSingleObjectEx(semaphores[unlock_sem],INFINITE, 0);
			}
		}

		bool try_lock_upgrade()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				if(new_state.exclusive || new_state.exclusive_waiting_blocked || new_state.upgrade)
				{
					return false;
				}
				else
				{
					++new_state.shared_count;
					if(!new_state.shared_count)
					{
						return false;
					}
					new_state.upgrade=true;
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			return true;
		}

		void unlock_upgrade()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				new_state.upgrade=false;
				bool const last_reader=!--new_state.shared_count;

				if(last_reader)
				{
					if(new_state.exclusive_waiting)
					{
						--new_state.exclusive_waiting;
						new_state.exclusive_waiting_blocked=false;
					}
					new_state.shared_waiting=0;
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					if(last_reader)
					{
						release_waiters(old_state);
					}
					else {
						release_shared_waiters(old_state);
					}
					// #7720
					//else {
					//    release_waiters(old_state);
					//}
					break;
				}
				old_state=current_state;
			}
		}

		void unlock_upgrade_and_lock()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				bool const last_reader=!--new_state.shared_count;

				if(last_reader)
				{
					new_state.upgrade=false;
					new_state.exclusive=true;
				}

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					if(!last_reader)
					{
						assert(!WaitForSingleObjectEx(upgrade_sem, INFINITE, 0));
					}
					break;
				}
				old_state=current_state;
			}
		}

		void unlock_and_lock_upgrade()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				new_state.exclusive=false;
				new_state.upgrade=true;
				++new_state.shared_count;
				if(new_state.exclusive_waiting)
				{
					--new_state.exclusive_waiting;
					new_state.exclusive_waiting_blocked=false;
				}
				new_state.shared_waiting=0;

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			release_waiters(old_state);
		}

		void unlock_and_lock_shared()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				new_state.exclusive=false;
				++new_state.shared_count;
				if(new_state.exclusive_waiting)
				{
					--new_state.exclusive_waiting;
					new_state.exclusive_waiting_blocked=false;
				}
				new_state.shared_waiting=0;

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			release_waiters(old_state);
		}
		void unlock_upgrade_and_lock_shared()
		{
			state_data old_state=state;
			for(;;)
			{
				state_data new_state=old_state;
				new_state.upgrade=false;
				if(new_state.exclusive_waiting)
				{
					--new_state.exclusive_waiting;
					new_state.exclusive_waiting_blocked=false;
				}
				new_state.shared_waiting=0;

				state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
				if(current_state==old_state)
				{
					break;
				}
				old_state=current_state;
			}
			release_waiters(old_state);
		}

	};
	typedef shared_mutex upgrade_mutex;
}