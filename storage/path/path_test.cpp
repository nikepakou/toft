// Copyright (c) 2012, The Toft Authors. All rights reserved.
// Author: Liu Xiaokang <hsiaokangliu@tencent.com>

#include "toft/storage/path/path.h"
#include <string>
#include <vector>
#include "thirdparty/gtest/gtest.h"

namespace toft {

TEST(Path, ToAbsolute)
{
    char cwd_buf[4096];
    std::string cwd = getcwd(cwd_buf, sizeof(cwd_buf));
    std::string filepath = "path_test";
    std::string fullpath = cwd + "/" + filepath;
    EXPECT_EQ(fullpath, Path::ToAbsolute(filepath));
}

TEST(Path, GetBaseName)
{
    EXPECT_EQ("", Path::GetBaseName("/"));
    EXPECT_EQ("a", Path::GetBaseName("a"));
    EXPECT_EQ("a", Path::GetBaseName("a/"));
    EXPECT_EQ("a.txt", Path::GetBaseName("a.txt"));
    EXPECT_EQ("a.txt", Path::GetBaseName("dir/a.txt"));
    EXPECT_EQ("a", Path::GetBaseName("dir/a"));
}

TEST(Path, GetExtension)
{
    EXPECT_EQ("", Path::GetExtension(""));
    EXPECT_EQ("", Path::GetExtension("/"));
    EXPECT_EQ("", Path::GetExtension("abc"));
    EXPECT_EQ(".txt", Path::GetExtension("abc.txt"));
    EXPECT_EQ(".", Path::GetExtension("abc."));
}

TEST(Path, GetDirectory)
{
    EXPECT_EQ(".", Path::GetDirectory(""));
    EXPECT_EQ("/", Path::GetDirectory("/"));
    EXPECT_EQ("/", Path::GetDirectory("/a"));
    EXPECT_EQ(".", Path::GetDirectory("abc"));
    EXPECT_EQ(".", Path::GetDirectory("abc/"));
    EXPECT_EQ("abc", Path::GetDirectory("abc/d"));
}

TEST(Path, Normalize)
{
    EXPECT_EQ(".", Path::Normalize(""));
    EXPECT_EQ("/", Path::Normalize("///"));
    EXPECT_EQ("//", Path::Normalize("//"));
    EXPECT_EQ("//abc", Path::Normalize("//abc"));
    EXPECT_EQ("/a/b/c", Path::Normalize("///a//b/c//"));
    EXPECT_EQ("../..", Path::Normalize("../../"));
    EXPECT_EQ("../../abc", Path::Normalize("../../abc"));
    EXPECT_EQ("/abc", Path::Normalize("/data/../abc"));
    EXPECT_EQ("/", Path::Normalize("/abc/../../../"));
}

} // namespace toft
