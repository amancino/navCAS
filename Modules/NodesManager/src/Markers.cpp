/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

// vtk
#include <vtkSmartPointer.h>
#include <vtkMath.h>

// qt
#include <QMessageBox>
#include <QString>
#include <QtSql>

#include "Markers.h"

using namespace std;


Markers::Markers()
{
  
}

Markers::~Markers()
{

}



/* Detect probe and update its geometry*/
void Markers::WriteGeometryFile(MarkerFileType type, unsigned int serie, const vector<mitk::Point3D> &fiducials, mitk::Point3D tip)
{
#ifdef WIN32
	QString filename("../");
#else
	QString filename("");
#endif // WIN32

	QString fileType = type == Temporal ? "_temporal" : "_final";
	filename += QString("../data/geometry_" + QString::number(serie) + fileType + "_probe.ini");

	// Create csv file
	// to do: check file path for windows
	std::ofstream data;
	data.open(filename.toStdString());

	data << "[geometry]" << std::endl;
	data << "count=" << fiducials.size() << std::endl;
	data << "id=" << serie << std::endl;

	for (unsigned int i = 0; i < fiducials.size(); i++)
	{
		data << "[fiducial" << i << "]" << std::endl;
		data << "x=" << fiducials[i][0] - fiducials[0][0] << std::endl;
		data << "y=" << fiducials[i][1] - fiducials[0][1] << std::endl;
		data << "z=" << fiducials[i][2] - fiducials[0][2] << std::endl;
	}

	if ((type == Temporal) || (type == Final))
	{
		data << "[tip]" << std::endl;
		data << "x=" << tip[0] - fiducials[0][0] << std::endl;
		data << "y=" << tip[1] - fiducials[0][1] << std::endl;
		data << "z=" << tip[2] - fiducials[0][2] << std::endl;
	}

	data.close();

	cout << "New geometry file written: " << filename.toStdString() << std::endl;
}

void Markers::UpdateTemporalProbeTipGeometryFile(MarkerFileType type, unsigned int serie, const mitk::Point3D tip)
{
#ifdef WIN32
  QString filename("../");
#else
  QString filename("");
#endif // WIN32

  QString fileType = type == Temporal ? "_temporal" : "_final";
  filename += QString("../data/geometry_" + QString::number(serie) + fileType + "_probe.ini");

  // Create csv file
  // to do: check file path for windows
  std::fstream data;
  data.open(filename.toStdString());

  std::string line;
  std::getline(data,line); // [geoemtry]
  std::getline(data,line); // count
  std::getline(data,line); // id

  // 5 fiducials
  for (unsigned int i = 0; i < 5; i++)
  {
    std::getline(data,line);
    std::getline(data,line);
    std::getline(data,line);
    std::getline(data,line);
  }

  data << "[tip]" << std::endl;
  data << "x=" << tip[0] << std::endl;
  data << "y=" << tip[1] << std::endl;
  data << "z=" << tip[2] << std::endl;

  data.close();

  cout << "Temporal probe geometry file updated: " << filename.toStdString() << std::endl;
}


QSqlDatabase* Markers::CreateCalibrationTable(QString tableName)
{
	cout << "Creating acquisitions data base table" << std::endl;

	// read PERSON table using SQL
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	QString filename = "../calibration/" + tableName + ".sqlite";
	db.setDatabaseName(filename);
	if (db.open())
	{
		// creating the table twice leads to error
		QString ins = "CREATE TABLE calibration ("
			"acquisition integer primary key,"
			"matrix00 double,"
			"matrix01 double,"
			"matrix02 double,"
			"matrix03 double,"
			"matrix10 double,"
			"matrix11 double,"
			"matrix12 double,"
			"matrix13 double,"
			"matrix20 double,"
			"matrix21 double,"
			"matrix22 double,"
			"matrix23 double,"
			"matrix30 double,"
			"matrix31 double,"
			"matrix32 double,"
			"matrix33 double,"
			"TemporalProbePositionX double,"
			"TemporalProbePositionY double,"
			"TemporalProbePositionZ double)";
		QSqlQuery query;
		if (!query.exec(ins))
			cout << "Error creating table" << std::endl;
		else
			cout << "Table created succesfully" << std::endl;
	}
	else
	{
		QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
			QObject::tr("Unable to establish a database connection.\n"
				"This example needs SQLite support. Please read "
				"the Qt SQL driver documentation for information how "
				"to build it.\n\n"
				"Click Cancel to exit."), QMessageBox::Cancel);
	}

	// return db pointer
	QSqlDatabase *dbp = new QSqlDatabase;
	*dbp = db;
	return dbp;
}

void Markers::StoreCalibrationData(QSqlDatabase* db, int index, const vtkSmartPointer<vtkMatrix4x4> mat, const mitk::Point3D temporalProbePosition)
{
	if (!db->isOpen())
	{
		if (!db->open())
		{
			cout << "Could not open database" << std::endl;
			return;
		}
	}

	// store new row
	QSqlQuery query;
	query.prepare("insert into calibration("
		"acquisition,"
		"matrix00,"
		"matrix01,"
		"matrix02,"
		"matrix03,"
		"matrix10,"
		"matrix11,"
		"matrix12,"
		"matrix13,"
		"matrix20,"
		"matrix21,"
		"matrix22,"
		"matrix23,"
		"matrix30,"
		"matrix31,"
		"matrix32,"
		"matrix33,"
		"TemporalProbePositionX,"
		"TemporalProbePositionY,"
		"TemporalProbePositionZ)"
		"VALUES (?,"
		"?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,"
		"?,?,?);");
	query.addBindValue(index);
	query.addBindValue(mat->GetElement(0, 0));
	query.addBindValue(mat->GetElement(0, 1));
	query.addBindValue(mat->GetElement(0, 2));
	query.addBindValue(mat->GetElement(0, 3));
	query.addBindValue(mat->GetElement(1, 0));
	query.addBindValue(mat->GetElement(1, 1));
	query.addBindValue(mat->GetElement(1, 2));
	query.addBindValue(mat->GetElement(1, 3));
	query.addBindValue(mat->GetElement(2, 0));
	query.addBindValue(mat->GetElement(2, 1));
	query.addBindValue(mat->GetElement(2, 2));
	query.addBindValue(mat->GetElement(2, 3));
	query.addBindValue(mat->GetElement(3, 0));
	query.addBindValue(mat->GetElement(3, 1));
	query.addBindValue(mat->GetElement(3, 2));
	query.addBindValue(mat->GetElement(3, 3));
	query.addBindValue(temporalProbePosition[0]);
	query.addBindValue(temporalProbePosition[1]);
	query.addBindValue(temporalProbePosition[2]);

	if (!query.exec())
		cout << "Error storing calibration data" << std::endl;

	// db intentionally left open
}

bool Markers::ReadCalibrationData(QSqlDatabase* db,
	std::vector<vtkSmartPointer<vtkMatrix4x4>> &matVec, 
  std::vector<mitk::Point3D> &tipVec,
  std::vector<mitk::NavigationData::Pointer> &ndVec)
{
	if (!db->isOpen())
	{
		if (!db->open()) {
			cout << "Cannot open database" << std::endl;
			return false;
		}
	}

	matVec.clear();
	tipVec.clear();

	// get stored number of stimulations
	QSqlQuery query("SELECT * FROM calibration");

	while (query.next())
	{
		int acquisition = query.value("acquisition").toInt();
		
    vnl_matrix_fixed<double,3,3> vnlMatrix;
		vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				QString position("matrix");
				position += QString::number(i);
				position += QString::number(j);

        const double val = query.value(position).toDouble();
        mat->SetElement(i, j, val);

        if ((i < 3) && (j<3))
          vnlMatrix.set(i,j,val);
			}
		}
		mitk::Point3D tip;
		tip[0] = query.value("TemporalProbePositionX").toDouble();
		tip[1] = query.value("TemporalProbePositionY").toDouble();
		tip[2] = query.value("TemporalProbePositionZ").toDouble();

		tipVec.push_back(tip);
		matVec.push_back(mat);

    // build nd
    auto nd = mitk::NavigationData::New();
    nd->SetOrientation(mitk::Quaternion(vnlMatrix));
    nd->SetPosition(tip);
    nd->SetDataValid(true);
    ndVec.push_back(nd);
		
		//cout << "acquisition: " << acquisition << ", mat00 = " << mat00 << std::endl;
	}

	db->close();
	return true;
}

mitk::Point3D Markers::GetProbeTipFromFiducials(unsigned int serie, std::vector<mitk::Point3D> &fiducials)
{
	mitk::Point3D tip(0.0);
	mitk::Vector3D dir;
	unsigned int lastFiducial = 0;
	bool found = false;
	const double tol = 1.0;

	// pasive 5-fiducials probe
	if (serie == 124)
	{
		// search for distances of 75 and 145 (tol=1mm)
		for (unsigned int x = 0; x < fiducials.size(); x++)
		{
			for (unsigned int y = 0; y < fiducials.size(); y++)
			{
				if (x == y)
					continue;

				// first find distance of 75
				double dif = sqrt(vtkMath::Distance2BetweenPoints(fiducials[x].GetDataPointer(), fiducials[y].GetDataPointer()));
				if ((dif < (75.0 + tol)) && (dif > (75.0 - tol)))
				{
					for (unsigned int z = 0; z < fiducials.size(); z++)
					{
						// search distance of 145 between x and z
						double dif = sqrt(vtkMath::Distance2BetweenPoints(fiducials[x].GetDataPointer(), fiducials[z].GetDataPointer()));
						if ((dif < (145.0 + tol)) && (dif > (145.0 - tol)))
						{
							// order is: x -> y -> z -> tip
							cout << "fid" << x << " -> fid" << y << " -> fid" << z << " -> tip" << std::endl;
							dir = fiducials[z] - fiducials[y];
							lastFiducial = z;
							found = true;
						}
					}
					// search distance of 145 between y and z
					if (!found)
					{
						for (unsigned int z = 0; z < fiducials.size(); z++)
						{
							// search distance of 145 between x and z
							double dif = sqrt(vtkMath::Distance2BetweenPoints(fiducials[y].GetDataPointer(), fiducials[z].GetDataPointer()));
							if ((dif < (145.0 + tol)) && (dif > (145.0 - tol)))
							{
								// order is: y -> x -> z -> tip
								cout << "fid" << y << " -> fid" << x << " -> fid" << z << " -> tip" << std::endl;
								dir = fiducials[z] - fiducials[x];
								lastFiducial = z;
								found = true;
							}
						}
					}
					if (!found)
					{
						cerr << "Error: acquired fiducials do not match with passive probe marker" << std::endl;
						return tip;
					}

					dir.Normalize();
					cout << "Normalized dir: " << dir << std::endl;
					tip = fiducials[lastFiducial] + dir * 160; // best guess
					return tip;
				}
			}
		}
	}
	// neuro line probe (no tip info actually needed)
	else if (serie == 223)
	{
		mitk::Vector3D dir = fiducials[1] - fiducials[0];
		dir.Normalize();
		tip = fiducials[1] + dir * 153;
	}
	return tip;
}
