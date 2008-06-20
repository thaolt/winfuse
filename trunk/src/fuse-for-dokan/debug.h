/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Dec 15 12:38:34 2007 texane
** Last update Fri May 30 22:47:32 2008 texane
*/



#ifndef DEBUG_H_INCLUDED
# define DEBUG_H_INCLUDED



#if defined(_DEBUG)
# include <stdio.h>
# define DEBUG_ERROR(s, ...) printf("[!] " __FUNCTION__ ": " s, __VA_ARGS__)
# define DEBUG_PRINTF(s, ...) printf("[?] " __FUNCTION__ ": " s, __VA_ARGS__)
# define DEBUG_ENTER() printf("[>] " __FUNCTION__ "\n")
# define DEBUG_LEAVE() printf("[<] " __FUNCTION__ "\n")
# define DEBUG_STEP() printf("[@] " __FUNCTION__ ": %d\n", __LINE__)
#else
# define DEBUG_ERROR(s, ...)
# define DEBUG_PRINTF(s, ...)
# define DEBUG_ENTER()
# define DEBUG_LEAVE()
# define DEBUG_STEP()
# define DEBUG_TODO(s, ...)
#endif /* _DEBUG */



#endif /* ! DEBUG_H_INCLUDED */
