#include "vm.h"

#include <sstream>
#include <stdexcept>

namespace fakeos {

bool VM::load(const std::string& source, std::string& error)
{
	std::vector<Instruction> parsed;
	std::istringstream input(source);
	std::string line;
	std::size_t lineNumber = 0;
	while (std::getline(input, line)) {
		++lineNumber;
		const auto comment = line.find(';');
		if (comment != std::string::npos) line.erase(comment);
		std::istringstream tokens(line);
		Instruction instruction;
		if (!(tokens >> instruction.opcode)) continue;
		if (instruction.opcode == "PUSH") {
			if (!(tokens >> instruction.operand)) {
				error = "line " + std::to_string(lineNumber) + ": PUSH requires an integer";
				return false;
			}
		} else if (instruction.opcode != "ADD" && instruction.opcode != "SUB" &&
				   instruction.opcode != "MUL" && instruction.opcode != "PRINT" &&
				   instruction.opcode != "DUP" && instruction.opcode != "DROP" &&
				   instruction.opcode != "HALT") {
			error = "line " + std::to_string(lineNumber) + ": unknown instruction '" + instruction.opcode + "'";
			return false;
		}
		std::string extra;
		if (tokens >> extra) {
			error = "line " + std::to_string(lineNumber) + ": unexpected argument";
			return false;
		}
		parsed.push_back(std::move(instruction));
	}
	program_ = std::move(parsed);
	stack_.clear();
	error.clear();
	return true;
}

bool VM::run(std::string& output, std::string& error, std::size_t instructionLimit)
{
	output.clear();
	std::size_t pc = 0;
	std::size_t steps = 0;
	auto pop = [&]() {
		if (stack_.empty()) throw std::runtime_error("stack underflow");
		const auto value = stack_.back();
		stack_.pop_back();
		return value;
	};

	try {
		while (pc < program_.size()) {
			if (steps++ >= instructionLimit) throw std::runtime_error("instruction limit reached");
			const auto& instruction = program_[pc++];
			if (instruction.opcode == "PUSH") stack_.push_back(instruction.operand);
			else if (instruction.opcode == "DUP") {
				if (stack_.empty()) throw std::runtime_error("stack underflow");
				stack_.push_back(stack_.back());
			} else if (instruction.opcode == "DROP") pop();
			else if (instruction.opcode == "PRINT") output += std::to_string(pop()) + "\n";
			else if (instruction.opcode == "HALT") break;
			else {
				const auto right = pop();
				const auto left = pop();
				if (instruction.opcode == "ADD") stack_.push_back(left + right);
				else if (instruction.opcode == "SUB") stack_.push_back(left - right);
				else if (instruction.opcode == "MUL") stack_.push_back(left * right);
			}
		}
	} catch (const std::exception& exception) {
		error = exception.what();
		return false;
	}
	error.clear();
	return true;
}

void VM::reset() noexcept
{
	program_.clear();
	stack_.clear();
}

} // namespace fakeos
