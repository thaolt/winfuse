/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 21:21:18 2008 texane
** Last update Wed Jun 18 00:11:45 2008 texane
*/



#include <ntifs.h>
#include "disk.h"
#include "debug.h"



/* globals
 */

extern PDRIVER_OBJECT gWinfuseDriver;



static NTSTATUS WinfuseFsctlDismountVolume(PIO_STACK_LOCATION IrpSp)
{
  DEBUG_ENTER();

#if 0
  KIRQL Irql;
  PVPB Vpb;

  Vpb = IrpSp->Parameters.MountVolume.Vpb;

  IoAcquireVpbSpinLock(&Irql);

  Vpb->Flags &= ~VPB_MOUNTED;

  IoReleaseVpbSpinLock(Irql);
#endif

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
  NTSTATUS Status;
  PDEVICE_OBJECT WinfuseDiskDevice;
  PDEVICE_OBJECT TargetDevice;
  PVPB Vpb;
  PDEVICE_OBJECT Vdo;

  DEBUG_ENTER();

  WinfuseDiskDevice = WinfuseReferenceDisk();

  if (WinfuseReferenceDisk == NULL)
    return STATUS_UNSUCCESSFUL;

  TargetDevice = IrpSp->Parameters.MountVolume.DeviceObject;
  Vpb = IrpSp->Parameters.MountVolume.Vpb;

  if (TargetDevice == WinfuseDiskDevice)
    {
      DEBUG_PRINTF("TargetDevice == WinfuseDiskDevice\n");

      Status = IoCreateDevice(gWinfuseDriver,
			      0,
			      NULL,
			      FILE_DEVICE_DISK_FILE_SYSTEM,
			      0,
			      FALSE,
			      &Vdo);

      if (Status != STATUS_SUCCESS)
	{
	  DEBUG_ERROR("IoCreateDevice() == 0x%08x\n", Status);
	  return Status;
	}

      Vdo->Flags &= ~DO_DEVICE_INITIALIZING;

      Vpb->DeviceObject = Vdo;

      Status = STATUS_SUCCESS;
    }
  else
    {
      DEBUG_ERROR("invalid device to mount\n");
      Status = STATUS_UNRECOGNIZED_VOLUME;
    }

  WinfuseDereferenceDisk(WinfuseDiskDevice);

  return Status;
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
      DEBUG_PRINTF("Returning STATUS_FS_DRIVER_REQUIRED\n");
      Status = STATUS_FS_DRIVER_REQUIRED;

    case IRP_MN_LOAD_FILE_SYSTEM:
      Status = WinfuseMountVolume(IrpSp);
      break;

    default:
      DEBUG_ERROR("MinorFunction == 0x%08x\n", IrpSp->MinorFunction);
      Status = STATUS_UNSUCCESSFUL;
      break;
    }

  return Status;
}

