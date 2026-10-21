#include "editor.h"

#include <iostream>
#include <sstream>
#include <utility>

namespace fakeos {

Editor::Editor(VFS& filesystem, std::string path)
	: filesystem_(filesystem), path_(std::move(path))
{
	std::string initial;
	if (filesystem_.readFile(path_, initial)) {
		std::istringstream input(initial);
		std::string line;
		while (std::getline(input, line)) lines_.push_back(std::move(line));
	}
}

bool Editor::run()
{
	std::cout << "Editing " << path_ << ". Enter text one line at a time.\n"
			  << "Commands: :w save, :q quit, :wq save and quit.\n";
	for (const auto& line : lines_) std::cout << line << '\n';

	std::string line;
	while (std::getline(std::cin, line)) {
		if (line == ":w") {
			if (!save()) {
				std::cerr << "editor: unable to save " << path_ << '\n';
				return false;
			}
			std::cout << "saved " << path_ << '\n';
		} else if (line == ":q") {
			if (dirty_) std::cout << "unsaved changes discarded\n";
			return true;
		} else if (line == ":wq") {
			if (!save()) {
				std::cerr << "editor: unable to save " << path_ << '\n';
				return false;
			}
			return true;
		} else {
			lines_.push_back(line);
			dirty_ = true;
		}
	}
	return !dirty_;
}

bool Editor::save()
{
	if (!filesystem_.writeFile(path_, contents())) return false;
	dirty_ = false;
	return true;
}

std::string Editor::contents() const
{
	std::string result;
	for (std::size_t index = 0; index < lines_.size(); ++index) {
		if (index != 0) result += '\n';
		result += lines_[index];
	}
	if (!lines_.empty()) result += '\n';
	return result;
}

} // namespace fakeos
