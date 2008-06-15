/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sun Jun 15 02:35:30 2008 texane
** Last update Sun Jun 15 02:48:23 2008 texane
*/



#include <ntddk.h>
#include "unit.h"



int UnitGetAv(PUNICODE_STRING path, int* ac, char*** av)
{
#if 0
  OBJECT_ATTRIBUTES Attributes;

  *av = NULL;
  *ac = 0;

  InitializeObjectAttributes();

  /* retrieve from registry
   */

  ZwOpenKey();
#endif

  return -1;
}



void UnitFreeAv(char** av)
{
}
