/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>
#include <fstream>
#include <sstream>

#include <QSettings>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QtSql>

#include <vtkMatrix4x4.h>

#include "IOCommands.h"

using namespace std;

void IOCommands::StoreCurrentPatientName(QString casSceneFilePath)
{
  QSettings settings("CAS","navCAS");
  QString name(casSceneFilePath);
  size_t pos = name.toStdString().find_last_of("/");
  name = name.right(name.size()-static_cast<int>(pos)-1);
  name.chop(4);
  settings.setValue("Patient",name);

  StoreCurrentPath(casSceneFilePath,Patient);
  StoreCurrentPath(casSceneFilePath+"_instrument",Instrument);
}

QString IOCommands::GetCurrentPatientName()
{
  QSettings settings("CAS","navCAS");
  return settings.value("Patient").toString();
}

void IOCommands::RemoveStoredRegistration(SeriesType type)
{
  std::string path;
  CheckRegistrationInDisk(&path,type);

  cout << "Removing registration file " << path << std::endl;

  if (remove(path.c_str()) != 0)
    cout << "Error removing file" << std::endl;
  else
    cout << "File succesfully removed" << std::endl;
}

void IOCommands::StoreCurrentPath(QString file, SeriesType type)
{
  QSettings settings("CAS","navCAS");

  if (type == Patient)
    settings.setValue("Current patient path",file);
  else if (type == Instrument)
    settings.setValue("Current instrument path",file);
}

QString IOCommands::GetCurrentPath()
{
  QSettings settings("CAS","navCAS");
  return settings.value("Current patient path").toString();
}

bool IOCommands::CheckRegistrationInDisk(std::string* path, SeriesType type)
{
  QSettings settings("CAS","navCAS");
  QString scenePath;
  if (type == Patient)
    scenePath = settings.value("Current patient path").toString();
  else if (type == Instrument)
    scenePath = settings.value("Current instrument path").toString();

  scenePath.chop(4);
  scenePath += "_registration.txt";

  if (path != nullptr)
    *path = scenePath.toStdString();

  std::ifstream infile(scenePath.toStdString());
  return infile.good();
}

void IOCommands::StoreRegistrationInDisk(const vtkMatrix4x4* transform, SeriesType type, std::string filepath)
{
  std::string path = filepath;
  if (filepath.empty() && (type != SeriesType::None))
    CheckRegistrationInDisk(&path, type);

  cout << "Writing registration matrix to " << path << std::endl;

  ofstream myfile;
  myfile.open(path);

  // store registration
  for (int i=0; i<4; i++)
  {
    for (int j=0; j<4; j++)
      myfile << transform->GetElement(i,j) << ";";
    myfile << std::endl;
  }
  myfile.close();
}

vtkSmartPointer<vtkMatrix4x4> IOCommands::ReadRegistrationFromDisk(SeriesType type, string filepath)
{
  auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();

  std::string path = filepath;
  if (path.empty() && (type != SeriesType::None))
    CheckRegistrationInDisk(&path,type);

  cout << "Reading registration matrix from " << path << std::endl;

  ifstream myfile(path);
  if (!myfile.is_open())
  {
    cout << "No registration file found for this scene" << std::endl;
    return nullptr;
  }

  setlocale(LC_ALL,"C");

  // 4 rows
  string line;
  for (int i=0; i<4; i++)
  {
    if (!getline(myfile,line))
    {
      cerr << "Invalid registration file" << std::endl;
      return nullptr;
    }

    // read 4 columns
    stringstream  lineStream(line);
    string cell;
    for (int j=0; j<4; j++)
    {
      getline(lineStream, cell, ';');

      double value = std::stod(cell);
      matrix->SetElement(i,j,value);
    }
		cout << std::endl;
  }
  myfile.close();

  return matrix;
}

bool IOCommands::ExportMeasurementsToCSV(QString filename, mitk::PointSet::Pointer planPs, mitk::PointSet::Pointer realPs)
{
  // open CSV file
  QFile data(filename);

  // if writing annular, overwrite file
  QFile::OpenMode openMode = QFile::WriteOnly | QFile::Truncate;

  if(!data.open(openMode))
  {
    QMessageBox msgBox;
    msgBox.setText("Error writing file.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return false;
  }

  // write csv header
  QTextStream output(&data);
	output << "n;planned X;planned Y;planned Z;real X;real Y;real Z" << endl;

  // write csv line
  for (int i=0; i<planPs->GetSize(); i++)
  {
    output << QString::number(i+1) + ";";

    mitk::Point3D pp, rp;
    if (!planPs->GetPointIfExists(i,&pp))
    {
			output << endl;
      continue;
    }

    output << QString::number(pp[0]) + ";" + QString::number(pp[1]) + ";" + QString::number(pp[0]) + ";";

    if (!realPs->GetPointIfExists(i,&rp))
    {
			output << endl;
      continue;
    }

    output << QString::number(rp[0]) + ";" + QString::number(rp[1]) + ";" + QString::number(rp[0]);
		output << endl;
  }

  data.close();
  return true;
}

void IOCommands::StoreRegistrationPoints(RegistrationPointsData data)
{
  int nOriginalReal = data.mOriginalRealPointsUsedInRegistration->GetSize();
  int nPlanned = data.mPlannedPointsUsedInRegistration->GetSize();
  int nTransformedReal = data.mTransformedRealPointsUsedInRegistration->GetSize();

  if ( (nOriginalReal == 0) || (nPlanned == 0) || (nTransformedReal == 0) )
  {
    cerr << "Error trying to store registration data. Pointset is empty" << std::endl;
    return;
  }

  if ( (nOriginalReal != nPlanned) || (nOriginalReal != nTransformedReal) )
  {
    cerr << "Error trying to store registration data. Sizes do not match" << std::endl;
    return;
  }

  if (data.mRegistrationType == None)
  {
    cerr << "Error trying to store registration data. Registration type was not defined" << std::endl;
    return;
  }

  // write data to local database

  // read PERSON table using SQL
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  auto scenePath = GetCurrentPath();
  scenePath.chop(4);
  QString filename = scenePath + "_registrationData" + ".sqlite";
	cout << "Database file: " << filename.toStdString() << std::endl;
  db.setDatabaseName(filename);
  if (!db.open())
  {
    QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
        QObject::tr("Unable to establish a database connection.\n"
                    "This example needs SQLite support. Please read "
                    "the Qt SQL driver documentation for information how "
                    "to build it.\n\n"
                    "Click Cancel to exit."), QMessageBox::Cancel);
    return;
  }

  // creating the table twice leads to an error
  QString type;
  if (data.mRegistrationType == Patient)
  {
    cout << "Creating patient registration table" << std::endl;
    type = "patient";
  }
  else
  {
    cout << "Creating instrument registration table" << std::endl;
    type = "instrument";
  }

  QString ins = "CREATE TABLE " + type + " ("
          "n integer primary key,"
          "plannedX double,"
          "plannedY double,"
          "plannedZ double,"
          "originalRealX double,"
          "originalRealY double,"
          "originalRealZ double,"
          "transformedRealX double,"
          "transformedRealY double,"
          "transformedRealZ double"
          ")";

  QSqlQuery createQuery;
  if (!createQuery.exec(ins))
  {
    QMessageBox::critical(nullptr, QObject::tr("Error creating registration table"),QObject::tr("Try renaming the old file"));
    return;
  }
  else
    cout << "Registration table created succesfully" << std::endl;


  // fill table
  // get stored number of stimulations
  QSqlQuery writeQuery;
  writeQuery.exec("SELECT * FROM " + type);

  bool failed = false;
  for (int n = 0; n<data.mTransformedRealPointsUsedInRegistration->GetSize(); n++)
  {
    auto trp = data.mTransformedRealPointsUsedInRegistration;
    auto orp = data.mOriginalRealPointsUsedInRegistration;
    auto pp = data.mPlannedPointsUsedInRegistration;

    // store new row
    writeQuery.prepare("insert into " + type+ "("
                  "n,"
                  "plannedX,"
                  "plannedY,"
                  "plannedZ,"
                  "originalRealX,"
                  "originalRealY,"
                  "originalRealZ,"
                  "transformedRealX,"
                  "transformedRealY,"
                  "transformedRealZ"
                  ")"
                  "VALUES (?,?,?,?,?,?,?,?,?,?);");
    writeQuery.addBindValue(n);
    writeQuery.addBindValue(pp->GetPoint(n)[0]);
    writeQuery.addBindValue(pp->GetPoint(n)[1]);
    writeQuery.addBindValue(pp->GetPoint(n)[2]);
    writeQuery.addBindValue(orp->GetPoint(n)[0]);
    writeQuery.addBindValue(orp->GetPoint(n)[1]);
    writeQuery.addBindValue(orp->GetPoint(n)[2]);
    writeQuery.addBindValue(trp->GetPoint(n)[0]);
    writeQuery.addBindValue(trp->GetPoint(n)[1]);
    writeQuery.addBindValue(trp->GetPoint(n)[2]);

    if (!writeQuery.exec())
    {
      failed = true;
      cout << "Error storing registration data " << n << std::endl;
    }
  }

  db.close();

  if (failed)
    QMessageBox::critical(nullptr, QObject::tr("Error storing some of the registration data"),QObject::tr("Check the queries"));
  else
    cout << "Data succesfully stored in " << filename.toStdString() << std::endl;

	// store path in settings
	QSettings settings("CAS", "navCAS");
	settings.setValue("Registration points for simulation", filename);
}

bool IOCommands::LoadRegistrationPoints(const QString& filename, SeriesType series, RegistrationPointsData& data)
{
  // read PERSON table using SQL
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(filename);

  if (!db.isOpen())
  {
    if (!db.open()) {
      cout << "Cannot open database" << std::endl;
      return false;
    }
  }

  QString type = series == Patient ? "patient" : "instrument";

  data.mRegistrationType = series;

  // get stored number of points
  QSqlQuery query("SELECT * FROM " + type);

  while (query.next())
  {
    int n = query.value("n").toInt();

    mitk::Point3D planned;
    planned[0] = query.value("plannedX").toDouble();
    planned[1] = query.value("plannedY").toDouble();
    planned[2] = query.value("plannedZ").toDouble();
    data.mPlannedPointsUsedInRegistration->InsertPoint(n,planned);

    mitk::Point3D original;
    original[0] = query.value("originalRealX").toDouble();
    original[1] = query.value("originalRealY").toDouble();
    original[2] = query.value("originalRealZ").toDouble();
    data.mOriginalRealPointsUsedInRegistration->InsertPoint(n,original);

    mitk::Point3D transformed;
    transformed[0] = query.value("transformedRealX").toDouble();
    transformed[1] = query.value("transformedRealY").toDouble();
    transformed[2] = query.value("transformedRealZ").toDouble();
    data.mTransformedRealPointsUsedInRegistration->InsertPoint(n,transformed);
  }

  db.close();

	// store path in settings
	QSettings settings("CAS", "navCAS");
	settings.setValue("Registration points for simulation", filename);

  return true;
}

QString IOCommands::GetSimulationPointsLastPath()
{
	QSettings settings("CAS", "navCAS");
	QString path = settings.value("Registration points for simulation", "/home/").toString();

	return path;
}
