/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef BSP_BSPRENDERIO_H
#define BSP_BSPRENDERIO_H

/**
*	@file BSP Rendering data structures file IO
*/

#include <gl/glew.h>

#include "BSPConstants.h"
#include "BSPRenderDefs.h"

namespace BSP
{
#define MAX_MOD_KNOWN 512
#define	MAX_LIGHTMAPS	256

extern bmodel_t mod_known[ MAX_MOD_KNOWN ];

extern int mod_numknown;

extern GLuint lightmapID[ MAX_LIGHTMAPS ];

bool FindWadList( const bmodel_t* pModel, char*& pszWadList );

bool LoadBrushModel( bmodel_t* pModel, dheader_t* pHeader );

void FreeModel( bmodel_t* pModel );
}

#endif //BSP_BSPRENDERIO_H