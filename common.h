// C STD
// ===============================
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// STB
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_TGA
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "stb/stretchy_buffer.h"

// OpenGL
// ===============================
#ifdef _WIN32
 #define GLFW_DLL
#endif
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Mathematics
// ===============================
#include "cglm/cglm.h"
#include "cglm_d/plane_d.h"
#include "util/twiddle.h"
#include "util/glsave.h"

#undef __min
#undef __max
#define __min(X,Y) ((X) < (Y) ? (X) : (Y))
#define __max(X,Y) ((X) > (Y) ? (X) : (Y))

// Util
// ===============================
#include "util/files.h"
#include "util/filebuffer.h"
#include "util/obj_write.h"

// Valve
#include "valve/vpk.h"
#include "valve/vdf.h"
#include "filesys.h"

#include "valve/vmf.h"
#include "valve/mdl.h"

// My stuff
#include "shader.h"
#include "rendering.h"
