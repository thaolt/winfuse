/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 20:55:06 2008 texane
** Last update Mon Jun 16 21:09:42 2008 texane
*/



#ifndef WINFUSE_FS_H
# define WINFUSE_FS_H



#include <ntddk.h>



NTSTATUS WinfuseCreateFs(PDRIVER_OBJECT);
void WinfuseDestroyFs(void);



#endif /* ! WINFUSE_FS_H */
