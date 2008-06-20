/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sun Jun 01 09:34:33 2008 texane
** Last update Tue Jun 03 23:44:42 2008 texane
*/



#include <windows.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include "debug.h"
#include "fuse.h"
#include "dokan.h"



/* time missing types
 */

struct utimbuf {
  time_t actime;
  time_t modtime;
};

struct timespec {
  time_t tv_sec;
  long tv_nsec;
};



/* fuse globals
 */

static const struct fuse_operations* gFuseOps = NULL;
static void* gFuseUserData = NULL;



/* helpers
 */

static char* nt_to_unix_filename(const wchar_t* src)
{
  size_t len;
  size_t i;
  char* dst;

  len = wcslen(src);

  dst = malloc(len + 1);
  if (dst == NULL)
    return NULL;

  for (i = 0; i < len; ++i)
    {
      if (src[i] != L'\\')
	dst[i] = (char)src[i];
      else
	dst[i] = '/';
    }

  dst[i] = 0;

  return dst;
}


static wchar_t* unix_to_nt_filename(const char* src, size_t* len)
{
  wchar_t* dst;
  size_t n;
  size_t i;

  if (len != NULL)
    *len = 0;

  n = strlen(src);

  dst = malloc((n + 1) * sizeof(wchar_t));
  if (dst == NULL)
    return NULL;

  for (i = 0; i < n; ++i)
    {
      if (src[i] != '/')
	dst[i] = (wchar_t)src[i];
      else
	dst[i] = L'\\';
    }

  dst[i] = 0;

  if (len != NULL)
    *len = n;

  return dst;
}


static void timet_to_filetime(time_t tt, PFILETIME ft)
{
  /* from msdn
   */

  LONGLONG ll;

  ll = Int32x32To64(tt, 10000000) + 116444736000000000;
  ft->dwLowDateTime = (DWORD)ll;
  ft->dwHighDateTime = (DWORD)(ll >> 32);
}


static void filetime_to_timet(const FILETIME* ft, time_t* tt)
{
  LARGE_INTEGER li;
  
  li.LowPart = ft->dwLowDateTime;
  li.HighPart = ft->dwHighDateTime;

  *tt = li.QuadPart;

#define SECS_TO_FT_MULT 10000000
  *tt /= SECS_TO_FT_MULT;
}


/* from win32 pthread
   time between jan 1, 1601 and jan 1, 1970 in units of 100 nanoseconds
 */

#define TIMESPEC_TO_FILETIME_OFFSET \
	(((LONGLONG)27111902 << 32) + (LONGLONG)3577643008)


static void timespec_to_filetime(const struct timespec* ts, FILETIME* ft)
{
  /* from win32 pthread
   */

  *(LONGLONG *)ft =
    ts->tv_sec * 10000000 +
    (ts->tv_nsec + 50) / 100 +
    TIMESPEC_TO_FILETIME_OFFSET;
}


static void filetime_to_timespec(const FILETIME* ft, struct timespec* ts)
{
  /* from win32 pthread
   */

  ts->tv_sec =
    (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET) / 10000000);

  ts->tv_nsec =
    (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET - 
	   ((LONGLONG)ts->tv_sec * (LONGLONG)10000000)) * 100);
}




/* errno to windows winerror
 */

static int errno_to_winerror(int e)
{
  static const DWORD table[] = {
    ERROR_SUCCESS, /* 0 */
    ERROR_ACCESS_DENIED, /* EPERM */
    ERROR_FILE_NOT_FOUND, /* ENOENT */
    ERROR_NOT_FOUND, /* ESRCH */
    ERROR_OPERATION_ABORTED, /* EINTR */
    ERROR_IO_DEVICE, /* EIO */
    ERROR_BAD_UNIT, /* ENXIO */
    ERROR_BAD_ARGUMENTS, /* E2BIG */
    ERROR_BAD_FORMAT, /* ENOEXEC */
    ERROR_INVALID_HANDLE, /* EBADF */
    ERROR_WAIT_NO_CHILDREN, /* ECHILD */
    ERROR_RETRY, /* EAGAIN */
    ERROR_OUTOFMEMORY, /* ENOMEM */
    ERROR_ACCESS_DENIED, /* EACCES */
    ERROR_INVALID_ADDRESS, /* EFAULT */
    ERROR_NOT_SUPPORTED, /* ENOTBLK */
    ERROR_BUSY, /* EBUSY */
    ERROR_ALREADY_EXISTS, /* EEXIST */
    ERROR_NOT_SAME_DEVICE, /* EXDEV */
    ERROR_NOT_FOUND, /* ENODEV */
    ERROR_DIRECTORY, /* ENOTDIR */
    ERROR_DIRECTORY, /* EISDIR */
    ERROR_INVALID_PARAMETER, /* EINVAL */
    ERROR_OUT_OF_STRUCTURES, /* ENFILE */
    ERROR_TOO_MANY_OPEN_FILES, /* EMFILE */
    ERROR_INVALID_FUNCTION, /* ENOTTY */
    ERROR_BUSY, /* ETXTBSY */
    ERROR_GEN_FAILURE, /* EFBIG */
    ERROR_DISK_FULL, /* ENOSPC */
    ERROR_SEEK, /* ESPIPE */
    ERROR_WRITE_PROTECT, /* EROFS */
    ERROR_TOO_MANY_LINKS, /* EMLINK */
    ERROR_BROKEN_PIPE, /* EPIPE */
    ERROR_ARITHMETIC_OVERFLOW, /* EDOM */
    ERROR_ARITHMETIC_OVERFLOW, /* ERANGE */
    ERROR_POSSIBLE_DEADLOCK, /* EDEADLK */
    ERROR_FILENAME_EXCED_RANGE, /* ENAMETOOLONG */
    ERROR_ATOMIC_LOCKS_NOT_SUPPORTED, /* ENOLCK */
    ERROR_INVALID_FUNCTION, /* ENOSYS */
    ERROR_DIR_NOT_EMPTY, /* ENOTEMPTY */
    ERROR_BAD_PATHNAME, /* ELOOP */
    ERROR_RETRY, /* EWOULDBLOCK */
    ERROR_INVALID_CATEGORY, /* ENOMSG */
    ERROR_NOT_FOUND, /* EIDRM */
    ERROR_GEN_FAILURE, /* ECHRNG */
    ERROR_GEN_FAILURE, /* EL2NSYNC */
    ERROR_GEN_FAILURE, /* EL3HLT */
    ERROR_GEN_FAILURE, /* EL3RST */
    ERROR_TOO_MANY_LINKS, /* ELNRNG */
    ERROR_NOT_READY, /* EUNATCH */
    ERROR_GEN_FAILURE, /* ENOCSI */
    ERROR_GEN_FAILURE, /* EL2HLT */
    ERROR_GEN_FAILURE, /* EBADE */
    ERROR_GEN_FAILURE, /* EBADR */
    ERROR_GEN_FAILURE, /* EXFULL */
    ERROR_GEN_FAILURE, /* ENOANO */
    ERROR_INVALID_FUNCTION, /* EBADRQC */
    ERROR_BAD_UNIT, /* EBADSLT */
    ERROR_POSSIBLE_DEADLOCK, /* EDEADLOCK */
    ERROR_BAD_FORMAT, /* EBFONT */
    ERROR_NOT_SUPPORTED, /* ENOSTR */
    ERROR_EMPTY, /* ENODATA */
    ERROR_TIMEOUT, /* ETIME */
    ERROR_NO_SYSTEM_RESOURCES, /* ENOSR */
    ERROR_DEV_NOT_EXIST, /* ENONET */
    ERROR_NOT_FOUND, /* ENOPKG */
    ERROR_NOT_SUPPORTED,  /* EREMOTE */
    ERROR_NETWORK_UNREACHABLE, /* ENOLINK */
    ERROR_GEN_FAILURE, /* EADV */
    ERROR_GEN_FAILURE, /* ESRMNT */
    ERROR_NET_WRITE_FAULT, /* ECOMM */
    ERROR_GEN_FAILURE, /* EPROTO */
    ERROR_GEN_FAILURE, /* EMULTIHOP */
    ERROR_GEN_FAILURE, /* EDOTDOT */
    ERROR_GEN_FAILURE, /* BADMSG */
    ERROR_BUFFER_OVERFLOW, /* EOVERFLOW */
    ERROR_DUP_NAME, /* ENOTUNIQ */
    ERROR_INVALID_HANDLE, /* EBADFD */
    ERROR_GEN_FAILURE, /* EREMCHG */
    ERROR_DLL_NOT_FOUND, /* ELIBACC */
    ERROR_INVALID_DLL, /* ELIBBAD */
    ERROR_INVALID_LIBRARY, /* ELIBSCN */
    ERROR_LIBRARY_FULL, /* ELIBMAX */
    ERROR_ACCESS_DENIED, /* ELIBEXEC */
    ERROR_INVALID_EXE_SIGNATURE, /* EILSEQ */
    ERROR_CONTINUE, /* ERESTART */
    ERROR_GEN_FAILURE, /* ESTRPIPE */
    WSAEUSERS, /* EUSERS */
    WSAENOTSOCK, /* ENOTSOCK */
    WSAEDESTADDRREQ, /* EDESTADDRREQ */
    WSAEMSGSIZE, /* EMSGSIZE */
    WSAEPROTOTYPE, /* EPROTOTYPE */
    WSAENOPROTOOPT, /* ENOPROTOOPT */
    WSAEPROTONOSUPPORT, /* EPROTONOSUPPORT */
    WSAESOCKTNOSUPPORT, /* ESOCKTNOSUPPORT */
    WSAEOPNOTSUPP, /* EOPNOTSUPP */
    WSAEPFNOSUPPORT, /* EPFNOSUPPORT */
    WSAEAFNOSUPPORT, /* EAFNOSUPPORT */
    WSAEADDRINUSE, /* EADDRINUSE */
    WSAEADDRNOTAVAIL, /* EADDRNOTAVAIL */
    WSAENETDOWN, /* ENETDOWN */
    WSAENETUNREACH, /* ENETUNREACH */
    WSAENETRESET, /* ENETRESET */
    WSAECONNABORTED, /* ECONNABORTED */
    WSAECONNRESET, /* ECONNRESET */
    WSAENOBUFS, /* ENOBUFS */
    WSAEISCONN, /* EISCONN */
    WSAENOTCONN, /* ENOTCONN */
    WSAESHUTDOWN, /* ESHUTDOWN */
    WSAETOOMANYREFS, /* ETOOMANYREFS */
    WSAETIMEDOUT, /* ETIMEDOUT */
    WSAECONNREFUSED, /* ECONNREFUSED */
    WSAEHOSTDOWN, /* EHOSTDOWN */
    WSAEHOSTUNREACH, /* EHOSTUNREACH */
    WSAEALREADY, /* WSAEALREADY */
    WSAEINPROGRESS, /* EINPROGRESS */
    WSAESTALE, /* ESTALE */
    ERROR_GEN_FAILURE, /* EUCLEAN */
    ERROR_INVALID_NAME, /* ENOTNAM */
    ERROR_SEM_NOT_FOUND, /* ENAVAIL */
    ERROR_INVALID_NAME, /* EISNAM */
    ERROR_GEN_FAILURE, /* EREMOTEIO */
    ERROR_NOT_ENOUGH_QUOTA, /* EDQUOT */
    ERROR_INVALID_MEDIA, /* ENOMEDIUM */
    ERROR_DRIVE_MEDIA_MISMATCH, /* EMEDIUMTYPE */
  };

  if ((e >= 0) && (e < (sizeof(table) / sizeof(table)[0])))
    return table[e];

  return ERROR_GEN_FAILURE;
}



/* dokan ops
 */

typedef struct _FUSEFS_FILE_CONTEXT
{
  struct fuse_file_info FuseFileInfo;
} FUSEFS_FILE_CONTEXT, *PFUSEFS_FILE_CONTEXT;



static int DOKAN_CALLBACK FusefsCreateFile(LPCWSTR Filename,
					   DWORD DesiredAccess,
					   DWORD ShareMode,
					   DWORD CreationDisposition,
					   DWORD FlagsAndAttributes,
					   PDOKAN_FILE_INFO DokanFileInfo)
{
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info FuseFileInfo;
  char* UnixFilename;
  int FuseError;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  DokanFileInfo->Context = (uintptr_t)NULL;

  if (gFuseOps->open == NULL)
    return -ERROR_INVALID_FUNCTION;

  memset(&FuseFileInfo, 0, sizeof(FuseFileInfo));

  /* translate CreateFile to open flags
   */
  
  if (DokanFileInfo->IsDirectory == FALSE)
    {
      if (FlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS)
	return -ERROR_INVALID_PARAMETER;

#define READ_ACCESS_MASK	\
        (FILE_READ_ATTRIBUTES |	\
        FILE_READ_DATA |	\
        FILE_READ_EA |		\
	STANDARD_RIGHTS_READ |	\
	SYNCHRONIZE |		\
	GENERIC_READ)

#define WRITE_ACCESS_MASK	\
        (FILE_APPEND_DATA |	\
        FILE_WRITE_ATTRIBUTES |	\
        FILE_WRITE_DATA |	\
	FILE_WRITE_EA |		\
	STANDARD_RIGHTS_WRITE |	\
	SYNCHRONIZE |		\
	GENERIC_WRITE)

      if (DesiredAccess & READ_ACCESS_MASK)
	{
	  if (DesiredAccess & WRITE_ACCESS_MASK)
	    FuseFileInfo.flags |= O_RDWR;
	  else
	    FuseFileInfo.flags |= O_RDONLY;
	}
      else if (DesiredAccess & READ_ACCESS_MASK)
	{
	  if (DesiredAccess & WRITE_ACCESS_MASK)
	    FuseFileInfo.flags |= O_RDWR;
	  else
	    FuseFileInfo.flags |= O_WRONLY;
	}
      else
	{
	  printf("rigths: 0x%08x\n", DesiredAccess);
	  return -ERROR_INVALID_PARAMETER;
	}
    }
  else
    {

#ifndef O_DIRECTORY
# define O_DIRECTORY 0100000
#endif /* ! O_DIRECTORY */

      FuseFileInfo.flags |= O_DIRECTORY;

#define GENERIC_WRITE_DIRECTORY (WRITE_ACCESS_MASK | FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY)
#define GENERIC_READ_DIRECTORY (READ_ACCESS_MASK | FILE_LIST_DIRECTORY)

      if (DesiredAccess & GENERIC_READ_DIRECTORY)
	{
	  if (DesiredAccess & GENERIC_WRITE_DIRECTORY)
	    FuseFileInfo.flags |= O_RDWR;
	  else
	    FuseFileInfo.flags |= O_RDONLY;
	}
      else if (DesiredAccess & GENERIC_WRITE_DIRECTORY)
	{
	  if (DesiredAccess & GENERIC_READ_DIRECTORY)
	    FuseFileInfo.flags |= O_RDWR;
	  else
	    FuseFileInfo.flags |= O_WRONLY;
	}
      else
	{
	  return -ERROR_INVALID_PARAMETER;
	}
    }

#ifndef O_DIRECT
# define O_DIRECT 040000
#endif /* ! O_DIRECT */

  if (FlagsAndAttributes & FILE_FLAG_NO_BUFFERING)
    FuseFileInfo.flags |= O_DIRECT;

  if (CreationDisposition == CREATE_NEW ||
      CreationDisposition == CREATE_ALWAYS)
    FuseFileInfo.flags |= O_CREAT;
  else if (CreationDisposition == TRUNCATE_EXISTING)
    FuseFileInfo.flags |= O_TRUNC;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  FuseError = gFuseOps->open(UnixFilename, &FuseFileInfo);

  free(UnixFilename);

  if (!FuseError)
    {
      FileContext = malloc(sizeof(FUSEFS_FILE_CONTEXT));
      if (FileContext == NULL)
	return -ERROR_OUTOFMEMORY;

      DokanFileInfo->Context = (uintptr_t)FileContext;

      memcpy(&FileContext->FuseFileInfo,
	     &FuseFileInfo,
	     sizeof(struct fuse_file_info));

      return 0;
    }

  return errno_to_winerror(FuseError);
}



static int DOKAN_CALLBACK FusefsOpenDirectory(LPCWSTR Filename,
					      PDOKAN_FILE_INFO DokanFileInfo)
{
  int res;

  DEBUG_ENTER();

  DokanFileInfo->IsDirectory = TRUE;

  res = FusefsCreateFile(Filename,
			 GENERIC_READ_DIRECTORY | GENERIC_WRITE_DIRECTORY,
			 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			 OPEN_EXISTING,
			 FILE_FLAG_BACKUP_SEMANTICS,
			 DokanFileInfo);

  DEBUG_LEAVE();

  return res;
}



static int DOKAN_CALLBACK FusefsCreateDirectory(LPCWSTR Filename,
						PDOKAN_FILE_INFO DokanFileInfo)
{
  int res;

  DEBUG_ENTER();

  DokanFileInfo->IsDirectory = TRUE;

  res = FusefsCreateFile(Filename,
			 GENERIC_READ_DIRECTORY | GENERIC_WRITE_DIRECTORY,
			 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			 CREATE_ALWAYS,
			 FILE_FLAG_BACKUP_SEMANTICS,
			 DokanFileInfo);

  DEBUG_LEAVE();

  return res;
}



static int DOKAN_CALLBACK FusefsCleanup(LPCWSTR Filename,
					PDOKAN_FILE_INFO DokanFileInfo)
{
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;
  char* UnixFilename;
  int (*release_fn)(const char*, struct fuse_file_info*);
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  if (DokanFileInfo->IsDirectory == FALSE)
    release_fn = gFuseOps->releasedir;
  else
    release_fn = gFuseOps->release;

  if (release_fn != NULL)
    {
      UnixFilename = nt_to_unix_filename(Filename);
      if (UnixFilename != NULL)
	{
	  err = release_fn(UnixFilename, FuseFileInfo);
	  free(UnixFilename);
	  err = -errno_to_winerror(-err);
	}
      else
	{
	  err = -ERROR_OUTOFMEMORY;
	}
    }
  else
    {
      err = -ERROR_INVALID_FUNCTION;
    }

  if (FileContext != NULL)
    {
      free(FileContext);
      DokanFileInfo->Context = (uintptr_t)NULL;
    }

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsCloseFile(LPCWSTR Filename,
					  PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixFilename;
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (gFuseOps->flush == NULL)
    return 0;

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  err = gFuseOps->flush(UnixFilename, FuseFileInfo);

  free(UnixFilename);

  if (err)
    err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsReadFile(LPCWSTR Filename,
					 LPVOID Buffer,
					 DWORD NumberOfBytesToRead,
					 LPDWORD NumberOfBytesRead,
					 LONGLONG Offset,
					 PDOKAN_FILE_INFO DokanFileInfo)
{
  int err;
  char* UnixFilename;
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (NumberOfBytesRead != NULL)
    *NumberOfBytesRead = 0;

  if (gFuseOps->read == NULL)
    return -ERROR_INVALID_FUNCTION;

  if (DokanFileInfo->IsDirectory)
    return -ERROR_NOT_SUPPORTED;

  if ((Offset < INT_MIN) || (Offset >= INT_MAX))
    return -ERROR_INVALID_PARAMETER;

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename != NULL)
    {
      err = gFuseOps->read(UnixFilename,
			   Buffer,
			   NumberOfBytesToRead,
			   (off_t)Offset,
			   FuseFileInfo);
      free(UnixFilename);

      if (err >= 0)
	{
	  if (NumberOfBytesRead != NULL)
	    *NumberOfBytesRead = err;
	  err = 0;
	}
      else
	{
	  err = -errno_to_winerror(-err);
	}

    }
  else
    {
      err = -ERROR_OUTOFMEMORY;
    }

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsWriteFile(LPCWSTR Filename,
					  LPCVOID Buffer,
					  DWORD NumberOfBytesToWrite,
					  LPDWORD NumberOfBytesWritten,
					  LONGLONG Offset,
					  PDOKAN_FILE_INFO DokanFileInfo)
{
  int err;
  char* UnixFilename;
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (NumberOfBytesWritten != NULL)
    *NumberOfBytesWritten = 0;

  if (gFuseOps->write == NULL)
    return -ERROR_INVALID_FUNCTION;

  if (DokanFileInfo->IsDirectory)
    return -ERROR_NOT_SUPPORTED;

  if ((Offset < INT_MIN) || (Offset >= INT_MAX))
    return -ERROR_INVALID_PARAMETER;

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename != NULL)
    {
      err = gFuseOps->write(UnixFilename,
			    Buffer,
			    NumberOfBytesToWrite,
			    (off_t)Offset,
			    FuseFileInfo);
      free(UnixFilename);

      if (err >= 0)
	{
	  if (NumberOfBytesWritten != NULL)
	    *NumberOfBytesWritten = err;
	  err = 0;
	}
      else
	{
	  err = -errno_to_winerror(-err);
	}

    }
  else
    {
      err = -ERROR_OUTOFMEMORY;
    }

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsFlushFileBuffers(LPCWSTR Filename,
						 PDOKAN_FILE_INFO DokanFileInfo)
{
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;
  char* UnixFilename;
  int (*fsync_fn)(const char*, int, struct fuse_file_info*);
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (DokanFileInfo->IsDirectory == FALSE)
    fsync_fn = gFuseOps->fsync;
  else
    fsync_fn = gFuseOps->fsyncdir;

  if (fsync_fn == NULL)
    return -ERROR_INVALID_FUNCTION;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  err = fsync_fn(UnixFilename, 0, FuseFileInfo);

  free(UnixFilename);

  err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsGetFileInformation(LPCWSTR Filename,
						   LPBY_HANDLE_FILE_INFORMATION Buffer,
						   PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixFilename;
  int err;
  struct stat stbuf;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (Buffer != NULL)
    memset(Buffer, 0, sizeof(*Buffer));

  if (gFuseOps->getattr == NULL)
    return -ERROR_INVALID_FUNCTION;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  err = gFuseOps->getattr(UnixFilename, &stbuf);

  free(UnixFilename);

  if (!err)
    {
      if (Buffer != NULL)
	{
	  if (stbuf.st_mode & _S_IFDIR)
	    {
	      Buffer->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
	    }
	  else
	    {
	      if (stbuf.st_mode & (_S_IFCHR | _S_IFIFO))
		Buffer->dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
	      else
		Buffer->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	    }

	  timet_to_filetime(stbuf.st_ctime, &Buffer->ftCreationTime);
      	  timet_to_filetime(stbuf.st_atime, &Buffer->ftLastAccessTime);
	  timet_to_filetime(stbuf.st_mtime, &Buffer->ftLastWriteTime);
	  Buffer->dwVolumeSerialNumber = stbuf.st_dev;
	  Buffer->nFileSizeLow = stbuf.st_size;
	  Buffer->nNumberOfLinks = stbuf.st_nlink;
	  Buffer->nFileIndexLow = stbuf.st_ino;
	}
    }
  else
    {
      err = -errno_to_winerror(-err);
    }

  DEBUG_LEAVE();

  return err;
}


struct filldir_context
{
  PFillFindData FillFindData;
  PDOKAN_FILE_INFO DokanFileInfo;
  int err;
};

static int fill_directory(void* p,
			  const char* name,
			  const struct stat* stbuf,
			  off_t off)
{
  WIN32_FIND_DATAW data;
  struct filldir_context* context;
  wchar_t* tmp;
  int res;
  size_t len;

  context = p;

  if (off)
    {
      /* not supported
       */
      
      return -ERROR_INVALID_FUNCTION;
    }

  DEBUG_PRINTF("name: %s\n", name);

  tmp = unix_to_nt_filename(name, &len);
  if (tmp == NULL)
    return -ERROR_OUTOFMEMORY;

  memset(&data, 0, sizeof(data));

  data.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

  if (stbuf != NULL)
    {
      if (stbuf->st_mode & _S_IFDIR)
	data.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      else if (stbuf->st_mode & (_S_IFCHR | _S_IFIFO))
	data.dwFileAttributes = FILE_ATTRIBUTE_SYSTEM;

      timet_to_filetime(stbuf->st_ctime, &data.ftCreationTime);
      timet_to_filetime(stbuf->st_mtime, &data.ftLastWriteTime);
      timet_to_filetime(stbuf->st_atime, &data.ftLastAccessTime);

      data.nFileSizeLow = stbuf->st_size;
    }

  if (len < MAX_PATH)
    wcscpy(data.cFileName, tmp);

  free(tmp);

  res = context->FillFindData(&data, context->DokanFileInfo);

  return 0;
}


static int DOKAN_CALLBACK FusefsFindFiles(LPCWSTR Pathname,
					  PFillFindData FillFindData,
					  PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixPathname;
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;
  struct filldir_context context;
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Pathname);

  if (DokanFileInfo->IsDirectory == FALSE)
    return -ERROR_NOT_SUPPORTED;

  if ((gFuseOps->readdir == NULL) && (gFuseOps->getdir == NULL))
    return -ERROR_INVALID_FUNCTION;

  UnixPathname = nt_to_unix_filename(Pathname);
  if (UnixPathname == NULL)
    return -ERROR_OUTOFMEMORY;

  FileContext = (PFUSEFS_FILE_CONTEXT)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  context.FillFindData = FillFindData;
  context.DokanFileInfo = DokanFileInfo;
  context.err = 0;

  if (gFuseOps->readdir != NULL)
    {
      err = gFuseOps->readdir(UnixPathname,
			      &context,
			      fill_directory,
			      0,
			      FuseFileInfo);
    }
  else
    {
      /* todo: use getdir
       */

      err = -ENOSYS;
    }

  free(UnixPathname);

  if (!err)
    err = context.err;
  else
    err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsFindFilesWithPattern(LPCWSTR Pathname,
						     LPCWSTR SearchPattern,
						     PFillFindData FillFindData,
						     PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Pathname);

  DEBUG_LEAVE();

  return -ERROR_INVALID_FUNCTION;
}



static int DOKAN_CALLBACK FusefsSetFileAttributes(LPCWSTR Filename,
						  DWORD FileAttributes,
						  PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static int DOKAN_CALLBACK FusefsSetFileTime(LPCWSTR Filename,
					    CONST FILETIME* CreationTime,
					    CONST FILETIME* LastAccessTime,
					    CONST FILETIME* LastWriteTime,
					    PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixFilename;
  int err;
  struct timespec tv[2];
  struct utimbuf utimbuf;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if ((gFuseOps->utimens == NULL) && (gFuseOps->utime == NULL))
    return -ERROR_INVALID_FUNCTION;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  if (gFuseOps->utimens != NULL)
    {
      filetime_to_timespec(LastAccessTime, &tv[0]);
      filetime_to_timespec(LastWriteTime, &tv[1]);
      err = gFuseOps->utimens(UnixFilename, tv);
    }
  else if (gFuseOps->utime != NULL)
    {
      filetime_to_timet(LastAccessTime, &utimbuf.actime);
      filetime_to_timet(LastWriteTime, &utimbuf.modtime);
      err = gFuseOps->utime(UnixFilename, &utimbuf);
    }

  err = -errno_to_winerror(-err);

  free(UnixFilename);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsDeleteFile(LPCWSTR Filename,
					   PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixFilename;
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (DokanFileInfo->IsDirectory == FALSE)
    return -ERROR_NOT_SUPPORTED;

  if (gFuseOps->unlink == NULL)
    return -ERROR_INVALID_FUNCTION;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  err = gFuseOps->unlink(UnixFilename);

  free(UnixFilename);

  err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsDeleteDirectory(LPCWSTR Pathname,
						PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixPathname;
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Pathname);

  if (DokanFileInfo->IsDirectory == FALSE)
    return -ERROR_NOT_SUPPORTED;

  if (gFuseOps->rmdir == NULL)
    return -ERROR_INVALID_FUNCTION;

  UnixPathname = nt_to_unix_filename(Pathname);
  if (UnixPathname == NULL)
    return -ERROR_OUTOFMEMORY;

  err = gFuseOps->rmdir(UnixPathname);

  free(UnixPathname);

  err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsMoveFile(LPCWSTR Filename,
					 LPCWSTR NewFilename,
					 BOOL ReplaceExisiting,
					 PDOKAN_FILE_INFO DokanFileInfo)
{
  int err;
  char* UnixFilename;
  char* NewUnixFilename;

  DEBUG_ENTER();

  DEBUG_PRINTF("%ws %ws %d\n", Filename, NewFilename, ReplaceExisiting);

  if (gFuseOps->rename == NULL)
    return -ERROR_INVALID_FUNCTION;

  if (!ReplaceExisiting)
    return -ERROR_NOT_SUPPORTED;

  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  NewUnixFilename = nt_to_unix_filename(NewFilename);
  if (NewUnixFilename == NULL)
    {
      free(UnixFilename);
      return -ERROR_OUTOFMEMORY;
    }

  if (DokanFileInfo->IsDirectory)
    err = gFuseOps->rename(UnixFilename, NewUnixFilename);

  free(UnixFilename);
  free(NewUnixFilename);

  err = -errno_to_winerror(-err);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsSetEndOfFile(LPCWSTR Filename,
					     LONGLONG Length,
					     PDOKAN_FILE_INFO DokanFileInfo)
{
  char* UnixFilename;
  PFUSEFS_FILE_CONTEXT FileContext;
  struct fuse_file_info* FuseFileInfo;
  int err;

  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  if (DokanFileInfo->IsDirectory)
    return -ERROR_NOT_SUPPORTED;

  if ((Length < 0) || (Length > INT_MAX))
    return -ERROR_INVALID_PARAMETER;
  
  UnixFilename = nt_to_unix_filename(Filename);
  if (UnixFilename == NULL)
    return -ERROR_OUTOFMEMORY;

  FileContext = (void*)DokanFileInfo->Context;
  if (FileContext != NULL)
    FuseFileInfo = &FileContext->FuseFileInfo;
  else
    FuseFileInfo = NULL;

  if (FuseFileInfo == NULL)
    {
      /* not opened, use truncate
       */

      if (gFuseOps->truncate != NULL)
	{
	  err = gFuseOps->truncate(UnixFilename, (off_t)Length);
	  err = -errno_to_winerror(err);
	}
      else
	{
	  err = -ERROR_INVALID_FUNCTION;
	}
    }
  else
    {
      /* opened, use ftruncate
       */

      if (gFuseOps->ftruncate != NULL)
	{
	  err = gFuseOps->ftruncate(UnixFilename, (off_t)Length, FuseFileInfo);
	  err = -errno_to_winerror(err);
	}
      else
	{
	  err = -ERROR_INVALID_FUNCTION;
	}
    }

  free(UnixFilename);

  DEBUG_LEAVE();

  return err;
}



static int DOKAN_CALLBACK FusefsLockFile(LPCWSTR Filename,
					 LONGLONG ByteOffset,
					 LONGLONG Length,
					 PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static int DOKAN_CALLBACK FusefsUnlockFile(LPCWSTR Filename,
					   LONGLONG ByteOffset,
					   LONGLONG Length,
					   PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_PRINTF("Filename: %ws\n", Filename);

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static int DOKAN_CALLBACK FusefsGetDiskFreeSpace(PULONGLONG FreeBytesAvailable,
						 PULONGLONG TotalNumberOfBytes,
						 PULONGLONG TotalNumberOfFreeBytes,
						 PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static int DOKAN_CALLBACK FusefsGetVolumeInformation(LPWSTR VolumeNameBuffer,
						     DWORD VolumeNameSize,
						     LPDWORD VolumeSerialNumber,
						     LPDWORD MaximumComponentLength,
						     LPDWORD FileSystemFlags,
						     LPWSTR FileSystemNameBuffer,
						     DWORD FileSystemNameSize,
						     PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static int DOKAN_CALLBACK FusefsUnmount(PDOKAN_FILE_INFO DokanFileInfo)
{
  DEBUG_ENTER();

  DEBUG_LEAVE();

  return ERROR_SUCCESS;
}



static DOKAN_OPERATIONS gFusefsDokanOps =
  {
    FusefsCreateFile,
    FusefsOpenDirectory,
    FusefsCreateDirectory,
    FusefsCleanup,
    FusefsCloseFile,
    FusefsReadFile,
    FusefsWriteFile,
    FusefsFlushFileBuffers,
    FusefsGetFileInformation,
    FusefsFindFiles,
    NULL, /* FusefsFindFilesWithPattern */
    FusefsSetFileAttributes,
    FusefsSetFileTime,
    FusefsDeleteFile,
    FusefsDeleteDirectory,
    FusefsMoveFile,
    FusefsSetEndOfFile,
    FusefsLockFile,
    FusefsUnlockFile,
    NULL, /* FusefsGetDiskFreeSpace */
    NULL, /* FusefsGetVolumeInformation */
    FusefsUnmount
  };



/* fuse_main_real
 */

int fuse_main_real(int argc,
		   char *argv[],
		   const struct fuse_operations *ops,
		   size_t op_size,
		   void *user_data)
{
  DOKAN_OPTIONS DokanOptions;
  int DokanResult;
  void* context;

  if (ops == NULL)
    return 1;

  gFuseOps = ops;
  gFuseUserData = user_data;

  memset(&DokanOptions, 0, sizeof(DokanOptions));
  DokanOptions.DriveLetter = (wchar_t)argv[3][0];
  DokanOptions.ThreadCount = 1;
  DokanOptions.DebugMode = 1;

  if (ops->init != NULL)
    context = ops->init(NULL);

  DokanResult = DokanMain(&DokanOptions, &gFusefsDokanOps);

  if (ops->destroy != NULL)
    ops->destroy(context);

  if (DokanResult != DOKAN_SUCCESS)
    {
      DEBUG_ERROR("DokanMain() == 0x%08x\n", DokanResult);
      return 1;
    }

  return 0;
}
