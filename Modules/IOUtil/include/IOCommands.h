/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_IOCOMMANDS_H
#define CAS_IOCOMMANDS_H

#include "IOUtilExports.h"

#include <mitkPointSet.h>

#include <vtkSmartPointer.h>

class QString;
class vtkMatrix4x4;


class IOUtil_EXPORT IOCommands
{

public:
  enum SeriesType{None, Patient, Instrument};

  struct RegistrationPointsData{

    RegistrationPointsData() :
      mPlannedPointsUsedInRegistration(mitk::PointSet::New()),
      mOriginalRealPointsUsedInRegistration(mitk::PointSet::New()),
      mTransformedRealPointsUsedInRegistration(mitk::PointSet::New()),
      mRegistrationType(None)
    {}

    mitk::PointSet::Pointer     mPlannedPointsUsedInRegistration;
    mitk::PointSet::Pointer     mOriginalRealPointsUsedInRegistration;
    mitk::PointSet::Pointer     mTransformedRealPointsUsedInRegistration;
    SeriesType                  mRegistrationType;
  };




  static void StoreCurrentPatientName(QString filePath);
  static QString GetCurrentPatientName();
  static QString GetCurrentPath();

  static bool CheckRegistrationInDisk(std::string *path = nullptr, SeriesType type = Patient);
  static void StoreRegistrationInDisk(const vtkMatrix4x4 *transform, SeriesType type, std::string filepath="");
  static vtkSmartPointer<vtkMatrix4x4> ReadRegistrationFromDisk(SeriesType type, std::string filepath="");

  static void RemoveStoredRegistration(SeriesType type);

  static bool ExportMeasurementsToCSV(QString filename, mitk::PointSet::Pointer planPs, mitk::PointSet::Pointer realPs);

  static void StoreRegistrationPoints(RegistrationPointsData data);

  static bool LoadRegistrationPoints(const QString &filename, SeriesType series, RegistrationPointsData &data);

	static QString GetSimulationPointsLastPath();

private:
  static void StoreCurrentPath(QString file, SeriesType type);
};


#endif // CAS_IOCOMMANDS_H
