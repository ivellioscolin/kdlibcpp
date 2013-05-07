// kdlibtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gtest/gtest.h"
#include "kdlib/dbgengine.h"
#include "processtest.h"
#include "moduletest.h"
#include "memorytest.h"
#include "typeinfotest.h"
#include "typedvartest.h"
#include "varianttest.h"

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
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment( new Environment );

    return RUN_ALL_TESTS();
}

