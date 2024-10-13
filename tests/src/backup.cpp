#include <gtest/gtest.h>

#include <fstream>
#include <pajlada/settings/backup.hpp>

#include "common.hpp"

namespace fs = std::filesystem;
namespace Backup = pajlada::Settings::Backup;

TEST(Backup, Basic)
{
    size_t writeCalls = 0;
    auto doSave = [&] {
        std::error_code ec;
        Backup::saveWithBackup(
            "files/out.backup.basic.json", {.enabled = true, .numSlots = 3},
            [&](const auto &path, auto &ec) {
                writeCalls++;
                std::ofstream of(path, std::ios::out);
                if (!of) {
                    ec = std::make_error_code(std::errc::io_error);
                    return;
                }
                of << "yo";
            },
            ec);
        return !ec;
    };

    RemoveFile("files/out.backup.basic.json");
    RemoveFile("files/out.backup.basic.json.bkp-1");
    RemoveFile("files/out.backup.basic.json.bkp-2");
    RemoveFile("files/out.backup.basic.json.bkp-3");

    EXPECT_TRUE(!fs::exists("files/out.backup.basic.json"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 1);

    EXPECT_TRUE(fs::exists("files/out.backup.basic.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.basic.json.bkp-1"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 2);

    EXPECT_TRUE(fs::exists("files/out.backup.basic.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.basic.json.bkp-2"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 3);

    EXPECT_TRUE(fs::exists("files/out.backup.basic.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.basic.json.bkp-3"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 4);

    EXPECT_TRUE(fs::exists("files/out.backup.basic.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-2"));
    EXPECT_TRUE(fs::exists("files/out.backup.basic.json.bkp-3"));
}

TEST(Backup, Single)
{
    size_t writeCalls = 0;
    auto doSave = [&] {
        std::error_code ec;
        Backup::saveWithBackup(
            "files/out.backup.single.json", {.enabled = true, .numSlots = 1},
            [&](const auto &path, auto &ec) {
                writeCalls++;
                std::ofstream of(path, std::ios::out);
                if (!of) {
                    ec = std::make_error_code(std::errc::io_error);
                    return;
                }
                of << "yo";
            },
            ec);
        return !ec;
    };

    RemoveFile("files/out.backup.single.json");
    RemoveFile("files/out.backup.single.json.bkp-1");

    EXPECT_TRUE(!fs::exists("files/out.backup.single.json"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 1);

    EXPECT_TRUE(fs::exists("files/out.backup.single.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-1"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 2);

    EXPECT_TRUE(fs::exists("files/out.backup.single.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.single.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-2"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 3);

    EXPECT_TRUE(fs::exists("files/out.backup.single.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.single.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-3"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 4);

    EXPECT_TRUE(fs::exists("files/out.backup.single.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.single.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.single.json.bkp-3"));
}

TEST(Backup, Disabled)
{
    size_t writeCalls = 0;
    auto doSave = [&] {
        std::error_code ec;
        Backup::saveWithBackup(
            "files/out.backup.disabled.json", {.enabled = false, .numSlots = 3},
            [&](const auto &path, auto &ec) {
                writeCalls++;
                std::ofstream of(path, std::ios::out);
                if (!of) {
                    ec = std::make_error_code(std::errc::io_error);
                    return;
                }
                of << "yo";
            },
            ec);
        return !ec;
    };

    RemoveFile("files/out.backup.disabled.json");
    RemoveFile("files/out.backup.disabled.json.bkp-1");
    RemoveFile("files/out.backup.disabled.json.bkp-2");
    RemoveFile("files/out.backup.disabled.json.bkp-3");

    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 1);

    EXPECT_TRUE(fs::exists("files/out.backup.disabled.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-1"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 2);

    EXPECT_TRUE(fs::exists("files/out.backup.disabled.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-2"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 3);

    EXPECT_TRUE(fs::exists("files/out.backup.disabled.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-3"));

    EXPECT_TRUE(doSave());
    EXPECT_EQ(writeCalls, 4);

    EXPECT_TRUE(fs::exists("files/out.backup.disabled.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.disabled.json.bkp-3"));
}

TEST(Backup, Failing)
{
    size_t writeCalls = 0;
    std::error_code ec;
    auto doSave = [&](bool fail) {
        Backup::saveWithBackup(
            "files/out.backup.failing.json", {.enabled = true, .numSlots = 3},
            [&](const auto &path, auto &ec) {
                writeCalls++;
                std::ofstream of(path, std::ios::out);
                if (fail) {
                    ec = std::make_error_code(std::errc::io_error);
                }
                of << "yo";
            },
            ec);
        return !ec;
    };

    RemoveFile("files/out.backup.failing.json");
    RemoveFile("files/out.backup.failing.json.bkp-1");
    RemoveFile("files/out.backup.failing.json.bkp-2");
    RemoveFile("files/out.backup.failing.json.bkp-3");

    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json"));

    EXPECT_TRUE(doSave(false));
    EXPECT_EQ(writeCalls, 1);

    EXPECT_TRUE(fs::exists("files/out.backup.failing.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-1"));

    EXPECT_TRUE(doSave(false));
    EXPECT_EQ(writeCalls, 2);

    EXPECT_TRUE(fs::exists("files/out.backup.failing.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-2"));

    EXPECT_FALSE(doSave(true));
    EXPECT_EQ(writeCalls, 3);
    EXPECT_EQ(ec, std::make_error_code(std::errc::io_error));
    ec = {};

    EXPECT_TRUE(fs::exists("files/out.backup.failing.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-3"));

    EXPECT_TRUE(doSave(false));
    EXPECT_EQ(writeCalls, 4);

    EXPECT_TRUE(fs::exists("files/out.backup.failing.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-3"));

    EXPECT_FALSE(doSave(true));
    EXPECT_EQ(writeCalls, 5);
    EXPECT_EQ(ec, std::make_error_code(std::errc::io_error));
    ec = {};

    EXPECT_TRUE(fs::exists("files/out.backup.failing.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.failing.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.failing.json.bkp-3"));
}
