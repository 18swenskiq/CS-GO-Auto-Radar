#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>

namespace wc
{
#pragma pack(push, 1)

	struct Command{
		int is_enabled; // 0/1, If command is enabled.
		int special;
		char executable[260]; // Name of EXE to run.
		char args[260]; // Arguments for executable.
		int is_long_filename; // Obsolete, but always set to true. Disables MS-DOS 8-char filenames.
		int ensure_check; // Ensure file post-exists after compilation
		char ensure_file[260]; // File to check exists.
		int use_proc_win; // Use Process Window (ignored if exectuable = $game_exe).

		// V 0.2+ only:
		int no_wait;  // Wait for keypress when done compiling.
	};

	struct SequenceHeader
	{
		char name[128]{ '\0' };
		uint32_t command_count; // Number of commands
	};

	struct Sequence
	{
		char name[128];
		std::vector<Command> commands;
	};

	struct Header {
		char signature[31] = { 'W','o','r','l','d','c','r','a','f','t',' ','C','o','m','m','a','n','d',' ','S','e','q','u','e','n','c','e','s','\r','\n','\x1a' }; // Yikes.
		float version = 0.2f;
		uint32_t seq_count;
	};

#pragma pack(pop)

	class filedata
	{
	public:
		std::vector<Sequence> sequences;

		filedata(std::string path)
		{
			std::ifstream reader(path, std::ios::in | std::ios::binary);

			if (!reader) {
				throw std::exception("WC::LOAD Failed"); return;
			}

			Header header = Header();
			reader.read((char*)&header, sizeof(header));

			for (int i = 0; i < header.seq_count; i++)
			{
				Sequence sequence = Sequence();
				reader.read((char*)&sequence.name, 128);
				
				uint32_t command_count;
				reader.read((char*)&command_count, sizeof(uint32_t));

				if (command_count > 1024) {
					throw std::exception("Too many commands!!!");
				}

				for (int cc = 0; cc < command_count; cc++)
				{
					Command command = Command();

					reader.read((char*)&command, sizeof(Command));

					sequence.commands.push_back(command);
				}

				this->sequences.push_back(sequence);
			}

			reader.close();
		}

		void serialize(std::string path)
		{
			std::fstream writer(path, std::ios::out | std::ios::binary);

			// Write header
			Header header = Header();
			header.seq_count = sequences.size();
			writer.write((char*)&header, sizeof(header));

			// Write Sequences
			for (auto && sequence : this->sequences){

				writer.write((char*)&sequence.name, 128);
				uint32_t cmdCount = sequence.commands.size();

				writer.write((char*)&cmdCount, sizeof(uint32_t));

				for (auto && command : sequence.commands)
				{
					writer.write((char*)&command, sizeof(command));
				}
			}

			writer.close();
		}
	};
}