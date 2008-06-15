/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sun Jun 15 02:33:34 2008 texane
** Last update Sun Jun 15 11:42:21 2008 texane
*/



#ifndef UNIT_H_INCLUDED
# define UNIT_H_INCLUDED



#include <ntddk.h>



int UnitGetAv(PUNICODE_STRING, int*, WCHAR***);
void UnitFreeAv(WCHAR**);



#endif /* ! UNIT_H_INCLUDED */
