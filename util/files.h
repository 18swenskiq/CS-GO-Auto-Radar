// Find file path extension, returns NULL if no ext (0x00)
char *str_findext( char *szPath, char const delim )
{
	char *c, *ptr ;

	c = szPath;
	ptr = NULL;
	
	while( *c )
	{
		if( *c == delim )
		{
			ptr = c + 1;
		}
	
		c ++;
	}

	return ptr;
}

// gets rid of extension on string only left with folder/filename
void path_stripext( char *szPath )
{
	char *point, *start;
	
	// Skip folders
	if( !(start = str_findext( szPath, '/' )) )
	{
		start = szPath;
	}
	
	if( (point = str_findext( start, '.' )) )
	{
		if( point > szPath )
		{
			*(point-1) = 0x00;
		}
	}
}

// Convert windows paths to unix-ish ( \something\\blahblah .. ) -> /something/blahblah/
void path_winunix( char *path )
{
	char *idx, *wr;
	wr = idx = path;
	
	while( *idx )
	{
		if( *idx == '\\' )
		{
			*idx = '/';
		}
		
		if( idx > path )
		{
			if( *(idx -1) == '/' && *idx == '/') idx ++;
		}
		
		*( wr ++ ) = *idx;
		
		idx ++;
	}
	
	*wr = 0x00;
}

int path_is_abs( char *path )
{
#ifdef _WIN32
	if( strlen( path ) < 2 ) return 0;
	return path[1] == ':';
#else
	return path[0] == '/';
#endif
}
