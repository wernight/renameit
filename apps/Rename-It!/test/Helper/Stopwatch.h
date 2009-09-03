/**
 * Copyright (c) 2003 Werner BEROUX. All Rights Reserved.
 *
 * The copyright to the contents herein is the property of Werner BEROUX.
 * The contents may be used and/or copied only with the written permission of
 * Werner BEROUX, or in accordance with the terms and conditions stipulated in
 * the agreement/contract under which the contents have been supplied.
 *
 * A simple chronometer in seconds. Can be used for multimedia.
 *
 * Example:
 * @code
 *   Stopwatch<> chrono;
 *   chrono.Init();
 *   chrono.Start();
 *   ...
 *   float elapsed = chrono.GetElapsed();
 * @codeend
 *
 * @brief Stopwatch.
 * @version 1.10
 * @date 2007.11.13
 */

#ifndef STOPWATCH_H_HEADER
#define STOPWATCH_H_HEADER

#ifdef _MSC_VER
#	pragma once
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

template <typename TFloat=float>
struct Stopwatch
{
	Stopwatch() :
		m_fResolution(0)
	{
	}

	// Initialize timer
	inline bool Stopwatch::Init(void)
	{
		LARGE_INTEGER nFreq;

		if (!QueryPerformanceFrequency(&nFreq))
			return false;
		m_fResolution = (TFloat)(1/(TFloat)nFreq.QuadPart);
		return true;
	}

	// Start timer
	inline void Stopwatch::Start(void)
	{
		assert(m_fResolution != 0);	// One should initialize the timer before calling Start.
		QueryPerformanceCounter(&m_nStart);
	}

	// Return elapsed time since start, in seconds.
	inline TFloat Stopwatch::GetElapsed(void)
	{
		QueryPerformanceCounter(&m_nNow);
		return (m_nNow.QuadPart - m_nStart.QuadPart) * m_fResolution;
	}

protected:
	LARGE_INTEGER m_nStart;
	LARGE_INTEGER m_nNow;
	TFloat m_fResolution; 
};

#endif	// STOPWATCH_H_HEADER