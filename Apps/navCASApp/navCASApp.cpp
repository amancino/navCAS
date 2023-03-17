/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <mitkBaseApplication.h>

#include <QVariant>

int main(int argc, char** argv)
{
  mitk::BaseApplication myApp(argc, argv);
  myApp.setApplicationName("CAS Navigation App");
  myApp.setOrganizationName("CAS");

  // -------------------------------------------------------------------
  // Here you can switch to your customizable application:
  // -------------------------------------------------------------------

  //myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "org.mitk.qt.extapplication");
  myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "navcas.navApp");

  return myApp.run();
}
