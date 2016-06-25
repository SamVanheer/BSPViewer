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

extern GLuint lightmapID[ MAX_LIGHTMAPS ];

bool FindWadList( const bmodel_t* pModel, char*& pszWadList );

bool LoadBrushModel( bmodel_t* pModel, dheader_t* pHeader );

void FreeModel( bmodel_t* pModel );
}

#endif //BSP_BSPRENDERIO_H