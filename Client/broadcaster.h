#pragma once

#include <asio.hpp>
#include <functional>
#include <print>

/*
	Broadcaster is an entity that sends all it's messages by broadcast endpoint
*/
template <typename St, typename Rt>
class Broadcaster {
	using BroadcastCallback = std::function<void(const Rt&, size_t bytes, asio::ip::udp::endpoint)>;
private:
	asio::ip::udp::endpoint broadcastEndpoint;
	asio::ip::udp::socket socket;
	BroadcastCallback onReceiveCallback;

public:
	Broadcaster(asio::io_context& io, int port);

	void send(const St& message);
	void onReceive(const BroadcastCallback& onReceiveCallback);
	void poll();
};

template <typename St, typename Rt>
Broadcaster::Broadcaster(asio::io_context& io, int port)
	: broadcastEndpoint(asio::ip::address_v4::broadcast(), port), socket(io) {

	asio::ip::udp::endpoint localEndpoint(asio::ip::address_v4::any(), port);

	socket.open(asio::ip::udp::v4());
	asio::socket_base::broadcast broadcastOption(true);
	socket.set_option(asio::ip::udp::socket::reuse_address(true));
	socket.set_option(broadcastOption);
	socket.bind(localEndpoint);
}

template <typename St, typename Rt>
void Broadcaster::send(const St& message) {
	try {
		asio::error_code someError;
		socket.send_to(asio::buffer(message, sizeof(St)), broadcastEndpoint, 0, someError);
		std::println("Message broadcasted!");
	}
	catch (std::exception& exc) {
		std::println("Callout exception: {}", exc.what());
	}
}

template <typename St, typename Rt>
void Broadcaster::onReceive(const BroadcastCallback& onReceiveCallback) {
	this->onReceiveCallback = onReceiveCallback;
}

template <typename St, typename Rt>
void Broadcaster::poll() {
	try {
		Rt message;
		asio::ip::udp::endpoint senderEndpoint;
		size_t bytesReceived = socket.receive_from(asio::buffer(message, sizeof(Rt)), senderEndpoint);

		//std::println("Sender: {0}\nMessage: {1}",
		//	senderEndpoint.address().to_string(),
		//	bytesReceived);

		onReceiveCallback(message, bytesReceived, senderEndpoint);
	}
	catch (std::exception& exc) {
		std::println("Listening exception: {}", exc.what());
	}
}