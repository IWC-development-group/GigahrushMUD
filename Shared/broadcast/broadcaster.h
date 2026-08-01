#pragma once

#include "../thirdparty/asio/include/asio.hpp"
#include <functional>
#include <print>

/*
  Broadcaster is an entity that sends all it's messages by broadcast endpoint
*/
template <typename St, typename Rt>
class Broadcaster {
    static_assert(std::is_trivially_copyable_v<St>, "St must be trivially copyable to send raw bytes");
    static_assert(std::is_trivially_copyable_v<Rt>, "Rt must be trivially copyable to receive raw bytes");
    using BroadcastCallback = std::function<void(const Rt&, size_t bytes, asio::ip::udp::endpoint)>;

private:
    asio::ip::udp::endpoint broadcastEndpoint;
    asio::ip::udp::socket socket;
    BroadcastCallback onReceiveCallback;

public:
    Broadcaster(asio::io_context& io, int localPort, int targetPort, bool nonBlocking = true);

    void send(const St& message);
    void send(const St& message, asio::ip::udp::endpoint receiver);
    void onReceive(const BroadcastCallback& onReceiveCallback);
    void close();

    bool poll();
};

template <typename St, typename Rt>
Broadcaster<St, Rt>::Broadcaster(asio::io_context& io, int localPort, int targetPort, bool nonBlocking)
    : broadcastEndpoint(asio::ip::address_v4::broadcast(), targetPort), socket(io) {

    asio::ip::udp::endpoint localEndpoint(asio::ip::address_v4::any(), localPort);

    socket.open(asio::ip::udp::v4());
    asio::socket_base::broadcast broadcastOption(true);
    socket.set_option(asio::ip::udp::socket::reuse_address(true));
    socket.set_option(broadcastOption);
    socket.non_blocking(nonBlocking);
    socket.bind(localEndpoint);
}

template <typename St, typename Rt>
void Broadcaster<St, Rt>::send(const St& message) {
    asio::error_code someError;
    socket.send_to(asio::buffer(&message, sizeof(St)), broadcastEndpoint, 0, someError);
}

template <typename St, typename Rt>
void Broadcaster<St, Rt>::send(const St& message, asio::ip::udp::endpoint receiver) {
    asio::error_code someError;
    socket.send_to(asio::buffer(&message, sizeof(St)), receiver, 0, someError);
}

template <typename St, typename Rt>
void Broadcaster<St, Rt>::onReceive(const BroadcastCallback& onReceiveCallback) {
    this->onReceiveCallback = onReceiveCallback;
}

template <typename St, typename Rt>
void Broadcaster<St, Rt>::close() {
    asio::error_code error;
    socket.close(error);
}

template <typename St, typename Rt>
bool Broadcaster<St, Rt>::poll() {
    Rt message;
    asio::ip::udp::endpoint senderEndpoint;
    asio::error_code error;

    size_t bytesReceived = socket.receive_from(
        asio::buffer(&message, sizeof(Rt)),
        senderEndpoint,
        0,
        error
    );

    if (error) return false;

    if (onReceiveCallback) {
        onReceiveCallback(message, bytesReceived, senderEndpoint);
    }

    return true;
}