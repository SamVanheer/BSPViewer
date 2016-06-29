/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#include <cctype>

#include "Tokenization.h"

char com_token[ MAX_COM_TOKEN ];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse( char *data )
{
	int             c;
	int             len;

	len = 0;
	com_token[ 0 ] = 0;

	if( !data )
		return nullptr;

	// skip whitespace
skipwhite:
	while( ( c = *data ) <= ' ' )
	{
		if( c == 0 )
			return nullptr;                    // end of file;
		data++;
	}

	// skip // comments
	if( c == '/' && data[ 1 ] == '/' )
	{
		while( *data && *data != '\n' )
			data++;
		goto skipwhite;
	}


	// handle quoted strings specially
	if( c == '\"' )
	{
		data++;
		while( 1 )
		{
			c = *data++;
			if( c == '\"' || !c )
			{
				com_token[ len ] = 0;
				return data;
			}
			com_token[ len ] = c;
			len++;
		}
	}

	// parse single characters
	if( c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ',' )
	{
		com_token[ len ] = c;
		len++;
		com_token[ len ] = 0;
		return data + 1;
	}

	// parse a regular word
	do
	{
		com_token[ len ] = c;
		data++;
		len++;
		c = *data;
		if( c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ',' )
			break;
	}
	while( c>32 );

	com_token[ len ] = 0;
	return data;
}

/*
==============
COM_TokenWaiting

Returns 1 if additional data is waiting to be processed on this line
==============
*/
int COM_TokenWaiting( char *buffer )
{
	char *p;

	p = buffer;
	while( *p && *p != '\n' )
	{
		if( !isspace( *p ) || isalnum( *p ) )
			return 1;

		p++;
	}

	return 0;
}