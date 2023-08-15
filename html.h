#ifndef __HTML_PARSER__
#define __HTML_PARSER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

#define	uint		unsigned int
#define ull		unsigned long long
#define	TAB		L" "
#define	BF_SZ		( 1024 * 1024 )
#define SZ_ATTR_BUFF	( 1024 * 1024 )
#define WCH_SZ		sizeof( wchar_t )
#define char_t		wchar_t
#define TAGS_LEN	10
#define VT_SIZE		17
#define AT_SIZE		4

extern char_t Void_Tags[ VT_SIZE ][ TAGS_LEN ] ;
extern char_t Add_Tags[ AT_SIZE ][ TAGS_LEN ] ;

#define SIZE_VOID_TAGS ( sizeof( Void_Tags ) / ( WCH_SZ * TAGS_LEN ) ) 
#define SIZE_ADD_TAGS ( sizeof( Add_Tags ) / ( WCH_SZ * TAGS_LEN ) ) 

struct attribute
{
	char_t	*attr	,
		*value	;
} ;

struct parse_attribute
{
	char_t			*attr	,
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
	char_t			*tag		,
				*text		;
} ;

struct node *html_parse( char_t* ) ;

char_t *readfile( FILE * ) ;

int free_nodes( struct node* ) ;

void print_tree_to_file( struct node * , uint , FILE * ) ;
void find_elems_by_tag( char_t * , struct node * , struct node * ) ;
void find_elems_by_attr( char_t * , char_t * , struct node *root , struct node *ret ) ;
void find_elems_by_text( char_t * , struct node * , struct node * ) ;

#endif
