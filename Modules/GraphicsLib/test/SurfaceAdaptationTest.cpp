/*===================================================================
The Medical Imaging Interaction Toolkit (MITK)
Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.
See LICENSE.txt or http://www.mitk.org for details.
===================================================================*/
// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
// MITK includes
#include <mitkStandaloneDataStorage.h>
// VTK includes
#include <vtkDebugLeaks.h>
// Module includes
#include "SurfaceAdaptation.h"

class SurfaceAdaptationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(SurfaceAdaptationTestSuite);
  // Test the append method
  MITK_TEST(AdaptationsRequired);
  CPPUNIT_TEST_SUITE_END();
private:
  SurfaceAdaptation* m_Data;
  mitk::DataStorage::Pointer mDs;

public:
  void setUp() override
  {
    m_Data = new SurfaceAdaptation;

    mDs = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    //mitk::DataNode::Pointer node = mitk::DataNode::New();
  }
  void tearDown() override
  {
    m_Data = nullptr;
    mDs = nullptr;
  }
  void AdaptationsRequired()
  {
    unsigned int required = m_Data->AdaptationsRequired(mDs);
    CPPUNIT_ASSERT_MESSAGE("Checking if no adaptations are required.", required == 0);
  }
};
MITK_TEST_SUITE_REGISTRATION(SurfaceAdaptation)
