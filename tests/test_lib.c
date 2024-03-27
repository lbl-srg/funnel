#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include "../src/data_structure.h"

#if defined(_WIN32)     /* Win32 or Win64              */
#include <windows.h>
#include <winbase.h>
#include <windef.h>
#else                   /* Mac or Linux                */
#include <dlfcn.h>
#endif

int main(void) {
    char *error;
    char* libname;
    typedef struct data (*_read_csv)(const char*, int);
    typedef int (*_compare_func)(
            const double*,
            const double*,
            const size_t,
            const double*,
            const double*,
            const size_t,
            const char *,
            const double,
            const double,
            const double,
            const double,
            const double,
            const double
    );
    _read_csv read_csv;
    _compare_func compare_func;

    #if defined(_WIN32)     /* Win32 or Win64              */

    libname = "funnel.dll";
    HINSTANCE handle = LoadLibraryA(libname);
    if (handle == NULL)
    {
        fputs("Load failed\n", stderr);
        return 1;
    }
    read_csv = (_read_csv)GetProcAddress(handle, "readCSV");
    if (read_csv == NULL)
    {
        fprintf(stderr, "GetProcAddress failed with message: %d\n", GetLastError());
        return 1;
    }
    compare_func = (_compare_func)GetProcAddress(handle, "compareAndReport");
    if (compare_func == NULL)
    {
        fprintf(stderr, "GetProcAddress failed with message: %d\n", GetLastError());
        return 1;
    }

    #else                   /* Mac or Linux                */

    #if defined(__linux__)  /* Linux */
    libname = "libfunnel.so";
    #else                   /* Mac */
    libname = "libfunnel.dylib";
    #endif

    void *handle = dlopen(libname, RTLD_LAZY);
    if (!handle) {
        fputs(dlerror(), stderr);
        exit(1);
    }
    /* Trick to deal with dlsym returning a void pointer instead of a function pointer. */
    *(void **) (&read_csv) = dlsym(handle, "readCSV");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }
    /* Trick to deal with dlsym returning a void pointer instead of a function pointer. */
    *(void **) (&compare_func) = dlsym(handle, "compareAndReport");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }

    #endif      /* Mac or Linux                */

    struct data baseCSV = read_csv("trended.csv", 1);
    struct data testCSV = read_csv("simulated.csv", 1);

    int exiVal = compare_func(
        baseCSV.x,
        baseCSV.y,
        baseCSV.n,
        testCSV.x,
        testCSV.y,
        testCSV.n,
        "results",
        0.002,
        0.002,
        0.0,
        0.0,
        0.0,
        0.0
    );

    if (exiVal != 0)  {
        fprintf(stderr, "Return code from compareAndReport: %i\n", exiVal);
        return exiVal;
    }

    #if defined(_WIN32)     /* Win32 or Win64              */
    FreeLibrary(handle);
    #else
    dlclose(handle);
    #endif

    return 0;
}
