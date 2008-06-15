/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:28:24 2008 texane
** Last update Sun Jun 15 02:47:03 2008 texane
*/



#include <ntddk.h>
#include "debug.h"



/* unit
 */

#if _UNIT


#include "unit.h"


static void main(int ac, char** av)
{
  int i;

  for (i = 0; av[i] != NULL; ++i)
    DbgPrint("%s\n", av[i]);
}


static void unit_main(PUNICODE_STRING path)
{
  int ac;
  char** av;

  if (UnitGetAv(path, &ac, &av) != -1)
    {
      main(ac, av);
      UnitFreeAv(av);
    }
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
