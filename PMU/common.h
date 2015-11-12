//------------------------------------------------------------------------------
/// @brief PMU common header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/11 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription common.h
/// PMU framework common description header
///
//------------------------------------------------------------------------------

#ifndef __PMU_COMMON_H_
#define __PMU_COMMON_H_

/// @brief define the architecture-specific management
///
/// @description currently we support two different platforms
//TODO: define AMD32 and distinguish AMD32 and AMD64
#if defined(__x86_64__)
#define AMD64
#elif defined(__tile__)
#define TILEGX36
#else
#error PMU manager does not support this platform
#endif

/// @brief turn on/off PMU options
///
/// @description 1: turn on the option, 0: turn off the option

#endif // __PMU_COMMON_H_

