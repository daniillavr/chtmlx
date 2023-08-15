Simple html parser, and also generator. But for now can only beautify parsed html text.

Available functions:
```
readFile( FILE *fl )
     params:
             fl - opened file with text
     return:
             converted text to wide characters

** Convert text to wide characters text
html_parse( char_t *content )
     params:
             content - poitner to stream of bytes( html text )
     return:
             DOM tree

** Generate DOM from text
print_tree_to_file( struct node *nod , uint tabs , FILE *out )
     params:
             nod - root of tree
             tabs - number of tabs to put before tag( etc per level, formula = level of nesting * tabs )
             out - opened file for output
     return:
             nothing

** Outputing tree to file
free_nodes( struct node *nd )
     params:
             nd - root of tree
     return:
             0

** Clearing memory of tree
find_elems_by_tag( char_t *elem , struct node *nd , struct node *ret )
     params:
             elem - name of tag
             nd - root of tree
             ret - returning element with all found elements with given tag
     return
             List of elements by given tag

** find all elements with given tag elem and write them to childs of ret
find_elems_by_attr( char_t *name , char_t *value , struct node *nd , struct node *ret )
     params:
             name - name of attribute
             value - value of given attribute
             nd - root of tree
             ret - returning element with all found elements with given attribute
     return
             List of elements by given attribute name and its value

** find all elements with given attribute and write them to childs of ret. Value for now cannot be NULL, same for name. Will implement later.
find_elems_by_text( char_t *text , struct node *nd , struct node *ret )
     params:
             text - text to find in body if tag
             nd - root of tree
             ret - returning element with all found elements with text
     return
             List of elements by given text in element body

** find all elements with given text and write them to childs of ret
```
example of a program:
```
	#include <html.h>

	int main( int argc , char **argv )
	{
		struct  node    *root   ,
				*find   ;
		uint            iter    ;
		char_t          *text =
				L"<!DOCTYPE html><html><head><script ref=\"/here\"></script></head><body><div id=\"newbody\">Sample text</div></body></html>" ;

		root = html_parse( text ) ;

		find = calloc( 1 , sizeof( struct node ) ) ;
		find_elems_by_attr( L"id" , L"newbody" , root , find ) ;
		wprintf( L"%ls\n" ,  find->f_child->text ) ;

		free_nodes( root ) ;
		free_nodes( find ) ;

		return 0 ;
	}
```
output:
	Sample text
