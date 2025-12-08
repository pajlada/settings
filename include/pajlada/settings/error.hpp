#pragma once

#include <cassert>
#include <string>
#include <type_traits>

namespace pajlada::Settings {

/// An Error
///
/// This error must be checked using `operator bool`, #isSuccess(), #isError(),
/// or #kind().
// FIXME: make constexpr (libstdc++ 12 required)
class [[nodiscard("Errors must be checked")]] Error
{
public:
    enum class Kind : uint8_t {
        None,
        ResolveSymlinks,
        OpenFile,
        ReadFileSize,
        ReadJSON,
    };

    static Error
    success()
    {
        return {};
    }

    static Error
    failure(Kind kind, std::string message)
    {
        return {kind, std::move(message)};
    }

    Error(const Error &) = delete;
    Error &operator=(const Error &) = delete;

    Error(Error &&other) noexcept
        : message_(std::move(other.message_))
        , kind_(other.kind_)
        , checked_(other.checked_)
    {
        other.checked_ = true;
    }

    Error &
    operator=(Error &&other) noexcept
    {
        this->message_ = std::move(other.message_);
        this->kind_ = other.kind_;
        this->checked_ = other.checked_;
        other.checked_ = true;
        return *this;
    }

    ~Error()
    {
        assert(this->checked_ && "Unchecked error");
    }

    explicit
    operator bool()
    {
        return this->isError();
    }

    [[nodiscard]]
    bool
    isSuccess()
    {
        this->checked_ = true;
        return this->message_.empty();
    }

    [[nodiscard]]
    bool
    isError()
    {
        return !this->isSuccess();
    }

    [[nodiscard]]
    std::string_view
    message() const
    {
        return this->message_;
    }

    [[nodiscard]]
    Kind
    kind()
    {
        this->checked_ = true;
        return this->kind_;
    }

private:
    Error() = default;

    Error(Kind kind, std::string message)
        : message_(std::move(message))
        , kind_(kind)
    {
    }

    std::string message_;
    Kind kind_ = Kind::None;
    bool checked_ = false;
};

}  // namespace pajlada::Settings
