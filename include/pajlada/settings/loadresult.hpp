#pragma once

#include <cassert>
#include <string>

namespace pajlada::Settings {

/// A result for loading a file.
///
/// This result must be checked using `operator bool`, #isSuccess(), #isError(),
/// or #getKind().
// FIXME: make constexpr (libstdc++ 12 required)
class [[nodiscard("Results must be checked")]] LoadResult
{
public:
    enum class Kind : uint8_t {
        None,
        ResolveSymlinks,
        OpenFile,
        ReadFileSize,
        ReadJSON,
    };

    static LoadResult
    success()
    {
        return {};
    }

    static LoadResult
    failure(Kind kind, std::string message)
    {
        return {kind, std::move(message)};
    }

    LoadResult(const LoadResult &) = delete;
    LoadResult &operator=(const LoadResult &) = delete;

    LoadResult(LoadResult &&other) noexcept
        : message(std::move(other.message))
        , kind(other.kind)
        , checked(other.checked)
    {
        other.checked = true;
    }

    LoadResult &
    operator=(LoadResult &&other) noexcept
    {
        this->message = std::move(other.message);
        this->kind = other.kind;
        this->checked = other.checked;
        other.checked = true;
        return *this;
    }

    ~LoadResult();

    explicit
    operator bool()
    {
        return this->isError();
    }

    [[nodiscard]]
    bool
    isSuccess()
    {
        this->checked = true;
        return this->message.empty();
    }

    [[nodiscard]]
    bool
    isError()
    {
        return !this->isSuccess();
    }

    [[nodiscard]]
    std::string_view
    getMessage() const
    {
        return this->message;
    }

    [[nodiscard]]
    Kind
    getKind()
    {
        this->checked = true;
        return this->kind;
    }

private:
    LoadResult() = default;

    LoadResult(Kind kind, std::string message)
        : message(std::move(message))
        , kind(kind)
    {
    }

    std::string message;
    Kind kind = Kind::None;
    bool checked = false;
};

}  // namespace pajlada::Settings
