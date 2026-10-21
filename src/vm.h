#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace fakeos {

class VM {
public:
	bool load(const std::string& source, std::string& error);
	bool run(std::string& output, std::string& error, std::size_t instructionLimit = 100000);
	void reset() noexcept;

private:
	struct Instruction {
		std::string opcode;
		std::int64_t operand = 0;
	};

	std::vector<Instruction> program_;
	std::vector<std::int64_t> stack_;
};

} // namespace fakeos
