/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sun Jun 15 02:35:30 2008 texane
** Last update Sun Jun 15 11:51:37 2008 texane
*/



#include <ntddk.h>
#include "debug.h"
#include "unit.h"



/* memory allocation
 */

#define UNIT_POOL_TAG 0x554e4954


static FORCEINLINE PVOID UnitAlloc(ULONG Size)
{
  return ExAllocatePoolWithTag(NonPagedPool, Size, UNIT_POOL_TAG);
}


static FORCEINLINE void UnitFree(PVOID p)
{
  ExFreePoolWithTag(p, UNIT_POOL_TAG);
}



/* exported
 */

int UnitGetAv(PUNICODE_STRING Path, int* Ac, WCHAR*** Av)
{
  NTSTATUS Status;
  HANDLE KeyHandle;
  OBJECT_ATTRIBUTES Attributes;
  PKEY_VALUE_PARTIAL_INFORMATION Info;
  PUCHAR Buffer;
  PUCHAR Tmp;
  PCHAR Data;
  ULONG Length;
  PWCHAR Begin;
  ULONG Off;
  int Count;
  int error;
  static UNICODE_STRING ValueName = RTL_CONSTANT_STRING(L"Cmdline");

  *Av = NULL;
  *Ac = 0;

  error = -1;

  KeyHandle = NULL;
  Buffer = NULL;
  
  InitializeObjectAttributes(&Attributes,
			     Path,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwOpenKey(&KeyHandle, GENERIC_READ, &Attributes);
  if (Status != STATUS_SUCCESS)
    {
      DEBUG_ERROR("ZwOpenKey() == 0x%08x\n", Status);
      return -1;
    }

  Length = 256;
  Buffer = UnitAlloc(Length);
  if (Buffer == NULL)
    goto Cleanup;

  Status = ZwQueryValueKey(KeyHandle,
			   &ValueName,
			   KeyValuePartialInformation,
			   Buffer,
			   Length,
			   &Length);

  if ((Status == STATUS_BUFFER_OVERFLOW) ||
      (Status == STATUS_BUFFER_TOO_SMALL))
    {
      UnitFree(Buffer);
      Buffer = UnitAlloc(Length);
      if (Buffer == NULL)
	goto Cleanup;

      Status = ZwQueryValueKey(KeyHandle,
			       &ValueName,
			       KeyValuePartialInformation,
			       Buffer,
			       Length,
			       &Length);
    }

  Info = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;

  if (Status != STATUS_SUCCESS)
    {
      if (Status != STATUS_NOT_FOUND)
	{
	  DEBUG_ERROR("ZwQueryValueKey() == 0x%08x\n", Status);
	  goto Cleanup;
	}

      Info->DataLength = 0;
    }

  if (!Info->DataLength || ((Info->DataLength == 1) && !Info->Data[0]))
    {
      *Av = UnitAlloc(sizeof(WCHAR*));
      if (*Av == NULL)
	goto Cleanup;

      (*Av)[0] = NULL;
      *Ac = 0;
    }
  else
    {
      Count = 0;
      Off = 0;

      /* -1 to not count the last 0
       */

      Length = Info->DataLength / sizeof(WCHAR) - 1;

      while (Off < Length)
	{
	  if (!((PWCHAR)Info->Data)[Off])
	    ++Count;
	  ++Off;
	}
      
      if (!Count)
	goto Cleanup;

      /* allocate enougth space for av and
	 contents, append contents after av
       */

      Length = (Count + 1) * sizeof(WCHAR*) + Info->DataLength;
      Tmp = UnitAlloc(Length);
      if (Tmp == NULL)
	goto Cleanup;

      Data = Tmp + (Count + 1) * sizeof(PWCHAR);

      RtlCopyMemory(Data, Info->Data, Info->DataLength);

      *Av = (WCHAR**)Tmp;
      *Ac = Count;
      Count = 0;

      Begin = (PWCHAR)Data;

      Off = 0;
      Length = Info->DataLength / sizeof(WCHAR) - 1;

      while (Off < Length)
	{
	  if (!((PWCHAR)Data)[Off])
	    {
	      (*Av)[Count++] = Begin;
	      Begin = (PWCHAR)(Data + (Off + 1) * sizeof(WCHAR));
	    }

	  ++Off;
	}

      (*Av)[Count] = NULL;
    }

  /* success
   */
  
  error = 0;

 Cleanup:

  if (Buffer != NULL)
    UnitFree(Buffer);

  if (KeyHandle != NULL)
    ZwClose(KeyHandle);

  return error;
}



void UnitFreeAv(WCHAR** Av)
{
  UnitFree(Av);
}
