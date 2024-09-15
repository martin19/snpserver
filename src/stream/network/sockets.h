#ifndef SNPSERVER_SOCKETS_H
#define SNPSERVER_SOCKETS_H

/*
 * File: sockets.h
 * Author: Will Eccles
 * Date: 2020-03-12
 *
 * 1.  Description
 *
 *    This header attempts to make it easy to use sockets across platforms,
 *    for both Windows and POSIX systems. While Winsock is often somewhat
 *    compatible with Berkeley sockets, it is not strictly compatible,
 *    and therefore must be treated differently.
 *    For more information on incompatibilities, see:
 *    https://docs.microsoft.com/en-us/windows/win32/winsock/porting-socket-applications-to-winsock
 *
 *    This header should be compatible with both C and C++ code using sockets,
 *    though if the C++ networking TS ever gets implemented, this will be
 *    unnecessary for C++ code. We all know how C++ standards go, however.
 *
 * 2.  Conventions
 *
 *    Functions will return int when any sort of error handling is required.
 *    When possible, 0 will be considered a success. The documentation for each
 *    individual function should explain its return value, but if one does not,
 *    you can safely assume 0 is a success value.
 *
 *    This header tries to make it as painless as possible to adapt code using
 *    Berkeley sockets to Winsock. Thus, the usage of this API is as identical
 *    as possible. You should be able to follow any Berkeley sockets guide and
 *    simply add one or two more function calls here and there (i.e. for errors
 *    [see Caveats], init, cleanup, etc.).
 *
 *    All functions contained in this header will be prefixed with sock_.
 *
 *    When referring to a socket, instead of using either int or SOCKET, you
 *    should use socket_t. This deals with each system's socket types for you.
 *
 * 3.  Flow
 *
 *    The flow of sockets using this header is nearly the same as a Berkeley
 *    sockets implementation without it. However, you should call sock_init()
 *    before doing anything else (this is for Winsock), and before exiting,
 *    you should call sock_cleanup() (this is also for Winsock). Note that
 *    because of the signature of sock_cleanup(), it should be compatible
 *    with atexit().
 *
 * 4.  Caveat(s)
 *
 *    While this header does all it can to make life easy for socket
 *    programmers, there are certain concessions which must be made in order to
 *    support Winsock.
 *
 * 4.1.  Error Handling
 *
 *    Winsock contains some functions which return int, and some functions
 *    which allow you to use WSAGetLastError(). The sock_perror() function only
 *    works for those functions which allow for WSAGetLastError(). Some
 *    functions (such as getaddrinfo()) do not support this. For those
 *    functions, you should *first* call sock_seterror(). This will do nothing
 *    on a POSIX system, but with Winsock, it will call WSASetLastError().
 *    Then, you can use sock_perror().
 */

#ifndef SOCKETS_H
#define SOCKETS_H

/* Includes */

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif /* WIN32_LEAN_AND_MEAN */
# ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#  define _WINSOCK_DEPRECATED_NO_WARNINGS
# endif /* _WINSOCK_DEPRECATED_NO_WARNINGS */
# include <winsock2.h>
# include <ws2tcpip.h>
# include <iphlpapi.h>
# include <windows.h>
# pragma comment(lib, "Ws2_32.lib")
# pragma comment(lib, "Mswsock.lib")
# pragma comment(lib, "AdvApi32.lib")
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netdb.h>
# include <unistd.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <stdint.h>

/* Types */

#ifdef _WIN32
typedef SOCKET socket_t;
typedef int socklen_t;
/* POSIX ssize_t is not a thing on Windows */
typedef signed long long int ssize_t;
/* Allow the use of close() (which is POSIX) for Winsock */
# define close(x) closesocket(x)
#else
typedef int socket_t;
// winsock has INVALID_SOCKET which is returned by socket(),
// this is the POSIX replacement
# define INVALID_SOCKET -1
#endif /* _WIN32 */

/* Functions */

/*
 * This function works like perror(), simply provide
 * a simple statement about what failed.
 */
inline void sock_perror(const char* msg) {
#ifdef _WIN32
    // TODO use Win32's awful message formatting functions
    printf("%s: %ld\n", msg, WSAGetLastError());
#else
    perror(msg);
#endif /* _WIN32 */
}

/*
 * Sets the last error on Windows so that sock_perror may
 * be used. This does not do anything on POSIX systems.
 */
inline void sock_seterror(int err) {
#ifdef _WIN32
    WSASetLastError(err);
#else
    (void)err;
    return;
#endif /* _WIN32 */
}

/*
 * This must be called before any subsequent socket function
 * calls. Only necessary for Windows; on unix systems, nothing
 * will be done. Returns 0 on success.
 */
inline int sock_init() {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2,2), &wsaData);
#else
    return 0;
#endif /* _WIN32 */
}

/*
 * Must be called on Windows before the program ends.
 * No-op on unix.
 */
inline void sock_cleanup() {
#ifdef _WIN32
    WSACleanup();
#else
    return;
#endif /* _WIN32 */
}

/*
 * Set the timeout for socket input (in milliseconds).
 * This has to be wrapped as Windows has a weird arg type.
 * Return value is the same as setsockopt().
 * You should probably be using poll() or select(), but
 * this can be useful in certain cases where those are not
 * practical.
 */
inline int sock_setrecvtimeout(socket_t sock, int32_t ms) {
#ifdef _WIN32
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<char*>(&ms), sizeof(ms));
#else
    struct timeval tv;
    tv.tv_usec = 1000L * ((long)ms - (long)ms / 1000L * 1000L);
    tv.tv_sec = (long)ms / 1000L;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<void*>(&tv), sizeof(tv));
#endif /* _WIN32 */
}

/*
 * Set the timeout for socket output (in milliseconds).
 * This has to be wrapped as Windows has a weird arg type.
 * Return value is the same as setsockopt().
 */
inline int sock_setsendtimeout(socket_t sock, int32_t ms) {
#ifdef _WIN32
    return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
                      reinterpret_cast<char*>(&ms), sizeof(ms));
#else
    struct timeval tv;
    tv.tv_usec = 1000L * ((long)ms - (long)ms / 1000L * 1000L);
    tv.tv_sec = (long)ms / 1000L;
    return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<void*>(&tv), sizeof(tv));
#endif /* _WIN32 */
}

#endif

#endif //SNPSERVER_SOCKETS_H
