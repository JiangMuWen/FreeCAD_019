// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include "Base/Exception.h"
#include "Base/Writer.h"

// Writer is designed to be a base class, so for testing we actually instantiate a StringWriter,
// which is derived from it

class WriterTest: public ::testing::Test
{
protected:
    // void SetUp() override {}

    // void TearDown() override {}
protected:
    Base::StringWriter _writer;
};

TEST_F(WriterTest, insertTextSimple)
{
    // Arrange
    std::string testTextData {"Simple ASCII data"};
    std::string expectedResult {"<![CDATA[" + testTextData + "]]>"};

    // Act
    _writer.insertText(testTextData);

    // Assert
    EXPECT_EQ(expectedResult, _writer.getString());
}

/// If the data happens to actually include an XML CDATA close marker, that needs to be "escaped" --
/// this is done by breaking it up into two separate CDATA sections, splitting apart the marker.
TEST_F(WriterTest, insertTextNeedsEscape)
{
    // Arrange
    std::string testDataA {"ASCII data with a close marker in it, like so: ]]"};
    std::string testDataB {"> "};
    std::string expectedResult {"<![CDATA[" + testDataA + "]]><![CDATA[" + testDataB + "]]>"};

    // Act
    _writer.insertText(testDataA + testDataB);

    // Assert
    EXPECT_EQ(expectedResult, _writer.getString());
}

TEST_F(WriterTest, insertNonAsciiData)
{
    // Arrange
    std::string testData {"\x01\x02\x03\x04\u0001F450😀"};
    std::string expectedResult {"<![CDATA[" + testData + "]]>"};

    // Act
    _writer.insertText(testData);

    // Assert
    EXPECT_EQ(expectedResult, _writer.getString());
}

TEST_F(WriterTest, beginCharStream)
{
    // Arrange & Act
    auto& checkStream {_writer.beginCharStream()};

    // Assert
    EXPECT_TRUE(checkStream.good());
}

TEST_F(WriterTest, beginCharStreamTwice)
{
    // Arrange
    _writer.beginCharStream();

    // Act & Assert
    EXPECT_THROW(_writer.beginCharStream(), Base::RuntimeError);
}

TEST_F(WriterTest, endCharStream)
{
    // Arrange
    _writer.beginCharStream();

    // Act
    _writer.endCharStream();

    // Assert
    EXPECT_EQ("<![CDATA[]]>", _writer.getString());
}

TEST_F(WriterTest, endCharStreamTwice)
{
    // Arrange
    _writer.beginCharStream();
    _writer.endCharStream();

    // Act
    _writer.endCharStream();  // Doesn't throw, or do anything at all

    // Assert
    EXPECT_EQ("<![CDATA[]]>", _writer.getString());
}

TEST_F(WriterTest, charStream)
{
    // Arrange
    auto& streamA {_writer.beginCharStream()};

    // Act
    auto& streamB {_writer.charStream()};

    // Assert
    EXPECT_EQ(&streamA, &streamB);
}

TEST_F(WriterTest, charStreamBase64Encoded)
{
    // Arrange
    auto& stream {_writer.beginCharStream(Base::CharStreamFormat::Base64Encoded)};
    std::string data {"FreeCAD rocks! 🪨🪨🪨"};

    // Act
    _writer.charStream() << data;
    _writer.endCharStream();

    // Assert
    // Conversion done using https://www.base64encode.org for testing purposes
    EXPECT_EQ(std::string("RnJlZUNBRCByb2NrcyEg8J+qqPCfqqjwn6qo\n"), _writer.getString());
}
