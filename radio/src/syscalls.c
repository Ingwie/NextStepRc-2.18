/*
 *************************************************************
 *                      NEXTSTEPRC                           *
 *                                                           *
 *             -> Build your DIY MEGA 2560 TX                *
 *                                                           *
 *      Based on code named                                  *
 *      OpenTx - https://github.com/opentx/opentx            *
 *                                                           *
 *         Only avr code here for lisibility ;-)             *
 *                                                           *
 *  License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html  *
 *                                                           *
 *************************************************************
 */

/**
  * \file syscalls.c
  *
  * Implementation of newlib syscall.
  *
  */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/


// #include "board.h"

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/*----------------------------------------------------------------------------
 *        Exported variables
 *----------------------------------------------------------------------------*/

#undef errno
extern int errno;
extern int _end;
extern int _heap_end;

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
extern void _exit( int status ) ;
extern void _kill( int pid, int sig ) ;
extern int _getpid ( void ) ;

unsigned char *heap = (unsigned char *)&_end;
extern caddr_t _sbrk(int nbytes)
{
  if (heap + nbytes < (unsigned char *)&_heap_end) {
    unsigned char *prev_heap = heap;
    heap += nbytes;
    return (caddr_t) prev_heap;
  }
  else {
    errno = ENOMEM;
    return ((void*)-1);
  }
}

extern int _gettimeofday(void *p1, void *p2)
{
   return 0 ;
}

extern int _link( char *old, char *nw )
{
    return -1 ;
}

extern int _unlink (const char *path)
{
    return -1 ;
}

extern int _open(const char *name, int flags, int mode)
{
    return -1 ;
}

extern int _close( int file )
{
    return -1 ;
}

extern int _fstat( int file, struct stat *st )
{
    st->st_mode = S_IFCHR ;

    return 0 ;
}

extern int _isatty( int file )
{
    return 1 ;
}

extern int _lseek( int file, int ptr, int dir )
{
    return 0 ;
}

extern int _read(int file, char *ptr, int len)
{
    return 0 ;
}

extern int _write( int file, char *ptr, int len )
{
    int iIndex ;
    
    
//    for ( ; *ptr != 0 ; ptr++ )
    for ( iIndex=0 ; iIndex < len ; iIndex++, ptr++ )
    {
// TODO        UART_PutChar( *ptr ) ;
    }

    return iIndex ;
}

extern void _exit( int status )
{
#if defined(SIMU)
    printf( "Exiting with status %d.\n", status ) ;
#endif
    for ( ; ; ) ;
}

extern void _kill( int pid, int sig )
{
    return ; 
}

extern int _getpid ( void )
{
    return -1 ;
}

void _init (void)
{
  
}
