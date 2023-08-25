#ifndef __HTML_PARSER__
#define __HTML_PARSER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>

#define	uint			unsigned int
#define ull			unsigned long long
#define HTML_VT_SIZE		17
#define HTML_AT_SIZE		4
#define HTML_TAGS_LEN		10
#define HTML_CH_SZ		sizeof( char )

extern char Void_Tags[ HTML_VT_SIZE ][ HTML_TAGS_LEN ] ;
extern char Add_Tags[ HTML_AT_SIZE ][ HTML_TAGS_LEN ] ;

#define SIZE_VOID_TAGS ( sizeof( Void_Tags ) / ( HTML_CH_SZ * HTML_TAGS_LEN ) ) 
#define SIZE_ADD_TAGS ( sizeof( Add_Tags ) / ( HTML_CH_SZ * HTML_TAGS_LEN ) ) 

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

struct html_node
{
	uint			attr_number	;
	uint			child_number	;
	ull			n_parsed	;

	struct html_node	*next		,
				*prev		,
				*parent		,
				*f_child	,
				*l_child	;
	struct attribute	*attrs		;
	char			*tag		,
				*text		;
} ;

struct html_node *html_parse( char* ) ;

char *readfile( FILE * ) ;

int free_nodes( struct html_node* ) ;

void print_tree_to_file( struct html_node * , uint , FILE * ) ;
void find_elems_by_tag( char * , struct html_node * , struct html_node * ) ;
void find_elems_by_attr( char * , char * , struct html_node *root , struct html_node *ret ) ;
void find_elems_by_text( char * , struct html_node * , struct html_node * ) ;

#undef	uint
#undef ull

#endif
