#include <stdafx.h>

#include "memdumpfixture.h"

#include "kdlib/tagged.h"

using namespace kdlib;

class TaggedTest : public MemDumpFixture
{
public:
    TaggedTest() : MemDumpFixture(MemDumpFixture::getKernelDumpName()) {}
};

struct __declspec(uuid("D03DC06F-D88E-44C5-BA2A-FAE035172D19")) ExistingId {};
struct __declspec(uuid("88597A32-1493-41CA-BF87-2A950DF4CEE0")) NonExistingId {};

TEST_F(TaggedTest, enum)
{
    std::list<TaggedId> ids;

    ASSERT_NO_THROW( ids = enumTagged() );

    EXPECT_NE( std::find(ids.cbegin(), ids.cend(), __uuidof(ExistingId)), ids.cend() );

    EXPECT_EQ( std::find(ids.cbegin(), ids.cend(), __uuidof(NonExistingId)), ids.cend() );
}

TEST_F(TaggedTest, load)
{
    TaggedBuffer buff;

    ASSERT_NO_THROW( buff = loadTaggedBuffer(__uuidof(ExistingId)) );
    EXPECT_EQ( buff.size(), 0x410 );

    EXPECT_THROW( loadTaggedBuffer(__uuidof(NonExistingId)), DbgException );
}
