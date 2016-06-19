/* 

Amine Rehioui
Created: November 27th 2011

*/

#ifndef _THREAD_H_INCLUDED_
#define _THREAD_H_INCLUDED_

namespace shoot
{
	//! Thread class
	class Thread
	{
	public:

		typedef void* ThreadFunc(void* pParam);

		//! creates a thread depending on the platform
		static Thread* CreateThread();

		//! yields a thread execution
		static void YieldThread();

		//! constructor
		Thread();

		//! destructor
		virtual ~Thread();

		//! starts the thread
		virtual void Start(ThreadFunc threadFunc, void* pParam = 0) = 0;
	};
}

#endif // _THREAD_H_INCLUDED_

