#pragma once

#include <expected>
#include <format>
#include <string_view>

namespace echo {
enum class Error {
    InvalidIpAddressFormat,
    // more variants to come
};

/// @brief Runs the echo exercise
std::expected<void, Error> run_exercise();
} // namespace echo

template <> struct std::formatter<echo::Error> : std::formatter<std::string_view> {
    auto format(echo::Error err, std::format_context& ctx) const {
        std::string_view msg;
        switch (err) {
        case echo::Error::InvalidIpAddressFormat:
            msg = "invalid IP address format";
            break;
        }
        return std::formatter<std::string_view>::format(msg, ctx);
    }
};
