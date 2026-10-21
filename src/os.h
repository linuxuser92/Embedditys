#pragma once

#include "command.h"
#include "net.h"
#include "vfs.h"
#include "vhs.h"
#include "vm.h"
#include "python.h"

#include <string>
#include <vector>

namespace fakeos {

class OS {
public:
	OS();
	void run();
	bool execute(const std::string& line);
	bool isRunning() const noexcept;
	VFS& filesystem() noexcept;
	Net& network() noexcept;

private:
	VFS vfs_;
	VHS hardware_;
	VM vm_;
	Net net_;
	bool running_ = true;
	std::vector<std::string> history_;

	bool dispatch(const Command& command);
	void printHelp() const;
};

} // namespace fakeos
