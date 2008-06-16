/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 21:21:18 2008 texane
** Last update Mon Jun 16 21:38:12 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



NTSTATUS WinfuseCreate(PDEVICE_OBJECT Device,
		       PIRP Irp,
		       PIO_STACK_LOCATION IrpSp)
{
  DEBUG_ENTER();

  return STATUS_SUCCESS;
}
