#include "vhs.h"

#include <iostream>

namespace fakeos {

VHS::VHS() : devices_{{"console", true}, {"disk", true}, {"network", true}, {"timer", true}}
{
}

void VHS::tick()
{
	++ticks_;
	instructions_ += 100;
}

bool VHS::setDevicePower(const std::string& name, bool online)
{
	for (auto& device : devices_) {
		if (device.name == name) {
			device.online = online;
			return true;
		}
	}
	return false;
}

void VHS::printStatus() const
{
	std::cout << "FakeOS hardware status\n"
			  << "Ticks: " << ticks_ << "\n"
			  << "Instructions: " << instructions_ << "\n"
			  << "Devices:\n";
	for (const auto& device : devices_)
		std::cout << "  " << device.name << ": " << (device.online ? "online" : "offline") << '\n';
}

} // namespace fakeos
