#include "common.h"
#include "time.h"
#include "tar_main.h"

GLFWwindow* g_hWindowMain;
float		g_fTime;
float		g_fTimeLast;
float		g_fTimeDelta;
int 		g_nWindowW;
int 		g_nWindowH;

// OPenGL error callback (4.5+)
void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	if (type == GL_DEBUG_TYPE_OTHER) return; // We dont want general openGL spam.

	fprintf( stderr, "OpenGL message: %s\n", message);

	uint32_t bShutdown = 0;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		fprintf( stderr,"Type: ERROR\n"); 
		bShutdown = 1;
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		fprintf( stderr,"Type: DEPRECATED_BEHAVIOR\n"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		fprintf( stderr,"Type: UNDEFINED_BEHAVIOR\n"); break;
	case GL_DEBUG_TYPE_PORTABILITY:
		fprintf( stderr,"Type: PORTABILITY\n"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		fprintf( stderr,"Type: PERFORMANCE\n"); break;
	case GL_DEBUG_TYPE_OTHER:
		fprintf( stderr,"Type: OTHER\n"); break;
	}

	fprintf( stderr,"ID: %u\n", id);
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		fprintf( stderr,"Severity: LOW\n"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		fprintf( stderr,"Severity: MEDIUM\n"); break;
	case GL_DEBUG_SEVERITY_HIGH:
		fprintf( stderr,"Severity: HIGH\n"); 
		bShutdown = 1;
		break;
	}
	
	if(bShutdown) glfwSetWindowShouldClose( g_hWindowMain, 1 );
}

int engine_init(void)
{
	printf( "engine::init()\n" );

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	
	g_hWindowMain = glfwCreateWindow(1024, 1024, "tar3", NULL, NULL);
	
	if(!g_hWindowMain){
		fprintf( stderr, "GLFW Failed to initialize\n");
		return 0;
	}
	
	glfwMakeContextCurrent(g_hWindowMain);
	
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf( stderr, "Glad failed to initialize\n");
		return 0;
	}

	const unsigned char* glver = glGetString(GL_VERSION);
	printf("Load setup complete, OpenGL version: %s\n", glver);

	if (glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, NULL);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			GL_TRUE);
	}
	else {
		printf("glDebugMessageCallback not availible\n");
	}
	
	return 1;
}

void engine_quit(void)
{
	glfwTerminate();
}


inline __attribute__((always_inline))
void engine_newframe(void)
{
	glfwPollEvents();

	g_fTimeLast = g_fTime;
	g_fTime = glfwGetTime();
	g_fTimeDelta = __min(g_fTime - g_fTimeLast, 0.1f);
	
	glfwGetFramebufferSize(g_hWindowMain, &g_nWindowW, &g_nWindowH);
	glViewport(0, 0, g_nWindowW, g_nWindowH);

	// Drawing code
	glClearColor(.05f, .05f, .05f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

inline __attribute__((always_inline))
void engine_endframe(void)
{
	glfwSwapBuffers(g_hWindowMain);
}

typedef struct {
	char 	test[30];
	mat4	yuh;
} test_t;

test_t *buffer[16];

void fbuffer_reset(void)
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, g_nWindowW, g_nWindowH );
}

int main(void)
{
	fs_set_gameinfo( "/home/harry/.steam/debian-installation/steamapps/common/Counter-Strike Global Offensive/csgo/gameinfo.txt" );
	
	
	uint32_t grp_layout = tar_push_group( "tar_layout" );
	uint32_t grp_overlap = tar_push_group( "tar_overlap" );
	
	tar_setvmf( "my_map.vmf" );
	
	if( !engine_init() )
	{
		engine_quit();
		return 0;
	}
	
	engine_newframe();
	
	GLuint shader_gbuffer = shader_compile( "shaders/base.vs", "shaders/gbuffer.fs" );
	GLuint shader_screentex = shader_compile( "shaders/screen.vs", "shaders/screen_texture.fs" );
	
	tar_bake();
	
	//
	gbuffer_t fb_layout;
	gbuffer_init( &fb_layout, 1024, 1024 );
	gbuffer_use( &fb_layout );
	
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glEnable(GL_DEPTH_TEST);
	
	tar_setshader( shader_gbuffer );
	tar_renderfiltered( 0xFFFFFFFF );
	
	fbuffer_reset();
	glDisable(GL_DEPTH_TEST);
	
	glUseProgram( shader_screentex );
	
	glActiveTexture( GL_TEXTURE0 );
	glUniform1i( glGetUniformLocation( shader_screentex, "in_Sampler" ), 0 );
	
	// Render position buffer
	glBindTexture( GL_TEXTURE_2D, fb_layout.texPosition );
	tar_drawquad();
	render_to_png_flat( 1024, 1024, "test_pos.png" );
	
	// Normal buffer
	glBindTexture( GL_TEXTURE_2D, fb_layout.texNormal );
	tar_drawquad();
	render_to_png_flat( 1024, 1024, "test_normal.png" );
	
	// Origin buffer
	glBindTexture( GL_TEXTURE_2D, fb_layout.texOrigin );
	tar_drawquad();
	render_to_png_flat( 1024, 1024, "test_origin.png" );
	
	engine_quit();
	
	fs_exit();
	
	printf( "tar::exit()\n" );
	
	return 0;
}

int __main(void)
{
	fs_set_gameinfo( "/home/harry/.steam/debian-installation/steamapps/common/Counter-Strike Global Offensive/csgo/gameinfo.txt" );
	
	mdl_mesh_t mesh_test;
	if( mdl_read_fs( "models/weapons/t_arms_anarchist.mdl", &mesh_test ) == k_EMdlLoad_valid )
	{
		mdl_to_obj( &mesh_test, "out.obj", "test" );
		mdl_free( &mesh_test );
	}
	
	fs_exit();
	return 0;

	if( !engine_init() )
	{
		engine_quit();
		return 0;
	}

	printf("vmf::load\n");
	vdf_node_t *vmf = vdf_open_file( "disp.vmf" );
	
	printf("vmf::gen\n");
	
	solidgen_ctx_t sgen ;
	solidgen_ctx_init( &sgen );
	
	// Pickup brushes from world
	vdf_node_t *world = vdf_find( vmf, "world" );
	if( world )
	{
		int it = 0; 
		vdf_node_t *node;
		while( (node = vdf_iter(world, "solid", &it)) )
		{
			if( solidgen_push( &sgen, node ) == k_ESolidResult_errnomem )
			{
				//todo: free
				printf("errnomem\n");
				break;
			}
		}
	}
	
	printf( "Statis: verts(%u), indices(%u)\n", sgen.unVerts, sgen.idx );
	
	solidgen_to_obj( &sgen, "test.obj" );
	
	solidgen_ctx_free( &sgen );
	vdf_free( vmf );
	
	while( !glfwWindowShouldClose( g_hWindowMain ) )
	{
		engine_newframe();
		engine_endframe();
	}
	
	engine_quit();

	return 0;
}
