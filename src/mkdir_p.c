#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include "compare.h"

#if defined(_WIN32)         /* Win32 or Win64 */

#include <limits.h>         /* PATH_MAX */
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#define S_IRWXU    0700     /* Mask for file owner permissions */

#else                       /* OSX or Linux */

#include <linux/limits.h>   /* PATH_MAX */
#include <string.h>
#include <sys/stat.h>       /* mkdir(2) */

#endif

const char *PATH_SEPERATOR_STR =
#ifdef _WIN32
"\\";
#else
"/";
#endif

bool PathValid( const char * path )
{
    if( path != NULL )
    {
        if( strlen( path ) <= PATH_MAX ) {
            int i;
            char *_path = NULL;
            char *ptr = NULL;
            const size_t len = strlen(path);
            size_t len_token;

            /* Copy string so its mutable (strtok requirement) */
            _path = (char*)malloc((len+1)*sizeof(char));
            if (_path == NULL){
                perror("Error: Failed to allocate memory for _path in mkdir_p.");
            }
            strcpy(_path, path);

            ptr = strtok( _path, PATH_SEPERATOR_STR );

            while(ptr != NULL) {  /* Iterate on path components */
                len_token = strlen(ptr);
                for( i = 0; i < len_token; i++ ) {  /* Iterate on component's tokens */
                    switch( ptr[i] )
                    {
                        case '?':
                        case '\"':
                        case '/':
                        case '\\':
                        case '<':
                        case '>':
                        case '*':
                        case '|':
                            return false;

                        /* Can meet only between a local disk letter and full path
                           for example D:\folder\file.txt */
                        case ':':
                        {
                            if( i != 1 )
                                return false;
                        }

                        /* Space and point can not be the last character of a file or folder names */
                        case ' ':
                        case '.':
                        {
                            if( ( i + 1 == len ) || ( ptr[i+1] == PATH_SEPERATOR_STR[0] ) )
                                return false;
                        }
                    }
                }
                ptr = strtok(NULL, PATH_SEPERATOR_STR);
            }
            return true;
        }
        return false;
    }
    return false;
}

int mkdir_p(const char *path)
{
    if (!(PathValid(path))) {
        return -1;
    }

    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char *_path = NULL;
    char *p;

    _path = (char*)malloc((len+1)*sizeof(char));
    if (_path == NULL){
      perror("Error: Failed to allocate memory for _path in mkdir_p.");
    }
    errno = 0;

    /* Copy string so its mutable */
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1;
    }

    return 0;
}
