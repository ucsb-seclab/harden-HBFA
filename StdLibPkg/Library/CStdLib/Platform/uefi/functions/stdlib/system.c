/** @file
  Implementation of system().

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Obtained from pdclib project licensed under CC0.

**/

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int fork( void );
extern int execve( const char * filename, char * const argv[], char * const envp[] );
extern int wait( int * status );

#ifdef __cplusplus
}
#endif

int system( const char * string )
{
    const char * argv[] = { "sh", "-c", NULL, NULL };
    argv[2] = string;

    if ( string != NULL )
    {
        int pid = fork();

        if ( pid == 0 )
        {
            execve( "/bin/sh", ( char * const *)argv, NULL );
        }
        else if ( pid > 0 )
        {
            while ( wait( NULL ) != pid )
            {
                /* EMPTY */
            }
        }
    }

    return -1;
}

#ifdef TEST

#include "_PDCLIB_test.h"

#define SHELLCOMMAND "echo 'SUCCESS testing system()'"

int main( void )
{
    FILE * fh;
    char buffer[25];
    buffer[24] = 'x';
    TESTCASE( ( fh = freopen( testfile, "wb+", stdout ) ) != NULL );
    TESTCASE( system( SHELLCOMMAND ) );
    rewind( fh );
    TESTCASE( fread( buffer, 1, 24, fh ) == 24 );
    TESTCASE( memcmp( buffer, "SUCCESS testing system()", 24 ) == 0 );
    TESTCASE( buffer[24] == 'x' );
    TESTCASE( fclose( fh ) == 0 );
    TESTCASE( remove( testfile ) == 0 );
    return TEST_RESULTS;
}

#endif
