#pragma pack(push, 1)
typedef struct
{
	uint32_t Signature;
	uint32_t Version;

	// The size, in bytes, of the directory tree
	uint32_t TreeSize;

	// How many bytes of file content are stored in this VPK file (0 in CSGO)
	uint32_t FileDataSectionSize;

	// The size, in bytes, of the section containing MD5 checksums for external archive content
	uint32_t ArchiveMD5SectionSize;

	// The size, in bytes, of the section containing MD5 checksums for content in this file (should always be 48)
	uint32_t OtherMD5SectionSize;

	// The size, in bytes, of the section containing the public key and signature. This is either 0 (CSGO & The Ship) or 296 (HL2, HL2:DM, HL2:EP1, HL2:EP2, HL2:LC, TF2, DOD:S & CS:S)
	uint32_t SignatureSectionSize;
} VPKHeader_t;

typedef struct
{
	uint32_t CRC; // A 32bit CRC of the file's data.
	uint16_t PreloadBytes; // The number of bytes contained in the index file.

	// A zero based index of the archive this file's data is contained in.
	// If 0x7fff, the data follows the directory.
	uint16_t ArchiveIndex;

	// If ArchiveIndex is 0x7fff, the offset of the file data relative to the end of the directory (see the header for more details).
	// Otherwise, the offset of the data from the start of the specified archive.
	uint32_t EntryOffset;

	// If zero, the entire file is stored in the preload data.
	// Otherwise, the number of bytes stored starting at EntryOffset.
	uint32_t EntryLength;

	uint16_t Terminator;
} VPKDirectoryEntry_t;
#pragma pack(pop)

VPKHeader_t *vpk_read( const char *path )
{
	EFileSysResult_t result;
	char *vpk_index = file_to_buffer( path, &result );
	
	if( result != k_EFileSysResult_complete ) return NULL;

	return (VPKHeader_t *)vpk_index;
}

void vpk_free( VPKHeader_t *self )
{
	free( self );
}

VPKDirectoryEntry_t *vpk_find( VPKHeader_t *self, const char *asset )
{
	if( !self ) return NULL;
	
	char wbuf[ 512 ];
	strcpy( wbuf, asset );
	
	char *ext = str_findext( wbuf, '.' );
	*(ext-1) = 0x00;
	char *fn = str_findext( wbuf, '/' );
	*(fn-1) = 0x00;
	char *dir = wbuf;
	
	char *pCur = ((char *)self) + sizeof( VPKHeader_t );
	
	while( 1 )
	{
		if( !*pCur ) break;
		
		int bExt = !strcmp( ext, pCur );
		
		while( *( pCur ++ ) ) {};
		while( 1 )
		{
			if( !*pCur ) { pCur ++; break; }

			int bDir = !strcmp( dir, pCur );

			while( *( pCur ++ ) ) {};
			while( 1 )
			{
				if( !*pCur ) { pCur ++; break; }
			
				const char *vpk_fn = pCur;
			
				while( *( pCur ++ ) ) {};
				VPKDirectoryEntry_t *entry = (VPKDirectoryEntry_t *)pCur;
				
				if( !strcmp( vpk_fn, fn ) && bExt && bDir ) 
				{
					return entry;
				}
				
				pCur += entry->PreloadBytes + sizeof( VPKDirectoryEntry_t );
			}
			
			if( bDir && bExt ) return NULL;
		}
		
		if( bExt ) return NULL;
	}
	
	return NULL;
}
