#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iostream>


// Attach to a class for verbosity debug message control
namespace util {
	class verboseControl {
	public:
		bool use_verbose = false;

		//Variadic print
		void debug() {
			std::cout << std::endl;
		}
		template<typename First, typename ... Strings>
		void debug(First arg, const Strings&... rest) {
			if (this->use_verbose) {
				std::cout << arg << " ";
				debug(rest...);
			}
		}
	};
}