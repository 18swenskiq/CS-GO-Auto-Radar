#include "globals.h"
#ifdef entry_point_testing

#include "tar_config.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// STB lib
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
	tar_style test_styler("");

	std::cout << test_styler.serialize_base64() << "\n";
	
	system("PAUSE");
	return 0;
}

#endif