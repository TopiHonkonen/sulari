#include <gtest/gtest.h>
#include "../src/time_parse.h"

TEST(TimeParserTest, TestCaseCorrectTime) {
	char time_test[] = "000005";
	EXPECT_EQ(time_parse(time_test), 5);

	char time_test2[] = "000105";
	EXPECT_EQ(time_parse(time_test2), 65);

	char time_test3[] = "010105";
	EXPECT_EQ(time_parse(time_test3), 3665);
}

TEST(TimeParserTest, TestCaseIncorrectTime) {
	char time_test[] = "000070";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test2[] = "008800";
	EXPECT_EQ(time_parse(time_test2), TIME_VALUE_ERROR);

	char time_test3[] = "240000";
	EXPECT_EQ(time_parse(time_test3), TIME_VALUE_ERROR);

}

// Test minimum and maximum value boundaries
TEST(TimeParserTest, TestBoundaryCheck) {
	char time_test[] = "000001";
	EXPECT_EQ(time_parse(time_test), 1);
	char time_test[] = "0000-1";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test[] = "000100";
	EXPECT_EQ(time_parse(time_test), 60);
	char time_test[] = "00-100";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test[] = "010000";
	EXPECT_EQ(time_parse(time_test), 3600);
	char time_test[] = "-10000";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test[] = "000059";
	EXPECT_EQ(time_parse(time_test), 59);
	char time_test[] = "000060";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test[] = "005900";
	EXPECT_EQ(time_parse(time_test), 3540);
	char time_test[] = "006000";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

	char time_test[] = "230000";
	EXPECT_EQ(time_parse(time_test), 82800);
	char time_test[] = "240000";
	EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}
