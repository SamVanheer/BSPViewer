#include <cassert>
#include <memory>

#include "GLUtil.h"

#include "GLMiptex.h"

const size_t PALETTE_ENTRIES = 256;

/**
*	Texture format.
*/
enum class TexFormat_t : int
{
	/**
	*	No transparency or translucency.
	*	This sprite will appear as a rectangle in the world, animating with whatever design you create it with.
	*/
	SPR_NORMAL = 0,

	/**
	*	Looks best for explosions, steam, lasers, or other gaseous or light effect.
	*	It acts as a brightness map in the scene, brightening whatever appears behind it.
	*	What this amounts to is a white pixel in your sprite is essentially totally opaque and white, and black pixel becomes transparent,
	*	and every value in between is appropriately translucent, modulo the background behind the sprite.
	*	The lighting of the surrounding area will not directly affect the lighting of this sprite,
	*	so in extremely dark areas the sprite may appear to be self-illuminating (which is often desirable, especially for explosions and lens flares).
	*	The brighter the background behind the sprite, the brighter the sprite will be.
	*/
	SPR_ADDITIVE = 1,

	/**
	*	Similar to Additive in that it has varying grades of opacity from totally opaque to totally transparent,
	*	but the blending within the world is done differently and the opacity is controlled not by the value of a given pixel,
	*	but by the palette register of the color of that pixel.
	*	Any pixel within the sprite that is colored by the first color in your palette will be transparent,
	*	and any pixel within the sprite that is colored by the last color in the palette will be opaque.
	*/
	SPR_INDEXALPHA = 2,

	/**
	*	This is a totally opaque sprite with one key color that is invisible.
	*	This sort of sprite can look jaggy and less realistic than Additive or Indexalpha sprites,
	*	but it renders significantly faster than either of those two, and so can be very useful for situations where speed is more important than appearance.
	*	For instance, if you are doing sprites for a multiplayer mod,
	*	you might choose to use Alphatest sprites for explosion effects even though they look worse than Additive,
	*	because you don't want the framerate to get too bad for any given player who is seeing a lot of sprites on their screen.
	*	The appearance of Alphatest sprites is similar to that of Masked Textures used on brushmodels,
	*	excepting that Masked Textures respond to the light level of their situations while Alphatest sprites do not.
	*/
	SPR_ALPHTEST = 3
};

/**
*	Converts an 8 bit indexed palette into a 32 bit palette using the given format.
*	@param pInPalette 8 bit indexed palette.
*	@param pRGBAPalette 32 bit RGBA palette.
*	@param format Texture format to convert to.
*/
void Convert8To32Bit( const byte* pInPalette, byte* pRGBAPalette, const TexFormat_t format )
{
	assert( pInPalette );
	assert( pRGBAPalette );

	byte* pRGBA = pRGBAPalette;

	switch( format )
	{
	default: //TODO: warn
	case TexFormat_t::SPR_NORMAL:
	case TexFormat_t::SPR_ADDITIVE:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = 0xFF;
			}

			break;
		}

	case TexFormat_t::SPR_INDEXALPHA:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = static_cast<byte>( uiIndex );
			}

			break;
		}

	case TexFormat_t::SPR_ALPHTEST:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = pInPalette[ uiIndex ] == 255 ? pInPalette[ uiIndex ] : 0xFF;
			}

			//Zero out the transparent color.
			memset( pRGBAPalette + 255 * 4, 0, sizeof( byte ) * 4 );

			break;
		}
	}
}

#define MAX_TEXTURE_DIMS 512

bool CalculateImageDimensions( const int iWidth, const int iHeight, int& iOutWidth, int& iOutHeight )
{
	if( iWidth <= 0 || iHeight <= 0 )
		return false;

	for( iOutWidth = 1; iOutWidth < iWidth; iOutWidth <<= 1 )
	{
	}

	if( iOutWidth > MAX_TEXTURE_DIMS )
		iOutWidth = MAX_TEXTURE_DIMS;

	for( iOutHeight = 1; iOutHeight < iHeight; iOutHeight <<= 1 )
	{
	}

	if( iOutHeight > MAX_TEXTURE_DIMS )
		iOutHeight = MAX_TEXTURE_DIMS;

	return true;
}

GLuint UploadMiptex( const miptex_t* pMiptex )
{
	assert( pMiptex );

	GLuint tex;

	glGenTextures( 1, &tex );

	check_gl_error();

	glBindTexture( GL_TEXTURE_2D, tex );

	check_gl_error();

	byte rgba[ PALETTE_ENTRIES * 4 ];

	TexFormat_t format = TexFormat_t::SPR_NORMAL;

	const byte* pBase = reinterpret_cast<const byte*>( pMiptex );

	const byte* pPal = pBase + pMiptex->offsets[ 0 ] + ( ( pMiptex->width * pMiptex->height * 85 ) >> 6 );

	if( *( reinterpret_cast<const short*>( pPal ) ) != 256 )
	{
		printf( "Invalid miptex\n" );
	}

	pPal += sizeof( short );

	Convert8To32Bit( pPal, rgba, format );

	// convert texture to power of 2. Otherwise it ends up having weird lines.
	int outwidth;
	int outheight;

	if( !CalculateImageDimensions( pMiptex->width, pMiptex->height, outwidth, outheight ) )
		return 0;

	const size_t uiSize = outwidth * outheight * 4;

	//Needs at least one pixel (satisfies code analysis)
	if( uiSize < 4 )
		return 0;

	std::unique_ptr<byte[]> image = std::make_unique<byte[]>( uiSize );

	if( !image )
	{
		return 0;
	}

	int row1[ MAX_TEXTURE_DIMS ], row2[ MAX_TEXTURE_DIMS ], col1[ MAX_TEXTURE_DIMS ], col2[ MAX_TEXTURE_DIMS ];

	for( int i = 0; i < outwidth; i++ )
	{
		col1[ i ] = ( int ) ( ( i + 0.25 ) * ( pMiptex->width / ( float ) outwidth ) );
		col2[ i ] = ( int ) ( ( i + 0.75 ) * ( pMiptex->width / ( float ) outwidth ) );
	}

	for( int i = 0; i < outheight; i++ )
	{
		row1[ i ] = ( int ) ( ( i + 0.25 ) * ( pMiptex->height / ( float ) outheight ) ) * pMiptex->width;
		row2[ i ] = ( int ) ( ( i + 0.75 ) * ( pMiptex->height / ( float ) outheight ) ) * pMiptex->width;
	}

	byte* out = image.get();

	const byte	*pix1, *pix2, *pix3, *pix4;

	const byte* pPixelData = reinterpret_cast<const byte*>( pBase + pMiptex->offsets[ 0 ] );

	// scale down and convert to 32bit RGBA
	for( int i = 0; i<outheight; i++ )
	{
		for( int j = 0; j<outwidth; j++, out += 4 )
		{
			pix1 = &rgba[ pPixelData[ row1[ i ] + col1[ j ] ] * 4 ];
			pix2 = &rgba[ pPixelData[ row1[ i ] + col2[ j ] ] * 4 ];
			pix3 = &rgba[ pPixelData[ row2[ i ] + col1[ j ] ] * 4 ];
			pix4 = &rgba[ pPixelData[ row2[ i ] + col2[ j ] ] * 4 ];

			out[ 0 ] = ( pix1[ 0 ] + pix2[ 0 ] + pix3[ 0 ] + pix4[ 0 ] ) >> 2;
			out[ 1 ] = ( pix1[ 1 ] + pix2[ 1 ] + pix3[ 1 ] + pix4[ 1 ] ) >> 2;
			out[ 2 ] = ( pix1[ 2 ] + pix2[ 2 ] + pix3[ 2 ] + pix4[ 2 ] ) >> 2;
			out[ 3 ] = ( pix1[ 3 ] + pix2[ 3 ] + pix3[ 3 ] + pix4[ 3 ] ) >> 2;
		}
	}

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, outwidth, outheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.get() );

	check_gl_error();

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	check_gl_error();

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	check_gl_error();

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	check_gl_error();

	glGenerateMipmap( GL_TEXTURE_2D );

	check_gl_error();

	return tex;
}