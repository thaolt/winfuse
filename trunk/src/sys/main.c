/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:28:24 2008 texane
** Last update Sun Jun 15 11:55:26 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



/* unit
 */

#if _UNIT


#include "unit.h"


static void unit_main(PUNICODE_STRING path)
{
  int i;
  int ac;
  WCHAR** av;

  if (UnitGetAv(path, &ac, &av) == -1)
    return ;

  DbgPrint("ac == %d\n", ac);

  for (i = 0; i < ac; ++i)
    DbgPrint("[0x%08x] %ws\n", av[i], av[i]);

  UnitFreeAv(av);
}


#endif /* _UNIT */



/* driver
 */

static void DriverUnload(IN PDRIVER_OBJECT Driver)
{
  DEBUG_ENTER();

  DEBUG_LEAVE();
}


NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path)
{
  DEBUG_ENTER();

  RtlZeroMemory(driver->MajorFunction, sizeof(driver->MajorFunction));
  driver->DriverUnload = DriverUnload;

#if _UNIT
  unit_main(path);
#endif /* _UNIT */

  DEBUG_LEAVE();
  
  return STATUS_SUCCESS;
}
