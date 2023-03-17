/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <QtSql>
#include <QString>
#include <QMessageBox>

#include "RegistrationPointsMetrics.h"

using namespace std;

void RegistrationPointsMetrics::ExportMetricsToFile(const std::vector<RegistrationPointsMetrics> &vec, const QString &filename)
{
  // write data to local database

  // read PERSON table using SQL
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
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

  QString tableName("REGISTRATION_POINTS_COMBINATIONS");
  unsigned int startingC = 0;

  // creating the table twice leads to an error
  if (!db.tables().contains( tableName ))
  {
    cout << "Creating registration points metrics table" << std::endl;

    QString ins = QString("CREATE TABLE ") + tableName + " ("
            "combination INT PRIMARY KEY,"

            "A_markertype INT,"
            "A_nFiducials INT,"
            "A_mean DOUBLE,"
            "A_std DOUBLE,"
            "A_FRE DOUBLE,"
            "A_FLE DOUBLE,"
            "A_volume DOUBLE,"
            "A_TRE DOUBLE,"
            "A_offset DOUBLE,"
            "A_angle DOUBLE,"

            "B_markertype INT,"
            "B_nFiducials INT,"
            "B_mean DOUBLE,"
            "B_std DOUBLE,"
            "B_FRE DOUBLE,"
            "B_FLE DOUBLE,"
            "B_volume DOUBLE,"
            "B_TRE DOUBLE,"
            "B_offset DOUBLE,"
            "B_angle DOUBLE,"

            "FLE DOUBLE,"
            "offset DOUBLE,"
            "angle DOUBLE"

            ")";

    QSqlQuery createQuery;
    if (!createQuery.exec(ins))
    {
      cout << "Error creating combinations table" << std::endl;
      return;
    }
    else
      cout << "Combinations table created succesfully" << std::endl;
  }
  // table already existed
  else
  {
    QSqlQuery readQuery("SELECT COUNT (*) FROM " + tableName);
    readQuery.first();
    startingC = readQuery.value(0).toUInt();
    cout << "Table already exists, and has " << startingC << " rows" << std::endl;
  }


  // fill table
  cout << "Storing data..." << std::endl;
  // get stored number of stimulations
  QSqlQuery writeQuery;
  writeQuery.exec(QString("SELECT * FROM ") + tableName);

  writeQuery.prepare("INSERT INTO " + tableName+ "("
      "combination,"

      "A_markertype,"
      "A_nFiducials,"
      "A_mean,"
      "A_std,"
      "A_FRE,"
      "A_FLE,"
      "A_volume,"
      "A_TRE,"
      "A_offset,"
      "A_angle,"

      "B_markertype,"
      "B_nFiducials,"
      "B_mean,"
      "B_std,"
      "B_FRE,"
      "B_FLE,"
      "B_volume,"
      "B_TRE,"
      "B_offset,"
      "B_angle,"

      "FLE,"
      "offset,"
      "angle"

      ")"
      "VALUES (?,"
              "?,?,?,?,?,?,?,?,?,?,"
              "?,?,?,?,?,?,?,?,?,?,"
              "?,?,?"
              ");");


  if (!db.transaction())
  {
    QMessageBox::critical(nullptr, QObject::tr("Could not start transaction"),QObject::tr("Aborting export"));
    return;
  }

  for (unsigned int c = 0; c<vec.size(); ++c)
  {
    auto metric = vec[c];
    // store new row

    writeQuery.addBindValue(startingC + c);

    writeQuery.addBindValue(metric.A_markerType);
    writeQuery.addBindValue(metric.A_nFiducials);
    writeQuery.addBindValue(metric.A_mean);
    writeQuery.addBindValue(metric.A_std);
    writeQuery.addBindValue(metric.A_FRE);
    writeQuery.addBindValue(metric.A_FLE);
    writeQuery.addBindValue(metric.A_volume);
    writeQuery.addBindValue(metric.A_TRE);
    writeQuery.addBindValue(metric.A_offset);
    writeQuery.addBindValue(metric.A_angle);

    writeQuery.addBindValue(metric.B_markerType);
    writeQuery.addBindValue(metric.B_nFiducials);
    writeQuery.addBindValue(metric.B_mean);
    writeQuery.addBindValue(metric.B_std);
    writeQuery.addBindValue(metric.B_FRE);
    writeQuery.addBindValue(metric.B_FLE);
    writeQuery.addBindValue(metric.B_volume);
    writeQuery.addBindValue(metric.B_TRE);
    writeQuery.addBindValue(metric.B_offset);
    writeQuery.addBindValue(metric.B_angle);

    writeQuery.addBindValue(metric.FLE);
    writeQuery.addBindValue(metric.offset);
    writeQuery.addBindValue(metric.angle);

    if (!writeQuery.exec())
    {
      cout << "Error storing combination data " << c << std::endl;
      cout << "Aborting" << std::endl;
      return;
    }

    if (c%1000 == 0)
      cout << (c+1)*100/vec.size() << "%" << std::endl;

  }

  cout << "Commiting transaction.." << std::endl;
  if(!db.commit())
  {
    QMessageBox::critical(nullptr, QObject::tr("Could not start transaction"),QObject::tr("Aborting export"));
    return;
  }

  cout << "Finished storing all data " << std::endl;

  db.close();

  // store path in settings
  QSettings settings("CAS", "navCAS");
  settings.setValue("Points combinations for export", filename);
}

const QString RegistrationPointsMetrics::GetCombinationsLastPath()
{
  QSettings settings("CAS", "navCAS");
  QString path = settings.value("Points combinations for export", "/home/").toString();

  return path;
}
