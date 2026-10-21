#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace fakeos {

struct NetworkSocket {
	int id = -1;
	std::string address;
	std::uint16_t port = 0;
	bool listening = false;
	bool connected = false;
	std::vector<std::string> inbox;
};

class Net {
public:
	int createSocket();
	bool bind(int socketId, const std::string& address, std::uint16_t port);
	bool listen(int socketId);
	bool connect(int socketId, const std::string& address, std::uint16_t port);
	bool send(int socketId, const std::string& message);
	bool receive(int socketId, std::string& message);
	bool close(int socketId);
	std::vector<std::string> status() const;

private:
	int nextSocketId_ = 1;
	std::map<int, NetworkSocket> sockets_;
};

} // namespace fakeos
