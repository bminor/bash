/*
 * HPUX 10.x stubs to implement dl* in terms of shl*
 *
 * Not needed for later versions; HPUX 11.x has dlopen() and friends.
 *
 * configure also needs to be faked out.  You can create a dummy libdl.a
 * with stub entries for dlopen, dlclose, dlsym, and dlerror:
 *
 *	int dlopen() { return(0);}
 *	int dlclose() { return(0);}
 *	int dlsym() { return(0);}
 *	int dlerror() { return(0);}
 *
 * This has not been tested; I just read the manual page and coded this up.
 *
 * According to the ld manual page, you need to link bash with -dld and add
 * the -E flag to LOCAL_LDFLAGS.
 */

#if !defined (__HPUX10_DLFCN_H__)

#define __HPUX10_DLFCN_H__

#include <dl.h>
#include <errno.h>

#ifndef errno
extern int errno;
#endif

#define RTLD_LAZY BIND_DEFERRED
#define RTLD_NOW BIND_IMMEDIATE
#define RTLD_GLOBAL DYNAMIC_PATH

char *bash_global_sym_addr;

#define dlopen(file,mode) (void *)shl_load((file), (mode), 0L)

#define dlclose(handle) shl_unload((shl_t)(handle))

#define dlsym(handle,name) (bash_global_sym_addr=0,shl_findsym((shl_t *)&(handle),name,TYPE_UNDEFINED,&bash_global_sym_addr), (void *)bash_global_sym_addr)

#define dlerror() strerror(errno)

#endif /*  __HPUX10_DLFCN_H__ */
