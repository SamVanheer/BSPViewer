#ifndef UTILITY_TOKENIZATION_H
#define UTILITY_TOKENIZATION_H

#define MAX_COM_TOKEN 1500

extern char com_token[ MAX_COM_TOKEN ];

//TODO: tidy the parse code, make it use user provided buffers - Solokiller
char *COM_Parse( char *data );

int COM_TokenWaiting( char *buffer );

#endif //UTILITY_TOKENIZATION_H