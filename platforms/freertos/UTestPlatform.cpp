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
#include <CppUTest/TestHarness.h>
#undef malloc
#undef calloc
#undef realloc
#undef free

/* STL Includes */
#include <array>

/* CppUTest Includes */
#include <CppUTest/PlatformSpecificFunctions.h>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/serial>

/* FreeRTOS Includes */
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/portable.h>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
static jmp_buf test_exit_jmp_buf[ 10 ];
static int jmp_buf_index = 0;


/*-------------------------------------------------------------------------------
Process Hooks
-------------------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------------
C++ Memory Overloads
-------------------------------------------------------------------------------*/
// For some reason this one is needed for test creation. It's a bit odd.
void *operator new( unsigned int x, char const *file, unsigned int line )
{
  return pvPortMalloc( x );
}

extern "C"
{
  /*-------------------------------------------------------------------------------
  Jumping Hooks
  -------------------------------------------------------------------------------*/
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


  /*-------------------------------------------------------------------------------
  Timing Hooks
  -------------------------------------------------------------------------------*/
  static long TimeInMillisImplementation()
  {
    return Chimera::millis();
  }

  static const char *DummyTimeStringImplementation()
  {
    time_t tm = time( NULL );
    return ctime( &tm );
  }

  long ( *GetPlatformSpecificTimeInMillis )()      = TimeInMillisImplementation;
  const char *( *GetPlatformSpecificTimeString )() = DummyTimeStringImplementation;


  /*-------------------------------------------------------------------------------
  Character Manipulation Hooks
  -------------------------------------------------------------------------------*/
  int ( *PlatformSpecificVSNprintf )( char *str, size_t size, const char *format, va_list args ) = vsnprintf;


  /*-------------------------------------------------------------------------------
  File Operation Hooks
  -------------------------------------------------------------------------------*/
  static PlatformSpecificFile PlatformSpecificFOpenImplementation( const char *filename, const char *flag )
  {
    return nullptr;
  }

  static void PlatformSpecificFPutsImplementation( const char *str, PlatformSpecificFile file )
  {
  }

  static void PlatformSpecificFCloseImplementation( PlatformSpecificFile file )
  {
  }


  PlatformSpecificFile ( *PlatformSpecificFOpen )( const char *, const char * ) = PlatformSpecificFOpenImplementation;
  void ( *PlatformSpecificFPuts )( const char *, PlatformSpecificFile )         = PlatformSpecificFPutsImplementation;
  void ( *PlatformSpecificFClose )( PlatformSpecificFile )                      = PlatformSpecificFCloseImplementation;


  /*-------------------------------------------------------------------------------
  Output Stream Stubs
  -------------------------------------------------------------------------------*/
  static std::array<uint8_t, 256> ostreamBuff;
  static size_t ostreamBuffIdx = 0;

  static int PlatformSpecificPutCharImplementation( int x )
  {
    /*-------------------------------------------------
    Reset the buffer if we try to write into or past
    the null terminator.
    -------------------------------------------------*/
    if ( ostreamBuffIdx >= ( ostreamBuff.size() - 1 ) )
    {
      ostreamBuff.fill( 0 );
      ostreamBuffIdx = 0;
    }

    /*-------------------------------------------------
    Store the new character
    -------------------------------------------------*/
    ostreamBuff[ ostreamBuffIdx ] = static_cast<uint8_t>( x );
    ostreamBuffIdx++;

    return x;
  }

  static void PlatformSpecificFlushImplementation()
  {
    auto serial = Chimera::Serial::getDriver( Chimera::Serial::Channel::SERIAL1 );

    /*-------------------------------------------------
    Dump the data to the serial console
    -------------------------------------------------*/
    serial->lock();
    serial->write( ostreamBuff.data(), ostreamBuffIdx );
    serial->await( Chimera::Event::Trigger::TRIGGER_WRITE_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    serial->unlock();

    /*-------------------------------------------------
    Reset the buffer
    -------------------------------------------------*/
    ostreamBuff.fill( 0 );
    ostreamBuffIdx = 0;
  }

  int ( *PlatformSpecificPutchar )( int ) = PlatformSpecificPutCharImplementation;
  void ( *PlatformSpecificFlush )()       = PlatformSpecificFlushImplementation;


  /*-------------------------------------------------------------------------------
  Memory Stubs
  -------------------------------------------------------------------------------*/
  static void *PlatformSpecificMallocImplementation( size_t size )
  {
    return pvPortMalloc( size );
  }


  static void *PlatformSpecificReallocImplementation( void *mem, size_t size )
  {
    vPortFree( mem );
    return pvPortMalloc( size );
  }


  static void PlatformSpecificFreeImplementation( void *mem )
  {
    vPortFree( mem );
  }

  void *( *PlatformSpecificMalloc )( size_t size )                  = PlatformSpecificMallocImplementation;
  void *( *PlatformSpecificRealloc )( void *, size_t )              = PlatformSpecificReallocImplementation;
  void ( *PlatformSpecificFree )( void *memory )                    = PlatformSpecificFreeImplementation;
  void *( *PlatformSpecificMemCpy )( void *, const void *, size_t ) = memcpy;
  void *( *PlatformSpecificMemset )( void *, int, size_t )          = memset;


  /*-------------------------------------------------------------------------------
  Math Operation Hooks
  -------------------------------------------------------------------------------*/
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
    srand( x );
  }

  static int RandImplementation()
  {
    return rand();
  }

  double ( *PlatformSpecificFabs )( double )                = fabs;
  int ( *PlatformSpecificIsNan )( double )                  = IsNanImplementation;
  int ( *PlatformSpecificIsInf )( double )                  = IsInfImplementation;
  int ( *PlatformSpecificAtExit )( void ( *func )( void ) ) = AtExitImplementation;
  void ( *PlatformSpecificSrand )( unsigned int )           = SrandImplementation;
  int ( *PlatformSpecificRand )( void )                     = RandImplementation;


  /*-------------------------------------------------------------------------------
  Threading Operation Hooks
  -------------------------------------------------------------------------------*/
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

}  // extern C
