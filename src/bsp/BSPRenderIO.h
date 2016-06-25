#ifndef BSP_BSPRENDERIO_H
#define BSP_BSPRENDERIO_H

/**
*	@file BSP Rendering data structures file IO
*/

#include "BSPConstants.h"
#include "BSPRenderDefs.h"

namespace BSP
{
#define MAX_MOD_KNOWN 512

extern bmodel_t mod_known[ MAX_MOD_KNOWN ];

bool LoadBrushModel( bmodel_t* pModel, dheader_t* pHeader );

void FreeModel( bmodel_t* pModel );
}

#endif //BSP_BSPRENDERIO_H