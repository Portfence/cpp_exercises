#include "echo.hpp"

#include <expected>

#include <asio.hpp>
#include <asio/awaitable.hpp>
#include <asio/error_code.hpp>
#include <asio/use_awaitable.hpp>

using asio::ip::udp;

asio::awaitable<void> udp_echo(asio::ip::udp::socket socket) {

    uint8_t buf[1024] = {};

    for (;;) {
        udp::endpoint sender;
        std::size_t recv_len = co_await socket.async_receive_from(asio::buffer(buf), sender, asio::use_awaitable);

        co_await socket.async_send_to(asio::buffer(buf, recv_len), sender, asio::use_awaitable);
    }
}

std::expected<void, echo::Error> echo::run_exercise() {
    asio::io_context io;
    asio::error_code ec;
    auto ip = asio::ip::make_address("0.0.0.", ec);

    if (ec) {
        return std::unexpected(echo::Error::InvalidIpAddressFormat);
    }

    udp::socket socket(io, udp::endpoint(ip, 8080));

    asio::co_spawn(io, udp_echo(std::move(socket)), asio::detached);

    io.run();
    return {};
}
