/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 21:38:20 2008 texane
** Last update Mon Jun 16 21:38:58 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



NTSTATUS WinfuseCleanup(PDEVICE_OBJECT Device,
			PIRP Irp,
			PIO_STACK_LOCATION IrpSp)
{
  DEBUG_ENTER();

  return STATUS_SUCCESS;
}
