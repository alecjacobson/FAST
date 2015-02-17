// overrides global new, new[], delete and delete[] with dlmalloc routines (much faster than the default which goes to kernel calls)

#define USE_DL_ALLOCS
#ifdef USE_DL_ALLOCS

#ifdef _WIN32 // only works on Windows, Mac version is TODO
#include "malloc.h"
#include <new>
#include <cstdio>
#include <windows.h>

CRITICAL_SECTION g_myNew_CriticalSection; 
bool g_myNew_CSinitialized = false;

void* generic_new (size_t size)
{	
	if (!g_myNew_CSinitialized)
	{
		if (!InitializeCriticalSectionAndSpinCount(&g_myNew_CriticalSection, 0x80000400))
		{
			fprintf(stderr, "FATAL ERROR: InitializeCriticalSectionAndSpinCount in new failed\n");
			exit(-1);
		}
		g_myNew_CSinitialized = true;
	}

	EnterCriticalSection(&g_myNew_CriticalSection);
	void *p=dlmalloc(size); 
	LeaveCriticalSection(&g_myNew_CriticalSection);
	if (p==0) // did malloc succeed?
		throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void* operator new (size_t size)
{
	return generic_new(size);
}

void* operator new[] (size_t size)
{
	return generic_new(size);
}

void operator delete (void *p)
{
	if (!g_myNew_CSinitialized)
	{
		fprintf(stderr, "FATAL ERROR: delete called before new!\n");
		exit(-1);
	}

	if (p == NULL)	return;
	EnterCriticalSection(&g_myNew_CriticalSection);
	dlfree(p);
	LeaveCriticalSection(&g_myNew_CriticalSection);
}

void operator delete[]( void * p )
{
	operator delete(p);
}

#endif
#endif