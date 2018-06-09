#include "binding.h"
#include "v8/third_party/googletest/src/googletest/include/gtest/gtest.h"

TEST(SnapshotTest, InitializesCorrectly) {
  EXPECT_TRUE(true);
  // TODO add actual tests
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
