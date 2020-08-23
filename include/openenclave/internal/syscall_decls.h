// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#ifndef _OE_INTERNAL_SYSCALL_DECLS_H
#define _OE_INTERNAL_SYSCALL_DECLS_H

#include <bits/syscall.h>
#include <openenclave/bits/defs.h>
#include <openenclave/bits/result.h>

// For OE_SYS_ defines.
// They are just used for asserting that they are equal to the corresponding
// SYS_ ones.
#if __x86_64__ || _M_X64
#include <openenclave/internal/syscall/sys/bits/syscall_x86_64.h>
#elif defined(__aarch64__)
#include <openenclave/internal/syscall/sys/bits/syscall_aarch64.h>
#else
#error Unsupported architecture
#endif

OE_EXTERNC_BEGIN

long oe_register_syscall(long idx, void* syscall_impl);

#define OE_SYSCALL_NAME(idx) oe##idx##_impl
#define OE_CHECK_IDX(idx) OE_STATIC_ASSERT((idx + 1) > 0)

#define OE_SYSCALL_DISPATCH(idx, ...) \
    case OE_##idx:                    \
        return OE_SYSCALL_NAME(_##idx)(__VA_ARGS__)

#define OE_SYSCALL_ARGS0 void
#define OE_SYSCALL_ARGS1 long arg1
#define OE_SYSCALL_ARGS2 OE_SYSCALL_ARGS1, long arg2
#define OE_SYSCALL_ARGS3 OE_SYSCALL_ARGS2, long arg3
#define OE_SYSCALL_ARGS4 OE_SYSCALL_ARGS3, long arg4
#define OE_SYSCALL_ARGS5 OE_SYSCALL_ARGS4, long arg5
#define OE_SYSCALL_ARGS6 OE_SYSCALL_ARGS5, long arg6
#define OE_SYSCALL_ARGS7 OE_SYSCALL_ARGS6, long arg7

#define OE_SYSCALL_IDX_NAME(idx) oe##idx##_index

#define OE_DECLARE_SYSCALL0(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS0)
#define OE_DECLARE_SYSCALL1(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS1)
#define OE_DECLARE_SYSCALL2(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS2)
#define OE_DECLARE_SYSCALL3(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS3)
#define OE_DECLARE_SYSCALL4(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS4)
#define OE_DECLARE_SYSCALL5(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS5)
#define OE_DECLARE_SYSCALL6(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS6)
#define OE_DECLARE_SYSCALL7(idx)       \
    OE_STATIC_ASSERT(idx == OE_##idx); \
    long OE_SYSCALL_NAME(_##idx)(OE_SYSCALL_ARGS7)

#define OE_DEFINE_SYSCALL0 OE_DECLARE_SYSCALL0
#define OE_DEFINE_SYSCALL1 OE_DECLARE_SYSCALL1
#define OE_DEFINE_SYSCALL2 OE_DECLARE_SYSCALL2
#define OE_DEFINE_SYSCALL3 OE_DECLARE_SYSCALL3
#define OE_DEFINE_SYSCALL4 OE_DECLARE_SYSCALL4
#define OE_DEFINE_SYSCALL5 OE_DECLARE_SYSCALL5
#define OE_DEFINE_SYSCALL6 OE_DECLARE_SYSCALL6
#define OE_DEFINE_SYSCALL7 OE_DECLARE_SYSCALL7

/** List of syscalls that are supported within enclaves.
 ** In alphabetical order.
 **/

OE_DECLARE_SYSCALL3(SYS_accept);
#if defined(OE_SYS_access)
OE_DECLARE_SYSCALL2(SYS_access);
#endif
OE_DECLARE_SYSCALL3(SYS_bind);
OE_DECLARE_SYSCALL1(SYS_chdir);
OE_DECLARE_SYSCALL2(SYS_clock_gettime);
OE_DECLARE_SYSCALL1(SYS_close);
OE_DECLARE_SYSCALL3(SYS_connect);
#if defined(OE_SYS_creat)
OE_DECLARE_SYSCALL2(SYS_creat);
#endif
OE_DECLARE_SYSCALL1(SYS_dup);
#if defined(OE_SYS_dup2)
OE_DECLARE_SYSCALL2(SYS_dup2);
#endif
OE_DECLARE_SYSCALL3(SYS_dup3);
#if defined(OE_SYS_epoll_create)
OE_DECLARE_SYSCALL1(SYS_epoll_create);
#endif
OE_DECLARE_SYSCALL1(SYS_epoll_create1);
OE_DECLARE_SYSCALL4(SYS_epoll_ctl);
OE_DECLARE_SYSCALL5(SYS_epoll_pwait);
#if defined(OE_SYS_epoll_wait)
OE_DECLARE_SYSCALL4(SYS_epoll_wait);
#endif
OE_DECLARE_SYSCALL1(SYS_exit);
OE_DECLARE_SYSCALL0(SYS_exit_group);
OE_DECLARE_SYSCALL4(SYS_faccessat);
OE_DECLARE_SYSCALL3(SYS_fcntl);
OE_DECLARE_SYSCALL1(SYS_fdatasync);
OE_DECLARE_SYSCALL2(SYS_flock);
OE_DECLARE_SYSCALL2(SYS_fstat);
OE_DECLARE_SYSCALL1(SYS_fsync);
OE_DECLARE_SYSCALL2(SYS_getcwd);
OE_DECLARE_SYSCALL3(SYS_getdents);
OE_DECLARE_SYSCALL3(SYS_getdents64);
OE_DECLARE_SYSCALL0(SYS_getegid);
OE_DECLARE_SYSCALL0(SYS_geteuid);
OE_DECLARE_SYSCALL0(SYS_getgid);
OE_DECLARE_SYSCALL2(SYS_getgroups);
OE_DECLARE_SYSCALL3(SYS_getpeername);
OE_DECLARE_SYSCALL1(SYS_getpgid);
#if defined(OE_SYS_getpgrp)
OE_DECLARE_SYSCALL0(SYS_getpgrp);
#endif
OE_DECLARE_SYSCALL0(SYS_getpid);
OE_DECLARE_SYSCALL0(SYS_getppid);
OE_DECLARE_SYSCALL3(SYS_getsockname);
OE_DECLARE_SYSCALL5(SYS_getsockopt);
OE_DECLARE_SYSCALL2(SYS_gettimeofday);
OE_DECLARE_SYSCALL0(SYS_getuid);
OE_DECLARE_SYSCALL6(SYS_ioctl);
#if defined(OE_SYS_link)
OE_DECLARE_SYSCALL2(SYS_link);
#endif
OE_DECLARE_SYSCALL5(SYS_linkat);
OE_DECLARE_SYSCALL2(SYS_listen);
OE_DECLARE_SYSCALL3(SYS_lseek);
#if defined(OE_SYS_mkdir)
OE_DECLARE_SYSCALL2(SYS_mkdir);
#endif
OE_DECLARE_SYSCALL3(SYS_mkdirat);
OE_DECLARE_SYSCALL5(SYS_mount);
OE_DECLARE_SYSCALL2(SYS_nanosleep);
OE_DECLARE_SYSCALL4(SYS_newfstatat);
OE_DECLARE_SYSCALL3(SYS_open);
OE_DECLARE_SYSCALL4(SYS_openat);
OE_DECLARE_SYSCALL3(SYS_poll);
OE_DECLARE_SYSCALL4(SYS_ppoll);
OE_DECLARE_SYSCALL4(SYS_pread64);
OE_DECLARE_SYSCALL5(SYS_pselect6);
OE_DECLARE_SYSCALL4(SYS_pwrite64);
OE_DECLARE_SYSCALL3(SYS_read);
OE_DECLARE_SYSCALL3(SYS_readv);
OE_DECLARE_SYSCALL6(SYS_recvfrom);
OE_DECLARE_SYSCALL3(SYS_recvmsg);
#if defined(OE_SYS_rename)
OE_DECLARE_SYSCALL2(SYS_rename);
#endif
OE_DECLARE_SYSCALL5(SYS_renameat);
#if defined(OE_SYS_rmdir)
OE_DECLARE_SYSCALL1(SYS_rmdir);
#endif
#if defined(OE_SYS_select)
OE_DECLARE_SYSCALL5(SYS_select);
#endif
OE_DECLARE_SYSCALL6(SYS_sendto);
OE_DECLARE_SYSCALL3(SYS_sendmsg);
OE_DECLARE_SYSCALL5(SYS_setsockopt);
OE_DECLARE_SYSCALL2(SYS_shutdown);
OE_DECLARE_SYSCALL3(SYS_socket);
OE_DECLARE_SYSCALL4(SYS_socketpair);
OE_DECLARE_SYSCALL2(SYS_stat);
OE_DECLARE_SYSCALL2(SYS_truncate);
OE_DECLARE_SYSCALL3(SYS_write);
OE_DECLARE_SYSCALL3(SYS_writev);
OE_DECLARE_SYSCALL1(SYS_uname);
#if defined(OE_SYS_unlink)
OE_DECLARE_SYSCALL1(SYS_unlink);
#endif
OE_DECLARE_SYSCALL3(SYS_unlinkat);
OE_DECLARE_SYSCALL2(SYS_umount2);

// These syscalls are not impelemented but are needed for compilation.
// Their use ought to be removed.
// Futex is special, can be called with 3 or 4 arguments
// OE_DECLARE_SYSCALL4(SYS_futex);
long OE_SYSCALL_NAME(_SYS_futex)(long arg1, long arg2, long arg3, ...);
OE_DECLARE_SYSCALL6(SYS_mmap);
OE_DECLARE_SYSCALL2(SYS_munmap);

// These don't seem to be used.
long OE_SYSCALL_NAME(_SYS_pread)(long arg1, long arg2, long arg3, long arg4);
long OE_SYSCALL_NAME(_SYS_pwrite)(long arg1, long arg2, long arg3, long arg4);

OE_DECLARE_SYSCALL4(SYS_wait4);

OE_EXTERNC_END

#endif /* _OE_INTERNAL_SYSCALL_DECLS_H */
