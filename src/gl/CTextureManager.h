#ifndef GL_CTEXTUREMANAGER_H
#define GL_CTEXTUREMANAGER_H

#include <unordered_map>
#include <vector>

#include <gl/glew.h>

#include "common/StringUtils.h"

#include "bsp/BSPRenderDefs.h"

struct miptex_t;

/**
*	Manages all textures used by the map.
*/
class CTextureManager final
{
private:
	typedef std::unordered_map<const char*, size_t, RawCharHashI, RawCharEqualToI> TexMap_t;
	typedef std::vector<texture_t> Textures_t;

public:
	/**
	*	Constructor.
	*/
	CTextureManager() = default;

	/**
	*	Destructor.
	*/
	~CTextureManager() = default;

	/**
	*	@return Whether the manager is initialized.
	*/
	bool IsInitialized() const { return m_bInitialized; }

	/**
	*	@return Whether any textures are loaded.
	*/
	bool HasTextures() const { return m_uiTexturesInUse > 0; }

	/**
	*	@return The maximum number of textures that can be loaded into memory.
	*/
	size_t GetMaxTextures() const { return m_Textures.size(); }

	/**
	*	@return The number of textures that are loaded into memory.
	*/
	size_t GetNumTextures() const { return m_uiTexturesInUse; }

	/**
	*	Initializes the manager.
	*	@param uiNumTextures Number of textures that will be loaded.
	*	@return Whether initialization succeeded.
	*/
	bool Initialize( const size_t uiNumTextures );

	/**
	*	Shuts down the manager. All textures are freed.
	*/
	void Shutdown();

	/**
	*	Finds a texture by name.
	*	@param pszName Texture name.
	*	@return Texture, or null if it couldn't be found.
	*/
	const texture_t* FindTexture( const char* const pszName ) const;

	/**
	*	@copydoc FindTexture( const char* const pszName ) const
	*/
	texture_t* FindTexture( const char* const pszName );

	/**
	*	Loads a new texture. If pMiptex is not null, uses it as a source to load the texture.
	*	@param pszName Name of the texture.
	*	@param pMiptex Optional. Texture data to use.
	*	@return Texture, or null if the texture could not be loaded.
	*/
	texture_t* LoadTexture( const char* const pszName, const miptex_t* pMiptex = nullptr );

	/**
	*	Set up animating texture chains. This should be called after all textures have been loaded.
	*	@return Whether setup succeeded.
	*/
	bool SetupAnimatingTextures();

private:
	bool m_bInitialized = false;

	//TODO: the map might not be needed.
	TexMap_t m_TexMap;
	Textures_t m_Textures;

	size_t m_uiTexturesInUse = 0;

private:
	CTextureManager( const CTextureManager& ) = delete;
	CTextureManager& operator=( const CTextureManager& ) = delete;
};

extern CTextureManager g_TextureManager;

#endif //GL_CTEXTUREMANAGER_H