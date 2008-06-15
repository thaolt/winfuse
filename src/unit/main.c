/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:43:20 2008 texane
** Last update Sun Jun 15 11:30:50 2008 texane
*/



#include <windows.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>



static _TCHAR* GetFullPath(const _TCHAR* name)
{
  DWORD len;
  static _TCHAR path[MAX_PATH];

  if ((name[1] == _T(':')) && (name[2] == _T('\\')))
    {
      _tcscpy(path, name);
      return path;
    }

  len = GetCurrentDirectory(sizeof(path), path);
  if (len == 0)
    return NULL;

  path[len++] = _T('\\');

  _tcscpy(path + len, name);

  return path;
}



static _TCHAR* GetServiceName(const _TCHAR* path)
{
  size_t len;
  size_t dot;
  size_t sep;
  static _TCHAR buf[32];

  len = _tcslen(path);
  if (!len)
    return NULL;

  while (len && (path[len] != _T('.')))
    --len;

  if (!len)
    return NULL;

  dot = len;

  while (len && (path[len] != _T('\\')))
    --len;

  if (!len)
    return NULL;

  sep = len;

  len = dot - sep - 1;
  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;

  memcpy(buf, path + sep + 1, len);
  buf[len] = 0;

  return buf;
}



static void* AvToMultiRegSz(const _TCHAR** av, DWORD* length)
{
  int i;
  int j;
  int len;
  unsigned char* buf;

  *length = 0;
  
  len = 0;

  for (i = 0; av[i]; ++i)
    len += _tcslen(av[i]) + sizeof(_TCHAR);

  len += sizeof(_TCHAR);

  buf = malloc(len);
  if (buf == NULL)
    return NULL;

  *length = len;

  j = 0;

  for (i = 0; av[i]; ++i)
    {
      len = _tcslen(av[i]) + 1;
      memcpy(buf + j, av[i], len);
      j += len;
    }

  *(_TCHAR*)(buf + j) = 0;

  return buf;
}



#if 0 /* VISTA */

static VOID CALLBACK onServiceChange(IN PVOID pParameter)
{
  SERVICE_NOTIFY* ServiceNotify;
  HANDLE EventHandle;

  ServiceNotify = pParameter;

  EventHandle = *(HANDLE*)ServiceNotify->pContext;

  SetEvent(EventHandle);
}

#endif /* VISTA */



struct _THREAD_PARAM
{
  HANDLE InputEventHandle;
};


static DWORD WINAPI InputThreadRoutine(LPVOID p)
{
  struct _THREAD_PARAM* Param;

  Param = p;

  printf("press any key\n");

  getchar();

  printf("key pressed\n");

  SetEvent(Param->InputEventHandle);

  return 0;
}



int main(int ac, _TCHAR** av)
{
  int error;
  struct _THREAD_PARAM InputThreadParam;
  HANDLE InputEventHandle;
  HANDLE InputThreadHandle;
  SC_HANDLE ManagerHandle;
  SC_HANDLE ServiceHandle;
  _TCHAR* DriverPath;
  _TCHAR* ServiceName;
  HKEY ServiceKey;
  void* Cmdline;
  DWORD CmdlineLength;
  SERVICE_STATUS ServiceStatus;
  BOOLEAN isDone;
  DWORD res;
  _TCHAR KeyName[1024];

  if (ac == 1)
    {
      printf("%s: <driver> <avs>\n", av[0]);
      return -1;
    }

  DriverPath = GetFullPath(av[1]);
  if (DriverPath == NULL)
    {
      printf("GetFullPath() == NULL\n");
      return -1;
    }

  ServiceName = GetServiceName(DriverPath);
  if (ServiceName == NULL)
    {
      printf("GetServiceName() == NULL\n");
      return -1;
    }

  Cmdline = AvToMultiRegSz(av + 2, &CmdlineLength);
  if (Cmdline == NULL)
    {
      printf("AvToMultiRegSz() == NULL\n");
      return -1;
    }

  error = -1;

  InputEventHandle = NULL;
  InputThreadHandle = NULL;
  ServiceKey = NULL;
  ManagerHandle = NULL;
  ServiceHandle = NULL;

  ManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (ManagerHandle == NULL)
    {
      printf("OpenSCManager() == %u\n", GetLastError());
      goto Cleanup;
    }

  /* remove the serice if already exist
   */

  ServiceHandle = OpenService(ManagerHandle,
			      ServiceName,
			      SC_MANAGER_ALL_ACCESS);
  if (ServiceHandle != NULL)
    {
      if (DeleteService(ServiceHandle) == FALSE)
	{
	  printf("DeleteService() == %u\n", GetLastError());
	  goto Cleanup;
	}

      ServiceHandle = NULL;
    }

  /* create the service
   */

  ServiceHandle = CreateService(ManagerHandle,
				ServiceName,
				ServiceName,
				SC_MANAGER_ALL_ACCESS,
				SERVICE_KERNEL_DRIVER,
				SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE,
				DriverPath,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL);
  if (ServiceHandle == NULL)
    {
      printf("CreateService() == %u\n", GetLastError());
      goto Cleanup;
    }

  /* set registry
   */

  _tcscpy(KeyName, _T("SYSTEM\\CurrentControlSet\\Services\\"));
  _tcscat(KeyName, ServiceName);

  if (RegOpenKey(HKEY_LOCAL_MACHINE,
		 KeyName,
		 &ServiceKey) != ERROR_SUCCESS)
    {
      printf("RegOpenKey() == %u\n", GetLastError());
      goto Cleanup;
    }

  if (RegSetValueEx(ServiceKey,
		    _T("Cmdline"),
		    0,
		    REG_MULTI_SZ,
		    Cmdline,
		    CmdlineLength) != ERROR_SUCCESS)
    {
      printf("RegSetKeyValue() == %u\n", GetLastError());
      goto Cleanup;
    }

  /* input thread
   */

  InputEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (InputEventHandle == NULL)
    {
      printf("CreateEvent() == %u\n", GetLastError());
      goto Cleanup;
    }

  InputThreadParam.InputEventHandle = InputEventHandle;
  InputThreadHandle = CreateThread(NULL,
				   0,
				   InputThreadRoutine,
				   &InputThreadParam,
				   0,
				   NULL);
  if (InputThreadHandle == NULL)
    {
      printf("CreateThread() == %u\n", GetLastError());
      goto Cleanup;
    }

  /* start the service
   */

  if (StartService(ServiceHandle, 0, NULL) == FALSE)
    {
      printf("StartService() == %u\n", GetLastError());
      goto Cleanup;
    }

#if 0 /* VISTA */
  {
    EventHandle = CreateHandle();
    if (EventHandle)
      {
	printf("CreateEvent() == %u\n", GetLastError());
	goto Cleanup;
      }

    memset(&ServiceNotify, 0, sizeof(ServiceNotify));
    ServiceNotify.dwVersion = SERVICE_NOTIFY_STATUS_CHANGE;
    ServiceNotify.pfnNotifyCallback = onServiceChange;
    ServiceNotify.pContext = &EventHandle;

    NotifyServiceStatusChange(ServiceHandle,
			      SERVICE_NOTIFY_STOPPED,
			      &ServiceNotify);

    WaitForSingleObject(EventHandle, INFINITE);
  }
#else /* ! VISTA */
  {

    /* wait for the service to be stopped
     */

    isDone = FALSE;

    while (isDone == FALSE)
      {
	res = WaitForSingleObject(InputEventHandle, 1000);

	if (res == WAIT_OBJECT_0)
	  ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);

	if (QueryServiceStatus(ServiceHandle,
			       &ServiceStatus) == FALSE)
	  {
	    printf("QueryServiceStatus() == %u\n", GetLastError());
	    ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
	    goto Cleanup;
	  }

	if (ServiceStatus.dwCurrentState == SERVICE_STOPPED)
	  {
	    if (ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR)
	      error = ServiceStatus.dwServiceSpecificExitCode;
	    else
	      error = ServiceStatus.dwWin32ExitCode;

	    isDone = TRUE;
	  }
      }
  }
#endif /* VISTA */

  /* success
   */
  
 Cleanup:

  if (InputThreadHandle != NULL)
    {
      TerminateThread(InputThreadHandle, 0);
      CloseHandle(InputThreadHandle);
    }
  
  if (InputEventHandle != NULL)
    CloseHandle(InputEventHandle);

  if (Cmdline != NULL)
    free(Cmdline);

  if (ServiceKey != NULL)
    RegCloseKey(ServiceKey);

  if (ServiceHandle != NULL)
    {
      if (DeleteService(ServiceHandle) == FALSE)
	printf("DeleteService() == %u\n", GetLastError());
      CloseServiceHandle(ServiceHandle);
    }

  if (ManagerHandle != NULL)
    CloseServiceHandle(ManagerHandle);

  return error;
}
