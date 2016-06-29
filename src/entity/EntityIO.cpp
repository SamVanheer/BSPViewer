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
/*
*	@file EntityIO.cpp
*	Modified version of Quake's entity loading code used to load Half-Life entities into memory for BSP rendering only. - Solokiller
*/
#include <cstdio>
#include <cstring>

#include "utility/Tokenization.h"

#include "CBaseEntity.h"
#include "CEntityList.h"

#include "EntityIO.h"

bool ED_FindClassName( char* data )
{
	char keyname[ 256 ];

	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		data = COM_Parse( data );
		if( com_token[ 0 ] == '}' )
			break;
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			return false;
		}

		strcpy( keyname, com_token );

		// another hack to fix heynames with trailing spaces
		size_t n = strlen( keyname );
		while( n && keyname[ n - 1 ] == ' ' )
		{
			keyname[ n - 1 ] = 0;
			n--;
		}

		// parse value	
		data = COM_Parse( data );
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			return false;
		}

		if( com_token[ 0 ] == '}' )
		{
			printf( "ED_ParseEntity: closing brace without data\n" );
			return false;
		}

		if( strcmp( keyname, "classname" ) == 0 )
			return true;
	}

	return false;
}

bool ED_ParseEdict( char *data, char*& pszOut, CBaseEntity*& pEnt )
{
	bool	anglehack;
	char		keyname[ 256 ];
	int			n;

	pEnt = nullptr;

	bool init = false;

	if( !ED_FindClassName( data ) )
	{
		printf( "ED_ParseEdict: couldn't find classname\n" );
		return false;
	}

	CBaseEntity* pEntity = g_EntList.Create( com_token );

	if( !pEntity )
	{
		printf( "ED_ParseEdict: Couldn't create entity '%s'\n", com_token );
		return false;
	}

	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		data = COM_Parse( data );
		if( com_token[ 0 ] == '}' )
			break;
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if( !strcmp( com_token, "angle" ) )
		{
			strcpy( com_token, "angles" );
			anglehack = true;
		}
		else
			anglehack = false;

		// FIXME: change light to _light to get rid of this hack
		if( !strcmp( com_token, "light" ) )
			strcpy( com_token, "light_lev" );	// hack for single light def

		strcpy( keyname, com_token );

		// another hack to fix heynames with trailing spaces
		n = strlen( keyname );
		while( n && keyname[ n - 1 ] == ' ' )
		{
			keyname[ n - 1 ] = 0;
			n--;
		}

		// parse value	
		data = COM_Parse( data );
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		if( com_token[ 0 ] == '}' )
		{
			printf( "ED_ParseEntity: closing brace without data\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if( keyname[ 0 ] == '_' )
			continue;

		if( anglehack )
		{
			char	temp[ 32 ];
			strcpy( temp, com_token );
			sprintf( com_token, "0 %s 0", temp );
		}

		if( !pEntity->KeyValue( keyname, com_token ) )
		{
			/*
			printf( "ED_ParseEdict: parse error\n" );
			g_EntList.Destroy( pEntity );
			return false;
			*/
		}
	}

	pszOut = data;

	pEnt = pEntity;

	return true;
}

bool ED_LoadFromFile( char *data )
{
	int inhibit = 0;

	CBaseEntity* pEnt;

	// parse ents
	while( 1 )
	{
		// parse the opening brace	
		data = COM_Parse( data );
		if( !data )
			break;
		if( com_token[ 0 ] != '{' )
		{
			printf( "ED_LoadFromFile: found %s when expecting {", com_token );
			return false;
		}

		if( !ED_ParseEdict( data, data, pEnt ) )
			return false;

		// remove things from different skill levels or deathmatch
		/*
		if( deathmatch.value )
		{
		if( ( ( int ) ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH ) )
		{
		ED_Free( ent );
		inhibit++;
		continue;
		}
		}
		*/

		pEnt->Spawn();
	}

	printf( "%i entities inhibited\n", inhibit );

	return true;
}