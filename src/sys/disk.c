/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 20:47:24 2008 texane
** Last update Mon Jun 16 23:21:44 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



static PDEVICE_OBJECT gWinfuseDevice = NULL;



static UNICODE_STRING gWinfuseLinkName = RTL_CONSTANT_STRING(L"\\DosDevices\\L:");



NTSTATUS WinfuseCreateDisk(PDRIVER_OBJECT Driver)
{
  NTSTATUS Status;
  PDEVICE_OBJECT Device;
  static UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\WinfuseDisk0");

  Status = IoCreateDevice(Driver, 0, &DeviceName, FILE_DEVICE_VIRTUAL_DISK, 0, FALSE, &Device);
  if (Status != STATUS_SUCCESS)
    {
      DEBUG_ERROR("IoCreateDevice() == 0x%08x\n", Status);
      return Status;
    }

  Device->Flags |= DO_DIRECT_IO;

  Status = IoCreateSymbolicLink(&gWinfuseLinkName, &DeviceName);
  if (Status != STATUS_SUCCESS)
    {
      DEBUG_ERROR("IoCreateDevice() == 0x%08x\n", Status);
      IoDeleteDevice(Device);
      return Status;
    }

  gWinfuseDevice = Device;

  return STATUS_SUCCESS;
}



void WinfuseDestroyDisk(void)
{
  if (gWinfuseDevice != NULL)
    {
      IoDeleteSymbolicLink(&gWinfuseLinkName);
      IoDeleteDevice(gWinfuseDevice);
      gWinfuseDevice = NULL;
    }
}



PDEVICE_OBJECT WinfuseReferenceDisk(void)
{
  if (gWinfuseDevice != NULL)
    ObReferenceObject(gWinfuseDevice);

  return gWinfuseDevice;
}



void WinfuseDereferenceDisk(PDEVICE_OBJECT Device)
{
  ObDereferenceObject(Device);
}
