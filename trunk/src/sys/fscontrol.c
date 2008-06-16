/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 21:21:18 2008 texane
** Last update Mon Jun 16 22:39:27 2008 texane
*/



#include <ntifs.h>
#include "debug.h"



static NTSTATUS WinfuseFsctlDismountVolume(PIO_STACK_LOCATION IrpSp)
{
  DEBUG_ENTER();

  return STATUS_SUCCESS;
}


static NTSTATUS WinfuseFsRequest(PIO_STACK_LOCATION IrpSp)
{
  NTSTATUS Status;
  ULONG FsControlCode;
  
  FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

  switch (FsControlCode)
    {
    case FSCTL_DISMOUNT_VOLUME:
      Status = WinfuseFsctlDismountVolume(IrpSp);
      break;

    default:
      DEBUG_ERROR("FsControlCode == 0x%x\n", FsControlCode);
      Status = STATUS_SUCCESS;
      break;
    }

  return Status;
}



static NTSTATUS WinfuseMountVolume(PIO_STACK_LOCATION IrpSp)
{
  PDEVICE_OBJECT Device;
  PVPB Vpb;

  Device = IrpSp->Parameters.MountVolume.DeviceObject;
  Vpb = IrpSp->Parameters.MountVolume.Vpb;

  return STATUS_UNSUCCESSFUL;
}



NTSTATUS WinfuseFsControl(PDEVICE_OBJECT Device,
			  PIRP Irp,
			  PIO_STACK_LOCATION IrpSp)
{
  /* if the disk device object is not for us,
     should return STATUS_UNRECOGNIZED_VOLUME
   */

  NTSTATUS Status;

  DEBUG_ENTER();

  switch (IrpSp->MinorFunction)
    {
    case IRP_MN_USER_FS_REQUEST:
      Status = WinfuseFsRequest(IrpSp);
      break;

    case IRP_MN_MOUNT_VOLUME:
      Status = WinfuseMountVolume(IrpSp);
      break;

    default:
      DEBUG_ERROR("MinorFunction == 0x%08x\n", IrpSp->MinorFunction);
      Status = STATUS_UNSUCCESSFUL;
      break;
    }

  return Status;
}

