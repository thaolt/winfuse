/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 21:09:55 2008 texane
** Last update Tue Jun 17 22:07:48 2008 rno
*/



#include <ntifs.h>
#include "debug.h"
#include "fs.h"



static PDEVICE_OBJECT gWinfuseDiskFsCDO = NULL;



extern NTSTATUS WinfuseCreate(PDEVICE_OBJECT, PIRP, PIO_STACK_LOCATION);
extern NTSTATUS WinfuseClose(PDEVICE_OBJECT, PIRP, PIO_STACK_LOCATION);
extern NTSTATUS WinfuseCleanup(PDEVICE_OBJECT, PIRP, PIO_STACK_LOCATION);
extern NTSTATUS WinfuseFsControl(PDEVICE_OBJECT, PIRP, PIO_STACK_LOCATION);
extern NTSTATUS WinfuseDirectoryControl(PDEVICE_OBJECT, PIRP, PIO_STACK_LOCATION);



static NTSTATUS DispatchIrp(PDEVICE_OBJECT Device, PIRP Irp)
{
  PIO_STACK_LOCATION IrpSp;
  NTSTATUS Status;

  DEBUG_ENTER();

  IrpSp = IoGetCurrentIrpStackLocation(Irp);

  switch (IrpSp->MajorFunction)
    {
    case IRP_MJ_CREATE:
      Status = WinfuseCreate(Device, Irp, IrpSp);
      break;

    case IRP_MJ_CLOSE:
      Status = WinfuseClose(Device, Irp, IrpSp);
      break;

    case IRP_MJ_CLEANUP:
      Status = WinfuseCleanup(Device, Irp, IrpSp);
      break;

    case IRP_MJ_FILE_SYSTEM_CONTROL:
      Status = WinfuseFsControl(Device, Irp, IrpSp);
      break;

    case IRP_MJ_DIRECTORY_CONTROL:
      Status = WinfuseDirectoryControl(Device, Irp, IrpSp);
      break;

    default:
      DEBUG_ERROR("MajorFunction == 0x%08x\n", IrpSp->MajorFunction);
      Status = STATUS_UNSUCCESSFUL;
      break;
    }

  /* complete the irp
   */

  Irp->IoStatus.Status = Status;
  Irp->IoStatus.Information = 0;

  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return Status;
}



NTSTATUS WinfuseCreateFs(PDRIVER_OBJECT Driver)
{
  NTSTATUS Status;
  PDEVICE_OBJECT Device;
  static UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\WinfuseDiskFsCDO");
  ULONG i;

  Status = IoCreateDevice(Driver, 0, &DeviceName, FILE_DEVICE_DISK_FILE_SYSTEM, 0, FALSE, &Device);
  if (Status != STATUS_SUCCESS)
    {
      DEBUG_ERROR("IoCreateDevice() == 0x%08x\n", Status);
      return Status;
    }

  for (i = 0; i < sizeof(Driver->MajorFunction) / sizeof(Driver->MajorFunction[0]); ++i)
    Driver->MajorFunction[i] = DispatchIrp;

  IoRegisterFileSystem(Device);

  gWinfuseDiskFsCDO = Device;

  return STATUS_SUCCESS;
}



void WinfuseDestroyFs(void)
{
  if (gWinfuseDiskFsCDO != NULL)
    {
      IoUnregisterFileSystem(gWinfuseDiskFsCDO);
      IoDeleteDevice(gWinfuseDiskFsCDO);
      gWinfuseDiskFsCDO = NULL;
    }
}
