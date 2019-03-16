#include "globals.h"

#ifdef entry_point_testing

#include "wc.hpp"

int main()
{
	wc::filedata fileData("CmdSeq.wc");

	for (auto && seq : fileData.sequences) {
		if (strstr(seq.name, "[TAR] Generate Radar") != NULL){
			seq.write_enable = false;
		}
	}

	wc::Sequence seq_new = wc::Sequence();
	char name_new[128] = "[TAR] Generate Radar";
	memcpy(&seq_new.name, &name_new, 128);
	
	wc::Command command_build = wc::Command();
	command_build.is_enabled = 1;
	command_build.special = 0;
	command_build.is_long_filename = 0;
	command_build.ensure_check = 0;
	command_build.use_proc_win = 0;
	command_build.no_wait = 0;
	
	char executable[260] = "$exedir\\bin\\tar\\AutoRadar.exe";
	memcpy(&command_build.executable, &executable, 260);

	char args[260] = "-g $gamedir $path\\$file";
	memcpy(&command_build.args, &args, 260);

	seq_new.commands.push_back(command_build);

	fileData.sequences.push_back(seq_new);
	fileData.serialize("CmdSeq.wc");
	system("PAUSE");
	return 0;
}


#endif