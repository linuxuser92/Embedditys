#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fakeos {

struct VNode {
	bool isDirectory = false;
	std::string content;
	std::map<std::string, std::unique_ptr<VNode>> children;
	VNode* parent = nullptr;

	explicit VNode(bool directory = false, VNode* parentNode = nullptr)
		: isDirectory(directory), parent(parentNode) {}
};

class VFS {
public:
	VFS();

	std::string pwd() const;
	bool cd(const std::string& path);
	bool mkdir(const std::string& path);
	bool touch(const std::string& path);
	bool remove(const std::string& path);
	bool writeFile(const std::string& path, const std::string& data, bool append = false);
	bool readFile(const std::string& path, std::string& data) const;
	std::vector<std::string> list(const std::string& path = ".") const;
	bool exists(const std::string& path) const;

private:
	std::unique_ptr<VNode> root_;
	VNode* cwd_ = nullptr;

	VNode* resolve(const std::string& path) const;
	bool resolveParent(const std::string& path, VNode*& parent, std::string& name) const;
};

} // namespace fakeos
