/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2010 Magic Lantern Team.
 * Derived from platform/5DC.111/stubs.S and include/stub.h, GPLv2.
 * https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253
 * Modified 2026-11-12: ten selected absolute symbols for offline research.
 * NO WARRANTY. NOT VERIFIED ON A CAMERA. NOT A BOOTLOADER. */
#define NSTUB(addr,name) \
 .global name; \
 .extern name; \
 name = addr
.text
NSTUB(0xFFAE5B1C, FIO_OpenFile)
NSTUB(0xFFAE5B54, FIO_ReadFile)
NSTUB(0xFFAE5B6C, FIO_WriteFile)
NSTUB(0xFFAE5B78, FIO_CloseFile)
NSTUB(0xFFAE5B2C, FIO_CreateFile)
NSTUB(0xFFB2F664, AllocateMemory)
NSTUB(0xFFB2F680, FreeMemory)
NSTUB(0xFFB18B40, task_create)
NSTUB(0xFF8190D0, GUI_Control)
NSTUB(0xFFB187F4, msleep)
