/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sat Jun 14 23:25:53 2008 texane
** Last update Sat Jun 14 23:25:55 2008 texane
*/



#ifndef DEBUG_H_INCLUDED
# define DEBUG_H_INCLUDED



#ifdef DBG
# define DEBUG_ENTER() do { DbgPrint("[>] " __FUNCTION__ "()\n"); } while (0)
# define DEBUG_LEAVE() do { DbgPrint("[<] " __FUNCTION__ "()\n"); } while (0)
# define DEBUG_PRINTF(fmt, ...) do { DbgPrint("[?] " __FUNCTION__ ": " fmt, __VA_ARGS__); } while (0)
# define DEBUG_ERROR(fmt, ...) do { DbgPrint("[!] " __FUNCTION__ ": " fmt, __VA_ARGS__); } while (0)
#else
# define DEBUG_ENTER()
# define DEBUG_LEAVE()
# define DEBUG_PRINTF(fmt, ...)
# define DEBUG_ERROR(fmt, ...)
#endif /* DBG */



#endif /* ! DEBUG_H_INCLUDED */
