#include <errno.h>

#if defined(_WIN32)     /* Win32 or Win64                */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#define S_IRWXU    0700  /* Mask for file owner permissions                 */

#else                   /* OSX or Linux                */

#include <string.h>
#include <limits.h>     /* PATH_MAX */
#include <sys/stat.h>   /* mkdir(2) */

#endif

int mkdir_p(const char *path)
{
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
