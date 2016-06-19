/* 

Amine Rehioui
Created: July 25th 2013

*/

#ifndef _THREAD_C11_H_INCLUDED_
#define _THREAD_C11_H_INCLUDED_

#include "Thread.h"

namespace shoot
{
	//! C++ 11 Thread class
	class ThreadC11 : public Thread
	{
	public:

		//! constructor
		ThreadC11();

		//! destructor
		~ThreadC11();

		//! starts the thread
		void Start(ThreadFunc threadFunc, void* pParam = 0);

		//! yields the thread execution
		static void YieldThread();

	private:

		void* m_pThread;
	};
}

#endif // _THREAD_C11_H_INCLUDED_

