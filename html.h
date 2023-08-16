#ifndef __HTML_PARSER__
#define __HTML_PARSER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>

#define	uint		unsigned int
#define ull		unsigned long long
#define	TAB		" "
#define	BF_SZ		( 1024 * 1024 * 10 )
#define SZ_ATTR_BUFF	( 1024 * 1024 )
#define WCH_SZ		sizeof( wchar_t )
#define CH_SZ		sizeof( char )
#define char_t		wchar_t
#define TAGS_LEN	10
#define VT_SIZE		17
#define AT_SIZE		4

extern char Void_Tags[ VT_SIZE ][ TAGS_LEN ] ;
extern char Add_Tags[ AT_SIZE ][ TAGS_LEN ] ;

#define SIZE_VOID_TAGS ( sizeof( Void_Tags ) / ( CH_SZ * TAGS_LEN ) ) 
#define SIZE_ADD_TAGS ( sizeof( Add_Tags ) / ( CH_SZ * TAGS_LEN ) ) 

struct attribute
{
	char	*attr	,
		*value	;
} ;

struct parse_attribute
{
	char			*attr	,
				*value	;
	struct parse_attribute	*next	;
} ;

struct node
{
	uint			attr_number	;
	uint			child_number	;
	ull			n_parsed	;

	struct node		*next		,
				*prev		,
				*parent		,
				*f_child	,
				*l_child	;
	struct attribute	*attrs		;
	char			*tag		,
				*text		;
} ;

struct node *html_parse( char* ) ;

char *readfile( FILE * ) ;

int free_nodes( struct node* ) ;

void print_tree_to_file( struct node * , uint , FILE * ) ;
void find_elems_by_tag( char * , struct node * , struct node * ) ;
void find_elems_by_attr( char * , char * , struct node *root , struct node *ret ) ;
void find_elems_by_text( char * , struct node * , struct node * ) ;

#endif
