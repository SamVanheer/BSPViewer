#ifndef WAD_CWADMANAGER_H
#define WAD_CWADMANAGER_H

#include <memory>
#include <vector>

#include "core/Platform.h"

class CWadFile;
struct miptex_t;

/**
*	Manages the list of wads.
*/
class CWadManager final
{
public:
	enum class AddResult
	{
		/**
		*	Wad file loaded and added.
		*/
		SUCCESS = 0,

		/**
		*	The wad file had already been added.
		*/
		ALREADY_ADDED,

		/**
		*	The file couldn't be found.
		*/
		FILE_NOT_FOUND,

		/**
		*	The wad name is invalid.
		*/
		INVALID_NAME,
	};

private:
	typedef std::vector<std::unique_ptr<CWadFile>> WadFiles_t;

public:
	/**
	*	Constructor.
	*/
	CWadManager() = default;

	/**
	*	Destructor.
	*/
	~CWadManager();

	/**
	*	@return The base path that will be searched for wads.
	*/
	const char* GetBasePath() const { return m_szBasePath; }

	/**
	*	Sets the base path.
	*/
	void SetBasePath( const char* const pszBasePath );

	/**
	*	Finds a wad by name and returns it.
	*	@param pszWadName Name to search for.
	*	@return Wad, or null if no such wad exists.
	*/
	const CWadFile* FindWadByName( const char* const pszWadName ) const;

	/**
	*	Finds a texture by name by searching all wads.
	*	@param pszTextureName Name to search for.
	*	@return Texture, or null if the texture couldn't be found.
	*/
	const miptex_t* FindTextureByName( const char* const pszTextureName ) const;

	/**
	*	Adds a wad. This will load the wad and add it if it exists.
	*	@param pszWadName Name of the wad to load. This excludes the path and extension.
	*	@return AddResult value.
	*	@see AddResult
	*/
	AddResult AddWad( const char* const pszWadName );

	/**
	*	Removes all wads.
	*/
	void Clear();

private:
	char m_szBasePath[ MAX_PATH_LENGTH ] = { '\0' };

	WadFiles_t m_WadFiles;

private:
	CWadManager( const CWadManager& ) = delete;
	CWadManager& operator=( const CWadManager& ) = delete;
};

extern CWadManager g_WadManager;

#endif //WAD_CWADMANAGER_H