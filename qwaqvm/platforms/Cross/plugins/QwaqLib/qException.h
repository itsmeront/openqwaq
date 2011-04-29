/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the GNU General Public
 * License, Version 2 (the "License"); you may not use this file
 * except in compliance with the License. A copy of the License is
 * available at http://www.opensource.org/licenses/gpl-2.0.php.
 *
 */

/******************************************************************************
 *
 * qException.h
 * QwaqLib (cross-platform)
 *
 * Simple exception classes.
 * Not used yet, but I want a framework where exceptions can be stashed
 * until Squeak-code asks for them (makes for better debugging).
 *
 ******************************************************************************/


#ifndef __Q_EXCEPTION_H__
#define __Q_EXCEPTION_H__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace Qwaq
{
	// Error values
	enum Status { 
		QS_OK					= 0, 
		QE_INDEX_OUT_OF_RANGE	= -1,
		QE_ALLOCATION_FAILED	= -2,
		QE_INVALID_DATA_SOURCE	= -3,
		QE_NO_FREE_OBJECT_SLOTS	= -4,
		QE_CAUGHT_EXCEPTION		= -5,
		QE_COM_ERROR			= -6,
		QE_DEVICE_UNAVAILABLE	= -7,
		QE_DSHOW_ERROR			= -8
	};

	// Base exception type
	class Exception
	{
	public:
		Exception(std::string description);
		std::vector<std::string>& getDescriptions() { return descriptions; }

	protected:
		std::vector<std::string> descriptions;
	};
	typedef boost::shared_ptr<Exception> ExceptionPtr;


	class FailedAllocation : public Exception {
		
	};


} //namespace Qwaq

#endif //#define __Q_EXCEPTION_H__
