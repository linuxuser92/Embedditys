#pragma once

#include "vfs.h"

#include <string>
#include <vector>

namespace fakeos {

class Editor {
public:
	Editor(VFS& filesystem, std::string path);
	bool run();

private:
	VFS& filesystem_;
	std::string path_;
	std::vector<std::string> lines_;
	bool dirty_ = false;

	bool save();
	std::string contents() const;
};

} // namespace fakeos
