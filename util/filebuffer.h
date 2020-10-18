typedef enum
{
	k_EFileSysResult_complete,
	k_EFileSysResult_notfound,
	k_EFileSysResult_pakerror,
	k_EFileSysResult_nomem
} EFileSysResult_t;

// Read entire file into memory
char *file_to_buffer( const char *strName, EFileSysResult_t *result )
{
	FILE *f = fopen( strName, "rb" );
	if( !f ){
		*result = k_EFileSysResult_notfound;
		return NULL;
	} // File not found
	
	fseek( f, 0, SEEK_END );
	int64_t fsize = ftell(f);
	fseek( f, 0, SEEK_SET );
	char *buf = malloc( fsize );
	
	if( !buf ) {
		*result = k_EFileSysResult_nomem;
		fclose( f );
		return NULL;
	}
	
	*result = k_EFileSysResult_complete;
	
	fread( buf, 1, fsize, f );
	fclose( f );
	return buf;
}

char *textfile_to_buffer( const char *strName, EFileSysResult_t *result )
{
	FILE *f = fopen( strName, "rb" );
	if( !f ){
		*result = k_EFileSysResult_notfound;
		return NULL;
	} // File not found
	
	fseek( f, 0, SEEK_END );
	int64_t fsize = ftell(f);
	fseek( f, 0, SEEK_SET );
	char *buf = malloc( fsize +1 );
	
	if( !buf ) {
		*result = k_EFileSysResult_nomem;
		fclose( f );
		return NULL;
	}

	*result = k_EFileSysResult_complete;
	
	fread( buf, 1, fsize, f );
	fclose( f );
	
	buf[ fsize ] = 0x00;
	return buf;
}
