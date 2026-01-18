#include <gtest/gtest.h>

#include <filesystem>
#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/settingmanager.hpp>

#ifdef WIN32
#include "winerror.h"
#endif

using namespace pajlada::Settings;
using SaveResult = pajlada::Settings::SettingManager::SaveResult;

namespace fs = std::filesystem;

namespace {

const fs::path PRE = "files/realpath";

bool
canonicallyEquivalent(const fs::path &actual, const fs::path &expected,
                      std::error_code &ec)
{
    return actual == fs::weakly_canonical(expected, ec);
}

}  // namespace

TEST(RealPath, RegularFile)
{
    std::error_code ec;
    auto f1 = PRE / "regular-file";
    ASSERT_FALSE(fs::is_symlink(f1));
    ASSERT_TRUE(fs::exists(f1));

    auto actual = detail::RealPath(f1, ec);
    EXPECT_FALSE(ec);

    EXPECT_TRUE(fs::equivalent(f1, actual, ec));
    EXPECT_FALSE(ec);
}

TEST(RealPath, RegularFileNonexistent)
{
    std::error_code ec;
    auto f1 = PRE / "nonexistent";
    ASSERT_FALSE(fs::is_symlink(f1));
    ASSERT_FALSE(fs::exists(f1));

    auto actual = detail::RealPath(f1, ec);
    EXPECT_FALSE(ec);

    EXPECT_TRUE(canonicallyEquivalent(actual, f1, ec));
    EXPECT_FALSE(ec);
}

TEST(RealPath, SymlinkToRegularFile)
{
    std::error_code ec;
    auto f1 = PRE / "symlink-to-file-relative";
    auto expected = PRE / "regular-file";
    ASSERT_TRUE(fs::is_symlink(f1));

    auto actual = detail::RealPath(f1, ec);
    EXPECT_FALSE(ec);

    EXPECT_TRUE(canonicallyEquivalent(actual, expected, ec));
    EXPECT_FALSE(ec);
}

TEST(RealPath, SymlinkToNonexistent)
{
    // Symlinks should _always_ return the path to the file, regardless if the final file exists or not
    std::error_code ec;
    auto f1 = PRE / "symlink-to-nonexistent-relative";
    auto expected = PRE / "nonexistent";
    ASSERT_TRUE(fs::is_symlink(f1));

    auto actual = detail::RealPath(f1, ec);
    EXPECT_FALSE(ec);

    EXPECT_TRUE(canonicallyEquivalent(actual, expected, ec))
        << actual << "(weakly canonical:" << fs::weakly_canonical(actual)
        << ") != " << expected;
    EXPECT_FALSE(ec);
}

TEST(RealPath, SymlinkToNonexistentAbsolute)
{
    // Symlinks should _always_ return the path to the file, regardless if the final file exists or not
    std::error_code ec;
    auto f1 = PRE / "tmp.symlink-to-nonexistent-absolute";
    auto tmpdir = fs::temp_directory_path();
    auto expected =
        fs::weakly_canonical(tmpdir / "pajlada-settings-nonexistent");

    fs::remove(f1, ec);
    ec.clear();

    fs::create_symlink(expected, f1);
    ASSERT_TRUE(fs::is_symlink(f1));

    auto actual = detail::RealPath(f1, ec);
    EXPECT_FALSE(ec);

    EXPECT_TRUE(canonicallyEquivalent(actual, expected, ec))
        << actual << "(weakly canonical:" << fs::weakly_canonical(actual)
        << ") != " << expected;
    EXPECT_FALSE(ec);
}

TEST(RealPath, SymlinkToSelf)
{
    std::error_code ec;
    auto f1 = PRE / "symlink-recursive-self";
    ASSERT_TRUE(fs::is_symlink(f1));

    std::ignore = detail::RealPath(f1, ec);

#ifdef WIN32
    EXPECT_EQ(ec.value(), ERROR_CANT_RESOLVE_FILENAME);
#else
    EXPECT_EQ(ec, std::errc::too_many_symbolic_link_levels);
#endif
}

TEST(RealPath, SymlinkRecursive)
{
    std::error_code ec;
    auto f1 = PRE / "symlink-recursive-foo";
    ASSERT_TRUE(fs::is_symlink(f1));

    auto f2 = PRE / "symlink-recursive-bar";
    ASSERT_TRUE(fs::is_symlink(f2));

    std::ignore = detail::RealPath(f1, ec);
#ifdef WIN32
    EXPECT_EQ(ec.value(), ERROR_CANT_RESOLVE_FILENAME);
#else
    EXPECT_EQ(ec, std::errc::too_many_symbolic_link_levels);
#endif

    std::ignore = detail::RealPath(f2, ec);
#ifdef WIN32
    EXPECT_EQ(ec.value(), ERROR_CANT_RESOLVE_FILENAME);
#else
    EXPECT_EQ(ec, std::errc::too_many_symbolic_link_levels);
#endif
}
