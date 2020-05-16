#pragma once

#ifndef UTF8_H_INCLUDED
#define UTF8_H_INCLUDED

/**
 * All Windows family OS's have problems with Unicode
 * This progect uses UTF-8, in *nix it is supported directly, and this file will have no effect
 * Anyway, to get the value of d_name of DIR struct it is required to call GetDname() function
 * In Windows we need to transform it into UTF-16LE and use _w-prefixed functions
 *
 * Note that console output will not be changed anyway (because it is impossible to fix it)
 */

#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>

#ifdef __WIN32
#include <windows.h>

/** Functions and types redirection */
#define fopen    _utf8_fopen
#define opendir  _utf8_opendir
#define readdir  _wreaddir
#define closedir _wclosedir
#define DIR      _WDIR
#define dirent   _wdirent

wchar_t* UTF8ToWchar();

FILE* _utf8_fopen(char *fname, char *mode);
DIR* _utf8_opendir(char *dirname);
char* GetDname(dirent* dir);

wchar_t* UTF8ToWchar(char *in)
{
	uint64_t len = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
	wchar_t *out = (wchar_t*)calloc(len + 1, sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, in, -1, out, len);
	return out;
}

char* WcharToUTF8(wchar_t *in)
{
	uint64_t len = WideCharToMultiByte(CP_UTF8, 0, in, -1, NULL, 0, NULL, NULL);
	char *out = (char*)calloc(len + 1, sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, in, -1, out, len, NULL, NULL);
	return out;
}

FILE* _utf8_fopen(char *fname, char *mode)
{
	wchar_t *wfname = UTF8ToWchar(fname);
	wchar_t *wmode = UTF8ToWchar(mode);
	FILE *out = _wfopen(wfname, wmode);
	free(wfname);
	free(wmode);
	return out;
}

DIR* _utf8_opendir(const char *dirname)
{
	wchar_t *wdirname = UTF8ToWchar(const_cast<char*>(dirname));
	DIR *out = _wopendir(wdirname);
	free(wdirname);
	return out;
}

char* GetDname(dirent* dir)
{
	return WcharToUTF8(dir->d_name);
}

#else

char* GetDname(dirent *dir)
{
	return (dir->d_name);
}

#endif // __WIN32

#endif // UTF8_H_INCLUDED
