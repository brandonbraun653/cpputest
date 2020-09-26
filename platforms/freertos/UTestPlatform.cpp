/********************************************************************************
 *  File Name:
 *    UTestPlatform.cpp
 *
 *  Description:
 *    FreeRTOS platform base for running CppUTests on embedded hardware
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* C Includes */
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* CppUTest Includes */
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/TestHarness.h>


static jmp_buf test_exit_jmp_buf[ 10 ];
static int jmp_buf_index = 0;

TestOutput::WorkingEnvironment PlatformSpecificGetWorkingEnvironment()
{
  return TestOutput::eclipse;
}

static void DummyPlatformSpecificRunTestInASeperateProcess( UtestShell *shell, TestPlugin *, TestResult *result )
{
  result->addFailure( TestFailure( shell, "-p doesn't work on this platform, as it is lacking fork.\b" ) );
}

static int DummyPlatformSpecificFork( void )
{
  return 0;
}

static int DummyPlatformSpecificWaitPid( int, int *, int )
{
  return 0;
}

void ( *PlatformSpecificRunTestInASeperateProcess )( UtestShell *shell, TestPlugin *plugin,
                                                     TestResult *result ) = DummyPlatformSpecificRunTestInASeperateProcess;
int ( *PlatformSpecificFork )( void )                                     = DummyPlatformSpecificFork;
int ( *PlatformSpecificWaitPid )( int, int *, int )                       = DummyPlatformSpecificWaitPid;

extern "C"
{
  static int PlatformSpecificSetJmpImplementation( void ( *function )( void *data ), void *data )
  {
    if ( 0 == setjmp( test_exit_jmp_buf[ jmp_buf_index ] ) )
    {
      jmp_buf_index++;
      function( data );
      jmp_buf_index--;
      return 1;
    }
    return 0;
  }


  static void PlatformSpecificLongJmpImplementation()
  {
    jmp_buf_index--;
    longjmp( test_exit_jmp_buf[ jmp_buf_index ], 1 );
  }


  static void PlatformSpecificRestoreJumpBufferImplementation()
  {
    jmp_buf_index--;
  }

  void ( *PlatformSpecificLongJmp )()                             = PlatformSpecificLongJmpImplementation;
  int ( *PlatformSpecificSetJmp )( void ( * )( void * ), void * ) = PlatformSpecificSetJmpImplementation;
  void ( *PlatformSpecificRestoreJumpBuffer )()                   = PlatformSpecificRestoreJumpBufferImplementation;


  ///////////// Time in millis
  /*
   *  In Keil MDK-ARM, clock() default implementation used semihosting.
   *  Resolutions is user adjustable (1 ms for now)
   */
  static long TimeInMillisImplementation()
  {
    return 0;
  }

  ///////////// Time in String

  static const char *DummyTimeStringImplementation()
  {
    time_t tm = time( NULL );
    return ctime( &tm );
  }

  long ( *GetPlatformSpecificTimeInMillis )()      = TimeInMillisImplementation;
  const char *( *GetPlatformSpecificTimeString )() = DummyTimeStringImplementation;

  int ( *PlatformSpecificVSNprintf )( char *str, size_t size, const char *format, va_list args ) = vsnprintf;

  static PlatformSpecificFile PlatformSpecificFOpenImplementation( const char *filename, const char *flag )
  {
    return fopen( filename, flag );
  }

  static void PlatformSpecificFPutsImplementation( const char *str, PlatformSpecificFile file )
  {
  }

  static void PlatformSpecificFCloseImplementation( PlatformSpecificFile file )
  {
  }

  static void PlatformSpecificFlushImplementation()
  {
  }

  PlatformSpecificFile ( *PlatformSpecificFOpen )( const char *, const char * ) = PlatformSpecificFOpenImplementation;
  void ( *PlatformSpecificFPuts )( const char *, PlatformSpecificFile )         = PlatformSpecificFPutsImplementation;
  void ( *PlatformSpecificFClose )( PlatformSpecificFile )                      = PlatformSpecificFCloseImplementation;

  int ( *PlatformSpecificPutchar )( int ) = putchar;
  void ( *PlatformSpecificFlush )()       = PlatformSpecificFlushImplementation;

  void *( *PlatformSpecificMalloc )( size_t size )                  = malloc;
  void *( *PlatformSpecificRealloc )( void *, size_t )              = realloc;
  void ( *PlatformSpecificFree )( void *memory )                    = free;
  void *( *PlatformSpecificMemCpy )( void *, const void *, size_t ) = memcpy;
  void *( *PlatformSpecificMemset )( void *, int, size_t )          = memset;




  static int IsNanImplementation( double d )
  {
    return isnan( d );
  }

  static int IsInfImplementation( double d )
  {
    return isinf( d );
  }

  static int AtExitImplementation( void ( *func )( void ) )
  {
    return atexit( func );
  }

  static void SrandImplementation( unsigned int x )
  {

  }

  static int RandImplementation()
  {
    return 1;
  }

  double ( *PlatformSpecificFabs )( double )                = fabs;
  int ( *PlatformSpecificIsNan )( double )                  = IsNanImplementation;
  int ( *PlatformSpecificIsInf )( double )                  = IsInfImplementation;
  int ( *PlatformSpecificAtExit )( void ( *func )( void ) ) = AtExitImplementation;
  void ( *PlatformSpecificSrand )( unsigned int )           = SrandImplementation;
  int ( *PlatformSpecificRand )( void )                     = RandImplementation;


  static PlatformSpecificMutex DummyMutexCreate( void )
  {
    return 0;
  }

  static void DummyMutexLock( PlatformSpecificMutex )
  {
  }

  static void DummyMutexUnlock( PlatformSpecificMutex )
  {
  }

  static void DummyMutexDestroy( PlatformSpecificMutex )
  {
  }

  PlatformSpecificMutex ( *PlatformSpecificMutexCreate )( void )  = DummyMutexCreate;
  void ( *PlatformSpecificMutexLock )( PlatformSpecificMutex )    = DummyMutexLock;
  void ( *PlatformSpecificMutexUnlock )( PlatformSpecificMutex )  = DummyMutexUnlock;
  void ( *PlatformSpecificMutexDestroy )( PlatformSpecificMutex ) = DummyMutexDestroy;

}    // extern C
