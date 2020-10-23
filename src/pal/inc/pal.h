// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*++

Module Name:

    pal.h

Abstract:

    Rotor Platform Adaptation Layer (PAL) header file.  This file
    defines all types and API calls required by the Rotor port of
    the Microsoft Common Language Runtime.

    Defines which control the behavior of this include file:
      UNICODE - define it to set the Ansi/Unicode neutral names to
                be the ...W names.  Otherwise the neutral names default
                to be the ...A names.
      PAL_IMPLEMENTATION - define it when implementing the PAL.  Otherwise
                leave it undefined when consuming the PAL.

    Note:  some fields in structs have been renamed from the original
    SDK documentation names, with _PAL_Undefined appended.  This leaves
    the structure layout identical to its Win32 version, but prevents
    PAL consumers from inadvertently referencing undefined fields.

    If you want to add a PAL_ wrapper function to a native function in
    here, you also need to edit palinternal.h and win32pal.h.
    


--*/

#ifndef __PAL_H__
#define __PAL_H__

#ifdef PAL_STDCPP_COMPAT
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

// This macro is used to standardize the wide character string literals between UNIX and Windows.
// Unix L"" is UTF32, and on windows it's UTF16.  Because of built-in assumptions on the size
// of string literals, it's important to match behaviour between Unix and Windows.  Unix will be defined
// as u"" (char16_t)
#define W(str)  u##str

// Undefine the QUOTE_MACRO_L helper and redefine it in terms of u.
// The reason that we do this is that quote macro is defined in ndp\common\inc,
// not inside of coreclr sources.

#define QUOTE_MACRO_L(x) QUOTE_MACRO_u(x)
#define QUOTE_MACRO_u_HELPER(x)     u###x
#define QUOTE_MACRO_u(x)            QUOTE_MACRO_u_HELPER(x)

#include <pal_char16.h>
#include <pal_error.h>
#include <pal_mstypes.h>

/******************* Processor-specific glue  *****************************/

#ifndef _MSC_VER

#if defined(__i686__) && !defined(_M_IX86)
#define _M_IX86 600
#elif defined(__i586__) && !defined(_M_IX86)
#define _M_IX86 500
#elif defined(__i486__) && !defined(_M_IX86)
#define _M_IX86 400
#elif defined(__i386__) && !defined(_M_IX86)
#define _M_IX86 300
#elif defined(__x86_64__) && !defined(_M_AMD64)
#define _M_AMD64 100
#elif defined(__arm__) && !defined(_M_ARM)
#define _M_ARM 7
#elif defined(__aarch64__) && !defined(_M_ARM64)
#define _M_ARM64 1
#elif defined(__mips64__) && !defined(_M_MIPS64)
#define _M_MIPS64 1
#endif

#if defined(_M_IX86) && !defined(_X86_)
#define _X86_
#elif defined(_M_AMD64) && !defined(_AMD64_)
#define _AMD64_
#elif defined(_M_ARM) && !defined(_ARM_)
#define _ARM_
#elif defined(_M_ARM64) && !defined(_ARM64_)
#define _ARM64_
#endif

#endif // !_MSC_VER

/******************* ABI-specific glue *******************************/

#ifdef __APPLE__
// Both PowerPC, i386 and x86_64 on Mac OS X use 16-byte alignment.
#define STACK_ALIGN_BITS             4
#define STACK_ALIGN_REQ             (1 << STACK_ALIGN_BITS)
#endif

#define MAX_PATH 260
#define _MAX_PATH 260
#define _MAX_DRIVE  3   /* max. length of drive component */
#define _MAX_DIR    256 /* max. length of path component */
#define _MAX_FNAME  256 /* max. length of file name component */
#define _MAX_EXT    256 /* max. length of extension component */

// In some Win32 APIs MAX_PATH is used for file names (even though 256 is the normal file system limit)
// use _MAX_PATH_FNAME to indicate these cases
#define MAX_PATH_FNAME MAX_PATH
#define MAX_LONGPATH   1024  /* max. length of full pathname */

#define MAXSHORT      0x7fff
#define MAXLONG       0x7fffffff
#define MAXCHAR       0x7f
#define MAXDWORD      0xffffffff

//  Sorting IDs.
//
//  Note that the named locale APIs (eg CompareStringExEx) are recommended.
//

#define LANG_CHINESE                     0x04
#define LANG_ENGLISH                     0x09
#define LANG_JAPANESE                    0x11
#define LANG_KOREAN                      0x12
#define LANG_THAI                        0x1e

/******************* Compiler-specific glue *******************************/

#if defined(_MSC_VER) || defined(__llvm__)
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#else
#define DECLSPEC_ALIGN(x) 
#endif

#define DECLSPEC_NORETURN   PAL_NORETURN

#if !defined(_MSC_VER) || defined(SOURCE_FORMATTING)
#define __assume(x) (void)0
#define __annotation(x)
#endif //!MSC_VER

#define UNALIGNED

#ifndef FORCEINLINE
#if _MSC_VER < 1200
#define FORCEINLINE inline
#else
#define FORCEINLINE __forceinline
#endif
#endif

#ifndef PAL_STDCPP_COMPAT

#if __GNUC__

typedef __builtin_va_list va_list;

/* We should consider if the va_arg definition here is actually necessary.
   Could we use the standard va_arg definition? */

#define va_start    __builtin_va_start
#define va_arg      __builtin_va_arg

#define va_copy     __builtin_va_copy
#define va_end      __builtin_va_end

#define VOID void

#define PUB __attribute__((visibility("default")))

#else // __GNUC__

typedef char * va_list;

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#if _MSC_VER >= 1400

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

#define _crt_va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define _crt_va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define _crt_va_end(ap)      ( ap = (va_list)0 )

#define va_start _crt_va_start
#define va_arg _crt_va_arg
#define va_end _crt_va_end

#else  // _MSC_VER

#define va_start(ap,v)    (ap = (va_list) (&(v)) + _INTSIZEOF(v))
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)

#endif // _MSC_VER

#define va_copy(dest,src) (dest = src)

#endif // __GNUC__

#endif // !PAL_STDCPP_COMPAT

/******************* PAL-Specific Entrypoints *****************************/

#define IsDebuggerPresent PAL_IsDebuggerPresent

PALIMPORT
BOOL
PALAPI
PAL_IsDebuggerPresent(VOID);
 
#define MAXIMUM_SUSPEND_COUNT  MAXCHAR

#define CHAR_BIT      8

#define SCHAR_MIN   (-128)
#define SCHAR_MAX     127
#define UCHAR_MAX     0xff

#define SHRT_MIN    (-32768)
#define SHRT_MAX      32767
#define USHRT_MAX     0xffff

#define INT_MIN     (-2147483647 - 1)
#define INT_MAX       2147483647
#define UINT_MAX      0xffffffff

#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX      2147483647L
#define ULONG_MAX     0xffffffffUL

#define FLT_MAX 3.402823466e+38F
#define DBL_MAX 1.7976931348623157e+308

/* minimum signed 64 bit value */
#define _I64_MIN    (I64(-9223372036854775807) - 1)
/* maximum signed 64 bit value */
#define _I64_MAX      I64(9223372036854775807)
/* maximum unsigned 64 bit value */
#define _UI64_MAX     UI64(0xffffffffffffffff)

#define _I8_MAX   SCHAR_MAX
#define _I8_MIN   SCHAR_MIN
#define _I16_MAX  SHRT_MAX
#define _I16_MIN  SHRT_MIN
#define _I32_MAX  INT_MAX
#define _I32_MIN  INT_MIN
#define _UI8_MAX  UCHAR_MAX
#define _UI8_MIN  UCHAR_MIN
#define _UI16_MAX USHRT_MAX
#define _UI16_MIN USHRT_MIN
#define _UI32_MAX UINT_MAX
#define _UI32_MIN UINT_MIN

#undef NULL

#if defined(__cplusplus)
#define NULL    0
#else
#define NULL    ((void *)0)
#endif

#if defined(PAL_STDCPP_COMPAT) && !defined(__cplusplus)
#define nullptr NULL
#endif // defined(PAL_STDCPP_COMPAT) && !defined(__cplusplus)

#ifndef PAL_STDCPP_COMPAT

#if _WIN64 || _MSC_VER >= 1400
typedef __int64 time_t;
#else
typedef long time_t;
#endif
#define _TIME_T_DEFINED
#endif // !PAL_STDCPP_COMPAT

#define C1_UPPER                  0x0001      /* upper case */
#define C1_LOWER                  0x0002      /* lower case */
#define C1_DIGIT                  0x0004      /* decimal digits */
#define C1_SPACE                  0x0008      /* spacing characters */
#define C1_PUNCT                  0x0010      /* punctuation characters */
#define C1_CNTRL                  0x0020      /* control characters */
#define C1_BLANK                  0x0040      /* blank characters */
#define C1_XDIGIT                 0x0080      /* other digits */
#define C1_ALPHA                  0x0100      /* any linguistic character */

#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH  2
#define DLL_THREAD_DETACH  3
#define DLL_PROCESS_DETACH 0

#define PAL_INITIALIZE_NONE                         0x00
#define PAL_INITIALIZE_SYNC_THREAD                  0x01
#define PAL_INITIALIZE_EXEC_ALLOCATOR               0x02
#define PAL_INITIALIZE_STD_HANDLES                  0x04
#define PAL_INITIALIZE_REGISTER_SIGTERM_HANDLER     0x08
#define PAL_INITIALIZE_DEBUGGER_EXCEPTIONS          0x10
#define PAL_INITIALIZE_ENSURE_STACK_SIZE            0x20

// PAL_Initialize() flags
#define PAL_INITIALIZE                 (PAL_INITIALIZE_SYNC_THREAD | PAL_INITIALIZE_STD_HANDLES)

// PAL_InitializeDLL() flags - don't start any of the helper threads
#define PAL_INITIALIZE_DLL             PAL_INITIALIZE_NONE       

// PAL_InitializeCoreCLR() flags
#define PAL_INITIALIZE_CORECLR         (PAL_INITIALIZE | PAL_INITIALIZE_EXEC_ALLOCATOR | PAL_INITIALIZE_REGISTER_SIGTERM_HANDLER | PAL_INITIALIZE_DEBUGGER_EXCEPTIONS | PAL_INITIALIZE_ENSURE_STACK_SIZE)

typedef DWORD (PALAPI *PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

/******************* PAL-Specific Entrypoints *****************************/

PALIMPORT
int
PALAPI
PAL_InitializeDLL(VOID);

PALIMPORT
HINSTANCE
PALAPI
PAL_RegisterModule(
    IN LPCSTR lpLibFileName);

PALIMPORT
VOID 
PALAPI
PAL_UnregisterModule(
    IN HINSTANCE hInstance);

PALIMPORT
BOOL
PALAPI
PAL_GetPALDirectoryW(
    OUT LPWSTR lpDirectoryName,
    IN OUT UINT* cchDirectoryName);

PALIMPORT
BOOL
PALAPI
PAL_GetPALDirectoryA(
    OUT LPSTR lpDirectoryName,
    IN OUT UINT* cchDirectoryName);

#ifdef UNICODE
#define PAL_GetPALDirectory PAL_GetPALDirectoryW
#else
#define PAL_GetPALDirectory PAL_GetPALDirectoryA
#endif

PALIMPORT
VOID
PALAPI
PAL_Random(
    IN OUT LPVOID lpBuffer,
    IN DWORD dwLength);

PALIMPORT
BOOL
PALAPI
PAL_ProbeMemory(
    PVOID pBuffer,
    DWORD cbBuffer,
    BOOL fWriteAccess);

/******************* winuser.h Entrypoints *******************************/
PALIMPORT
LPSTR
PALAPI
CharNextA(
            IN LPCSTR lpsz);

PALIMPORT
LPSTR
PALAPI
CharNextExA(
        IN WORD CodePage,
        IN LPCSTR lpCurrentChar,
        IN DWORD dwFlags);

#ifndef UNICODE
#define CharNext CharNextA
#define CharNextEx CharNextExA
#endif


#define MB_OK                   0x00000000L
#define MB_OKCANCEL             0x00000001L
#define MB_ABORTRETRYIGNORE     0x00000002L
#define MB_YESNO                0x00000004L
#define MB_RETRYCANCEL          0x00000005L

#define MB_ICONHAND             0x00000010L
#define MB_ICONQUESTION         0x00000020L
#define MB_ICONEXCLAMATION      0x00000030L
#define MB_ICONASTERISK         0x00000040L

#define MB_ICONINFORMATION      MB_ICONASTERISK
#define MB_ICONSTOP             MB_ICONHAND
#define MB_ICONERROR            MB_ICONHAND

#define MB_DEFBUTTON1           0x00000000L
#define MB_DEFBUTTON2           0x00000100L
#define MB_DEFBUTTON3           0x00000200L

#define MB_SYSTEMMODAL          0x00001000L
#define MB_TASKMODAL            0x00002000L
#define MB_SETFOREGROUND        0x00010000L
#define MB_TOPMOST              0x00040000L

#define MB_NOFOCUS                  0x00008000L
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L

// Note: this is the NT 4.0 and greater value.
#define MB_SERVICE_NOTIFICATION 0x00200000L

#define MB_TYPEMASK             0x0000000FL
#define MB_ICONMASK             0x000000F0L
#define MB_DEFMASK              0x00000F00L

#define IDOK                    1
#define IDCANCEL                2
#define IDABORT                 3
#define IDRETRY                 4
#define IDIGNORE                5
#define IDYES                   6
#define IDNO                    7

PALIMPORT
int
PALAPI
MessageBoxA(
    IN LPVOID hWnd,  // NOTE: diff from winuser.h
    IN LPCSTR lpText,
    IN LPCSTR lpCaption,
    IN UINT uType);

PALIMPORT
int
PALAPI
MessageBoxW(
    IN LPVOID hWnd,  // NOTE: diff from winuser.h
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType);


#ifdef UNICODE
#define MessageBox MessageBoxW
#else
#define MessageBox MessageBoxA
#endif

// From win32.h
#ifndef _CRTIMP
#ifdef __llvm__
#define _CRTIMP
#else // __llvm__
#define _CRTIMP __declspec(dllimport)
#endif // __llvm__
#endif // _CRTIMP

/******************* winbase.h Entrypoints and defines ************************/
typedef struct _SECURITY_ATTRIBUTES {
            DWORD nLength;
            LPVOID lpSecurityDescriptor;
            BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#define _SH_DENYWR      0x20    /* deny write mode */

#define FILE_READ_DATA            ( 0x0001 )    // file & pipe
#define FILE_APPEND_DATA          ( 0x0004 )    // file

#define GENERIC_READ               (0x80000000L)
#define GENERIC_WRITE              (0x40000000L)

#define FILE_SHARE_READ            0x00000001
#define FILE_SHARE_WRITE           0x00000002
#define FILE_SHARE_DELETE          0x00000004

#define CREATE_NEW                 1
#define CREATE_ALWAYS              2
#define OPEN_EXISTING              3
#define OPEN_ALWAYS                4
#define TRUNCATE_EXISTING          5

#define FILE_ATTRIBUTE_READONLY                 0x00000001
#define FILE_ATTRIBUTE_HIDDEN                   0x00000002
#define FILE_ATTRIBUTE_SYSTEM                   0x00000004
#define FILE_ATTRIBUTE_DIRECTORY                0x00000010
#define FILE_ATTRIBUTE_ARCHIVE                  0x00000020
#define FILE_ATTRIBUTE_DEVICE                   0x00000040
#define FILE_ATTRIBUTE_NORMAL                   0x00000080

#define FILE_FLAG_WRITE_THROUGH    0x80000000
#define FILE_FLAG_NO_BUFFERING     0x20000000
#define FILE_FLAG_RANDOM_ACCESS    0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN  0x08000000
#define FILE_FLAG_BACKUP_SEMANTICS 0x02000000

#define FILE_BEGIN                 0
#define FILE_CURRENT               1
#define FILE_END                   2

#define STILL_ACTIVE (0x00000103L)

#define INVALID_SET_FILE_POINTER   ((DWORD)-1)

PALIMPORT
HANDLE
PALAPI
CreateFileA(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile);

PALIMPORT
HANDLE
PALAPI
CreateFileW(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile);

#ifdef UNICODE
#define CreateFile CreateFileW
#else
#define CreateFile CreateFileA
#endif


PALIMPORT
DWORD
PALAPI
SearchPathA(
    IN LPCSTR lpPath,
    IN LPCSTR lpFileName,
    IN LPCSTR lpExtension,
    IN DWORD nBufferLength,
    OUT LPSTR lpBuffer,
    OUT LPSTR *lpFilePart);

PALIMPORT
DWORD
PALAPI
SearchPathW(
    IN LPCWSTR lpPath,
    IN LPCWSTR lpFileName,
    IN LPCWSTR lpExtension,
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer,
    OUT LPWSTR *lpFilePart);

#ifdef UNICODE
#define SearchPath  SearchPathW
#else
#define SearchPath  SearchPathA
#endif // !UNICODE


PALIMPORT
BOOL
PALAPI
CopyFileA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN BOOL bFailIfExists);

PALIMPORT
BOOL
PALAPI
CopyFileW(
    IN LPCWSTR lpExistingFileName,
    IN LPCWSTR lpNewFileName,
    IN BOOL bFailIfExists);

#ifdef UNICODE
#define CopyFile CopyFileW
#else
#define CopyFile CopyFileA
#endif


PALIMPORT
BOOL
PALAPI
DeleteFileA(
    IN LPCSTR lpFileName);

PALIMPORT
BOOL
PALAPI
DeleteFileW(
    IN LPCWSTR lpFileName);

#ifdef UNICODE
#define DeleteFile DeleteFileW
#else
#define DeleteFile DeleteFileA
#endif


#define MOVEFILE_REPLACE_EXISTING      0x00000001
#define MOVEFILE_COPY_ALLOWED          0x00000002

PALIMPORT
BOOL
PALAPI
MoveFileExA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN DWORD dwFlags);

PALIMPORT
BOOL
PALAPI
MoveFileExW(
    IN LPCWSTR lpExistingFileName,
    IN LPCWSTR lpNewFileName,
    IN DWORD dwFlags);

#ifdef UNICODE
#define MoveFileEx MoveFileExW
#else
#define MoveFileEx MoveFileExA
#endif


PALIMPORT
BOOL
PALAPI
CreateDirectoryA(
    IN LPCSTR lpPathName,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes);

PALIMPORT
BOOL
PALAPI
CreateDirectoryW(
    IN LPCWSTR lpPathName,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes);

#ifdef UNICODE
#define CreateDirectory CreateDirectoryW
#else
#define CreateDirectory CreateDirectoryA
#endif


PALIMPORT
BOOL
PALAPI
RemoveDirectoryA(
    IN LPCSTR lpPathName);

PALIMPORT
BOOL
PALAPI
RemoveDirectoryW(
    IN LPCWSTR lpPathName);

#ifdef UNICODE
#define RemoveDirectory RemoveDirectoryW
#else
#define RemoveDirectory RemoveDirectoryA
#endif

typedef struct _BY_HANDLE_FILE_INFORMATION {  
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[ MAX_PATH_FNAME ];
    CHAR cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[ MAX_PATH_FNAME ];
    WCHAR cAlternateFileName[ 14 ];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

#ifdef UNICODE
typedef WIN32_FIND_DATAW WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAW PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAW LPWIN32_FIND_DATA;
#else
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;
#endif

PALIMPORT
HANDLE
PALAPI
FindFirstFileA(
    IN LPCSTR lpFileName,
    OUT LPWIN32_FIND_DATAA lpFindFileData);

PALIMPORT
HANDLE
PALAPI
FindFirstFileW(
    IN LPCWSTR lpFileName,
    OUT LPWIN32_FIND_DATAW lpFindFileData);

#ifdef UNICODE
#define FindFirstFile FindFirstFileW
#else
#define FindFirstFile FindFirstFileA
#endif


PALIMPORT
BOOL
PALAPI
FindNextFileA(
    IN HANDLE hFindFile,
    OUT LPWIN32_FIND_DATAA lpFindFileData);

PALIMPORT
BOOL
PALAPI
FindNextFileW(
    IN HANDLE hFindFile,
    OUT LPWIN32_FIND_DATAW lpFindFileData);

#ifdef UNICODE
#define FindNextFile FindNextFileW
#else
#define FindNextFile FindNextFileA
#endif


PALIMPORT
BOOL
PALAPI
FindClose(
      IN OUT HANDLE hFindFile);


PALIMPORT
DWORD
PALAPI
GetFileAttributesA(
    IN LPCSTR lpFileName);

PALIMPORT
DWORD
PALAPI
GetFileAttributesW(
    IN LPCWSTR lpFileName);

#ifdef UNICODE
#define GetFileAttributes GetFileAttributesW
#else
#define GetFileAttributes GetFileAttributesA
#endif


typedef enum _GET_FILEEX_INFO_LEVELS {
  GetFileExInfoStandard
} GET_FILEEX_INFO_LEVELS;

typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard,
    FindExInfoBasic,
    FindExInfoMaxInfoLevel
} FINDEX_INFO_LEVELS;

typedef enum _FINDEX_SEARCH_OPS {
    FindExSearchNameMatch,
    FindExSearchLimitToDirectories,
    FindExSearchLimitToDevices,
    FindExSearchMaxSearchOp
} FINDEX_SEARCH_OPS;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD      dwFileAttributes;
    FILETIME   ftCreationTime;
    FILETIME   ftLastAccessTime;
    FILETIME   ftLastWriteTime;
    DWORD      nFileSizeHigh;
    DWORD      nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

PALIMPORT
BOOL
PALAPI
GetFileAttributesExW(
    IN LPCWSTR lpFileName,
    IN GET_FILEEX_INFO_LEVELS fInfoLevelId,
    OUT LPVOID lpFileInformation);

#ifdef UNICODE
#define GetFileAttributesEx GetFileAttributesExW
#endif

PALIMPORT
BOOL
PALAPI
SetFileAttributesA(
    IN LPCSTR lpFileName,
    IN DWORD dwFileAttributes);

PALIMPORT
BOOL
PALAPI
SetFileAttributesW(
           IN LPCWSTR lpFileName,
           IN DWORD dwFileAttributes);

#ifdef UNICODE
#define SetFileAttributes SetFileAttributesW
#else
#define SetFileAttributes SetFileAttributesA
#endif


typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    DWORD Offset;
    DWORD OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

PALIMPORT
BOOL
PALAPI
WriteFile(
      IN HANDLE hFile,
      IN LPCVOID lpBuffer,
      IN DWORD nNumberOfBytesToWrite,
      OUT LPDWORD lpNumberOfBytesWritten,
      IN LPOVERLAPPED lpOverlapped);

PALIMPORT
BOOL
PALAPI
ReadFile(
     IN HANDLE hFile,
     OUT LPVOID lpBuffer,
     IN DWORD nNumberOfBytesToRead,
     OUT LPDWORD lpNumberOfBytesRead,
     IN LPOVERLAPPED lpOverlapped);

#define STD_INPUT_HANDLE         ((DWORD)-10)
#define STD_OUTPUT_HANDLE        ((DWORD)-11)
#define STD_ERROR_HANDLE         ((DWORD)-12)

PALIMPORT
HANDLE
PALAPI
GetStdHandle(
         IN DWORD nStdHandle);

PALIMPORT
BOOL
PALAPI
SetEndOfFile(
         IN HANDLE hFile);

PALIMPORT
DWORD
PALAPI
SetFilePointer(
           IN HANDLE hFile,
           IN LONG lDistanceToMove,
           IN PLONG lpDistanceToMoveHigh,
           IN DWORD dwMoveMethod);

PALIMPORT
BOOL
PALAPI
SetFilePointerEx(
           IN HANDLE hFile,
           IN LARGE_INTEGER liDistanceToMove,
           OUT PLARGE_INTEGER lpNewFilePointer,
           IN DWORD dwMoveMethod);

PALIMPORT
DWORD
PALAPI
GetFileSize(
        IN HANDLE hFile,
        OUT LPDWORD lpFileSizeHigh);

PALIMPORT
BOOL
PALAPI GetFileSizeEx(
        IN   HANDLE hFile,
        OUT  PLARGE_INTEGER lpFileSize);

PALIMPORT
BOOL
PALAPI
GetFileInformationByHandle(
        IN HANDLE hFile,
        OUT BY_HANDLE_FILE_INFORMATION* lpFileInformation);

PALIMPORT
LONG
PALAPI
CompareFileTime(
        IN CONST FILETIME *lpFileTime1,
        IN CONST FILETIME *lpFileTime2);

PALIMPORT
VOID
PALAPI
GetSystemTimeAsFileTime(
            OUT LPFILETIME lpSystemTimeAsFileTime);

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

PALIMPORT
VOID
PALAPI
GetSystemTime(
    OUT LPSYSTEMTIME lpSystemTime);

PALIMPORT
BOOL
PALAPI
FileTimeToSystemTime(
    IN CONST FILETIME *lpFileTime,
    OUT LPSYSTEMTIME lpSystemTime);

PALIMPORT
BOOL
PALAPI
FlushFileBuffers(
    IN HANDLE hFile);

PALIMPORT
UINT
PALAPI
GetConsoleOutputCP(
    VOID);

PALIMPORT
DWORD
PALAPI
GetFullPathNameA(
    IN LPCSTR lpFileName,
    IN DWORD nBufferLength,
    OUT LPSTR lpBuffer,
    OUT LPSTR *lpFilePart);

PALIMPORT
DWORD
PALAPI
GetFullPathNameW(
    IN LPCWSTR lpFileName,
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer,
    OUT LPWSTR *lpFilePart);

#ifdef UNICODE
#define GetFullPathName GetFullPathNameW
#else
#define GetFullPathName GetFullPathNameA
#endif


PALIMPORT
DWORD
PALAPI
GetLongPathNameW(
    IN LPCWSTR lpszShortPath,
    OUT LPWSTR lpszLongPath,
    IN DWORD cchBuffer);

#ifdef UNICODE
#define GetLongPathName GetLongPathNameW
#endif

PALIMPORT
DWORD
PALAPI
GetShortPathNameW(
    IN LPCWSTR lpszLongPath,
    OUT LPWSTR lpszShortPath,
    IN DWORD cchBuffer);

#ifdef UNICODE
#define GetShortPathName GetShortPathNameW
#endif


PALIMPORT
UINT
PALAPI
GetTempFileNameA(
    IN LPCSTR lpPathName,
    IN LPCSTR lpPrefixString,
    IN UINT uUnique,
    OUT LPSTR lpTempFileName);

PALIMPORT
UINT
PALAPI
GetTempFileNameW(
    IN LPCWSTR lpPathName,
    IN LPCWSTR lpPrefixString,
    IN UINT uUnique,
    OUT LPWSTR lpTempFileName);

#ifdef UNICODE
#define GetTempFileName GetTempFileNameW
#else
#define GetTempFileName GetTempFileNameA
#endif


PALIMPORT
DWORD
PALAPI
GetTempPathA(
    IN DWORD nBufferLength,
    OUT LPSTR lpBuffer);

PALIMPORT
DWORD
PALAPI
GetTempPathW(
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer);

#ifdef UNICODE
#define GetTempPath GetTempPathW
#else
#define GetTempPath GetTempPathA
#endif


PALIMPORT
DWORD
PALAPI
GetCurrentDirectoryA(
    IN DWORD nBufferLength,
    OUT LPSTR lpBuffer);

PALIMPORT
DWORD
PALAPI
GetCurrentDirectoryW(
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer);

#ifdef UNICODE
#define GetCurrentDirectory GetCurrentDirectoryW
#else
#define GetCurrentDirectory GetCurrentDirectoryA
#endif


PALIMPORT
BOOL
PALAPI
SetCurrentDirectoryA(
    IN LPCSTR lpPathName);

PALIMPORT
BOOL
PALAPI
SetCurrentDirectoryW(
    IN LPCWSTR lpPathName);


#ifdef UNICODE
#define SetCurrentDirectory SetCurrentDirectoryW
#else
#define SetCurrentDirectory SetCurrentDirectoryA
#endif

PALIMPORT
DWORD
PALAPI
GetCurrentProcessId(
    VOID);

PALIMPORT
DWORD
PALAPI
GetCurrentSessionId(
    VOID);

PALIMPORT
DWORD
PALAPI
GetCurrentThreadId(
   VOID);

#define STARTF_USESTDHANDLES       0x00000100

typedef struct _STARTUPINFOW {
    DWORD cb;
    LPWSTR lpReserved_PAL_Undefined;
    LPWSTR lpDesktop_PAL_Undefined;
    LPWSTR lpTitle_PAL_Undefined;
    DWORD dwX_PAL_Undefined;
    DWORD dwY_PAL_Undefined;
    DWORD dwXSize_PAL_Undefined;
    DWORD dwYSize_PAL_Undefined;
    DWORD dwXCountChars_PAL_Undefined;
    DWORD dwYCountChars_PAL_Undefined;
    DWORD dwFillAttribute_PAL_Undefined;
    DWORD dwFlags;
    WORD wShowWindow_PAL_Undefined;
    WORD cbReserved2_PAL_Undefined;
    LPBYTE lpReserved2_PAL_Undefined;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;

typedef struct _STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved_PAL_Undefined;
    LPSTR lpDesktop_PAL_Undefined;
    LPSTR lpTitle_PAL_Undefined;
    DWORD dwX_PAL_Undefined;
    DWORD dwY_PAL_Undefined;
    DWORD dwXSize_PAL_Undefined;
    DWORD dwYSize_PAL_Undefined;
    DWORD dwXCountChars_PAL_Undefined;
    DWORD dwYCountChars_PAL_Undefined;
    DWORD dwFillAttribute_PAL_Undefined;
    DWORD dwFlags;
    WORD wShowWindow_PAL_Undefined;
    WORD cbReserved2_PAL_Undefined;
    LPBYTE lpReserved2_PAL_Undefined;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

#ifdef UNICODE
typedef STARTUPINFOW STARTUPINFO;
typedef LPSTARTUPINFOW LPSTARTUPINFO;
#else
typedef STARTUPINFOA STARTUPINFO;
typedef LPSTARTUPINFOW LPSTARTUPINFO;
#endif

#define CREATE_NEW_CONSOLE          0x00000010

#define NORMAL_PRIORITY_CLASS             0x00000020

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId_PAL_Undefined;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

#ifdef _X86_

//
// ***********************************************************************************
//
// NOTE: These context definitions are replicated in ndp/clr/src/debug/inc/DbgTargetContext.h (for the
// purposes manipulating contexts from different platforms during remote debugging). Be sure to keep those
// definitions in sync if you make any changes here.
//
// ***********************************************************************************
//

#define SIZE_OF_80387_REGISTERS      80

#define CONTEXT_i386            0x00010000
#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L) // SS:SP, CS:IP, FLAGS, BP
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L) // AX, BX, CX, DX, SI, DI
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L) // 387 state
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS)
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)
#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS)

#define MAXIMUM_SUPPORTED_EXTENSION     512

#define CONTEXT_XSTATE (CONTEXT_i386 | 0x40L)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000L
#define CONTEXT_SERVICE_ACTIVE 0x10000000L
#define CONTEXT_EXCEPTION_REQUEST 0x40000000L
#define CONTEXT_EXCEPTION_REPORTING 0x80000000L

//
// This flag is set by the unwinder if it has unwound to a call
// site, and cleared whenever it unwinds through a trap frame.
// It is used by language-specific exception handlers to help
// differentiate exception scopes during dispatching.
//

#define CONTEXT_UNWOUND_TO_CALL 0x20000000

typedef struct _FLOATING_SAVE_AREA {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
    DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

typedef struct _CONTEXT {
    ULONG ContextFlags;

    ULONG   Dr0_PAL_Undefined;
    ULONG   Dr1_PAL_Undefined;
    ULONG   Dr2_PAL_Undefined;
    ULONG   Dr3_PAL_Undefined;
    ULONG   Dr6_PAL_Undefined;
    ULONG   Dr7_PAL_Undefined;

    FLOATING_SAVE_AREA FloatSave;

    ULONG   SegGs_PAL_Undefined;
    ULONG   SegFs_PAL_Undefined;
    ULONG   SegEs_PAL_Undefined;
    ULONG   SegDs_PAL_Undefined;

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;
    ULONG   Esp;
    ULONG   SegSs;

    UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXT, *PCONTEXT, *LPCONTEXT;

// To support saving and loading xmm register context we need to know the offset in the ExtendedRegisters
// section at which they are stored. This has been determined experimentally since I have found no
// documentation thus far but it corresponds to the offset we'd expect if a fxsave instruction was used to
// store the regular FP state along with the XMM registers at the start of the extended registers section.
// Technically the offset doesn't really matter if no code in the PAL or runtime knows what the offset should
// be either (as long as we're consistent across GetThreadContext() and SetThreadContext() and we don't
// support any other values in the ExtendedRegisters) but we might as well be as accurate as we can.
#define CONTEXT_EXREG_XMM_OFFSET 160

typedef struct _KNONVOLATILE_CONTEXT {

    DWORD Edi;
    DWORD Esi;
    DWORD Ebx;
    DWORD Ebp;

} KNONVOLATILE_CONTEXT, *PKNONVOLATILE_CONTEXT;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {

    // The ordering of these fields should be aligned with that
    // of corresponding fields in CONTEXT
    //
    // (See FillRegDisplay in inc/regdisp.h for details)
    PDWORD Edi;
    PDWORD Esi;
    PDWORD Ebx;
    PDWORD Edx;
    PDWORD Ecx;
    PDWORD Eax;

    PDWORD Ebp;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

#elif defined(_AMD64_)
// copied from winnt.h

#define CONTEXT_AMD64   0x100000

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#define CONTEXT_XSTATE (CONTEXT_AMD64 | 0x40L)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000
#define CONTEXT_SERVICE_ACTIVE 0x10000000
#define CONTEXT_EXCEPTION_REQUEST 0x40000000
#define CONTEXT_EXCEPTION_REPORTING 0x80000000

typedef struct DECLSPEC_ALIGN(16) _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

typedef struct _XMM_SAVE_AREA32 {
    WORD   ControlWord;
    WORD   StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD   ErrorOpcode;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   Reserved2;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    BYTE  Reserved4[96];
} XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

#define LEGACY_SAVE_AREA_LENGTH sizeof(XMM_SAVE_AREA32)

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) is is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies SegSs, Rsp, SegCs, Rip, and EFlags.
//
// CONTEXT_INTEGER specifies Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, and R8-R15.
//
// CONTEXT_SEGMENTS specifies SegDs, SegEs, SegFs, and SegGs.
//
// CONTEXT_DEBUG_REGISTERS specifies Dr0-Dr3 and Dr6-Dr7.
//
// CONTEXT_MMX_REGISTERS specifies the floating point and extended registers
//     Mm0/St0-Mm7/St7 and Xmm0-Xmm15).
//

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

    //
    // Register parameter home addresses.
    //
    // N.B. These fields are for convience - they could be used to extend the
    //      context record in the future.
    //

    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;

    //
    // Control flags.
    //

    DWORD ContextFlags;
    DWORD MxCsr;

    //
    // Segment Registers and processor flags.
    //

    WORD   SegCs;
    WORD   SegDs;
    WORD   SegEs;
    WORD   SegFs;
    WORD   SegGs;
    WORD   SegSs;
    DWORD EFlags;

    //
    // Debug registers
    //

    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;

    //
    // Integer registers.
    //

    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;

    //
    // Program counter.
    //

    DWORD64 Rip;

    //
    // Floating point state.
    //

    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        };
    };

    //
    // Vector registers.
    //

    M128A VectorRegister[26];
    DWORD64 VectorControl;

    //
    // Special debug control registers.
    //

    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT, *LPCONTEXT;

//
// Nonvolatile context pointer record.
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    union {
        PM128A FloatingContext[16];
        struct {
            PM128A Xmm0;
            PM128A Xmm1;
            PM128A Xmm2;
            PM128A Xmm3;
            PM128A Xmm4;
            PM128A Xmm5;
            PM128A Xmm6;
            PM128A Xmm7;
            PM128A Xmm8;
            PM128A Xmm9;
            PM128A Xmm10;
            PM128A Xmm11;
            PM128A Xmm12;
            PM128A Xmm13;
            PM128A Xmm14;
            PM128A Xmm15;
        } ;
    } ;

    union {
        PDWORD64 IntegerContext[16];
        struct {
            PDWORD64 Rax;
            PDWORD64 Rcx;
            PDWORD64 Rdx;
            PDWORD64 Rbx;
            PDWORD64 Rsp;
            PDWORD64 Rbp;
            PDWORD64 Rsi;
            PDWORD64 Rdi;
            PDWORD64 R8;
            PDWORD64 R9;
            PDWORD64 R10;
            PDWORD64 R11;
            PDWORD64 R12;
            PDWORD64 R13;
            PDWORD64 R14;
            PDWORD64 R15;
        } ;
    } ;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

#elif defined(_ARM_)

#define CONTEXT_ARM   0x00200000L

// end_wx86

#define CONTEXT_CONTROL (CONTEXT_ARM | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_ARM | 0x2L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_ARM | 0x4L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_ARM | 0x8L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000L
#define CONTEXT_SERVICE_ACTIVE 0x10000000L
#define CONTEXT_EXCEPTION_REQUEST 0x40000000L
#define CONTEXT_EXCEPTION_REPORTING 0x80000000L

//
// This flag is set by the unwinder if it has unwound to a call
// site, and cleared whenever it unwinds through a trap frame.
// It is used by language-specific exception handlers to help
// differentiate exception scopes during dispatching.
//

#define CONTEXT_UNWOUND_TO_CALL 0x20000000

//
// Specify the number of breakpoints and watchpoints that the OS
// will track. Architecturally, ARM supports up to 16. In practice,
// however, almost no one implements more than 4 of each.
//

#define ARM_MAX_BREAKPOINTS     8
#define ARM_MAX_WATCHPOINTS     1

typedef struct _NEON128 {
    ULONGLONG Low;
    LONGLONG High;
} NEON128, *PNEON128;

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies Sp, Lr, Pc, and Cpsr
//
// CONTEXT_INTEGER specifies R0-R12
//
// CONTEXT_FLOATING_POINT specifies Q0-Q15 / D0-D31 / S0-S31
//
// CONTEXT_DEBUG_REGISTERS specifies up to 16 of DBGBVR, DBGBCR, DBGWVR,
//      DBGWCR.
//

typedef struct DECLSPEC_ALIGN(8) _CONTEXT {

    //
    // Control flags.
    //

    DWORD ContextFlags;

    //
    // Integer registers
    //

    DWORD R0;
    DWORD R1;
    DWORD R2;
    DWORD R3;
    DWORD R4;
    DWORD R5;
    DWORD R6;
    DWORD R7;
    DWORD R8;
    DWORD R9;
    DWORD R10;
    DWORD R11;
    DWORD R12;

    //
    // Control Registers
    //

    DWORD Sp;
    DWORD Lr;
    DWORD Pc;
    DWORD Cpsr;

    //
    // Floating Point/NEON Registers
    //

    DWORD Fpscr;
    DWORD Padding;
    union {
        NEON128 Q[16];
        ULONGLONG D[32];
        DWORD S[32];
    };

    //
    // Debug registers
    //

    DWORD Bvr[ARM_MAX_BREAKPOINTS];
    DWORD Bcr[ARM_MAX_BREAKPOINTS];
    DWORD Wvr[ARM_MAX_WATCHPOINTS];
    DWORD Wcr[ARM_MAX_WATCHPOINTS];

    DWORD Padding2[2];

} CONTEXT, *PCONTEXT, *LPCONTEXT;

//
// Nonvolatile context pointer record.
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {

    PDWORD R4;
    PDWORD R5;
    PDWORD R6;
    PDWORD R7;
    PDWORD R8;
    PDWORD R9;
    PDWORD R10;
    PDWORD R11;
    PDWORD Lr;

    PULONGLONG D8;
    PULONGLONG D9;
    PULONGLONG D10;
    PULONGLONG D11;
    PULONGLONG D12;
    PULONGLONG D13;
    PULONGLONG D14;
    PULONGLONG D15;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

typedef struct _IMAGE_ARM_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    union {
        DWORD UnwindData;
        struct {
            DWORD Flag : 2;
            DWORD FunctionLength : 11;
            DWORD Ret : 2;
            DWORD H : 1;
            DWORD Reg : 3;
            DWORD R : 1;
            DWORD L : 1;
            DWORD C : 1;
            DWORD StackAdjust : 10;
        };
    };
} IMAGE_ARM_RUNTIME_FUNCTION_ENTRY, * PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY;

#elif defined(_ARM64_)

#define CONTEXT_ARM64   0x00400000L

#define CONTEXT_CONTROL (CONTEXT_ARM64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_ARM64 | 0x2L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_ARM64 | 0x4L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_ARM64 | 0x8L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000L
#define CONTEXT_SERVICE_ACTIVE 0x10000000L
#define CONTEXT_EXCEPTION_REQUEST 0x40000000L
#define CONTEXT_EXCEPTION_REPORTING 0x80000000L

//
// This flag is set by the unwinder if it has unwound to a call
// site, and cleared whenever it unwinds through a trap frame.
// It is used by language-specific exception handlers to help
// differentiate exception scopes during dispatching.
//

#define CONTEXT_UNWOUND_TO_CALL 0x20000000

//
// Define initial Cpsr/Fpscr value
//

#define INITIAL_CPSR 0x10
#define INITIAL_FPSCR 0

// begin_ntoshvp

//
// Specify the number of breakpoints and watchpoints that the OS
// will track. Architecturally, ARM64 supports up to 16. In practice,
// however, almost no one implements more than 4 of each.
//

#define ARM64_MAX_BREAKPOINTS     8
#define ARM64_MAX_WATCHPOINTS     2

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies Sp, Lr, Pc, and Cpsr
//
// CONTEXT_INTEGER specifies R0-R12
//
// CONTEXT_FLOATING_POINT specifies Q0-Q15 / D0-D31 / S0-S31
//
// CONTEXT_DEBUG_REGISTERS specifies up to 16 of DBGBVR, DBGBCR, DBGWVR,
//      DBGWCR.
//

typedef struct _NEON128 {
    ULONGLONG Low;
    LONGLONG High;
} NEON128, *PNEON128;

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

    //
    // Control flags.
    //

    /* +0x000 */ DWORD ContextFlags;

    //
    // Integer registers
    //

    /* +0x004 */ DWORD Cpsr;       // NZVF + DAIF + CurrentEL + SPSel
    /* +0x008 */ union {
                    struct {
                        DWORD64 X0;
                        DWORD64 X1;
                        DWORD64 X2;
                        DWORD64 X3;
                        DWORD64 X4;
                        DWORD64 X5;
                        DWORD64 X6;
                        DWORD64 X7;
                        DWORD64 X8;
                        DWORD64 X9;
                        DWORD64 X10;
                        DWORD64 X11;
                        DWORD64 X12;
                        DWORD64 X13;
                        DWORD64 X14;
                        DWORD64 X15;
                        DWORD64 X16;
                        DWORD64 X17;
                        DWORD64 X18;
                        DWORD64 X19;
                        DWORD64 X20;
                        DWORD64 X21;
                        DWORD64 X22;
                        DWORD64 X23;
                        DWORD64 X24;
                        DWORD64 X25;
                        DWORD64 X26;
                        DWORD64 X27;
                        DWORD64 X28;
                    };
                    DWORD64 X[29];
                };
    /* +0x0f0 */ DWORD64 Fp;
    /* +0x0f8 */ DWORD64 Lr;
    /* +0x100 */ DWORD64 Sp;
    /* +0x108 */ DWORD64 Pc;

    //
    // Floating Point/NEON Registers
    //

    /* +0x110 */ NEON128 V[32];
    /* +0x310 */ DWORD Fpcr;
    /* +0x314 */ DWORD Fpsr;

    //
    // Debug registers
    //

    /* +0x318 */ DWORD Bcr[ARM64_MAX_BREAKPOINTS];
    /* +0x338 */ DWORD64 Bvr[ARM64_MAX_BREAKPOINTS];
    /* +0x378 */ DWORD Wcr[ARM64_MAX_WATCHPOINTS];
    /* +0x380 */ DWORD64 Wvr[ARM64_MAX_WATCHPOINTS];
    /* +0x390 */

} CONTEXT, *PCONTEXT, *LPCONTEXT;

//
// Nonvolatile context pointer record.
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {

    PDWORD64 X19;
    PDWORD64 X20;
    PDWORD64 X21;
    PDWORD64 X22;
    PDWORD64 X23;
    PDWORD64 X24;
    PDWORD64 X25;
    PDWORD64 X26;
    PDWORD64 X27;
    PDWORD64 X28;
    PDWORD64 Fp;
    PDWORD64 Lr;

    PDWORD64 D8;
    PDWORD64 D9;
    PDWORD64 D10;
    PDWORD64 D11;
    PDWORD64 D12;
    PDWORD64 D13;
    PDWORD64 D14;
    PDWORD64 D15;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

#elif defined(_MIPS64_)

#define CONTEXT_MIPS64   0x00800000

#define CONTEXT_CONTROL (CONTEXT_MIPS64 | 0x1)
#define CONTEXT_INTEGER (CONTEXT_MIPS64 | 0x2)
#define CONTEXT_FLOATING_POINT  (CONTEXT_MIPS64 | 0x4)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_MIPS64 | 0x8)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000
#define CONTEXT_SERVICE_ACTIVE 0x10000000
#define CONTEXT_EXCEPTION_REQUEST 0x40000000
#define CONTEXT_EXCEPTION_REPORTING 0x80000000

//
// This flag is set by the unwinder if it has unwound to a call
// site, and cleared whenever it unwinds through a trap frame.
// It is used by language-specific exception handlers to help
// differentiate exception scopes during dispatching.
//

#define CONTEXT_UNWOUND_TO_CALL 0x20000000

// begin_ntoshvp

//
// Specify the number of breakpoints and watchpoints that the OS
// will track. Architecturally, MIPS64 supports up to 16. In practice,
// however, almost no one implements more than 4 of each.
//

#define MIPS64_MAX_BREAKPOINTS     8
#define MIPS64_MAX_WATCHPOINTS     2

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

    //
    // Control flags.
    //

    DWORD ContextFlags;

    //
    // Integer registers, abi=N64.
    //
    DWORD64 R0;
    DWORD64 At;
    DWORD64 V0;
    DWORD64 V1;
    DWORD64 A0;
    DWORD64 A1;
    DWORD64 A2;
    DWORD64 A3;
    DWORD64 A4;
    DWORD64 A5;
    DWORD64 A6;
    DWORD64 A7;
    DWORD64 T0;
    DWORD64 T1;
    DWORD64 T2;
    DWORD64 T3;
    DWORD64 S0;
    DWORD64 S1;
    DWORD64 S2;
    DWORD64 S3;
    DWORD64 S4;
    DWORD64 S5;
    DWORD64 S6;
    DWORD64 S7;
    DWORD64 T8;
    DWORD64 T9;
    DWORD64 K0;
    DWORD64 K1;
    DWORD64 Gp;
    DWORD64 Sp;
    DWORD64 Fp;
    DWORD64 Ra;
    DWORD64 Pc;

    //
    // Floating Point Registers
    //
    DWORD64 F[32];
    DWORD Fcsr;

    DWORD64 Hi;
    DWORD64 Lo;
} CONTEXT, *PCONTEXT, *LPCONTEXT;

//
// Nonvolatile context pointer record.
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {

    PDWORD64 S0;
    PDWORD64 S1;
    PDWORD64 S2;
    PDWORD64 S3;
    PDWORD64 S4;
    PDWORD64 S5;
    PDWORD64 S6;
    PDWORD64 S7;
    PDWORD64 Gp;
    PDWORD64 Fp;
    PDWORD64 Ra;

    PDWORD64 F24;
    PDWORD64 F25;
    PDWORD64 F26;
    PDWORD64 F27;
    PDWORD64 F28;
    PDWORD64 F29;
    PDWORD64 F30;
    PDWORD64 F31;

    PDWORD64 Hi;
    PDWORD64 Lo;
} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

#else
#error Unknown architecture for defining CONTEXT.
#endif

PALIMPORT
BOOL
PALAPI
GetThreadTimes(
        IN HANDLE hThread,
        OUT LPFILETIME lpCreationTime,
        OUT LPFILETIME lpExitTime,
        OUT LPFILETIME lpKernelTime,
        OUT LPFILETIME lpUserTime);
    
/* PAL_CS_NATIVE_DATA_SIZE is defined as sizeof(PAL_CRITICAL_SECTION_NATIVE_DATA) */

#if defined(__APPLE__) && defined(__i386__)
#define PAL_CS_NATIVE_DATA_SIZE 76
#elif defined(__APPLE__) && defined(__x86_64__)
#define PAL_CS_NATIVE_DATA_SIZE 120
#elif defined(__APPLE__) && defined(__arm64__)
#define PAL_CS_NATIVE_DATA_SIZE 120
#elif defined(__FreeBSD__) && defined(_X86_)
#define PAL_CS_NATIVE_DATA_SIZE 12
#elif defined(__FreeBSD__) && defined(__x86_64__)
#define PAL_CS_NATIVE_DATA_SIZE 24
#elif defined(__linux__) && defined(_ARM_)
#define PAL_CS_NATIVE_DATA_SIZE 80
#elif defined(__linux__) && defined(_ARM64_)
#define PAL_CS_NATIVE_DATA_SIZE 116
#elif defined(__linux__) && defined(__i386__)
#define PAL_CS_NATIVE_DATA_SIZE 76
#elif defined(__linux__) && defined(__x86_64__)
#define PAL_CS_NATIVE_DATA_SIZE 96
#elif defined(__NetBSD__) && defined(__amd64__)
#define PAL_CS_NATIVE_DATA_SIZE 96
#elif defined(__NetBSD__) && defined(__earm__)
#define PAL_CS_NATIVE_DATA_SIZE 56
#elif defined(__NetBSD__) && defined(__i386__)
#define PAL_CS_NATIVE_DATA_SIZE 56
#elif defined(__linux__) && defined(_MIPS64_)
#define PAL_CS_NATIVE_DATA_SIZE 116
#else 
#warning 
#error  PAL_CS_NATIVE_DATA_SIZE is not defined for this architecture
#endif
    
// 
typedef struct _CRITICAL_SECTION {
    PVOID DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;

    BOOL bInternal;
    volatile DWORD dwInitState;
    union CSNativeDataStorage
    {
        BYTE rgNativeDataStorage[PAL_CS_NATIVE_DATA_SIZE]; 
        VOID * pvAlign; // make sure the storage is machine-pointer-size aligned
    } csnds;    
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

PALIMPORT VOID PALAPI EnterCriticalSection(IN OUT LPCRITICAL_SECTION lpCriticalSection);
PALIMPORT VOID PALAPI LeaveCriticalSection(IN OUT LPCRITICAL_SECTION lpCriticalSection);
PALIMPORT VOID PALAPI InitializeCriticalSection(OUT LPCRITICAL_SECTION lpCriticalSection);
PALIMPORT BOOL PALAPI InitializeCriticalSectionEx(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount, DWORD Flags);
PALIMPORT VOID PALAPI DeleteCriticalSection(IN OUT LPCRITICAL_SECTION lpCriticalSection);
PALIMPORT BOOL PALAPI TryEnterCriticalSection(IN OUT LPCRITICAL_SECTION lpCriticalSection);

#define SEM_FAILCRITICALERRORS          0x0001
#define SEM_NOOPENFILEERRORBOX          0x8000

PALIMPORT
UINT
PALAPI
SetErrorMode(
         IN UINT uMode);

#define PAGE_NOACCESS                   0x01
#define PAGE_READONLY                   0x02
#define PAGE_READWRITE                  0x04
#define PAGE_WRITECOPY                  0x08
#define PAGE_EXECUTE                    0x10
#define PAGE_EXECUTE_READ               0x20
#define PAGE_EXECUTE_READWRITE          0x40
#define PAGE_EXECUTE_WRITECOPY          0x80
#define MEM_COMMIT                      0x1000
#define MEM_RESERVE                     0x2000
#define MEM_DECOMMIT                    0x4000
#define MEM_RELEASE                     0x8000
#define MEM_RESET                       0x80000
#define MEM_FREE                        0x10000
#define MEM_PRIVATE                     0x20000
#define MEM_MAPPED                      0x40000
#define MEM_TOP_DOWN                    0x100000
#define MEM_WRITE_WATCH                 0x200000
#define MEM_RESERVE_EXECUTABLE          0x40000000 // reserve memory using executable memory allocator

PALIMPORT
HANDLE
PALAPI
CreateFileMappingA(
    IN HANDLE hFile,
    IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    IN DWORD flProtect,
    IN DWORD dwMaximumSizeHigh,
    IN DWORD dwMaximumSizeLow,
    IN LPCSTR lpName);

PALIMPORT
HANDLE
PALAPI
CreateFileMappingW(
    IN HANDLE hFile,
    IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    IN DWORD flProtect,
    IN DWORD dwMaxmimumSizeHigh,
    IN DWORD dwMaximumSizeLow,
    IN LPCWSTR lpName);

#ifdef UNICODE
#define CreateFileMapping CreateFileMappingW
#else
#define CreateFileMapping CreateFileMappingA
#endif


#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_ALL_ACCESS  (SECTION_MAP_READ | SECTION_MAP_WRITE) // diff from winnt.h

#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS SECTION_ALL_ACCESS
#define FILE_MAP_COPY       SECTION_QUERY

PALIMPORT
HANDLE
PALAPI
OpenFileMappingA(
         IN DWORD dwDesiredAccess,
         IN BOOL bInheritHandle,
         IN LPCSTR lpName);

PALIMPORT
HANDLE
PALAPI
OpenFileMappingW(
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN LPCWSTR lpName);

#ifdef UNICODE
#define OpenFileMapping OpenFileMappingW
#else
#define OpenFileMapping OpenFileMappingA
#endif


PALIMPORT
LPVOID
PALAPI
MapViewOfFile(
    IN HANDLE hFileMappingObject,
    IN DWORD dwDesiredAccess,
    IN DWORD dwFileOffsetHigh,
    IN DWORD dwFileOffsetLow,
    IN SIZE_T dwNumberOfBytesToMap);

PALIMPORT
LPVOID
PALAPI
MapViewOfFileEx(
    IN HANDLE hFileMappingObject,
    IN DWORD dwDesiredAccess,
    IN DWORD dwFileOffsetHigh,
    IN DWORD dwFileOffsetLow,
    IN SIZE_T dwNumberOfBytesToMap,
    IN LPVOID lpBaseAddress);

PALIMPORT
BOOL
PALAPI
UnmapViewOfFile(
    IN LPCVOID lpBaseAddress);


PALIMPORT
HMODULE
PALAPI
LoadLibraryA(
    IN LPCSTR lpLibFileName);

PALIMPORT
HMODULE
PALAPI
LoadLibraryExA(
    IN LPCSTR lpLibFileName,
    IN /*Reserved*/ HANDLE hFile,
    IN DWORD dwFlags);

PALIMPORT
HMODULE
PALAPI
LoadLibraryW(
    IN LPCWSTR lpLibFileName);

PALIMPORT
HMODULE
PALAPI
LoadLibraryExW(
        IN LPCWSTR lpLibFileName,
        IN /*Reserved*/ HANDLE hFile,
        IN DWORD dwFlags);

#ifdef UNICODE
#define LoadLibrary LoadLibraryW
#define LoadLibraryEx LoadLibraryExW
#else
#define LoadLibrary LoadLibraryA
#define LoadLibraryEx LoadLibraryExA
#endif

typedef INT_PTR (PALAPI *FARPROC)();

PALIMPORT
FARPROC
PALAPI
GetProcAddress(
    IN HMODULE hModule,
    IN LPCSTR lpProcName);

PALIMPORT
BOOL
PALAPI
FreeLibrary(
    IN OUT HMODULE hLibModule);

PALIMPORT
BOOL
PALAPI
DisableThreadLibraryCalls(
    IN HMODULE hLibModule);


PALIMPORT
DWORD
PALAPI
GetModuleFileNameA(
    IN HMODULE hModule,
    OUT LPSTR lpFileName,
    IN DWORD nSize);

PALIMPORT
DWORD
PALAPI
GetModuleFileNameW(
    IN HMODULE hModule,
    OUT LPWSTR lpFileName,
    IN DWORD nSize);

#ifdef UNICODE
#define GetModuleFileName GetModuleFileNameW
#else
#define GetModuleFileName GetModuleFileNameA
#endif


PALIMPORT
DWORD
PALAPI
GetModuleFileNameExW(
    IN HANDLE hProcess,
    IN HMODULE hModule,
    OUT LPWSTR lpFilename,
    IN DWORD nSize
    );

#ifdef UNICODE
#define GetModuleFileNameEx GetModuleFileNameExW
#endif

// Get base address of the module containing a given symbol 
PALAPI
LPCVOID
PAL_GetSymbolModuleBase(void *symbol);

PALIMPORT
LPCSTR
PALAPI
PAL_GetLoadLibraryError();

PALIMPORT
LPVOID
PALAPI
VirtualAlloc(
         IN LPVOID lpAddress,
         IN SIZE_T dwSize,
         IN DWORD flAllocationType,
         IN DWORD flProtect);

PALIMPORT
BOOL
PALAPI
VirtualFree(
        IN LPVOID lpAddress,
        IN SIZE_T dwSize,
        IN DWORD dwFreeType);

PALIMPORT
BOOL
PALAPI
VirtualProtect(
           IN LPVOID lpAddress,
           IN SIZE_T dwSize,
           IN DWORD flNewProtect,
           OUT PDWORD lpflOldProtect);

typedef struct _MEMORYSTATUSEX {
  DWORD     dwLength;
  DWORD     dwMemoryLoad;
  DWORDLONG ullTotalPhys;
  DWORDLONG ullAvailPhys;
  DWORDLONG ullTotalPageFile;
  DWORDLONG ullAvailPageFile;
  DWORDLONG ullTotalVirtual;
  DWORDLONG ullAvailVirtual;
  DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

PALIMPORT 
BOOL
PALAPI
GlobalMemoryStatusEx(
            IN OUT LPMEMORYSTATUSEX lpBuffer);

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase_PAL_Undefined;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

PALIMPORT
SIZE_T
PALAPI
VirtualQuery(
         IN LPCVOID lpAddress,
         OUT PMEMORY_BASIC_INFORMATION lpBuffer,
         IN SIZE_T dwLength);

PALIMPORT
VOID
PALAPI
RtlMoveMemory(
          IN PVOID Destination,
          IN CONST VOID *Source,
          IN SIZE_T Length);

#define MoveMemory memmove
#define CopyMemory memcpy
#define FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))


PALIMPORT
HANDLE
PALAPI
GetProcessHeap(
           VOID);

#define HEAP_ZERO_MEMORY 0x00000008

PALIMPORT
HANDLE
PALAPI
HeapCreate(
         IN DWORD flOptions,
         IN SIZE_T dwInitialSize,
         IN SIZE_T dwMaximumSize);

PALIMPORT
LPVOID
PALAPI
HeapAlloc(
      IN HANDLE hHeap,
      IN DWORD dwFlags,
      IN SIZE_T dwBytes);

PALIMPORT
LPVOID
PALAPI
HeapReAlloc(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPVOID lpMem,
    IN SIZE_T dwBytes
    );

PALIMPORT
BOOL
PALAPI
HeapFree(
     IN HANDLE hHeap,
     IN DWORD dwFlags,
     IN LPVOID lpMem);

typedef enum _HEAP_INFORMATION_CLASS {
    HeapCompatibilityInformation,
    HeapEnableTerminationOnCorruption
} HEAP_INFORMATION_CLASS;

PALIMPORT
BOOL
PALAPI
HeapSetInformation(
        IN OPTIONAL HANDLE HeapHandle,
        IN HEAP_INFORMATION_CLASS HeapInformationClass,
        IN PVOID HeapInformation,
        IN SIZE_T HeapInformationLength);

#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_ZEROINIT       0x0040
#define LPTR                (LMEM_FIXED | LMEM_ZEROINIT)

PALIMPORT
HLOCAL
PALAPI
LocalAlloc(
       IN UINT uFlags,
       IN SIZE_T uBytes);

PALIMPORT
HLOCAL
PALAPI
LocalReAlloc(
       IN HLOCAL hMem,
       IN SIZE_T uBytes,
       IN UINT   uFlags);

PALIMPORT
HLOCAL
PALAPI
LocalFree(
      IN HLOCAL hMem);

#define MAX_LEADBYTES         12
#define MAX_DEFAULTCHAR       2

PALIMPORT
UINT
PALAPI
GetACP(void);

typedef struct _cpinfo {
    UINT MaxCharSize;
    BYTE DefaultChar[MAX_DEFAULTCHAR];
    BYTE LeadByte[MAX_LEADBYTES];
} CPINFO, *LPCPINFO;

PALIMPORT
BOOL
PALAPI
GetCPInfo(
      IN UINT CodePage,
      OUT LPCPINFO lpCPInfo);

PALIMPORT
BOOL
PALAPI
IsDBCSLeadByteEx(
         IN UINT CodePage,
         IN BYTE TestChar);

PALIMPORT
BOOL
PALAPI
IsDBCSLeadByte(
        IN BYTE TestChar);

PALIMPORT
BOOL
PALAPI
IsValidCodePage(
        IN UINT CodePage);
        

#define MB_PRECOMPOSED            0x00000001
#define MB_ERR_INVALID_CHARS      0x00000008

PALIMPORT
int
PALAPI
MultiByteToWideChar(
            IN UINT CodePage,
            IN DWORD dwFlags,
            IN LPCSTR lpMultiByteStr,
            IN int cbMultiByte,
            OUT LPWSTR lpWideCharStr,
            IN int cchWideChar);

#define WC_NO_BEST_FIT_CHARS      0x00000400

PALIMPORT
int
PALAPI
WideCharToMultiByte(
            IN UINT CodePage,
            IN DWORD dwFlags,
            IN LPCWSTR lpWideCharStr,
            IN int cchWideChar,
            OUT LPSTR lpMultiByteStr,
            IN int cbMultyByte,
            IN LPCSTR lpDefaultChar,
            OUT LPBOOL lpUsedDefaultChar);

#define EXCEPTION_NONCONTINUABLE 0x1
#define EXCEPTION_UNWINDING 0x2

#ifdef FEATURE_PAL_SXS

#define EXCEPTION_EXIT_UNWIND 0x4       // Exit unwind is in progress (not used by PAL SEH)
#define EXCEPTION_NESTED_CALL 0x10      // Nested exception handler call
#define EXCEPTION_TARGET_UNWIND 0x20    // Target unwind in progress
#define EXCEPTION_COLLIDED_UNWIND 0x40  // Collided exception handler call
#define EXCEPTION_SKIP_VEH 0x200

#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)

#endif // FEATURE_PAL_SXS

#define EXCEPTION_IS_SIGNAL 0x100

#define EXCEPTION_MAXIMUM_PARAMETERS 15

// Index in the ExceptionInformation array where we will keep the reference
// to the native exception that needs to be deleted when dispatching
// exception in managed code.
#define NATIVE_EXCEPTION_ASYNC_SLOT (EXCEPTION_MAXIMUM_PARAMETERS-1)

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *LPEXCEPTION_POINTERS;

//
// A function table entry is generated for each frame function.
//
typedef struct _RUNTIME_FUNCTION {
    DWORD BeginAddress;
#ifdef _TARGET_AMD64_
    DWORD EndAddress;
#endif
    DWORD UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

#define STANDARD_RIGHTS_REQUIRED  (0x000F0000L)
#define SYNCHRONIZE               (0x00100000L)
#define READ_CONTROL              (0x00020000L)
#define MAXIMUM_ALLOWED           (0x02000000L)

#define EVENT_MODIFY_STATE        (0x0002)
#define EVENT_ALL_ACCESS          (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3) 

#define MUTANT_QUERY_STATE        (0x0001)
#define MUTANT_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | MUTANT_QUERY_STATE)
#define MUTEX_ALL_ACCESS          MUTANT_ALL_ACCESS

#define SEMAPHORE_MODIFY_STATE    (0x0002)
#define SEMAPHORE_ALL_ACCESS      (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3)

#define PROCESS_TERMINATE         (0x0001)  
#define PROCESS_CREATE_THREAD     (0x0002)  
#define PROCESS_SET_SESSIONID     (0x0004)  
#define PROCESS_VM_OPERATION      (0x0008)  
#define PROCESS_VM_READ           (0x0010)  
#define PROCESS_VM_WRITE          (0x0020)  
#define PROCESS_DUP_HANDLE        (0x0040)  
#define PROCESS_CREATE_PROCESS    (0x0080)  
#define PROCESS_SET_QUOTA         (0x0100)  
#define PROCESS_SET_INFORMATION   (0x0200)  
#define PROCESS_QUERY_INFORMATION (0x0400)  
#define PROCESS_SUSPEND_RESUME    (0x0800)  
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)

PALIMPORT
BOOL
PALAPI
EnumProcessModules(
    IN HANDLE hProcess,
    OUT HMODULE *lphModule,
    IN DWORD cb,
    OUT LPDWORD lpcbNeeded
    );

PALIMPORT
VOID
PALAPI
OutputDebugStringA(
    IN LPCSTR lpOutputString);

PALIMPORT
VOID
PALAPI
OutputDebugStringW(
    IN LPCWSTR lpOutputStrig);

#ifdef UNICODE
#define OutputDebugString OutputDebugStringW
#else
#define OutputDebugString OutputDebugStringA
#endif

PALIMPORT
VOID
PALAPI
DebugBreak(
    VOID);

PALIMPORT
int
PALAPI
lstrlenA(
     IN LPCSTR lpString);

PALIMPORT
int
PALAPI
lstrlenW(
     IN LPCWSTR lpString);

#ifdef UNICODE
#define lstrlen lstrlenW
#else
#define lstrlen lstrlenA
#endif

PALIMPORT
DWORD
PALAPI
GetEnvironmentVariableA(
    IN LPCSTR lpName,
    OUT LPSTR lpBuffer,
    IN DWORD nSize);

PALIMPORT
DWORD
PALAPI
GetEnvironmentVariableW(
    IN LPCWSTR lpName,
    OUT LPWSTR lpBuffer,
    IN DWORD nSize);

#ifdef UNICODE
#define GetEnvironmentVariable GetEnvironmentVariableW
#else
#define GetEnvironmentVariable GetEnvironmentVariableA
#endif


PALIMPORT
BOOL
PALAPI
SetEnvironmentVariableA(
    IN LPCSTR lpName,
    IN LPCSTR lpValue);

PALIMPORT
BOOL
PALAPI
SetEnvironmentVariableW(
    IN LPCWSTR lpName,
    IN LPCWSTR lpValue);

#ifdef UNICODE
#define SetEnvironmentVariable SetEnvironmentVariableW
#else
#define SetEnvironmentVariable SetEnvironmentVariableA
#endif

PALIMPORT
LPSTR
PALAPI
GetEnvironmentStringsA(
    VOID);

PALIMPORT
LPWSTR
PALAPI
GetEnvironmentStringsW(
    VOID);

#ifdef UNICODE
#define GetEnvironmentStrings GetEnvironmentStringsW
#else
#define GetEnvironmentStrings GetEnvironmentStringsA
#endif


PALIMPORT
BOOL
PALAPI
FreeEnvironmentStringsA(
    IN LPSTR);

PALIMPORT
BOOL
PALAPI
FreeEnvironmentStringsW(
    IN LPWSTR);

#ifdef UNICODE
#define FreeEnvironmentStrings FreeEnvironmentStringsW
#else
#define FreeEnvironmentStrings FreeEnvironmentStringsA
#endif


PALIMPORT
BOOL
PALAPI
CloseHandle(
    IN OUT HANDLE hObject);

PALIMPORT
DWORD
PALAPI
GetTickCount(
    VOID);

PALIMPORT
ULONGLONG
PALAPI
GetTickCount64(
    VOID);

PALIMPORT
BOOL
PALAPI
QueryPerformanceCounter(
    OUT LARGE_INTEGER *lpPerformanceCount
    );

PALIMPORT
BOOL
PALAPI
QueryPerformanceFrequency(
    OUT LARGE_INTEGER *lpFrequency
    );

PALIMPORT
INT
PALAPI
PAL_nanosleep(
    IN long timeInNs);

#ifndef FEATURE_PAL_SXS

typedef LONG (PALAPI *PTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS *ExceptionInfo);
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

PALIMPORT
LPTOP_LEVEL_EXCEPTION_FILTER
PALAPI
SetUnhandledExceptionFilter(
                IN LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

#else // FEATURE_PAL_SXS

typedef EXCEPTION_DISPOSITION (PALAPI *PVECTORED_EXCEPTION_HANDLER)(
                           struct _EXCEPTION_POINTERS *ExceptionPointers);

#endif // FEATURE_PAL_SXS

// Define BitScanForward64 and BitScanForward
// Per MSDN, BitScanForward64 will search the mask data from LSB to MSB for a set bit.
// If one is found, its bit position is returned in the out PDWORD argument and 1 is returned.
// Otherwise, 0 is returned.
//
// On GCC, the equivalent function is __builtin_ffsl. It returns 1+index of the least
// significant set bit, or 0 if if mask is zero.
//
// The same is true for BitScanForward, except that the GCC function is __builtin_ffs.
EXTERN_C
PALIMPORT
inline
unsigned char
PALAPI
BitScanForward(
    IN OUT PDWORD Index,
    IN UINT qwMask)
{
    unsigned char bRet = FALSE;
    int iIndex = __builtin_ffsl(qwMask);
    if (iIndex != 0)
    {
        // Set the Index after deducting unity
        *Index = (DWORD)(iIndex - 1);
        bRet = TRUE;
    }

    return bRet;
}

EXTERN_C
PALIMPORT
inline
unsigned char
PALAPI
BitScanForward64(
    IN OUT PDWORD Index,
    IN UINT64 qwMask)
{
    unsigned char bRet = FALSE;
    int iIndex = __builtin_ffsl(qwMask);
    if (iIndex != 0)
    {
        // Set the Index after deducting unity
        *Index = (DWORD)(iIndex - 1);
        bRet = TRUE;
    }

    return bRet;
}

// Define BitScanReverse64 and BitScanReverse
// Per MSDN, BitScanReverse64 or BitScanReverse will search the mask data from most significant bit (MSB) 
// to least significant bit (LSB) for a set bit (1).
// If one is found, its bit position is returned in the out PDWORD argument and 1 is returned.
// Otherwise, 0 is returned.
//
// On GCC, the equivalent function is __builtin_clzll or __builtin_clz. It returns 1+index of the most
// significant set bit, or undefined result if mask is zero.
EXTERN_C
PALIMPORT
inline
unsigned char
PALAPI
BitScanReverse(
    IN OUT PDWORD Index,
    IN UINT qwMask)
{
    unsigned char bRet = FALSE;
    if (qwMask != 0)
    {
        *Index = (UINT) (8 * sizeof (UINT) - __builtin_clz(qwMask) - 1);
        bRet = TRUE;
    }

    return bRet;
}

EXTERN_C
PALIMPORT
inline
unsigned char
PALAPI
BitScanReverse64(
    IN OUT PDWORD Index,
    IN UINT64 qwMask)
{
    unsigned char bRet = FALSE;
    if (qwMask != 0)
    {
        *Index = (UINT) (8 * sizeof (UINT64) - __builtin_clzll(qwMask) - 1);
        bRet = TRUE;
    }

    return bRet;
}

FORCEINLINE void PAL_ArmInterlockedOperationBarrier()
{
#ifdef _ARM64_
    // On arm64, most of the __sync* functions generate a code sequence like:
    //   loop:
    //     ldaxr (load acquire exclusive)
    //     ...
    //     stlxr (store release exclusive)
    //     cbnz loop
    //
    // It is possible for a load following the code sequence above to be reordered to occur prior to the store above due to the
    // release barrier, this is substantiated by https://github.com/dotnet/coreclr/pull/17508. Interlocked operations in the PAL
    // require the load to occur after the store. This memory barrier should be used following a call to a __sync* function to
    // prevent that reordering. Code generated for arm32 includes a 'dmb' after 'cbnz', so no issue there at the moment.
    __sync_synchronize();
#endif // _ARM64_
}

/*++
Function:
InterlockedIncrement

The InterlockedIncrement function increments (increases by one) the
value of the specified variable and checks the resulting value. The
function prevents more than one thread from using the same variable
simultaneously.

Parameters

lpAddend 
[in/out] Pointer to the variable to increment. 

Return Values

The return value is the resulting incremented value. 

--*/
EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedIncrement(
    IN OUT LONG volatile *lpAddend)
{
    LONG result = __sync_add_and_fetch(lpAddend, (LONG)1);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
LONGLONG
PALAPI
InterlockedIncrement64(
    IN OUT LONGLONG volatile *lpAddend)
{
    LONGLONG result = __sync_add_and_fetch(lpAddend, (LONGLONG)1);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

/*++
Function:
InterlockedDecrement

The InterlockedDecrement function decrements (decreases by one) the
value of the specified variable and checks the resulting value. The
function prevents more than one thread from using the same variable
simultaneously.

Parameters

lpAddend 
[in/out] Pointer to the variable to decrement. 

Return Values

The return value is the resulting decremented value.

--*/
EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedDecrement(
    IN OUT LONG volatile *lpAddend)
{
    LONG result = __sync_sub_and_fetch(lpAddend, (LONG)1);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement

EXTERN_C
PALIMPORT
inline
LONGLONG
PALAPI
InterlockedDecrement64(
    IN OUT LONGLONG volatile *lpAddend)
{
    LONGLONG result = __sync_sub_and_fetch(lpAddend, (LONGLONG)1);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

/*++
Function:
InterlockedExchange

The InterlockedExchange function atomically exchanges a pair of
values. The function prevents more than one thread from using the same
variable simultaneously.

Parameters

Target 
[in/out] Pointer to the value to exchange. The function sets
this variable to Value, and returns its prior value.
Value 
[in] Specifies a new value for the variable pointed to by Target. 

Return Values

The function returns the initial value pointed to by Target. 

--*/
EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value)
{
    LONG result = __sync_swap(Target, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
LONGLONG
PALAPI
InterlockedExchange64(
    IN OUT LONGLONG volatile *Target,
    IN LONGLONG Value)
{
    LONGLONG result = __sync_swap(Target, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

/*++
Function:
InterlockedCompareExchange

The InterlockedCompareExchange function performs an atomic comparison
of the specified values and exchanges the values, based on the outcome
of the comparison. The function prevents more than one thread from
using the same variable simultaneously.

If you are exchanging pointer values, this function has been
superseded by the InterlockedCompareExchangePointer function.

Parameters

Destination     [in/out] Specifies the address of the destination value. The sign is ignored.
Exchange        [in]     Specifies the exchange value. The sign is ignored.
Comperand       [in]     Specifies the value to compare to Destination. The sign is ignored.

Return Values

The return value is the initial value of the destination.

--*/
EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG Exchange,
    IN LONG Comperand)
{
    LONG result =
        __sync_val_compare_and_swap(
            Destination, /* The pointer to a variable whose value is to be compared with. */
            Comperand, /* The value to be compared */
            Exchange /* The value to be stored */);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange

// See the 32-bit variant in interlock2.s
EXTERN_C
PALIMPORT
inline
LONGLONG
PALAPI
InterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Exchange,
    IN LONGLONG Comperand)
{
    LONGLONG result =
        __sync_val_compare_and_swap(
            Destination, /* The pointer to a variable whose value is to be compared with. */
            Comperand, /* The value to be compared */
            Exchange /* The value to be stored */);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

/*++
Function:
InterlockedExchangeAdd

The InterlockedExchangeAdd function atomically adds the value of 'Value'
to the variable that 'Addend' points to.

Parameters

lpAddend
[in/out] Pointer to the variable to to added.

Return Values

The return value is the original value that 'Addend' pointed to.

--*/
EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value)
{
    LONG result = __sync_fetch_and_add(Addend, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
LONGLONG
PALAPI
InterlockedExchangeAdd64(
    IN OUT LONGLONG volatile *Addend,
    IN LONGLONG Value)
{
    LONGLONG result = __sync_fetch_and_add(Addend, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedAnd(
    IN OUT LONG volatile *Destination,
    IN LONG Value)
{
    LONG result = __sync_fetch_and_and(Destination, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
LONG
PALAPI
InterlockedOr(
    IN OUT LONG volatile *Destination,
    IN LONG Value)
{
    LONG result = __sync_fetch_and_or(Destination, Value);
    PAL_ArmInterlockedOperationBarrier();
    return result;
}

EXTERN_C
PALIMPORT
inline
UCHAR
PALAPI
InterlockedBitTestAndReset(
    IN OUT LONG volatile *Base,
    IN LONG Bit)
{
    return (InterlockedAnd(Base, ~(1 << Bit)) & (1 << Bit)) != 0;
}

EXTERN_C
PALIMPORT
inline
UCHAR
PALAPI
InterlockedBitTestAndSet(
    IN OUT LONG volatile *Base,
    IN LONG Bit)
{
    return (InterlockedOr(Base, (1 << Bit)) & (1 << Bit)) != 0;
}

#if defined(BIT64)
#define InterlockedExchangePointer(Target, Value) \
    ((PVOID)InterlockedExchange64((PLONG64)(Target), (LONGLONG)(Value)))

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    ((PVOID)InterlockedCompareExchange64((PLONG64)(Destination), (LONGLONG)(ExChange), (LONGLONG)(Comperand)))
#else
#define InterlockedExchangePointer(Target, Value) \
    ((PVOID)(UINT_PTR)InterlockedExchange((PLONG)(UINT_PTR)(Target), (LONG)(UINT_PTR)(Value)))

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    ((PVOID)(UINT_PTR)InterlockedCompareExchange((PLONG)(UINT_PTR)(Destination), (LONG)(UINT_PTR)(ExChange), (LONG)(UINT_PTR)(Comperand)))
#endif

/*++
Function:
MemoryBarrier

The MemoryBarrier function creates a full memory barrier.

--*/
EXTERN_C
PALIMPORT
inline
VOID
PALAPI
MemoryBarrier(
    VOID)
{
    __sync_synchronize();
}

EXTERN_C
PALIMPORT
inline
VOID
PALAPI
YieldProcessor(
    VOID)
{
#if defined(_X86_) || defined(_AMD64_)
    __asm__ __volatile__(
        "rep\n"
        "nop");
#elif defined(_ARM64_)
    __asm__ __volatile__( "yield");
#else
    return;
#endif
}

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define FORMAT_MESSAGE_FROM_STRING     0x00000400
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF

PALIMPORT
DWORD
PALAPI
FormatMessageW(
           IN DWORD dwFlags,
           IN LPCVOID lpSource,
           IN DWORD dwMessageId,
           IN DWORD dwLanguageId,
           OUT LPWSTR lpBffer,
           IN DWORD nSize,
           IN va_list *Arguments);

#ifdef UNICODE
#define FormatMessage FormatMessageW
#endif


PALIMPORT
DWORD
PALAPI
GetLastError(
         VOID);

PALIMPORT
VOID
PALAPI
SetLastError(
         IN DWORD dwErrCode);

#define VER_PLATFORM_WIN32_WINDOWS        1
#define VER_PLATFORM_WIN32_NT        2
#define VER_PLATFORM_UNIX            10
#define VER_PLATFORM_MACOSX          11

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[ 128 ];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[ 128 ];
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW;

#ifdef UNICODE
typedef OSVERSIONINFOW OSVERSIONINFO;
typedef POSVERSIONINFOW POSVERSIONINFO;
typedef LPOSVERSIONINFOW LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#endif

typedef struct _OSVERSIONINFOEXA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[ 128 ];
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;

typedef struct _OSVERSIONINFOEXW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[ 128 ];
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW;

#ifdef UNICODE
typedef OSVERSIONINFOEXW OSVERSIONINFOEX;
typedef POSVERSIONINFOEXW POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXW LPOSVERSIONINFOEX;
#else
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef POSVERSIONINFOEXA POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXA LPOSVERSIONINFOEX;
#endif

#define IMAGE_FILE_MACHINE_I386              0x014c
#define IMAGE_FILE_MACHINE_ARM64             0xAA64  // ARM64 Little-Endian

typedef struct _SYSTEM_INFO {
    WORD wProcessorArchitecture_PAL_Undefined;
    WORD wReserved_PAL_Undefined; // NOTE: diff from winbase.h - no obsolete dwOemId union
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask_PAL_Undefined;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType_PAL_Undefined;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel_PAL_Undefined;
    WORD wProcessorRevision_PAL_Undefined;
} SYSTEM_INFO, *LPSYSTEM_INFO;

PALIMPORT
VOID
PALAPI
GetSystemInfo(
          OUT LPSYSTEM_INFO lpSystemInfo);

PALIMPORT
BOOL
PALAPI
CreatePipe(
    OUT PHANDLE hReadPipe,
    OUT PHANDLE hWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize
    );

/******************* C Runtime Entrypoints *******************************/

/* Some C runtime functions needs to be reimplemented by the PAL.
   To avoid name collisions, those functions have been renamed using
   defines */
#ifndef PAL_STDCPP_COMPAT
#define atexit        PAL_atexit
#define printf        PAL_printf
#define vprintf       PAL_vprintf
#define wprintf       PAL_wprintf
#define wcsspn        PAL_wcsspn
#define wcstod        PAL_wcstod
#define wcstol        PAL_wcstol
#define wcstoul       PAL_wcstoul
#define wcscat        PAL_wcscat
#define wcscpy        PAL_wcscpy
#define wcslen        PAL_wcslen
#define wcsncmp       PAL_wcsncmp
#define wcschr        PAL_wcschr
#define wcsrchr       PAL_wcsrchr
#define wcsstr        PAL_wcsstr
#define swscanf       PAL_swscanf
#define wcspbrk       PAL_wcspbrk
#define wcscmp        PAL_wcscmp
#define wcsncat       PAL_wcsncat
#define wcsncpy       PAL_wcsncpy
#define wcstok        PAL_wcstok
#define wcscspn       PAL_wcscspn
#define iswprint      PAL_iswprint
#define iswalpha      PAL_iswalpha
#define iswdigit      PAL_iswdigit
#define iswspace      PAL_iswspace
#define iswupper      PAL_iswupper
#define iswxdigit     PAL_iswxdigit
#define towlower      PAL_towlower
#define towupper      PAL_towupper
#define realloc       PAL_realloc
#define fopen         PAL_fopen
#define strtok        PAL_strtok
#define strtoul       PAL_strtoul
#define fprintf       PAL_fprintf
#define fwprintf      PAL_fwprintf
#define vfprintf      PAL_vfprintf
#define vfwprintf     PAL_vfwprintf
#define ctime         PAL_ctime
#define localtime     PAL_localtime
#define mktime        PAL_mktime
#define rand          PAL_rand
#define time          PAL_time
#define getenv        PAL_getenv
#define fgets         PAL_fgets
#define fgetws        PAL_fgetws
#define fputc         PAL_fputc
#define putchar       PAL_putchar
#define qsort         PAL_qsort
#define bsearch       PAL_bsearch
#define ferror        PAL_ferror
#define fread         PAL_fread
#define fwrite        PAL_fwrite
#define feof          PAL_feof
#define ftell         PAL_ftell
#define fclose        PAL_fclose
#define setbuf        PAL_setbuf
#define fflush        PAL_fflush
#define fputs         PAL_fputs
#define fseek         PAL_fseek
#define fgetpos       PAL_fgetpos
#define fsetpos       PAL_fsetpos
#define getc          PAL_getc
#define fgetc         PAL_getc // not a typo
#define ungetc        PAL_ungetc
#define setvbuf       PAL_setvbuf
#define atol          PAL_atol
#define labs          PAL_labs
#define acos          PAL_acos
#define acosh         PAL_acosh
#define asin          PAL_asin
#define asinh         PAL_asinh
#define atan2         PAL_atan2
#define exp           PAL_exp
#define log           PAL_log
#define log10         PAL_log10
#define acosf         PAL_acosf
#define acoshf        PAL_acoshf
#define asinf         PAL_asinf
#define asinhf        PAL_asinhf
#define atan2f        PAL_atan2f
#define expf          PAL_expf
#define logf          PAL_logf
#define log10f        PAL_log10f
#define malloc        PAL_malloc
#define free          PAL_free
#define mkstemp       PAL_mkstemp
#define rename        PAL_rename
#define _getcwd       PAL__getcwd
#define _open         PAL__open
#define _close        PAL__close
#define _wcstoui64    PAL__wcstoui64
#define _flushall     PAL__flushall
#define strnlen       PAL_strnlen
#define wcsnlen       PAL_wcsnlen

#ifdef _AMD64_ 
#define _mm_getcsr    PAL__mm_getcsr
#define _mm_setcsr    PAL__mm_setcsr
#endif // _AMD64_

#endif // !PAL_STDCPP_COMPAT

#define _strdup       strdup

#ifndef _CONST_RETURN
#ifdef  __cplusplus
#define _CONST_RETURN  const
#define _CRT_CONST_CORRECT_OVERLOADS
#else
#define _CONST_RETURN
#endif
#endif

/* For backwards compatibility */
#define _WConst_return _CONST_RETURN

#define EOF     (-1)

typedef int errno_t;

#ifndef PAL_STDCPP_COMPAT

typedef struct {
    int quot;
    int rem;
} div_t;

PALIMPORT div_t div(int numer, int denom);

#ifndef __THROW
#define __THROW
#endif 

PALIMPORT void * __cdecl memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n) __THROW;
PALIMPORT int    __cdecl memcmp(const void *, const void *, size_t);
PALIMPORT void * __cdecl memset(void *, int, size_t);
PALIMPORT void * __cdecl memmove(void *, const void *, size_t);
PALIMPORT void * __cdecl memchr(const void *, int, size_t);
PALIMPORT size_t __cdecl strlen(const char *);
PALIMPORT int __cdecl strcmp(const char*, const char *);
PALIMPORT int __cdecl strncmp(const char*, const char *, size_t);
PALIMPORT int __cdecl _strnicmp(const char *, const char *, size_t);
PALIMPORT char * __cdecl strcat(char *, const char *);
PALIMPORT char * __cdecl strncat(char *, const char *, size_t);
PALIMPORT char * __cdecl strcpy(char *, const char *);
PALIMPORT char * __cdecl strncpy(char *, const char *, size_t);
PALIMPORT char * __cdecl strchr(const char *, int);
PALIMPORT char * __cdecl strrchr(const char *, int);
PALIMPORT char * __cdecl strpbrk(const char *, const char *);
PALIMPORT char * __cdecl strstr(const char *, const char *);
PALIMPORT char * __cdecl strtok(char *, const char *);
PALIMPORT size_t __cdecl strspn(const char *, const char *);
PALIMPORT size_t  __cdecl strcspn(const char *, const char *);
PALIMPORT int __cdecl atoi(const char *);
PALIMPORT LONG __cdecl atol(const char *);
PALIMPORT ULONG __cdecl strtoul(const char *, char **, int);
PALIMPORT double __cdecl atof(const char *);
PALIMPORT double __cdecl strtod(const char *, char **);
PALIMPORT int __cdecl isprint(int);
PALIMPORT int __cdecl isspace(int);
PALIMPORT int __cdecl isalpha(int);
PALIMPORT int __cdecl isalnum(int);
PALIMPORT int __cdecl isdigit(int);
PALIMPORT int __cdecl isxdigit(int);
PALIMPORT int __cdecl isupper(int);
PALIMPORT int __cdecl islower(int);
PALIMPORT int __cdecl tolower(int);
PALIMPORT int __cdecl toupper(int);

#endif // PAL_STDCPP_COMPAT

/* _TRUNCATE */
#if !defined(_TRUNCATE)
#define _TRUNCATE ((size_t)-1)
#endif

PALIMPORT errno_t __cdecl memcpy_s(void *, size_t, const void *, size_t);
PALIMPORT errno_t __cdecl memmove_s(void *, size_t, const void *, size_t);
PALIMPORT char * __cdecl _strlwr(char *);
PALIMPORT int __cdecl _stricmp(const char *, const char *);
PALIMPORT int __cdecl vsprintf_s(char *, size_t, const char *, va_list);
PALIMPORT char * __cdecl _gcvt_s(char *, int, double, int);
PALIMPORT int __cdecl __iscsym(int);
PALIMPORT unsigned char * __cdecl _mbsinc(const unsigned char *);
PALIMPORT unsigned char * __cdecl _mbsninc(const unsigned char *, size_t);
PALIMPORT unsigned char * __cdecl _mbsdec(const unsigned char *, const unsigned char *);
PALIMPORT int __cdecl _wcsicmp(const WCHAR *, const WCHAR*);
PALIMPORT int __cdecl _wcsnicmp(const WCHAR *, const WCHAR *, size_t);
PALIMPORT int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
PALIMPORT int __cdecl _vsnprintf_s(char *, size_t, size_t, const char *, va_list);
PALIMPORT int __cdecl _vsnwprintf_s(WCHAR *, size_t, size_t, const WCHAR *, va_list);
PALIMPORT int __cdecl _snwprintf_s(WCHAR *, size_t, size_t, const WCHAR *, ...);
PALIMPORT int __cdecl _snprintf_s(char *, size_t, size_t, const char *, ...);
PALIMPORT int __cdecl sprintf_s(char *, size_t, const char *, ... );
PALIMPORT int __cdecl swprintf_s(WCHAR *, size_t, const WCHAR *, ... );
PALIMPORT int __cdecl _snwprintf_s(WCHAR *, size_t, size_t, const WCHAR *, ...);
PALIMPORT int __cdecl vswprintf_s( WCHAR *, size_t, const WCHAR *, va_list);
PALIMPORT int __cdecl sscanf_s(const char *, const char *, ...);
PALIMPORT errno_t __cdecl _itow_s(int, WCHAR *, size_t, int);

PALIMPORT size_t __cdecl PAL_wcslen(const WCHAR *);
PALIMPORT int __cdecl PAL_wcscmp(const WCHAR*, const WCHAR*);
PALIMPORT int __cdecl PAL_wcsncmp(const WCHAR *, const WCHAR *, size_t);
PALIMPORT WCHAR * __cdecl PAL_wcscat(WCHAR *, const WCHAR *);
PALIMPORT WCHAR * __cdecl PAL_wcsncat(WCHAR *, const WCHAR *, size_t);
PALIMPORT WCHAR * __cdecl PAL_wcscpy(WCHAR *, const WCHAR *);
PALIMPORT WCHAR * __cdecl PAL_wcsncpy(WCHAR *, const WCHAR *, size_t);
PALIMPORT const WCHAR * __cdecl PAL_wcschr(const WCHAR *, WCHAR);
PALIMPORT const WCHAR * __cdecl PAL_wcsrchr(const WCHAR *, WCHAR);
PALIMPORT WCHAR _WConst_return * __cdecl PAL_wcspbrk(const WCHAR *, const WCHAR *);
PALIMPORT WCHAR _WConst_return * __cdecl PAL_wcsstr(const WCHAR *, const WCHAR *);
PALIMPORT WCHAR * __cdecl PAL_wcstok(WCHAR *, const WCHAR *);
PALIMPORT size_t __cdecl PAL_wcscspn(const WCHAR *, const WCHAR *);
PALIMPORT int __cdecl PAL_swprintf(WCHAR *, const WCHAR *, ...);
PALIMPORT int __cdecl PAL_vswprintf(WCHAR *, const WCHAR *, va_list);
PALIMPORT int __cdecl PAL_swscanf(const WCHAR *, const WCHAR *, ...);
PALIMPORT LONG __cdecl PAL_wcstol(const WCHAR *, WCHAR **, int);
PALIMPORT ULONG __cdecl PAL_wcstoul(const WCHAR *, WCHAR **, int);
PALIMPORT size_t __cdecl PAL_wcsspn (const WCHAR *, const WCHAR *);
PALIMPORT double __cdecl PAL_wcstod(const WCHAR *, WCHAR **);
PALIMPORT int __cdecl PAL_iswalpha(WCHAR);
PALIMPORT int __cdecl PAL_iswprint(WCHAR);
PALIMPORT int __cdecl PAL_iswupper(WCHAR);
PALIMPORT int __cdecl PAL_iswspace(WCHAR);
PALIMPORT int __cdecl PAL_iswdigit(WCHAR);
PALIMPORT int __cdecl PAL_iswxdigit(WCHAR);
PALIMPORT WCHAR __cdecl PAL_towlower(WCHAR);
PALIMPORT WCHAR __cdecl PAL_towupper(WCHAR);

PALIMPORT WCHAR * __cdecl _wcslwr(WCHAR *);
PALIMPORT ULONGLONG _wcstoui64(const WCHAR *, WCHAR **, int);
PALIMPORT errno_t __cdecl _i64tow_s(long long, WCHAR *, size_t, int);
PALIMPORT int __cdecl _wtoi(const WCHAR *);

#ifdef __cplusplus
extern "C++" {
inline WCHAR *PAL_wcschr(WCHAR *_S, WCHAR _C)
        {return ((WCHAR *)PAL_wcschr((const WCHAR *)_S, _C)); }
inline WCHAR *PAL_wcsrchr(WCHAR *_S, WCHAR _C)
        {return ((WCHAR *)PAL_wcsrchr((const WCHAR *)_S, _C)); }
inline WCHAR *PAL_wcspbrk(WCHAR *_S, const WCHAR *_P)
        {return ((WCHAR *)PAL_wcspbrk((const WCHAR *)_S, _P)); }
inline WCHAR *PAL_wcsstr(WCHAR *_S, const WCHAR *_P)
        {return ((WCHAR *)PAL_wcsstr((const WCHAR *)_S, _P)); }
}
#endif

#if !__has_builtin(_rotl)
/*++
Function:
_rotl

See MSDN doc.
--*/
EXTERN_C
PALIMPORT
inline
unsigned int __cdecl _rotl(unsigned int value, int shift)
{
    unsigned int retval = 0;

    shift &= 0x1f;
    retval = (value << shift) | (value >> (sizeof(int) * CHAR_BIT - shift));
    return retval;
}
#endif // !__has_builtin(_rotl)

// On 64 bit unix, make the long an int.
#ifdef BIT64
#define _lrotl _rotl
#endif // BIT64

#if !__has_builtin(_rotr)

/*++
Function:
_rotr

See MSDN doc.
--*/
EXTERN_C
PALIMPORT
inline
unsigned int __cdecl _rotr(unsigned int value, int shift)
{
    unsigned int retval;

    shift &= 0x1f;
    retval = (value >> shift) | (value << (sizeof(int) * CHAR_BIT - shift));
    return retval;
}

#endif // !__has_builtin(_rotr)

PALIMPORT int __cdecl abs(int);

// clang complains if this is declared with __int64
PALIMPORT long long __cdecl llabs(long long);

PALIMPORT double __cdecl fabs(double);
PALIMPORT float __cdecl fabsf(float);

#ifndef PAL_STDCPP_COMPAT

#ifdef __cplusplus
extern "C++" {

inline __int64 abs(__int64 _X) {
    return llabs(_X);
}

}
#endif // __cplusplus

PALIMPORT void * __cdecl malloc(size_t);
PALIMPORT void   __cdecl free(void *);
PALIMPORT void * __cdecl realloc(void *, size_t);

#if defined(_MSC_VER)
#define alloca _alloca
#else
#define _alloca alloca
#endif //_MSC_VER

#define alloca  __builtin_alloca

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#endif // !PAL_STDCPP_COMPAT

PALIMPORT void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)(const void *, const void *));
PALIMPORT void * __cdecl bsearch(const void *, const void *, size_t, size_t,
int (__cdecl *)(const void *, const void *));

PALIMPORT char * __cdecl _fullpath(char *, const char *, size_t);

#ifndef PAL_STDCPP_COMPAT
PALIMPORT time_t __cdecl time(time_t *);

struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };

PALIMPORT struct tm * __cdecl localtime(const time_t *);
PALIMPORT time_t __cdecl mktime(struct tm *);
PALIMPORT char * __cdecl ctime(const time_t *);
#endif // !PAL_STDCPP_COMPAT

PALIMPORT int __cdecl _open_osfhandle(INT_PTR, int);
PALIMPORT int __cdecl _close(int);
PALIMPORT int __cdecl _flushall();

#ifdef PAL_STDCPP_COMPAT

struct _PAL_FILE;
typedef struct _PAL_FILE PAL_FILE;

#else // PAL_STDCPP_COMPAT

struct _FILE;
typedef struct _FILE FILE;
typedef struct _FILE PAL_FILE;

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

/* Locale categories */
#define LC_ALL          0
#define LC_COLLATE      1
#define LC_CTYPE        2
#define LC_MONETARY     3
#define LC_NUMERIC      4
#define LC_TIME         5

#define _IOFBF  0       /* setvbuf should set fully buffered */
#define _IOLBF  1       /* setvbuf should set line buffered */
#define _IONBF  2       /* setvbuf should set unbuffered */

#endif // PAL_STDCPP_COMPAT

PALIMPORT int __cdecl PAL_fclose(PAL_FILE *);
PALIMPORT void __cdecl PAL_setbuf(PAL_FILE *, char*);
PALIMPORT int __cdecl PAL_fflush(PAL_FILE *);
PALIMPORT size_t __cdecl PAL_fwrite(const void *, size_t, size_t, PAL_FILE *);
PALIMPORT size_t __cdecl PAL_fread(void *, size_t, size_t, PAL_FILE *);
PALIMPORT char * __cdecl PAL_fgets(char *, int, PAL_FILE *);
PALIMPORT int __cdecl PAL_fputs(const char *, PAL_FILE *);
PALIMPORT int __cdecl PAL_fputc(int c, PAL_FILE *stream);
PALIMPORT int __cdecl PAL_putchar(int c);
PALIMPORT int __cdecl PAL_fprintf(PAL_FILE *, const char *, ...);
PALIMPORT int __cdecl PAL_vfprintf(PAL_FILE *, const char *, va_list);
PALIMPORT int __cdecl PAL_fseek(PAL_FILE *, LONG, int);
PALIMPORT LONG __cdecl PAL_ftell(PAL_FILE *);
PALIMPORT int __cdecl PAL_feof(PAL_FILE *);
PALIMPORT int __cdecl PAL_ferror(PAL_FILE *);
PALIMPORT PAL_FILE * __cdecl PAL_fopen(const char *, const char *);
PALIMPORT int __cdecl PAL_getc(PAL_FILE *stream);
PALIMPORT int __cdecl PAL_fgetc(PAL_FILE *stream);
PALIMPORT int __cdecl PAL_ungetc(int c, PAL_FILE *stream);
PALIMPORT int __cdecl PAL_setvbuf(PAL_FILE *stream, char *, int, size_t);
PALIMPORT WCHAR * __cdecl PAL_fgetws(WCHAR *, int, PAL_FILE *);
PALIMPORT int __cdecl PAL_fwprintf(PAL_FILE *, const WCHAR *, ...);
PALIMPORT int __cdecl PAL_vfwprintf(PAL_FILE *, const WCHAR *, va_list);
PALIMPORT int __cdecl PAL_wprintf(const WCHAR*, ...);

PALIMPORT int __cdecl _getw(PAL_FILE *);
PALIMPORT int __cdecl _putw(int, PAL_FILE *);
PALIMPORT PAL_FILE * __cdecl _fdopen(int, const char *);
PALIMPORT PAL_FILE * __cdecl _wfopen(const WCHAR *, const WCHAR *);
PALIMPORT PAL_FILE * __cdecl _wfsopen(const WCHAR *, const WCHAR *, int);

/* Maximum value that can be returned by the rand function. */

#ifndef PAL_STDCPP_COMPAT
#define RAND_MAX 0x7fff
#endif // !PAL_STDCPP_COMPAT

PALIMPORT int __cdecl rand(void);
PALIMPORT void __cdecl srand(unsigned int);

PALIMPORT int __cdecl printf(const char *, ...);
PALIMPORT int __cdecl vprintf(const char *, va_list);

#ifdef _MSC_VER
#define PAL_get_caller _MSC_VER
#else
#define PAL_get_caller 0
#endif

PALIMPORT PAL_FILE * __cdecl PAL_get_stdout(int caller);
PALIMPORT PAL_FILE * __cdecl PAL_get_stdin(int caller);
PALIMPORT PAL_FILE * __cdecl PAL_get_stderr(int caller);
PALIMPORT int * __cdecl PAL_errno(int caller);

#ifdef PAL_STDCPP_COMPAT
#define PAL_stdout (PAL_get_stdout(PAL_get_caller))
#define PAL_stdin  (PAL_get_stdin(PAL_get_caller))
#define PAL_stderr (PAL_get_stderr(PAL_get_caller))
#define PAL_errno   (*PAL_errno(PAL_get_caller))
#else // PAL_STDCPP_COMPAT
#define stdout (PAL_get_stdout(PAL_get_caller))
#define stdin  (PAL_get_stdin(PAL_get_caller))
#define stderr (PAL_get_stderr(PAL_get_caller))
#define errno  (*PAL_errno(PAL_get_caller))
#endif // PAL_STDCPP_COMPAT

PALIMPORT char * __cdecl getenv(const char *);
PALIMPORT int __cdecl _putenv(const char *);

#define ERANGE          34

/******************* PAL functions for SIMD extensions *****************/

PALIMPORT
unsigned int _mm_getcsr(void);

PALIMPORT
void _mm_setcsr(unsigned int i);

/******************* PAL functions for CPU capability detection *******/

#ifdef __cplusplus

#define PAL_CPP_THROW(type, obj) { throw obj; }
#define PAL_CPP_RETHROW { throw; }
#define PAL_CPP_TRY                     try {
#define PAL_CPP_CATCH_EXCEPTION(ident)  } catch (Exception *ident) {
#define PAL_CPP_CATCH_EXCEPTION_NOARG   } catch (Exception *) {
#define PAL_CPP_CATCH_DERIVED(type, ident) } catch (type *ident) {
#define PAL_CPP_CATCH_ALL               } catch (...) {                                           \
                                            try { throw; }                                        \
                                            catch (...) {}
#define PAL_CPP_ENDTRY                  }

#ifdef FEATURE_PAL_SXS

#define PAL_TRY_FOR_DLLMAIN(ParamType, paramDef, paramRef, _reason) PAL_TRY(ParamType, paramDef, paramRef)

#else // FEATURE_PAL_SXS

#define PAL_TRY(ParamType, paramDef, paramRef)                          \
    {                                                                   \
        ParamType __param = paramRef;                                   \
        ParamType paramDef; paramDef = __param;                         \
        try {                                                           \
            HardwareExceptionHolder

#define PAL_TRY_FOR_DLLMAIN(ParamType, paramDef, paramRef, _reason)     \
    {                                                                   \
        ParamType __param = paramRef;                                   \
        ParamType paramDef; paramDef = __param;                         \
        try {                                                           \
            HardwareExceptionHolder

#define PAL_ENDTRY                                                      \
        }                                                               \
    }

#endif // FEATURE_PAL_SXS

#endif // __cplusplus

// Platform-specific library naming
// 
#ifdef __APPLE__
#define MAKEDLLNAME_W(name) u"lib" name u".dylib"
#define MAKEDLLNAME_A(name)  "lib" name  ".dylib"
#else
#define MAKEDLLNAME_W(name) u"lib" name u".so"
#define MAKEDLLNAME_A(name)  "lib" name  ".so"
#endif

#ifdef UNICODE
#define MAKEDLLNAME(x) MAKEDLLNAME_W(x)
#else
#define MAKEDLLNAME(x) MAKEDLLNAME_A(x)
#endif

#define PAL_SHLIB_PREFIX    "lib"
#define PAL_SHLIB_PREFIX_W  u"lib"

#if __APPLE__
#define PAL_SHLIB_SUFFIX    ".dylib"
#define PAL_SHLIB_SUFFIX_W  u".dylib"
#else
#define PAL_SHLIB_SUFFIX    ".so"
#define PAL_SHLIB_SUFFIX_W  u".so"
#endif

#define DBG_EXCEPTION_HANDLED            ((DWORD   )0x00010001L)    
#define DBG_CONTINUE                     ((DWORD   )0x00010002L)    
#define DBG_EXCEPTION_NOT_HANDLED        ((DWORD   )0x80010001L)    

#define DBG_TERMINATE_THREAD             ((DWORD   )0x40010003L)    
#define DBG_TERMINATE_PROCESS            ((DWORD   )0x40010004L)    
#define DBG_CONTROL_C                    ((DWORD   )0x40010005L)    
#define DBG_RIPEXCEPTION                 ((DWORD   )0x40010007L)    
#define DBG_CONTROL_BREAK                ((DWORD   )0x40010008L)    
#define DBG_COMMAND_EXCEPTION            ((DWORD   )0x40010009L)    

#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)
#define STATUS_LONGJUMP                  ((DWORD   )0x80000026L)
#define STATUS_UNWIND_CONSOLIDATE        ((DWORD   )0x80000029L)
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)
#define STATUS_INVALID_HANDLE            ((DWORD   )0xC0000008L)
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)

#define WAIT_IO_COMPLETION                  STATUS_USER_APC

#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE            STATUS_INVALID_HANDLE

#define CONTROL_C_EXIT                      STATUS_CONTROL_C_EXIT

/*  These are from the <FCNTL.H> file in windows.
    They are needed for _open_osfhandle.*/
#define _O_RDONLY   0x0000
#define _O_APPEND   0x0008
#define _O_TEXT     0x4000
#define _O_BINARY   0x8000

#ifdef  __cplusplus
}
#endif

#endif // __PAL_H__
