#ifndef WAD_WADIO_H
#define WAD_WADIO_H

#include "WadFile.h"

wadinfo_t* LoadWadFile( const char* const pszFileName );

void CleanupWadLumpName( const char* in, char* out, const size_t uiBufferSize );

const miptex_t* Wad_FindTexture( const wadinfo_t* pWad, const char* const pszName );

#endif //WAD_WADIO_H