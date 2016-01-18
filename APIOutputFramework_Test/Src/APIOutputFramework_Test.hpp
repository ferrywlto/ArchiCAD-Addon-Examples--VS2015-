/**
 * @file Contains the includes and definitions necessary for the Add-On to
 *       function.
 */

#if !defined (__APIOUTPUTFRAMEWORK_TEST_HPP__)
#define __APIOUTPUTFRAMEWORK_TEST_HPP__

#ifdef _WIN32
	#pragma warning (push, 3)
	#include	<Win32Interface.hpp>
	#pragma warning (pop)

	#ifndef WINDOWS
	#define WINDOWS
	#endif
#endif

#ifdef macintosh
	#include <CoreServices/CoreServices.h>
#endif

#ifndef ACExtension
	#define	ACExtension
#endif

#ifdef WINDOWS
	#pragma warning (disable: 4068 4786)
#endif

#include "ACAPinc.h"
#include "Location.hpp"

#ifdef WINDOWS
	#pragma warning (default: 4068)
#endif

#endif //__APIOUTPUTFRAMEWORK_TEST_HPP__
