char fs_gamedir[ 512 ] = {0};
char fs_exedir[ 512 ] = {0};
char fs_bin[ 512 ] = {0};

VPKHeader_t *fs_vpk = NULL;
char **fs_searchpaths = NULL;

FILE *fs_current_archive = NULL;
uint16_t fs_current_idx = 0;

void fs_set_gameinfo( const char *path )
{
	vdf_node_t *info = vdf_open_file( path );
	if( !info ) return;
	
	// Set gamedir
	strcpy( fs_gamedir, path );
	path_winunix( fs_gamedir );
	*str_findext( fs_gamedir, '/' ) = 0x00;
	
	// Set exe dir
	strcpy( fs_exedir, fs_gamedir );
	strcat( fs_exedir, "../" );
	
	// Get all search paths from file
	vdf_node_t *spaths = VDF( VDF( VDF( info, "GameInfo" ), "FileSystem" ), "SearchPaths" );
	VDF_KV_ITER( spaths, "Game",
		if( KV[0] == '|' ) continue; //TODO: deal with engine replacements?? maybe??
	
		char *buf;
		if( path_is_abs( KV ) )
		{
			buf = malloc( strlen( KV ) + 2 );
			strcpy( buf, KV );
			strcat( buf, "/" );
		}
		else
		{
			buf = malloc( strlen( fs_exedir ) + strlen( KV ) + 2 );
			strcpy( buf, fs_exedir );
			strcat( buf, KV );
			strcat( buf, "/" );
		}
		
		sb_push( fs_searchpaths, buf );
	);
	
	vdf_free( info );
	
	// Look for pak01_dir
	char pack_path[512];
	for( int i = 0; i < sb_count( fs_searchpaths ); i ++ )
	{
		strcpy( pack_path, fs_searchpaths[i] );
		strcat( pack_path, "pak01_dir.vpk" );
	
		if( (fs_vpk = vpk_read( pack_path )) )
		{
			break;
		}
	}
	
	if( !fs_vpk )
	{
		fprintf( stderr, "Could not locate pak01_dir.vpk in %i searchpaths", sb_count( fs_searchpaths ) );
	}

	printf( "fs_info:\n\
  gamedir: %s\n\
   exedir: %s\n\
      bin: %s\n\
     pack: %s\n\
  searchpaths:\n", fs_gamedir, fs_exedir, fs_bin, pack_path );
  
	for( int i = 0; i < sb_count( fs_searchpaths ); i ++ )
	{
		printf( "    %s\n", fs_searchpaths[i] );  
	}
}

void fs_exit(void)
{
	for( int i = 0; i < sb_count( fs_searchpaths ); i ++ )
	{
		free( fs_searchpaths[ i ] );
	}
	sb_free( fs_searchpaths );
	fs_searchpaths = NULL;

	vpk_free( fs_vpk );
	fs_vpk = NULL;
	
	if( fs_current_archive )
	{
		fclose( fs_current_archive );
		fs_current_archive = NULL;
	}
}

char *fs_get( const char *path, EFileSysResult_t *result )
{
	VPKDirectoryEntry_t *entry;

	if( (entry = vpk_find( fs_vpk, path )) )
	{
		if( entry->ArchiveIndex != fs_current_idx )
		{
			if( fs_current_archive )
			{
				fclose( fs_current_archive );
				fs_current_archive = NULL;
			}
			
			fs_current_idx = entry->ArchiveIndex;
		}
		if( !fs_current_archive )
		{
			char strPak[ 533 ];
			sprintf( strPak, "%scsgo/pak01_%03hu.vpk", fs_exedir, fs_current_idx );
			fs_current_archive = fopen( strPak, "rb" );
			
			if( !fs_current_archive )
			{
				fprintf( stderr, "Could not locate %s\n", strPak );
				*result = k_EFileSysResult_pakerror;
				return NULL;
			}
		}
		
		char *filebuf = malloc( entry->EntryLength );
		if( !filebuf )
		{
			fprintf( stderr, "Out of memory getting file: %s\n", path );
			*result = k_EFileSysResult_nomem;
			return NULL;
		}
		
		fseek( fs_current_archive, entry->EntryOffset, SEEK_SET );
		fread( filebuf, 1, entry->EntryLength, fs_current_archive );
		
		*result = k_EFileSysResult_complete;
		return filebuf;
	}
	else
	{
		// Use physical searchpaths
		char strPath[ 512 ];
		
		for( int i = 0; i < sb_count( fs_searchpaths ); i ++ )
		{
			strcpy( strPath, fs_searchpaths[ i ] );
			strcat( strPath, path );
			
			char *filebuf;
			
			EFileSysResult_t fb_result;
			
			if( (filebuf = file_to_buffer( strPath, &fb_result )) )
			{
				*result = k_EFileSysResult_complete;
				return filebuf;
			}
			
			if( fb_result == k_EFileSysResult_nomem )
			{
				fprintf( stderr, "Out of memory getting file: %s\n", path );
				*result = k_EFileSysResult_nomem;
				return NULL;
			}
		}
		
		*result = k_EFileSysResult_notfound;
		return NULL;
	}
}
