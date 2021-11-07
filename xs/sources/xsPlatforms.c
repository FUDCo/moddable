/*
 * Copyright (c) 2016-2017  Moddable Tech, Inc.
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
 * This file incorporates work covered by the following copyright and  
 * permission notice:  
 *
 *       Copyright (C) 2010-2016 Marvell International Ltd.
 *       Copyright (C) 2002-2010 Kinoma, Inc.
 *
 *       Licensed under the Apache License, Version 2.0 (the "License");
 *       you may not use this file except in compliance with the License.
 *       You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *       Unless required by applicable law or agreed to in writing, software
 *       distributed under the License is distributed on an "AS IS" BASIS,
 *       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *       See the License for the specific language governing permissions and
 *       limitations under the License.
 */

#include "xsAll.h"
#include "xsScript.h"

/* old programming interface defaults to 0 */

#ifndef mxUseDefaultMachinePlatform
	#define mxUseDefaultMachinePlatform 0
#endif
#ifndef mxUseDefaultBuildKeys
	#define mxUseDefaultBuildKeys 0
#endif
#ifndef mxUseDefaultChunkAllocation
	#define mxUseDefaultChunkAllocation 0
#endif
#ifndef mxUseDefaultSlotAllocation
	#define mxUseDefaultSlotAllocation 0
#endif
#ifndef mxUseDefaultFindModule
	#define mxUseDefaultFindModule 0
#endif
#ifndef mxUseDefaultLoadModule
	#define mxUseDefaultLoadModule 0
#endif
#ifndef mxUseDefaultParseScript
	#define mxUseDefaultParseScript 0
#endif
#ifndef mxUseDefaultQueuePromiseJobs
	#define mxUseDefaultQueuePromiseJobs 0
#endif
#ifndef mxUseDefaultDebug
	#define mxUseDefaultDebug 0
#endif

#if mxUseDefaultFindModule
	static txBoolean fxFindPreparation(txMachine* the, txSlot* realm, txString path, txID* id);
#endif

#ifdef mxParse
	#if mxUseDefaultFindModule
		static txBoolean fxFindScript(txMachine* the, txString path, txID* id);
	#endif
	#if mxUseDefaultLoadModule
		extern txScript* fxLoadScript(txMachine* the, txString path, txUnsigned flags);
	#endif
#endif

#if mxUseDefaultMachinePlatform

void fxCreateMachinePlatform(txMachine* the)
{
}

void fxDeleteMachinePlatform(txMachine* the)
{
}

#endif /* mxUseDefaultMachinePlatform */

#if mxUseDefaultBuildKeys

void fxBuildKeys(txMachine* the)
{
	int i;
	for (i = 0; i < XS_SYMBOL_ID_COUNT; i++) {
		txID id = the->keyIndex;
		txSlot* description = fxNewSlot(the);
		fxCopyStringC(the, description, gxIDStrings[i]);
		the->keyArray[id] = description;
		the->keyIndex++;
	}
	for (; i < XS_ID_COUNT; i++) {
		fxID(the, gxIDStrings[i]);
	}
}

#endif	/* mxUseDefaultBuildKeys */ 


#if mxUseDefaultChunkAllocation

void* fxAllocateChunks(txMachine* the, txSize theSize)
{
	return c_malloc(theSize);
}

void fxFreeChunks(txMachine* the, void* theChunks)
{
	c_free(theChunks);
}

#endif /* mxUseDefaultChunkAllocation */ 


#if mxUseDefaultSlotAllocation

txSlot* fxAllocateSlots(txMachine* the, txSize theCount)
{
	return(txSlot*)c_malloc(theCount * sizeof(txSlot));
}

void fxFreeSlots(txMachine* the, void* theSlots)
{
	c_free(theSlots);
}

#endif /* mxUseDefaultSlotAllocation */ 

#if mxUseDefaultLoadModule

extern void fxDebugImport(txMachine* the, txString path);
txBoolean fxLoadModule(txMachine* the, txSlot* realm, txID moduleID, txScript* result)
{
	char path[C_PATH_MAX];
	char* dot;
	txByte* code;
	txSize size;
	c_strcpy(path, fxGetKeyName(the, moduleID));
	dot = c_strchr(path, '.');
	if (dot)
		*dot = 0;
	c_strcat(path, ".xsb");
	code = fxGetArchiveCode(the, path, &size);
	if (code) {
		result->callback = NULL;
		result->symbolsBuffer = NULL;
		result->symbolsSize = 0;
		result->codeBuffer = code;
		result->codeSize = size;
		result->hostsBuffer = NULL;
		result->hostsSize = 0;
		result->path = path;
		result->version[0] = XS_MAJOR_VERSION;
		result->version[1] = XS_MINOR_VERSION;
		result->version[2] = XS_PATCH_VERSION;
		result->version[3] = 0;
		return 1;
	}
	else {
 		txPreparation* preparation = the->preparation;
		if (preparation) {
			txInteger c = preparation->scriptCount;
			txScript* script = preparation->scripts;
			while (c > 0) {
				if (!c_strcmp(path + preparation->baseLength, script->path)) {
					c_memcpy(result, script, sizeof(txScript));
					return 1;
				}
				c--;
				script++;
			}
		}
	}
#ifdef mxParse
	{
 		txScript* script;
	#ifdef mxDebug
		txUnsigned flags = mxDebugFlag;
	#else
		txUnsigned flags = 0;
	#endif
 		c_strcpy(path, fxGetKeyName(the, moduleID));
 		script = fxLoadScript(the, path, flags);
		if (script) {
			c_memcpy(result, script, sizeof(txScript));
			return 1;
		}
 	}
#else
	#ifdef mxDebug
	{
		if (!c_strncmp(path, "xsbug://", 8)) {
			fxDebugImport(the, path);
			return 1;
		}
	}
	#endif
#endif
	return 0;
}

#ifdef mxParse
txScript* fxLoadScript(txMachine* the, txString path, txUnsigned flags)
{
	txParser _parser;
	txParser* parser = &_parser;
	txParserJump jump;
	FILE* file = NULL;
	txString name = NULL;
	char map[C_PATH_MAX];
	txScript* script = NULL;
	fxInitializeParser(parser, the, the->parserBufferSize, the->parserTableModulo);
	parser->firstJump = &jump;
	file = fopen(path, "r");
	if (c_setjmp(jump.jmp_buf) == 0) {
		mxParserThrowElse(file);
		parser->path = fxNewParserSymbol(parser, path);
		fxParserTree(parser, file, (txGetter)fgetc, flags, &name);
		fclose(file);
		file = NULL;
		if (name) {
			txString slash = c_strrchr(path, mxSeparator);
			if (slash) *slash = 0;
			c_strcat(path, name);
			mxParserThrowElse(c_realpath(path, map));
			parser->path = fxNewParserSymbol(parser, map);
			file = fopen(map, "r");
			mxParserThrowElse(file);
			fxParserSourceMap(parser, file, (txGetter)fgetc, flags, &name);
			fclose(file);
			file = NULL;
			if (parser->errorCount == 0) {
				if (slash) *slash = 0;
				c_strcat(path, name);
				mxParserThrowElse(c_realpath(path, map));
				parser->path = fxNewParserSymbol(parser, map);
			}
		}
		fxParserHoist(parser);
		fxParserBind(parser);
		script = fxParserCode(parser);
	}
	if (file)
		fclose(file);
#ifdef mxInstrument
	if (the->peakParserSize < parser->total)
		the->peakParserSize = parser->total;
#endif
	fxTerminateParser(parser);
	return script;
}
#endif

#endif  /* mxUseDefaultLoadModule */

#if mxUseDefaultParseScript

txScript* fxParseScript(txMachine* the, void* stream, txGetter getter, txUnsigned flags)
{
	txParser _parser;
	txParser* parser = &_parser;
	txParserJump jump;
	txScript* script = NULL;
	fxInitializeParser(parser, the, the->parserBufferSize, the->parserTableModulo);
	parser->firstJump = &jump;
	if (c_setjmp(jump.jmp_buf) == 0) {
		fxParserTree(parser, stream, getter, flags, NULL);
#ifdef mxDebug
		parser->flags |= mxDebugFlag;
		if (!parser->source) {
			char tag[16];
			parser->flags |= mxDebugFlag;
			fxGenerateTag(the, tag, sizeof(tag), C_NULL);
			parser->source = fxNewParserSymbol(parser, tag);
		}
		if (fxIsConnected(the)) {
			if (getter == fxStringGetter)
				fxFileEvalString(the, ((txStringStream*)stream)->slot->value.string, parser->source->string);
			else if (getter == fxStringCGetter)
				fxFileEvalString(the, ((txStringCStream*)stream)->buffer, parser->source->string);
		}
#endif
		fxParserHoist(parser);
		fxParserBind(parser);
		script = fxParserCode(parser);
	}
#ifdef mxInstrument
	if (the->peakParserSize < parser->total)
		the->peakParserSize = parser->total;
#endif
	fxTerminateParser(parser);
	return script;
}

#endif  /* mxUseDefaultParseScript */

#if mxUseDefaultQueuePromiseJobs

void fxQueuePromiseJobs(txMachine* the)
{
	mxUnknownError("promise: no queue");
}

#endif  /* mxUseDefaultQueuePromiseJobs */

#if mxUseDefaultDebug

void fxAbort(txMachine* the, int status)
{
	c_exit(status);
}

#endif  /* mxUseDefaultDebug */

#ifdef mxDebug

#if mxUseDefaultDebug

void fxAbort(txMachine* the, int status)
{
	mxTry(the) {
		if (status == XS_NOT_ENOUGH_MEMORY_EXIT)
			mxUnknownError("not enough memory");
		else if (status == XS_STACK_OVERFLOW_EXIT)
			mxUnknownError("stack overflow");
		else if (status == XS_DEAD_STRIP_EXIT)
			mxUnknownError("dead strip");
		else if (status == XS_NO_MORE_KEYS_EXIT)
			mxUnknownError("not enough keys");
	}
	mxCatch(the) {
	}
	c_exit(status);
}

void fxConnect(txMachine* the)
{
}

void fxDisconnect(txMachine* the)
{
}

txBoolean fxIsConnected(txMachine* the)
{
	return 0;
}

txBoolean fxIsReadable(txMachine* the)
{
	return 0;
}

void fxReceive(txMachine* the)
{
}

void fxSend(txMachine* the, txBoolean more)
{
}

#endif /* mxUseDefaultDebug */

#endif

#if mxWindows || mxMacOSX || mxLinux || mxWasm
uint32_t modMilliseconds()
{
	c_timeval tv;
	c_gettimeofday(&tv, NULL);
#if (mxWasm || mxWindows || mxMacOSX)
	return (uint32_t)(uint64_t)(((double)(tv.tv_sec) * 1000.0) + ((double)(tv.tv_usec) / 1000.0));
#else
	return (uint32_t)(((double)(tv.tv_sec) * 1000.0) + ((double)(tv.tv_usec) / 1000.0));
#endif
}
#endif

#if mxWindows

#if _MSC_VER < 1800

unsigned long c_nan[2]={0xffffffff, 0x7fffffff};
unsigned long c_infinity[2]={0x00000000, 0x7ff00000};

int c_fpclassify(double x)
{
	int result = FP_NORMAL;
	switch (_fpclass(x)) {
	case _FPCLASS_SNAN:
	case _FPCLASS_QNAN:
		result = FP_NAN;
		break;
	case _FPCLASS_NINF:
	case _FPCLASS_PINF:
		result = FP_INFINITE;
		break;
	case _FPCLASS_NZ:
	case _FPCLASS_PZ:
		result = FP_ZERO;
		break;
	case _FPCLASS_ND:
	case _FPCLASS_PD:
		result = FP_SUBNORMAL;
		break;
	}
	return result;
}

#endif /* _MSC_VER < 1800 */

int c_gettimeofday(c_timeval *tp, struct c_timezone *tzp)
{
  struct _timeb tb;

  _ftime(&tb);
  if (tp != 0) {
    tp->tv_sec = (long)tb.time;
    tp->tv_usec = tb.millitm * 1000;
  }
  if (tzp != 0) {
    tzp->tz_minuteswest = tb.timezone;
    tzp->tz_dsttime = tb.dstflag;
  }
  return (0);
}

#ifdef mxParse
char *c_realpath(const char *path, char *real)
{
	if (_fullpath(real, path, C_PATH_MAX) != NULL) {
		DWORD attributes = GetFileAttributes(real);
		if (attributes != 0xFFFFFFFF) {
			return real;
		}
	}
	return C_NULL;
}
#endif

#endif
