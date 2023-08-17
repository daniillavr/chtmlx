#include "html.h"

#define	TAB		" "
#define	BF_SZ		( 1024 * 1024 * 10 )
#define SZ_ATTR_BUFF	( 1024 * 1024 )
#define WHTML_CH_SZ		sizeof( wchar_t )
#define char_t		wchar_t

static	unsigned long long Serial = 0 ;

char	Void_Tags[ HTML_VT_SIZE ][ HTML_TAGS_LEN ] =
		{
			"area" ,
			"base" ,
			"br" ,
			"co" ,
			"command" ,
			"embed" ,
			"hr" ,
			"img" ,
			"input" ,
			"keygen" ,
			"link" ,
			"meta" ,
			"param" ,
			"source" ,
			"track" ,
			"wbr" ,
			"!DOCTYPE" ,
		} ;

char	Add_Tags[ HTML_AT_SIZE ][ HTML_TAGS_LEN ] =
		{
			"math" ,
			"script" ,
			"sv1" ,
			"style" ,
		} ;


// readFile( FILE *fl )
// 	params:
//	 	fl - opened file with text
// 	return:
// 		text from file
//
//
char *
readfile( FILE *fl )
{
	char	*bf	,
		*stt	;
	uint	cnt	;

	setlocale( LC_ALL , "en_US.utf8" ) ;
	cnt = 0 ;

	for( ; !feof( fl ) && !ferror( fl ) ; fgetc( fl ) , ++cnt ) ;

	bf = malloc( HTML_CH_SZ * ( cnt + 1 ) ) ;
	bf[ cnt ] = '\0' ;

	stt = bf ;

	fseek( fl , 0L , SEEK_SET ) ;

	for( ; cnt > 0 ; ++bf , --cnt )
		*bf = fgetc( fl ) ;
		
	return stt ;
}


//readText( char **text , char **stream , char typeRead , char *tag )
//	params:
//		text - pointer to memory, where need to be placed result of readed text of body of tag
//		stream - poitner to stream of bytes( html text )
//		typeRead - [
//				0x0 - read any other tag ,
//				0x1 - read Additional Tags ,
//				0x2 - read comment 
//			}
//		tag - if chosen 0x1 typeRead, then tag is a closed tag, after what we need to stop
//	return:
//		pointer to text ;
//
// ** Read body of a tag( text within >< )
static char *
readText( char **text , char **stream , char typeRead , char *tag )
{
	uint		size		,
			count		;
	char		*ret		,
			*buffer		,
			*pTag		; // Parsed tag

	buffer = malloc( HTML_CH_SZ * BF_SZ ) ;

	if( typeRead & 0x1 )
	{
		count = 0 ;
		size = strlen( tag ) ;
		pTag = *stream ;

		for( ; **stream != '\0' && strncmp( pTag , tag , size ) ; )
		{
			if( pTag != *stream && **stream == '<' )
				buffer[ count++ ] = *( *stream )++ ;

			for( ; **stream != '<' && **stream != '\0' ; ( *stream )++ )
				if( isprint( **stream ) )
					buffer[ count++ ] = **stream ;

			pTag = *stream ;
			++pTag ;
			++pTag ;
		}
		for( ; *pTag != '\0' && *pTag != '>' ; ++pTag ) ;
		++pTag ;
		*stream = pTag ;
	}
	else if( typeRead & 0x2 )
	{
		count = 0 ;
		size = strlen( tag ) ;

		for( ; **stream != '\0' && strncmp( *stream , tag , size ) ; )
		{
			if( **stream == '-' )
				buffer[ count++ ] = *( *stream )++ ;

			for( ; **stream != '-' && **stream != '\0' ; ( *stream )++ )
				if( isprint( **stream ) )
					buffer[ count++ ] = **stream ;
		}
	}
	else
		for( count = 0 ; **stream != '<' && **stream != '\0' ; ( *stream )++ )
			if( isprint( **stream ) )
				buffer[ count++ ] = **stream ;


	if( !count )
	{
		free( buffer ) ;
		return NULL ;
	}

	size = 0 ;

	if( *text )
	{
		size = strlen( *text ) ;
		ret = malloc( HTML_CH_SZ * ( size + count + 2 ) ) ;
		strcpy( ret , *text ) ;

		free( *text ) ;
		*text = ret ;

		( *text )[ size ] = ' ' ;
		memcpy( ( *text ) + size + 1 , buffer , HTML_CH_SZ * count ) ;
		( *text )[ count + size + 1 ] = '\0' ;
	}
	else
	{
		if( ( ret = calloc( count + 2 , HTML_CH_SZ ) ) == NULL )
		{
			fprintf( stderr , "readText: Can't allocate %d bytes for buffer.\n" , count ) ;
			exit( 0 ) ;
		}

		*text = ret ;

		memcpy( ( *text ) + size  , buffer , HTML_CH_SZ * count ) ;
		( *text )[ count + size + 1 ] = '\0' ;
	}

	free( buffer ) ;

	return *text ;
}


// compTag( char *in , char *list , int size )
// 	params:
// 		in - tag for compare
// 		list - list of tags for compared with in
// 		size - size of list tags
// 	return
// 		if this tag in a list, then 1
// 		else 0
//
// ** compare one tag with tags from list
static char
compTag( char *in , char *list , int size )
{
	uint	sz1	,
		i	;

	sz1 = strlen( in ) ;

	for( i = 0 ; i < size ; ++i )
		if( sz1 == strlen( &list[ i * HTML_TAGS_LEN ] ) && !strcmp( in , &list[ i * HTML_TAGS_LEN ] ) )
			return 1 ;
	return 0 ;
}

// strnac( char **dest , char *source , uint size )
// 	params:
// 		dest - pointer for allocating memory
// 		source - source to copying
// 		size - length of a string
// 	return:
// 		pointer to dest
//
// ** Allocate memory for dest and copy string from source to dest
char *
strnac( char **dest , char *source , uint size )
{
	if( !size )
		*dest = NULL ;
	{
		*dest = malloc( HTML_CH_SZ * ( size + 1 ) ) ;
		memcpy( *dest , source , size * HTML_CH_SZ ) ;
		(*dest )[ size ] = '\0' ;
	}

	return *dest ;
}

// parse_attributes( char **content , struct attriute **attrs )
// 	params:
// 		content - poitner to stream of bytes( html text )
// 		attrs - pointer to attributes of current html element
// 	return:
// 		number of attributes parsed
//
// ** Parse attributes for current html element
static uint
parse_attributes( char **content , struct attribute **attrs )
{
	struct parse_attribute	*iterAttrs	,
				*headAttrs	;
	uint			ui_attr_cnt	,
				ui_cntr		,
				ui_iter		;
	char			*c_buffer	,
				quotes[ ]	=
				" '\""		;
	
	c_buffer = malloc( HTML_CH_SZ * SZ_ATTR_BUFF ) ;
	headAttrs = iterAttrs = calloc( 1 , sizeof( struct parse_attribute ) ) ;
	
	ui_attr_cnt = 0 ;
	*attrs = NULL ;


	while( **content != '>' )
	{
		for( ; **content == ' ' ; ++( *content ) ) ;
		for( ui_cntr = 0 ; **content != '>' && **content != '=' && **content != ' ' ;  ++( *content ) , ++ui_cntr )
			c_buffer[ ui_cntr ] = **content ;

		strnac( &iterAttrs->attr , c_buffer , ui_cntr ) ;

		if( **content == '=' )
		{
			++( *content ) ;
			
			for( ui_iter = 2 ; ui_iter && **content != quotes[ ui_iter ] ; --ui_iter ) ;

			if( ui_iter )
				( *content )++ ;

			for( ui_cntr = 0 ; **content != quotes[ ui_iter ] ;  ++( *content ) , ++ui_cntr )
				c_buffer[ ui_cntr ] = **content ;

			if( ui_iter )
				( *content )++ ;

			strnac( &iterAttrs->value , c_buffer , ui_cntr ) ;
		}
		else
			iterAttrs->value = NULL ;

		iterAttrs = iterAttrs->next = calloc( 1 , sizeof( struct parse_attribute ) ) ;
		
		++ui_attr_cnt ;
	}

	free( c_buffer ) ;

	if( ui_attr_cnt )
	{
		*attrs = malloc( sizeof( struct attribute ) * ui_attr_cnt ) ;

		for( iterAttrs = headAttrs , ui_iter = 0 ; ui_iter < ui_attr_cnt ; ++ui_iter , iterAttrs = iterAttrs->next )
		{
			ui_cntr = strlen( iterAttrs->attr ) ;
			strnac( &( *attrs )[ ui_iter ].attr , iterAttrs->attr , ui_cntr ) ;

			if( iterAttrs->value )
			{
				ui_cntr = strlen( iterAttrs->value ) ;
				strnac( &( *attrs )[ ui_iter ].value , iterAttrs->value , ui_cntr ) ;
			}
			else
				( *attrs )[ ui_iter ].value = NULL ;
				
		}
	}

	while( headAttrs )
	{
		iterAttrs = headAttrs->next ;
		free( headAttrs->attr ) ;
		if( headAttrs->value )
			free( headAttrs->value ) ;
		free( headAttrs ) ;
		headAttrs = iterAttrs ;
	}

	return ui_attr_cnt ;
}

// parse_tag( char **stream , char buffer , uint size )
// 	params:
// 		stream - poitner to stream of bytes( html text )
// 		buffer - point to buffer, where to write tag name
// 		size - size of buffer
// 	return:
// 		count of symbols of tag name
//
// ** Parse name of tag
static uint
parse_tag( char **stream , char *buffer , uint size )
{
	uint	i	;

	if( **stream != '<' )
		return 0 ;
	
	(*stream)++ ;
	i = 0 ;

	if( **stream == '/' || **stream == '!' )
		buffer[ i++ ] = *( ( *stream )++ ) ;
	
	if( isalnum( **stream ) )
		for( ; i < size && **stream != '\0' && ( isalnum( **stream ) || **stream == '-' ) ; (*stream)++ )
			buffer[ i++ ] = **stream ;
	else
		for( ; i < size && i < 3 && **stream != '\0' ; (*stream)++ )
			buffer[ i++ ] = **stream ;
	return i ;
}

// html_parse( char *content )
// 	params:
// 		content - poitner to stream of bytes( html text )
// 	return:
// 		DOM tree
//
// ** Generate DOM from text
struct html_node *
html_parse( char *content )
{
	struct html_node	*node_root	,
				*node_curr	,
				*node_temp	;
	uint			i_cntr		;
	char			*c_buffer	;

	c_buffer = calloc( 1024 , HTML_CH_SZ ) ;

	node_curr = node_root = calloc( 1 , sizeof(  struct html_node ) )	;

	for( ; *content != '\0' ; )
	{
		for( ; *content != '<' && *content != '\0' ; ++content ) ;
		i_cntr = parse_tag( &content , c_buffer , HTML_CH_SZ * 1024 ) ; 

		if( i_cntr )
		{
			if( *c_buffer == '/' && node_curr && node_curr->tag && !strncmp( &c_buffer[ 1 ] , node_curr->tag , i_cntr - 1 ) )
			{
#ifdef DEBUG
				fprintf( stderr , "%d: %ls closed.\n" , node_curr->n_parsed , node_curr->tag ) ;
#endif
				if( node_curr->parent )
					node_curr = node_curr->parent ;
			}
			else
			{
				node_temp = calloc( 1 , sizeof( struct html_node ) ) ;
				node_temp->parent = node_curr ;
				node_temp->n_parsed = Serial++ ;
				
				if( node_curr->child_number )
				{
					node_curr->l_child->next = node_temp ;
					node_temp->prev = node_curr->l_child ;
					node_curr->l_child = node_temp ;
				}
				else
					node_curr->l_child = node_curr->f_child = node_temp ;
				node_curr->child_number++ ;

				strnac( &node_temp->tag , c_buffer , i_cntr ) ;
#ifdef DEBUG
				fprintf( stderr , "%d: %ls opened.\n" , node_temp->n_parsed , node_temp->tag ) ;
#endif
				if( !strncmp( node_temp->tag , "!--" , 3 ) )
#ifdef DEBUG
				{
#endif
					readText( &node_temp->text ,  &content , 2 , "-->" ) ;
#ifdef DEBUG
					fprintf( stderr , "%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
				}
#endif
				else
				{
					node_temp->attr_number = parse_attributes( &content , &node_temp->attrs ) ;
					node_curr = node_temp ;
					
					for( ; *content != '>' ; ++content ) ;
					++content ;
		
					if( compTag( node_temp->tag , ( char* )Void_Tags , SIZE_VOID_TAGS ) )
#ifdef DEBUG
					{
#endif
						node_curr = node_curr->parent ;
#ifdef DEBUG
						fprintf( stderr , "%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
					}
#endif
					else
					{		
						if( compTag( node_curr->tag , ( char* )Add_Tags , SIZE_ADD_TAGS ) )
						{
							readText( &node_curr->text , &content , 1 , node_curr->tag ) ;
							node_curr = node_curr->parent ;
#ifdef DEBUG
							fprintf( stderr , "%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
#endif
						}
						else
							readText( &node_curr->text , &content , 0 , NULL ) ;
					}
				}

			}
		}
	}

	free( c_buffer ) ;

	return node_root ;
}

// print_tree_to_file( struct html_node *nod , uint tabs , FILE *out )
// 	params:
// 		nod - root of tree
// 		tabs - number of tabs to put before tag( etc per level, formula = level of nesting * tabs )
// 		out - opened file for output
// 	return:
// 		nothing
//
// ** Outputing tree to file
void
print_tree_to_file( struct html_node *nod , uint tabs , FILE *out )
{
	
	setlocale( LC_ALL , "en_US.utf8" ) ;

	if( !nod )
		return ;

	for( uint iter = 0 ; iter < tabs ; ++iter )
		fprintf( out , TAB ) ;

	if( nod->parent )
	{
#ifdef DEBUG
		fprintf( out , "%Lu: " , nod->n_parsed ) ;
#endif
		fprintf( out , "<%s" , nod->tag ) ;

		for( uint iter = 0 ; iter < nod->attr_number ; ++iter )
			fprintf( out , " %s=\"%s\"" , nod->attrs[ iter ].attr , nod->attrs[ iter ].value ) ;
		
		if( strcmp( nod->tag , "!--" ) )
			fprintf( out , ">\n" ) ;

		if( nod->text )
		{	
			if( strcmp( nod->tag , "!--" ) )
			{
				for( uint iter = 0 ; iter < tabs ; ++iter )
					fprintf( out , TAB ) ;
				fprintf( out , "'%s'\n" , nod->text ) ;
			}
			else
				fprintf( out , "'%s'" , nod->text ) ;
		}

		if( !strcmp( nod->tag , "!--" ) )
			fprintf( out , "-->\n" ) ;
	}
	else
		tabs = -1 ;

	if( nod->child_number )
	{
		print_tree_to_file( nod->f_child , tabs + 1 , out ) ;

		struct html_node *tmp = nod->f_child->next ;
		for( uint iter = 1 ; iter < nod->child_number ; ++iter , tmp = tmp->next )
			print_tree_to_file( tmp , tabs + 1 , out ) ;
	}

	if( nod->parent && strcmp( nod->tag , "!--" ) )
	{
		for( uint iter = 0 ; iter < tabs ; ++iter )
			fprintf( out , TAB ) ;
#ifdef DEBUG
		fprintf( out , "%Lu: " , nod->n_parsed ) ;
#endif

		if( strcmp( nod->tag , "!--" ) )
			fprintf( out , "</%s>\n" , nod->tag ) ;
	}

	return ;

}

// free_nodes( struct html_node *nd )
// 	params:
// 		nd - root of tree
// 	return:
// 		0
//
// ** Clearing memory of tree
int
free_nodes( struct html_node* nd )
{
	struct html_node	*chld	,
			*tmp	;
	int		i	;

	chld = nd->l_child ;

	for( ; chld ; )
	{
		if( chld->child_number )
			free_nodes( chld ) ;

		tmp = chld->prev ;

		if( chld->tag )
			free( chld->tag ) ;

		if( chld->text )
			free( chld->text ) ;

		for( i = 0 ; i < chld->attr_number ; ++i )
		{
			free( chld->attrs[ i ].attr ) ;
			if( chld->attrs[ i ].value )
				free( chld->attrs[ i ].value ) ;
		}

		free( chld->attrs ) ;

		free( chld ) ;

		chld = tmp ;
	}

	if( !nd->parent )
		free( nd ) ;
	else
		nd->f_child = nd->l_child = NULL ;

	return 0 ;
}

// addChild( struct html_node *root , struct html_node *child )
// 	params:
// 		root - parent
// 		child - an element to add to parent
// 	return:
// 		1
//
// ** Adding an html element to parent( copying everything, included with children of child )
static uint
addChild( struct html_node *root , struct html_node *child )
{
	struct html_node	*tmp	,
			*n_i	;
	uint		i	;

	root->child_number++ ;
	tmp = calloc( 1 , sizeof( struct html_node ) ) ;
	
	tmp->attr_number = child->attr_number ;
	tmp->n_parsed = child->n_parsed ;

	for( n_i = child->f_child ; n_i ; n_i = n_i->next )
		addChild( tmp , n_i ) ;

	strnac( &tmp->tag , child->tag , strlen( child->tag ) ) ;

	if( child->text )
		strnac( &tmp->text , child->text , strlen( child->text ) ) ;

	if( child->attr_number )
	{
		tmp->attrs = malloc( sizeof( struct attribute ) * child->attr_number ) ;

		for( i = 0 ; i < child->attr_number ; ++i )
		{
			strnac( &tmp->attrs[ i ].attr , child->attrs[ i ].attr , strlen( child->attrs[ i ].attr ) ) ;

			if( child->attrs[ i ].value )
				strnac( &tmp->attrs[ i ].value , child->attrs[ i ].value , strlen( child->attrs[ i ].value ) ) ;
			else
				tmp->attrs[ i ].value = NULL ;
		}
	}

	if( !root->f_child )
		root->l_child = root->f_child = tmp ;
	else
	{
		root->l_child->next = tmp ;
		tmp->prev = root->l_child ;
		root->l_child = tmp ;
	}
	
	tmp->parent = root ;

	return 1 ;
}

// find_elems_by_tag( char *elem , struct html_node *nd , struct html_node *ret )
//	params:
//		elem - name of tag
//		nd - root of tree
//		ret - returning element with all found elements with given tag
//	return
//		nothing
//
// ** find all elements with given tag elem and write them to childs of ret
void
find_elems_by_tag( char *elem , struct html_node *nd , struct html_node *ret )
{
	struct html_node	*chld	;

	if( nd->tag && !strcmp( nd->tag , elem ) )
		addChild( ret , nd ) ;
	

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_tag( elem , chld , ret ) ;

	return ;
}

// find_elems_by_attr( char *name , char *value , struct html_node *nd , struct html_node *ret )
//	params:
//		name - name of attribute
//		value - value of given attribute
//		nd - root of tree
//		ret - returning element with all found elements with given attribute
//	return
//		nothing
//
// ** find all elements with given attribute and write them to childs of ret. Value for now cannot be NULL, same for name. Will implement later.
void
find_elems_by_attr( char *name , char *value , struct html_node *nd , struct html_node *ret )
{
	struct html_node	*chld	;

	int		i	,
			c	;

	if( !nd )
		return ;

	c = 0 ;

	if( nd->attr_number )
		for( i = 0 ; !c && i < nd->attr_number ; ++i )
			if( ( name && !strcmp( nd->attrs[ i ].attr , name ) ) || !name )
				if( ( value && strstr( nd->attrs[ i ].value , value ) ) || !value )
					++c ;
	if( c )
		addChild( ret , nd ) ;

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_attr( name , value , chld , ret ) ;

	return ;
}

// find_elems_by_text( char *text , struct html_node *nd , struct html_node *ret )
//	params:
//		text - text to find in body if tag
//		nd - root of tree
//		ret - returning element with all found elements with text 
//	return
//		nothing
//
// ** find all elements with given text and write them to childs of ret
void
find_elems_by_text( char *text , struct html_node *nd , struct html_node *ret )
{
	struct html_node	*chld	;

	if( nd->text && strstr( nd->text , text ) )
		addChild( ret , nd ) ;

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_text( text , chld , ret ) ;

	return ;
}
