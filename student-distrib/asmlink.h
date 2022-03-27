#ifndef _LINKAGE_H
#define _LINKAGE_H

#define asmlinkage  __attribute__((regparm(0)))
#define regcall	__attribute__((regparm(3)))

#define __user

#endif