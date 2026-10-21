#include "vfs.h"

#include <sstream>
#include <utility>

namespace fakeos {

VFS::VFS() : root_(std::make_unique<VNode>(true)), cwd_(root_.get())
{
	mkdir("/home");
	mkdir("/tmp");
	mkdir("/etc");
	mkdir("/dev");
	writeFile("/etc/motd", "Welcome to Embedditys. Type 'help' for commands.\n");
	writeFile("/etc/hostname", "embedditys\n");
}

VNode* VFS::resolve(const std::string& path) const
{
	if (path.empty() || path == ".") return cwd_;
	VNode* node = path.front() == '/' ? root_.get() : cwd_;
	std::istringstream parts(path);
	std::string part;
	while (std::getline(parts, part, '/')) {
		if (part.empty() || part == ".") continue;
		if (part == "..") {
			if (node->parent != nullptr) node = node->parent;
			continue;
		}
		if (!node->isDirectory) return nullptr;
		const auto it = node->children.find(part);
		if (it == node->children.end()) return nullptr;
		node = it->second.get();
	}
	return node;
}

bool VFS::resolveParent(const std::string& path, VNode*& parent, std::string& name) const
{
	if (path.empty() || path == "/") return false;
	const auto separator = path.find_last_of('/');
	name = path.substr(separator == std::string::npos ? 0 : separator + 1);
	if (name.empty() || name == "." || name == "..") return false;
	const auto parentPath = separator == std::string::npos ? "." :
		(separator == 0 ? "/" : path.substr(0, separator));
	parent = resolve(parentPath);
	return parent != nullptr && parent->isDirectory;
}

std::string VFS::pwd() const
{
	if (cwd_ == root_.get()) return "/";
	std::vector<std::string> names;
	for (auto* node = cwd_; node != root_.get(); node = node->parent) {
		if (node->parent == nullptr) return "/";
		for (const auto& entry : node->parent->children) {
			if (entry.second.get() == node) {
				names.push_back(entry.first);
				break;
			}
		}
	}
	std::string result;
	for (auto it = names.rbegin(); it != names.rend(); ++it) result += "/" + *it;
	return result.empty() ? "/" : result;
}

bool VFS::cd(const std::string& path)
{
	auto* node = resolve(path);
	if (node == nullptr || !node->isDirectory) return false;
	cwd_ = node;
	return true;
}

bool VFS::mkdir(const std::string& path)
{
	VNode* parent = nullptr;
	std::string name;
	if (!resolveParent(path, parent, name) || parent->children.count(name) != 0) return false;
	parent->children.emplace(name, std::make_unique<VNode>(true, parent));
	return true;
}

bool VFS::touch(const std::string& path)
{
	if (auto* node = resolve(path)) return !node->isDirectory;
	VNode* parent = nullptr;
	std::string name;
	if (!resolveParent(path, parent, name) || parent->children.count(name) != 0) return false;
	parent->children.emplace(name, std::make_unique<VNode>(false, parent));
	return true;
}

bool VFS::remove(const std::string& path)
{
	VNode* parent = nullptr;
	std::string name;
	if (!resolveParent(path, parent, name)) return false;
	const auto it = parent->children.find(name);
	if (it == parent->children.end() || !it->second->children.empty() || it->second.get() == cwd_) return false;
	parent->children.erase(it);
	return true;
}

bool VFS::writeFile(const std::string& path, const std::string& data, bool append)
{
	if (!exists(path) && !touch(path)) return false;
	auto* node = resolve(path);
	if (node == nullptr || node->isDirectory) return false;
	if (append) node->content += data;
	else node->content = data;
	return true;
}

bool VFS::readFile(const std::string& path, std::string& data) const
{
	const auto* node = resolve(path);
	if (node == nullptr || node->isDirectory) return false;
	data = node->content;
	return true;
}

std::vector<std::string> VFS::list(const std::string& path) const
{
	std::vector<std::string> names;
	const auto* node = resolve(path);
	if (node == nullptr || !node->isDirectory) return names;
	names.reserve(node->children.size());
	for (const auto& entry : node->children)
		names.push_back(entry.first + (entry.second->isDirectory ? "/" : ""));
	return names;
}

bool VFS::exists(const std::string& path) const
{
	return resolve(path) != nullptr;
}

} // namespace fakeos
