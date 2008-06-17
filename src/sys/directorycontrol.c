/*
** Made by texane <texane@gmail.com>
** 
** Started on  Tue Jun 17 22:55:46 2008 rno
** Last update Wed Jun 18 00:36:34 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



static NTSTATUS WinfuseNotifyDirectory(PIRP Irp,
				       PIO_STACK_LOCATION Irpsp)
{
  DEBUG_ENTER();

  return STATUS_SUCCESS;
}



static NTSTATUS WinfuseQueryDirectory(PIRP Irp,
				      PIO_STACK_LOCATION IrpSp)
{
  NTSTATUS Status;

  DEBUG_ENTER();

  DEBUG_PRINTF("UserBuffer: 0x%08x\n", Irp->UserBuffer);

  __try
    {
      ProbeForWrite(Irp->UserBuffer,
		    IrpSp->Parameters.QueryDirectory.Length,
		    1);

      RtlZeroMemory(Irp->UserBuffer,
		    IrpSp->Parameters.QueryDirectory.Length);
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
      return STATUS_UNSUCCESSFUL;
    }

  switch (IrpSp->Parameters.QueryDirectory.FileInformationClass)
    {
      /* http://msdn.microsoft.com/en-us/library/ms795825.aspx
       */

    case FileBothDirectoryInformation:
      {
	/* FILE_BOTH_DIR_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    case FileDirectoryInformation:
      {
 	/* FILE_DIRECTORY_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    case FileFullDirectoryInformation:
      {
	/* FILE_FULL_DIR_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    case FileIdBothDirectoryInformation:
      {
	/* FILE_ID_BOTH_DIR_INFORMATION
	 */
	
	Status = STATUS_SUCCESS;

	break;
      }

    case FileIdFullDirectoryInformation:
      {
	/* FILE_ID_FULL_DIR_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    case FileNamesInformation:
      {
	/* FILE_NAMES_INFORMATION
	 */
	  
	Status = STATUS_SUCCESS;

	break;
      }
 	
    case FileObjectIdInformation:
      {
 	/* FILE_OBJECTID_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    case FileQuotaInformation:
      {
	/* obsolete, IRP_MJ_QUERY_QUOTA
	 */

	Status = STATUS_UNSUCCESSFUL;

	break;
      }

    case FileReparsePointInformation:
      {
	/* FILE_REPARSE_POINT_INFORMATION
	 */

	Status = STATUS_SUCCESS;

	break;
      }

    default:
      {
	DEBUG_ERROR("FileInformationClass == 0x%x\n",
		    IrpSp->Parameters.QueryDirectory.FileInformationClass);

	Status = STATUS_UNSUCCESSFUL;

	break;
      }
    }

  return Status;
}



NTSTATUS WinfuseDirectoryControl(PDEVICE_OBJECT Device,
				 PIRP Irp,
				 PIO_STACK_LOCATION IrpSp)
{
  NTSTATUS Status;

  DEBUG_ENTER();

  switch (IrpSp->MinorFunction)
    {
    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
      Status = WinfuseNotifyDirectory(Irp, IrpSp);
      break;

    case IRP_MN_QUERY_DIRECTORY:
      Status = WinfuseQueryDirectory(Irp, IrpSp);
      break;

    default:
      DEBUG_PRINTF("Irp->MinorFunction == 0x%x\n", IrpSp->MinorFunction);
      Status = STATUS_UNSUCCESSFUL;
      break;
    }

  return Status;
}
