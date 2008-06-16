/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:28:24 2008 texane
** Last update Mon Jun 16 23:32:07 2008 texane
*/



#include <ntddk.h>
#include "fs.h"
#include "disk.h"
#include "debug.h"



PDRIVER_OBJECT gWinfuseDriver = NULL;



static void DriverUnload(IN PDRIVER_OBJECT Driver)
{
  DEBUG_ENTER();

  WinfuseDestroyFs();
  WinfuseDestroyDisk();

  DEBUG_LEAVE();
}


NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, PUNICODE_STRING Path)
{
  NTSTATUS Status;

  DEBUG_ENTER();

  RtlZeroMemory(Driver->MajorFunction, sizeof(Driver->MajorFunction));
  Driver->DriverUnload = DriverUnload;

  gWinfuseDriver = Driver;

  Status = WinfuseCreateDisk(Driver);
  if (Status != STATUS_SUCCESS)
    return Status;

  Status = WinfuseCreateFs(Driver);
  if (Status != STATUS_SUCCESS)
    {
      WinfuseDestroyDisk();
      return Status;
    }

  DEBUG_LEAVE();
  
  return STATUS_SUCCESS;
}
