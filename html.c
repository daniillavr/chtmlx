#include "html.h"

static	unsigned long long Serial = 0 ;

char_t	Void_Tags[ VT_SIZE ][ TAGS_LEN ] =
		{
			L"area" ,
			L"base" ,
			L"br" ,
			L"col" ,
			L"command" ,
			L"embed" ,
			L"hr" ,
			L"img" ,
			L"input" ,
			L"keygen" ,
			L"link" ,
			L"meta" ,
			L"param" ,
			L"source" ,
			L"track" ,
			L"wbr" ,
			L"!DOCTYPE" ,
		} ;

char_t	Add_Tags[ AT_SIZE ][ TAGS_LEN ] =
		{
			L"math" ,
			L"script" ,
			L"svg" ,
			L"style" ,
		} ;


// readFile( FILE *fl )
// 	params:
//	 	fl - opened file with text
// 	return:
// 		converted text to wide characters
//
// ** Convert text to wide characters text
char_t *
readfile( FILE *fl )
{
	char_t	*bf	,
		*stt	;
	uint	cnt	;

	setlocale( LC_ALL , "en_US.utf8" ) ;
	cnt = 0 ;

	for( ; !feof( fl ) && !ferror( fl ) ; fgetwc( fl ) , ++cnt ) ;

	bf = malloc( WCH_SZ * ( cnt + 1 ) ) ;
	bf[ cnt ] = L'\0' ;

	stt = bf ;

	fseek( fl , 0L , SEEK_SET ) ;

	for( ; cnt > 0 ; ++bf , --cnt )
		*bf = fgetwc( fl ) ;
		
	return stt ;
}

//readText( char **text , char_t **stream , char typeRead , char_t *tag )
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
static char_t *
readText( char_t **text , char_t **stream , char typeRead , char_t *tag )
{
	uint		size		,
			count		;
	char_t		*ret		,
			*buffer		,
			*pTag		; // Parsed tag

	buffer = malloc( WCH_SZ * BF_SZ ) ;

	if( typeRead & 0x1 )
	{
		count = 0 ;
		size = wcslen( tag ) ;
		pTag = *stream ;

		for( ; **stream != L'\0' && wcsncmp( pTag , tag , size ) ; )
		{
			if( pTag != *stream && **stream == L'<' )
				buffer[ count++ ] = *( *stream )++ ;

			for( ; **stream != L'<' && **stream != L'\0' ; ( *stream )++ )
				if( iswprint( **stream ) )
					buffer[ count++ ] = **stream ;

			pTag = *stream ;
			++pTag ;
			++pTag ;
		}
		for( ; *pTag != L'\0' && *pTag != L'>' ; ++pTag ) ;
		++pTag ;
		*stream = pTag ;
	}
	else if( typeRead & 0x2 )
	{
		count = 0 ;
		size = wcslen( tag ) ;

		for( ; **stream != L'\0' && wcsncmp( *stream , tag , size ) ; )
		{
			if( **stream == L'-' )
				buffer[ count++ ] = *( *stream )++ ;

			for( ; **stream != L'-' && **stream != L'\0' ; ( *stream )++ )
				if( iswprint( **stream ) )
					buffer[ count++ ] = **stream ;
		}
	}
	else
		for( count = 0 ; **stream != L'<' && **stream != L'\0' ; ( *stream )++ )
			if( iswprint( **stream ) )
				buffer[ count++ ] = **stream ;


	if( !count )
	{
		free( buffer ) ;
		return NULL ;
	}

	size = 0 ;

	if( *text )
	{
		size = wcslen( *text ) ;
		ret = malloc( WCH_SZ * ( size + count + 2 ) ) ;
		wcscpy( ret , *text ) ;

		free( *text ) ;
		*text = ret ;

		( *text )[ size ] = L' ' ;
		memcpy( ( *text ) + size + 1 , buffer , WCH_SZ * count ) ;
		( *text )[ count + size + 1 ] = L'\0' ;
	}
	else
	{
		if( ( ret = calloc( count + 2 , WCH_SZ ) ) == NULL )
		{
			fwprintf( stderr , L"readText: Can't allocate %d bytes for buffer.\n" , count ) ;
			exit( 0 ) ;
		}

		*text = ret ;

		memcpy( ( *text ) + size  , buffer , WCH_SZ * count ) ;
		( *text )[ count + size + 1 ] = L'\0' ;
	}

	free( buffer ) ;

	return *text ;
}


// compTag( char_t *in , char_t *list , int size )
// 	params:
// 		in - tag for compare
// 		list - list of tags for compared with in
// 		size - size of list tags
// 	return
// 		if this tag in a list, then 1
// 		else 0
//
// ** compare one tag with tags from list
static char_t
compTag( char_t *in , char_t *list , int size )
{
	uint	sz1	,
		i	;

	sz1 = wcslen( in ) ;

	for( i = 0 ; i < size ; ++i )
		if( sz1 == wcslen( &list[ i * TAGS_LEN ] ) && !wcscmp( in , &list[ i * TAGS_LEN ] ) )
			return 1 ;
	return 0 ;
}

// wcsnac( char_t **dest , char_t *source , uint size )
// 	params:
// 		dest - pointer for allocating memory
// 		source - source to copying
// 		size - length of a string
// 	return:
// 		pointer to dest
//
// ** Allocate memory for dest and copy string from source to dest
char_t *
wcsnac( char_t **dest , char_t *source , uint size )
{
	if( !size )
		*dest = NULL ;
	{
		*dest = malloc( WCH_SZ * ( size + 1 ) ) ;
		memcpy( *dest , source , size * WCH_SZ ) ;
		(*dest )[ size ] = L'\0' ;
	}

	return *dest ;
}

// parse_attributes( char_t **content , struct attriute **attrs )
// 	params:
// 		content - poitner to stream of bytes( html text )
// 		attrs - pointer to attributes of current html element
// 	return:
// 		number of attributes parsed
//
// ** Parse attributes for current html element
static uint
parse_attributes( char_t **content , struct attribute **attrs )
{
	struct parse_attribute	*iterAttrs	,
				*headAttrs	;
	uint			ui_attr_cnt	,
				ui_cntr		,
				ui_iter		;
	char_t			*c_buffer	,
				quotes[ ]	=
				L" '\""		;
	
	c_buffer = malloc( WCH_SZ * SZ_ATTR_BUFF ) ;
	headAttrs = iterAttrs = calloc( 1 , sizeof( struct parse_attribute ) ) ;
	
	ui_attr_cnt = 0 ;
	*attrs = NULL ;


	while( **content != L'>' )
	{
		for( ; **content == L' ' ; ++( *content ) ) ;
		for( ui_cntr = 0 ; **content != L'>' && **content != L'=' && **content != L' ' ;  ++( *content ) , ++ui_cntr )
			c_buffer[ ui_cntr ] = **content ;

		wcsnac( &iterAttrs->attr , c_buffer , ui_cntr ) ;

		if( **content == L'=' )
		{
			++( *content ) ;
			
			for( ui_iter = 2 ; ui_iter && **content != quotes[ ui_iter ] ; --ui_iter ) ;

			if( ui_iter )
				( *content )++ ;

			for( ui_cntr = 0 ; **content != quotes[ ui_iter ] ;  ++( *content ) , ++ui_cntr )
				c_buffer[ ui_cntr ] = **content ;

			if( ui_iter )
				( *content )++ ;

			wcsnac( &iterAttrs->value , c_buffer , ui_cntr ) ;
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
			ui_cntr = wcslen( iterAttrs->attr ) ;
			wcsnac( &( *attrs )[ ui_iter ].attr , iterAttrs->attr , ui_cntr ) ;

			if( iterAttrs->value )
			{
				ui_cntr = wcslen( iterAttrs->value ) ;
				wcsnac( &( *attrs )[ ui_iter ].value , iterAttrs->value , ui_cntr ) ;
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

// parse_tag( char_t **stream , char_t buffer , uint size )
// 	params:
// 		stream - poitner to stream of bytes( html text )
// 		buffer - point to buffer, where to write tag name
// 		size - size of buffer
// 	return:
// 		count of symbols of tag name
//
// ** Parse name of tag
static uint
parse_tag( char_t **stream , char_t *buffer , uint size )
{
	uint	i	;

	if( **stream != L'<' )
		return 0 ;
	
	(*stream)++ ;
	i = 0 ;

	if( **stream == L'/' || **stream == '!' )
		buffer[ i++ ] = *( ( *stream )++ ) ;
	
	if( iswalpha( **stream ) )
		for( ; i < size && **stream != L'\0' && iswalpha( **stream ) ; (*stream)++ )
			buffer[ i++ ] = **stream ;
	else
		for( ; i < size && i < 3 && **stream != L'\0' ; (*stream)++ )
			buffer[ i++ ] = **stream ;
	return i ;
}

// html_parse( char_t *content )
// 	params:
// 		content - poitner to stream of bytes( html text )
// 	return:
// 		DOM tree
//
// ** Generate DOM from text
struct node *
html_parse( char_t *content )
{
	struct node	*node_root	,
			*node_curr	,
			*node_temp	;
	uint		i_cntr		;
	char_t		*c_buffer	;

	c_buffer = calloc( 1024 , WCH_SZ ) ;

	node_curr = node_root = calloc( 1 , sizeof(  struct node ) )	;

	for( ; *content != L'\0' ; )
	{
		for( ; *content != L'<' && *content != L'\0' ; ++content ) ;
		i_cntr = parse_tag( &content , c_buffer , WCH_SZ * 1024 ) ; 

		if( i_cntr )
		{
			if( *c_buffer == L'/' && node_curr && node_curr->tag && !wcsncmp( &c_buffer[ 1 ] , node_curr->tag , i_cntr - 1 ) )
			{
#ifdef DEBUG
				fwprintf( stderr , L"%d: %ls closed.\n" , node_curr->n_parsed , node_curr->tag ) ;
#endif
				if( node_curr->parent )
					node_curr = node_curr->parent ;
			}
			else
			{
				node_temp = calloc( 1 , sizeof( struct node ) ) ;
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

				wcsnac( &node_temp->tag , c_buffer , i_cntr ) ;
#ifdef DEBUG
				fwprintf( stderr , L"%d: %ls opened.\n" , node_temp->n_parsed , node_temp->tag ) ;
#endif
				if( !wcsncmp( node_temp->tag , L"!--" , 3 ) )
#ifdef DEBUG
				{
#endif
					readText( &node_temp->text ,  &content , 2 , L"-->" ) ;
#ifdef DEBUG
					fwprintf( stderr , L"%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
				}
#endif
				else
				{
					node_temp->attr_number = parse_attributes( &content , &node_temp->attrs ) ;
					node_curr = node_temp ;
					
					for( ; *content != L'>' ; ++content ) ;
					++content ;
		
					if( compTag( node_temp->tag , ( char_t* )Void_Tags , SIZE_VOID_TAGS ) )
#ifdef DEBUG
					{
#endif
						node_curr = node_curr->parent ;
#ifdef DEBUG
						fwprintf( stderr , L"%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
					}
#endif
					else
					{		
						if( compTag( node_curr->tag , ( char_t* )Add_Tags , SIZE_ADD_TAGS ) )
						{
							readText( &node_curr->text , &content , 1 , node_curr->tag ) ;
							node_curr = node_curr->parent ;
#ifdef DEBUG
							fwprintf( stderr , L"%d: %ls closed.\n" , node_temp->n_parsed , node_temp->tag ) ;
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

// print_tree_to_file( struct node *nod , uint tabs , FILE *out )
// 	params:
// 		nod - root of tree
// 		tabs - number of tabs to put before tag( etc per level, formula = level of nesting * tabs )
// 		out - opened file for output
// 	return:
// 		nothing
//
// ** Outputing tree to file
void
print_tree_to_file( struct node *nod , uint tabs , FILE *out )
{
	
	setlocale( LC_ALL , "en_US.utf8" ) ;

	if( !nod )
		return ;

	for( uint iter = 0 ; iter < tabs ; ++iter )
		fwprintf( out , TAB ) ;

	if( nod->parent )
	{
#ifdef DEBUG
		fwprintf( out , L"%Lu: " , nod->n_parsed ) ;
#endif
		fwprintf( out , L"<%ls" , nod->tag ) ;

		for( uint iter = 0 ; iter < nod->attr_number ; ++iter )
			fwprintf( out , L" %ls=\"%ls\"" , nod->attrs[ iter ].attr , nod->attrs[ iter ].value ) ;
		
		if( wcscmp( nod->tag , L"!--" ) )
			fwprintf( out , L">\n" ) ;

		if( nod->text )
		{	
			if( wcscmp( nod->tag , L"!--" ) )
			{
				for( uint iter = 0 ; iter < tabs ; ++iter )
					fwprintf( out , TAB ) ;
				fwprintf( out , L"'%ls'\n" , nod->text ) ;
			}
			else
				fwprintf( out , L"'%ls'" , nod->text ) ;
		}

		if( !wcscmp( nod->tag , L"!--" ) )
			fwprintf( out , L"-->\n" ) ;
	}
	else
		tabs = -1 ;

	if( nod->child_number )
	{
		print_tree_to_file( nod->f_child , tabs + 1 , out ) ;

		struct node *tmp = nod->f_child->next ;
		for( uint iter = 1 ; iter < nod->child_number ; ++iter , tmp = tmp->next )
			print_tree_to_file( tmp , tabs + 1 , out ) ;
	}

	if( nod->parent && wcscmp( nod->tag , L"!--" ) )
	{
		for( uint iter = 0 ; iter < tabs ; ++iter )
			fwprintf( out , TAB ) ;
#ifdef DEBUG
		fwprintf( out , L"%Lu: " , nod->n_parsed ) ;
#endif

		if( wcscmp( nod->tag , L"!--" ) )
			fwprintf( out , L"</%ls>\n" , nod->tag ) ;
	}

	return ;

}

// free_nodes( struct node *nd )
// 	params:
// 		nd - root of tree
// 	return:
// 		0
//
// ** Clearing memory of tree
int
free_nodes( struct node* nd )
{
	struct node	*chld	,
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

// addChild( struct node *root , struct node *child )
// 	params:
// 		root - parent
// 		child - an element to add to parent
// 	return:
// 		1
//
// ** Adding an html element to parent( copying everything, included with children of child )
static uint
addChild( struct node *root , struct node *child )
{
	struct node	*tmp	,
			*n_i	;
	uint		i	;

	root->child_number++ ;
	tmp = calloc( 1 , sizeof( struct node ) ) ;
	
	tmp->attr_number = child->attr_number ;
	tmp->n_parsed = child->n_parsed ;

	for( n_i = child->f_child ; n_i ; n_i = n_i->next )
		addChild( tmp , n_i ) ;

	wcsnac( &tmp->tag , child->tag , wcslen( child->tag ) ) ;

	if( child->text )
		wcsnac( &tmp->text , child->text , wcslen( child->text ) ) ;

	if( child->attr_number )
	{
		tmp->attrs = malloc( sizeof( struct attribute ) * child->attr_number ) ;

		for( i = 0 ; i < child->attr_number ; ++i )
		{
			wcsnac( &tmp->attrs[ i ].attr , child->attrs[ i ].attr , wcslen( child->attrs[ i ].attr ) ) ;

			if( child->attrs[ i ].value )
				wcsnac( &tmp->attrs[ i ].value , child->attrs[ i ].value , wcslen( child->attrs[ i ].value ) ) ;
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

// find_elems_by_tag( char_t *elem , struct node *nd , struct node *ret )
//	params:
//		elem - name of tag
//		nd - root of tree
//		ret - returning element with all found elements with given tag
//	return
//		nothing
//
// ** find all elements with given tag elem and write them to childs of ret
void
find_elems_by_tag( char_t *elem , struct node *nd , struct node *ret )
{
	struct node	*chld	;

	if( nd->tag && !wcscmp( nd->tag , elem ) )
		addChild( ret , nd ) ;
	

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_tag( elem , chld , ret ) ;

	return ;
}

// find_elems_by_attr( char_t *name , char_t *value , struct node *nd , struct node *ret )
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
find_elems_by_attr( char_t *name , char_t *value , struct node *nd , struct node *ret )
{
	struct node	*chld	;

	int		i	,
			c	;

	if( !nd )
		return ;

	c = 0 ;

	if( nd->attr_number )
		for( i = 0 ; !c && i < nd->attr_number ; ++i )
			if( ( name && !wcscmp( nd->attrs[ i ].attr , name ) ) || !name )
				if( ( value && wcsstr( nd->attrs[ i ].value , value ) ) || !value )
					++c ;
	if( c )
		addChild( ret , nd ) ;

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_attr( name , value , chld , ret ) ;

	return ;
}

// find_elems_by_text( char_t *text , struct node *nd , struct node *ret )
//	params:
//		text - text to find in body if tag
//		nd - root of tree
//		ret - returning element with all found elements with text 
//	return
//		nothing
//
// ** find all elements with given text and write them to childs of ret
void
find_elems_by_text( char_t *text , struct node *nd , struct node *ret )
{
	struct node	*chld	;

	if( nd->text && wcsstr( nd->text , text ) )
		addChild( ret , nd ) ;

	if( nd->child_number )
		for( chld = nd->f_child ; chld ; chld = chld->next )
			find_elems_by_text( text , chld , ret ) ;

	return ;
}
