/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
// qt include
#include <QtSql>
// MITK includes
#include <mitkStandaloneDataStorage.h>
#include <mitkPivotCalibration.h>
// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkMath.h>
// Module includes
#include "Markers.h"

class MarkersTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(MarkersTestSuite);
  // Test the append method
	MITK_TEST(Probe);
	MITK_TEST(TemporalProbe);
	MITK_TEST(ReadDB);
	MITK_TEST(CorrectProbe);
  CPPUNIT_TEST_SUITE_END();
private:
  Markers* m_Data;
  mitk::DataStorage::Pointer mDs;
  //QSqlDatabase* m_db;

public:
  void setUp() override
  {
    m_Data = new Markers;

    mDs = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    //mitk::DataNode::Pointer node = mitk::DataNode::New();
  }
	
  void tearDown() override
  {
    m_Data = nullptr;
    mDs = nullptr;
  }

	void Probe()
	{
		// sorted acquisitions with tip put by hand
		std::vector<mitk::Point3D> fiducials;
		double p0[3] = { 0.0, 0.0, 0.0 };
		double p1[3] = { 112.217, -5.59278, 10.0763 };
		double p2[3] = { 62.9886, -30.3898, 6.39166 };
		double p3[3] = { 130.569, -62.5927, 14.1854 };
		double p4[3] = { 68.127,  -80.2376, 8.85944 };
		mitk::Point3D point0(p0);
		mitk::Point3D point1(p1);
		mitk::Point3D point2(p2);
		mitk::Point3D point3(p3);
		mitk::Point3D point4(p4);
		fiducials.push_back(point0);
		fiducials.push_back(point1);
		fiducials.push_back(point2);
		fiducials.push_back(point3);
		fiducials.push_back(point4);

		//mitk::Point3D tip = Markers::GetProbeTipFromFiducials(124, fiducials);
		double tip[3] = { -143.507, 69.237, -14.5621 };

		double dist = sqrt(vtkMath::Distance2BetweenPoints(fiducials[3].GetDataPointer(), tip));
		cout << "Distance: " << dist << std::endl;
		double TOL = 1.0;
		CPPUNIT_ASSERT_MESSAGE("Checking if Probe geometry is right.", (dist+TOL > 305.0) && (dist-TOL < 305.0));
	}
	
  void TemporalProbe()
  {
		// unsorted acquisitions
		std::vector<mitk::Point3D> fiducials;
		double p0[3] = { 0.0, 0.0, 0.0 };
		double p1[3] = { 106.204, -35.8875, 13.0632 };
		double p2[3] = { 52.7789, -46.2794, 2.94952 };
		double p3[3] = { 109.577, -95.5408, 7.3399 };
		double p4[3] = { 45.31, -95.4464, -3.29095 };
		mitk::Point3D point0(p0);
		mitk::Point3D point1(p1);
		mitk::Point3D point2(p2);
		mitk::Point3D point3(p3);
		mitk::Point3D point4(p4);
		fiducials.push_back(point0);
		fiducials.push_back(point1);
		fiducials.push_back(point2);
		fiducials.push_back(point3);
		fiducials.push_back(point4);

		mitk::Point3D tip = Markers::GetProbeTipFromFiducials(124, fiducials);
		
		double dist = sqrt(vtkMath::Distance2BetweenPoints(fiducials[3].GetDataPointer(), tip.GetDataPointer()));
		cout << "Distance: " << dist << std::endl;
		double TOL = 1.0;
		CPPUNIT_ASSERT_MESSAGE("Checking if Temporal Probe geometry is right.", (dist + TOL > 305.0) && (dist - TOL < 305.0));
  }

	void ReadDB()
	{
		std::vector<mitk::Point3D> tip;
		std::vector< vtkSmartPointer<vtkMatrix4x4> > mat;
    std::vector<mitk::NavigationData::Pointer> nd;
    QDir testPath("../Modules/NodesManager/test");
    QString currentPath = testPath.absolutePath();

    // open database
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(currentPath + "/data/calibration1.sqlite");
    bool read = Markers::ReadCalibrationData(&db, mat, tip, nd);

		CPPUNIT_ASSERT_MESSAGE("Checking if the 100 rows of DB have been correctly read.", read&&(tip.size()==100)&&(mat.size()==100));
	}

	void CorrectProbe()
	{
		std::vector<mitk::Point3D> tip;
		std::vector< vtkSmartPointer<vtkMatrix4x4> > mat;
    std::vector<mitk::NavigationData::Pointer> ndVec;

    QDir testPath("../Modules/NodesManager/test");
    QString currentPath = testPath.absolutePath();

		// open database
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(currentPath + "/data/calibration2.sqlite");
    Markers::ReadCalibrationData(&db, mat, tip, ndVec);

    // use MITK pivot calibration
    auto calib = mitk::PivotCalibration::New();
    for (unsigned int i=0; i<ndVec.size(); ++i)
      calib->AddNavigationData(ndVec[i]);

    calib->ComputePivotResult();
    MITK_INFO << "pivot point: " << calib->GetResultPivotPoint();
    MITK_INFO << "rms: " << calib->GetResultRMSError();
		
    //CPPUNIT_ASSERT_MESSAGE("Checking if the correction lowered the measured error.", (errorAfterCorrection < measuredError)&&(dif.GetNorm()>0.01));
	}
};
MITK_TEST_SUITE_REGISTRATION(Markers)
