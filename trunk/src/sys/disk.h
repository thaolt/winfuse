/*
** Made by texane <texane@gmail.com>
** 
** Started on  Mon Jun 16 20:55:06 2008 texane
** Last update Mon Jun 16 23:21:12 2008 texane
*/



#ifndef WINFUSE_DISK_H
# define WINFUSE_DISK_H



#include <ntddk.h>



NTSTATUS WinfuseCreateDisk(PDRIVER_OBJECT);
void WinfuseDestroyDisk(void);
PDEVICE_OBJECT WinfuseReferenceDisk(void);
void WinfuseDereferenceDisk(PDEVICE_OBJECT);



#endif /* ! WINFUSE_DISK_H */
