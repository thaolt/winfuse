/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:28:24 2008 texane
** Last update Sat Jun 14 23:29:38 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



static void DriverUnload(IN PDRIVER_OBJECT Driver)
{
  DEBUG_ENTER();

  DEBUG_LEAVE();
}



NTSTATUS DriverEntry(PDRIVER_OBJECT Driver,
		     PUNICODE_STRING Path)
{
  DEBUG_ENTER();

  RtlZeroMemory(Driver->MajorFunction, sizeof(Driver->MajorFunction));
  Driver->DriverUnload = DriverUnload;

  DEBUG_LEAVE();
  
  return STATUS_SUCCESS;
}
