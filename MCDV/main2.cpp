#include "globals.h"

#ifdef entry_point_testing

#include "wc.hpp"

int main()
{
	wc::filedata fileData("sample_stuff/CmdSeq.wc");

	wc::Sequence seq_new = wc::Sequence();
	char name_new[128] = "Testeroonie BAP";
	memcpy(&seq_new.name, &name_new, 128);
	

	fileData.sequences.push_back(seq_new);
	fileData.serialize("CmdSeq.wc");
	system("PAUSE");
	return 0;
}


#endif