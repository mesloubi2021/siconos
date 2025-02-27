/* Siconos is a program dedicated to modeling, simulation and control
 * of non smooth dynamical systems.
 *
 * Copyright 2022 INRIA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/*!\file numerics_verbose.h
 * verbosity related functions*/

#ifndef _NUMERICS_VERBOSE_INTERNAL_H_
#define _NUMERICS_VERBOSE_INTERNAL_H_

#include "SiconosConfig.h"
#include "NumericsVerbose.h"
#include <errno.h>
#include <stdint.h>
#include <stddef.h>

#include "tlsdef.h"

/** Verbose mode */
extern tlsvar int verbose;

#if defined(__cplusplus) && !defined (BUILD_AS_CPP)
extern "C"
{
#endif

/* Check return code of an expression. */
#define CHECK_RETURN(EXPR)                                              \
  do                                                                    \
  {                                                                     \
    if (!EXPR)                                                          \
    {                                                                   \
      fprintf (stderr, "Siconos Numerics: Warning %s failed, %s:%d\n",  \
               #EXPR, __FILE__, __LINE__);                              \
    }                                                                   \
  } while (0)

/* Check return code of a first expression and execute the second */
#define CHECK_AND_DO(EXPR, TODOIFOK)                                    \
  do                                                                    \
  {                                                                     \
    if (!EXPR)                                                          \
    {                                                                   \
      fprintf (stderr, "Siconos Numerics: Warning %s failed, %s:%d\n",  \
               #EXPR, __FILE__, __LINE__);                              \
    }                                                                   \
    else                                                                \
    {                                                                   \
      (TODOIFOK);                                                       \
    }                                                                   \
  } while (0)

/** check IO
 */
#define CHECK_IO(EXPR, ...)                                             \
  do                                                                    \
  {                                                                     \
    if (!EXPR)                                                          \
    {                                                                   \
      int * _arr_[] = {NULL, __VA_ARGS__};                              \
      if (errno != 0)                                                   \
      {                                                                 \
        perror(#EXPR);                                                  \
        fprintf (stderr, "Siconos Numerics: Warning %s failed, %s:%d\n", #EXPR, __FILE__, __LINE__); \
        if (sizeof(_arr_) == 2*sizeof(intptr_t)) { *_arr_[sizeof(_arr_)/sizeof(intptr_t)-1] = errno; } \
      }                                                                 \
      else                                                              \
      {                                                                 \
        fprintf (stderr, "Siconos Numerics: Unknown error for %s, %s:%d\n", #EXPR, __FILE__, __LINE__); \
        if (sizeof(_arr_) == 2*sizeof(intptr_t)) { *_arr_[sizeof(_arr_)/sizeof(intptr_t)-1] = 1; }                           \
      }                                                                 \
    }                                                                   \
  } while (0)

/* ignore IO. Think carefully before using this ...*/
#define IGNORE_IO(EXPR)                                                 \
  do                                                                    \
  {                                                                     \
    if(EXPR){};                                                         \
  } while (0)

#ifdef __clang_analyzer__
#define NO_RETURN  __attribute__((analyzer_noreturn))
#else
#define NO_RETURN
#endif

#ifdef __GNUC__
#define MAYBE_UNUSED __attribute__((unused))
#else
#define MAYBE_UNUSED
#endif

/* Functions that insert NaN have error codes that should not be ignored! */
#ifdef __GNUC__
#define WARN_RESULT_IGNORED __attribute__ ((warn_unused_result))
#else
#define WARN_RESULT_IGNORED
#endif

  

  /** msg output and exit with error
   *
   * \param fn_name name of the function where error occurs
   * \param msg formatted output msg
   */
  void numerics_error(const char* fn_name, const char* msg, ...) NO_RETURN;

  /** log error message without exit
   *
   *  \param fn_name name of the function where the error occurs
   *  \param msg formatted output message
   */
  void numerics_error_nonfatal(const char* fn_name, const char* msg, ...);

  /** log warning
   *
   *  \param fn_name name of the function where warning occurs
   *  \param msg formatted output message
   */
  void numerics_warning(const char * fn_name, const char* msg, ...);

  /** log message (if verbosity is on)
   *
   *  \param fmt formatted output message
  */
  void numerics_printf(const char * fmt, ...);

  /** log message if verbose is non null
   *
   *  \param verbose log message is non null
   *  \param fmt formatted output message
   */
  void numerics_printf_verbose(int verbose, const char * fmt, ...);

#if defined(__cplusplus) && !defined (BUILD_AS_CPP)
}
#endif

#endif
