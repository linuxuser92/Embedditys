#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fakeos {

struct Device {
	std::string name;
	bool online = true;
};

class VHS {
public:
	VHS();
	void tick();
	bool setDevicePower(const std::string& name, bool online);
	void printStatus() const;

private:
	std::uint64_t ticks_ = 0;
	std::uint64_t instructions_ = 0;
	std::vector<Device> devices_;
};

} // namespace fakeos
