/*
 * Copyright (c) 2016-2020  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "xsmc.h"
#include "xsHost.h"
#include "mc.xs.h"			// for xsID_ values

#include "nrf_fstorage_sd.h"

extern nrf_fstorage_t fstorage;
extern int __start_unused_space;

struct modFlashRecord {
	uint32_t partitionStart;
	uint32_t partitionByteLength;
};

typedef struct modFlashRecord modFlashRecord;
typedef struct modFlashRecord *modFlash;

//static uint32_t nrf5_flash_end_addr_get()
//{
//    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
//    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
//    uint32_t const code_sz         = NRF_FICR->CODESIZE;
//
//    return (bootloader_addr != 0xFFFFFFFF ?
//            bootloader_addr : (code_sz * page_sz));
//}

void xs_flash(xsMachine *the)
{
	modFlashRecord flash;

	if (!modSPIFlashInit())
		xsUnknownError("init failed");

//	uint32_t lastAddr = nrf5_flash_end_addr_get();
//	xsLog("  flash end addreess = %d\n", lastAddr);
//
//	xsLog("  &__start_unused_space = %d\n", &__start_unused_space);


	if (xsStringType == xsmcTypeOf(xsArg(0))) {
		char *partition = xsmcToString(xsArg(0));
		uint32_t modStart = (uintptr_t)(kModulesStart - kFlashStart);

		if (0 == c_strcmp(partition, "xs")) {
			flash.partitionStart = modStart;
			flash.partitionByteLength = 0xF4000 - flash.partitionStart;
		}
		else if (0 == c_strcmp(partition, "running")) {
			flash.partitionStart = 0x26000;
			flash.partitionByteLength = modStart - flash.partitionStart;
		}
		else
			xsUnknownError("unknown partition");
	}
	else {
		flash.partitionStart = xsmcToInteger(xsArg(0));
		flash.partitionByteLength = xsmcToInteger(xsArg(1));
	}

	xsmcSetHostChunk(xsThis, &flash, sizeof(flash));
}

void xs_flash_destructor(void *data)
{
}

void xs_flash_erase(xsMachine *the)
{
	int sector = xsmcToInteger(xsArg(0));
	modFlash flash = xsmcGetHostChunk(xsThis);
	if ((sector < 0) || (sector >= (flash->partitionByteLength / fstorage.p_flash_info->erase_unit)))
		xsUnknownError("invalid sector");

	if (!modSPIErase(flash->partitionStart + (sector * fstorage.p_flash_info->erase_unit), fstorage.p_flash_info->erase_unit))
		xsUnknownError("erase failed");
}

void xs_flash_read(xsMachine *the)
{
	int offset = xsmcToInteger(xsArg(0));
	int byteLength = xsmcToInteger(xsArg(1));
	modFlash flash = xsmcGetHostChunk(xsThis);

	if ((offset < 0) || (offset >= flash->partitionByteLength))
		xsUnknownError("invalid offset");

	if ((byteLength <= 0) || ((offset + byteLength) > flash->partitionByteLength))
		xsUnknownError("invalid length");

	xsmcSetArrayBuffer(xsResult, NULL, byteLength);

	if (!modSPIRead(offset + flash->partitionStart, byteLength, xsmcToArrayBuffer(xsResult)))
		xsUnknownError("read failed");
}

void xs_flash_write(xsMachine *the)
{
	int offset = xsmcToInteger(xsArg(0));
	int byteLength = xsmcToInteger(xsArg(1));
	modFlash flash = xsmcGetHostChunk(xsThis);

	if ((offset < 0) || (offset >= flash->partitionByteLength))
		xsUnknownError("invalid offset");

	if ((byteLength <= 0) || ((offset + byteLength) > flash->partitionByteLength))
		xsUnknownError("invalid length");

	if (!modSPIWrite(offset + flash->partitionStart, byteLength, xsmcToArrayBuffer(xsArg(2))))
		xsUnknownError("write failed");
}

void xs_flash_map(xsMachine *the)
{
	xsUnknownError("unsupported");
}

void xs_flash_byteLength(xsMachine *the)
{
	modFlash flash = xsmcGetHostChunk(xsThis);
	xsmcSetInteger(xsResult, flash->partitionByteLength);
}

void xs_flash_blockSize(xsMachine *the)
{
	xsmcSetInteger(xsResult, fstorage.p_flash_info->erase_unit);
}
