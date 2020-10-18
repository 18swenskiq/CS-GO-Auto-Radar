GLuint shader_compile_subshader(const char* sSource, GLint gliShaderType){
	GLint shader = glCreateShader( gliShaderType );
	glShaderSource( shader, 1, &sSource, NULL );
	glCompileShader( shader );

	int success;
	char infoLog[512];
	glGetShaderiv( shader, GL_COMPILE_STATUS, &success );

	if( !success ){
		glGetShaderInfoLog( shader, 512, NULL, infoLog );
		fprintf( stderr, "Error info:\n%s\n", infoLog );
		return 0;
	}

	return shader;
}

// Give source get shader
GLuint shader_compile( const char* sPathVert, const char* sPathFrag ){
	printf( "Compiling shader: vert( %s ), frag( %s )\n", sPathVert, sPathFrag );

	EFileSysResult_t result;

	// Load source buffers 
	char* source_vert = textfile_to_buffer( sPathVert, &result );
	char* source_frag = textfile_to_buffer( sPathFrag, &result );

	if( !source_vert || !source_frag ){
		fprintf( stderr, "Error opening file: %s\n", ((!source_vert)? sPathVert: sPathFrag));
		free( source_vert ); free( source_frag );
		return 0;
	}

	// Compile shaders
	GLuint vert = shader_compile_subshader( source_vert, GL_VERTEX_SHADER );
	GLuint frag = shader_compile_subshader( source_frag, GL_FRAGMENT_SHADER );

	// Free memory from text sources
	free( source_vert ); free( source_frag );

	if( !vert || !frag ){
		fprintf( stderr, "Shader compilation failed.\n" );
		abort();
		return 0;
	}

	// Create and attach shaders to program
	GLuint program = glCreateProgram();

	glAttachShader( program, vert );
	glAttachShader( program, frag );
	glLinkProgram( program );

	// Delete sub shaders
	glDeleteShader( vert ); glDeleteShader( frag );

	// Check for link errors
	char infoLog[512];
	int success_link = 1;
	glGetProgramiv( program, GL_LINK_STATUS, &success_link );
	if( !success_link ){
		glGetProgramInfoLog( program, 512, NULL, infoLog );
		fprintf( stderr, "Link failed: %s\n", infoLog );
		glDeleteProgram( program );
		return 0;
	}

	// Return a fully compiled shader name
	return program;
}
