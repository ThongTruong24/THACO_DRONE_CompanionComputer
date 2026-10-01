#include <gtest/gtest.h>
#include "cc_telemetry/parameter_store.hpp"
#include <limits>
#include <unistd.h>
TEST(ParameterStore, ValidationPersistenceAndPending) {
  auto dir=std::filesystem::temp_directory_path()/("cc-store-test-"+std::to_string(getpid()));
  std::filesystem::remove_all(dir);
  cc::ParameterStore s(dir);
  s.add({"CC_LINK_HZ",1,.1,10,false,true});
  s.add({"CC_CAM_FPS",30,1,60,true,false});
  EXPECT_FALSE(s.set("CC_LINK_HZ",2,false).accepted);
  EXPECT_FALSE(s.set("CC_LINK_HZ",std::numeric_limits<double>::quiet_NaN(),true).accepted);
  EXPECT_FALSE(s.set("CC_CAM_FPS",30.5,true).accepted);
  EXPECT_FALSE(s.set("UNKNOWN",1,true).accepted);
  EXPECT_TRUE(s.set("CC_LINK_HZ",2,true).accepted);
  EXPECT_DOUBLE_EQ(s.active("CC_LINK_HZ"),2);
  EXPECT_TRUE(s.set("CC_CAM_FPS",20,true).pending);
  EXPECT_DOUBLE_EQ(s.active("CC_CAM_FPS"),30);
  cc::ParameterStore loaded(dir);
  loaded.add({"CC_LINK_HZ",1,.1,10,false,true});
  loaded.add({"CC_CAM_FPS",30,1,60,true,false});loaded.load();
  EXPECT_DOUBLE_EQ(loaded.active("CC_LINK_HZ"),2);
  EXPECT_DOUBLE_EQ(loaded.find("CC_CAM_FPS")->value,20);
  EXPECT_DOUBLE_EQ(loaded.active("CC_CAM_FPS"),30);
  std::filesystem::remove_all(dir);
}
TEST(ParameterStore, PersistFailureDoesNotApply) {
  cc::ParameterStore s("/dev/null/unwritable");s.add({"CC_LINK_HZ",1,.1,10,false,true});
  EXPECT_FALSE(s.set("CC_LINK_HZ",2,true).accepted);
  EXPECT_DOUBLE_EQ(s.find("CC_LINK_HZ")->value,1);
  EXPECT_DOUBLE_EQ(s.active("CC_LINK_HZ"),1);
}
