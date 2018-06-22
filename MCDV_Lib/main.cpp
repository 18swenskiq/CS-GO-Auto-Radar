#include <Windows.h>
/*
#include "../MCDV/vector.h"
#include "../MCDV/interpolation.h"
#include "../MCDV/interpolation.cpp"
#include "../MCDV/structures.h"
#include "../MCDV/structures.cpp"
#include "../MCDV/vbsp_level.h"
#include "../MCDV/vbsp_level.cpp"
#include "../MCDV/vtx_mesh.h"
#include "../MCDV/vtx_mesh.cpp"
#include "../MCDV/vvd_data.h"
#include "../MCDV/vvd_data.cpp"

extern "C"
{
	/* Converts BSP level into a TBSP filetype 
	__declspec(dllexport) int BSPtoTBSP(const char* path, const char* out)
	{
		try
		{
			vbsp_level level(path);
			level.convertToTBSP(out);
		}
		catch (std::exception e)
		{
			std::cout << e.what() << std::endl;
			return -1;
		}

		return 0;
	}

	/* Converts a .mdl's vtex and vvd formats into TMDL filetype 
	__declspec(dllexport) int MDLDatatoTMDL(const char* path_vtex, const char* path_vvd, const char* out)
	{
		try
		{
			vtx_mesh vtx(path_vtex);

			vvd_data vvd(path_vvd);

			mesh m(vvd, vtx);

			m.test_save_tmdl(out);
		}
		catch (std::exception e)
		{
			std::cout << e.what() << std::endl;
			return -1;
		}

		return 0;
	}
}
*/