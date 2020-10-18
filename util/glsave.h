void render_to_png( uint32_t const x, uint32_t const y, const char *filepath ){
	char* data = malloc( 4 * x * y );

	glReadPixels( 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data );

	stbi_flip_vertically_on_write(1);
	stbi_write_png( filepath, x, y, 4, data, x * 4 );

	free( data );
}

void render_to_png_flat( uint32_t const x, uint32_t const y, const char *filepath ){
	char* data = malloc( 3 * x * y );

	glReadPixels( 0, 0, x, y, GL_RGB, GL_UNSIGNED_BYTE, data );

	stbi_flip_vertically_on_write(1);
	stbi_write_png( filepath, x, y, 3, data, x * 3 );

	free( data );
}
