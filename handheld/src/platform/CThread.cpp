/*
 *  CThread.cpp
 *  oxeye
 *
 *  Created by aegzorz on 2007-02-09.
 *  Copyright 2007 Mojang AB. All rights reserved.
 *
 */

#include "CThread.h"

#ifdef __VITA__
#include <psp2/kernel/threadmgr.h>
#endif

static int vita_thread_entry(size_t argc, void* argv) {
	void** args = (void**)argv;
	((void(*)(void*))args[0])(args[1]);
	return 0;
}


	CThread::CThread( pthread_fn threadFunc, void* threadParam )
	{
	#ifdef WIN32
		mp_threadFunc = (LPTHREAD_START_ROUTINE) threadFunc;
		
		m_threadHandle = CreateThread(
			NULL,				// pointer to security attributes
			NULL,               // initial thread stack size
			mp_threadFunc,		// pointer to thread function
			threadParam,        // argument for new thread
			NULL,               // creation flags
			&m_threadID        // pointer to receive thread ID
		);
	#endif
	#if defined(LINUX) || defined(ANDROID) || defined(__APPLE__) || defined(POSIX) || defined(__SWITCH__) || defined(__3DS__) || defined(__NDS__)
		mp_threadFunc = (pthread_fn)threadFunc;

		pthread_attr_init(&m_attributes);
		pthread_attr_setdetachstate( &m_attributes, PTHREAD_CREATE_DETACHED );
		/*int error =*/ pthread_create(&m_thread, &m_attributes, mp_threadFunc,threadParam);
	#endif
	#ifdef MACOSX
		mp_threadFunc = (TaskProc) threadFunc;
	
		MPCreateTask(
			mp_threadFunc,		// pointer to thread function
			threadParam,		// argument for new thread
			0,					// initial thread stack size
			NULL,				// queue id
			NULL,				// termination param 1
			NULL,				// termination param 2
			0,					// task options
			&m_threadID			// pointer to receive task ID
		);
	#endif
	#ifdef __VITA__
		m_thread = sceKernelCreateThread("CThread", vita_thread_entry, 0x10000100, 2 * 1024 * 1024, 0, 0, NULL);
		void* args[2] = {
			(void*)threadFunc,
			threadParam
		};
		sceKernelStartThread(m_thread, 8, args);
	#endif
	}
	
	void CThread::sleep( const unsigned int millis )
	{
		#ifdef WIN32
			Sleep( millis );
		#endif
		#if defined(LINUX) || defined(ANDROID) || defined(__APPLE__) || defined(POSIX) || defined(__SWITCH__) || defined(__3DS__) || defined(__NDS__)
			usleep(millis * 1000);
		#endif
		#if defined(__VITA__)
			sceKernelDelayThread(millis * 1000);
		#endif
	}

	CThread::~CThread()
	{
	#ifdef WIN32
		TerminateThread(m_threadHandle, 0);
	#endif
	#if defined(LINUX) || defined(ANDROID) || defined(__APPLE__) || defined(POSIX) || defined (__SWITCH__) || defined(__3DS__) || defined(__NDS__)
		pthread_join(m_thread, NULL);
		pthread_attr_destroy(&m_attributes);
	#endif
	#if defined(__VITA__)
		// this is impossible on vita, oh well
	#endif
	}


