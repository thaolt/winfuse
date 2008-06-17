


#include <ntddk.h>
#include "debug.h"



NTSTATUS WinfuseDirectoryControl(PDEVICE_OBJECT Device,
				 PIRP Irp,
				 PIO_STACK_LOCATION IrpSp)
{
  DEBUG_ENTER();

  return STATUS_SUCCESS;
}
