// kdlibtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmock/gmock.h"
#include "kdlib/dbgengine.h"

class Environment : public ::testing::Environment {
public:
  virtual ~Environment() {}
  virtual void SetUp() {
      kdlib::initialize();
  }

  virtual void TearDown()
  {
      kdlib::uninitialize();
  }
};

int _tmain(int argc, _TCHAR* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);

    ::testing::AddGlobalTestEnvironment( new ::Environment );

    return RUN_ALL_TESTS();
}
