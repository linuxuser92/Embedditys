#include "net.h"

namespace fakeos {

int Net::createSocket()
{
	const int id = nextSocketId_++;
	sockets_.emplace(id, NetworkSocket{id, {}, 0, false, false, {}});
	return id;
}

bool Net::bind(int socketId, const std::string& address, std::uint16_t port)
{
	const auto socket = sockets_.find(socketId);
	if (socket == sockets_.end() || port == 0) return false;
	for (const auto& entry : sockets_) {
		if (entry.first != socketId && entry.second.port == port && entry.second.listening) return false;
	}
	socket->second.address = address;
	socket->second.port = port;
	return true;
}

bool Net::listen(int socketId)
{
	const auto socket = sockets_.find(socketId);
	if (socket == sockets_.end() || socket->second.port == 0) return false;
	socket->second.listening = true;
	return true;
}

bool Net::connect(int socketId, const std::string& address, std::uint16_t port)
{
	const auto client = sockets_.find(socketId);
	if (client == sockets_.end()) return false;
	for (auto& entry : sockets_) {
		auto& server = entry.second;
		if (server.listening && server.port == port && (server.address == address || server.address == "0.0.0.0")) {
			client->second.address = address;
			client->second.port = port;
			client->second.connected = true;
			return true;
		}
	}
	return false;
}

bool Net::send(int socketId, const std::string& message)
{
	const auto sender = sockets_.find(socketId);
	if (sender == sockets_.end() || !sender->second.connected) return false;
	for (auto& entry : sockets_) {
		auto& receiver = entry.second;
		if (entry.first != socketId && receiver.listening && receiver.port == sender->second.port) {
			receiver.inbox.push_back(message);
			return true;
		}
	}
	return false;
}

bool Net::receive(int socketId, std::string& message)
{
	const auto socket = sockets_.find(socketId);
	if (socket == sockets_.end() || socket->second.inbox.empty()) return false;
	message = std::move(socket->second.inbox.front());
	socket->second.inbox.erase(socket->second.inbox.begin());
	return true;
}

bool Net::close(int socketId)
{
	return sockets_.erase(socketId) != 0;
}

std::vector<std::string> Net::status() const
{
	std::vector<std::string> result;
	for (const auto& entry : sockets_) {
		const auto& socket = entry.second;
		result.push_back("socket " + std::to_string(socket.id) + " " +
			(socket.listening ? "listening" : socket.connected ? "connected" : "open") +
			(socket.port == 0 ? "" : " " + socket.address + ":" + std::to_string(socket.port)));
	}
	return result;
}

} // namespace fakeos
