/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#define _WIN32_WINNT 0x0A00

#ifdef FORCED_DEVICE_DLL_PATH
#include <Windows.h>
#endif

#include <iostream>
#include <iomanip>

#include <QTimer>

#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>

#include "helpers.hpp"
#include "geometryHelper.hpp"

#include "navAPI.h"

using namespace std;
static const unsigned ENABLE_ONBOARD_PROCESSING_OPTION = 6000;
static const unsigned SENDING_IMAGES_OPTION = 6003;

// Depending on the OS being used, this condition might change
const bool isNotFromConsole = isLaunchedFromExplorer();

navAPI::navAPI()
{
  // Set '.' as decimal separator
  setlocale(LC_ALL,"C");

  mLib = 0;
  mSn = 0uLL;
  mFrame = 0;

  mMarker.push_back(nullptr);
  mMarker.push_back(nullptr);
  mMarker.push_back(nullptr);

  mSamplingPeriod=100;  //200ms = 5Hz
}

navAPI::~navAPI()
{
  cout << "navAPI destructor." << std::endl;
  cout << "mLib: " << mLib << std::endl;
  cout << "mSn: " << mSn << std::endl;
  cout << "mFrame: " << mFrame << std::endl;
}

bool navAPI::SetModeToStandard()
{
  cout << "Disabling sending images" << std::endl;
  if (ftkSetInt32( mLib, mSn, SENDING_IMAGES_OPTION, 0) != ftkError::FTK_OK)
  {
    error("Cannot disable images sending on the SpryTrack.", !isNotFromConsole);
    return false;
  }

  mFrame = ftkCreateFrame();

  if ( mFrame == 0 )
  {
    cerr << "Cannot create frame instance" << std::endl;
    checkError( mLib, !isNotFromConsole  );
  }

  // retrieve left and rigth images
  ftkError err( ftkSetFrameOptions( false, 0, 0, 0,
                                    16u, 4u,
                                    mFrame ) );

  if ( err != ftkError::FTK_OK )
  {
    ftkDeleteFrame( mFrame );
    cerr << "Cannot initialise frame" << std::endl;
    checkError( mLib, !isNotFromConsole );
  }

  cout.setf( ios::fixed, ios::floatfield );
  cout.precision( 2u );

  return true;
}

bool navAPI::DetectCamera()
{
  // Defines where to find Atracsys SDK dlls when FORCED_DEVICE_DLL_PATH is set.
#ifdef FORCED_DEVICE_DLL_PATH
  SetDllDirectory( (LPCTSTR) FORCED_DEVICE_DLL_PATH );
#endif

  // Initialize driver
  mLib = ftkInit();

  if (!mLib)
  {
    error( "Cannot initialize driver" , !isNotFromConsole );
    return false;
  }

  DeviceData device;
  device.SerialNumber = 0uLL;

  ftkError err( ftkError::FTK_OK );
  err = ftkEnumerateDevices( mLib, deviceEnumerator, &device );

  if ( (err > ftkError::FTK_OK) || ( device.SerialNumber == 0uLL ))
  {
    return false;
  }

  // camera was detected and has to be closed
  CloseCamera();
  return true;
}

bool navAPI::InitializeCamera()
{
    // -----------------------------------------------------------------------
    // Defines where to find Atracsys SDK dlls when FORCED_DEVICE_DLL_PATH is
    // set.
#ifdef FORCED_DEVICE_DLL_PATH
		SetDllDirectory( (LPCTSTR) FORCED_DEVICE_DLL_PATH );
#endif

  // ----------------------------------------------------------------------
  // Initialize driver
  mLib = ftkInit();

  if ( ! mLib )
  {
    error( "Cannot initialize driver" , !isNotFromConsole );
    return false;
  }

  // ----------------------------------------------------------------------
  // Retrieve the device

  DeviceData device( retrieveLastDevice( mLib, true, false, !isNotFromConsole ) );
  mSn = device.SerialNumber;

  if (mSn == 0uLL)
    return false;

  // ------------------------------------------------------------------------
  // When using a spryTrack, onboard processing of the images is preferred.
  // Sending of the images is disabled so that the sample operates on a USB2
  // connection
  if (ftkDeviceType::DEV_SPRYTRACK_180 == device.Type)
  {
    cout << "Enable onboard processing" << std::endl;
    if ( ftkSetInt32( mLib, mSn, ENABLE_ONBOARD_PROCESSING_OPTION, 1 ) != ftkError::FTK_OK )
    {
      error( "Cannot process data directly on the SpryTrack.", !isNotFromConsole );
    }

    cout << "Disable images sending" << std::endl;
    if (ftkSetInt32( mLib, mSn, SENDING_IMAGES_OPTION, 0) != ftkError::FTK_OK)
		{
			error("Cannot disable images sending on the SpryTrack.", !isNotFromConsole);
		}

  }
  return true;
}

bool navAPI::CloseCamera()
{
  cout << "Closing camera" << std::endl;
  if (!mLib)
    return false;

  if ( ftkError::FTK_OK != ftkClose( &mLib ) )
  {
    checkError( mLib, !isNotFromConsole  );
    return false;
  }

  mLib = 0;
  mSn = 0uLL;
  return true;
}


std::string navAPI::GetGeometryFileName(MarkerId type)
{
#ifdef WIN32
  string filename("../");
#else
  string filename("");
#endif // WIN32

  switch(type)
  {
  case navAPI::MarkerId::SmallTracker:
    filename += "../data/geometry001.ini";
    break;

  case navAPI::MarkerId::MediumTracker:
    filename += "../data/geometry002.ini";
    break;

  case navAPI::MarkerId::BigTracker:
    filename += "../data/geometry004.ini";
    break;

  case navAPI::MarkerId::Probe:
    filename += "../data/geometry_123_probe.ini";
    break;

  case navAPI::MarkerId::TemporalProbe:
    filename += "../data/geometry_124_temporal_probe.ini";
    break;

  default:
    cerr << "Error: Wrong marker passed to GetGeometryFileName function" << std::endl;
    break;
  }

  return filename;
}

bool navAPI::AddGeometry(const MarkerId type)
{
  string geomFile = GetGeometryFileName(type);

  ftkGeometry geom;

  switch ( loadGeometry( mLib, mSn, geomFile, geom ) )
  {
  case 1:
    cout << "Loaded from installation directory." << std::endl;

  case 0:
    if (ftkError::FTK_OK != ftkSetGeometry( mLib, mSn, &geom ) )
    {
      checkError( mLib, !isNotFromConsole  );
    }
    break;

  default:

    cerr << "Error, cannot load geometry file '"
         << geomFile << "'." << std::endl;
    if (ftkError::FTK_OK != ftkClose( &mLib ) )
    {
      checkError( mLib, !isNotFromConsole  );
    }
    return false;
  }
  return true;
}


void navAPI::run()
{
  cout << "Starting threaded navigation" << std::endl;

  mFrame = ftkCreateFrame();

  if ( mFrame == 0 )
  {
    cerr << "Cannot create frame instance" << std::endl;
    checkError( mLib, !isNotFromConsole  );
  }

  ftkError err( ftkSetFrameOptions( false, 0, 0, 0,
                                      13u, 4u, mFrame ) );

  if ( err != ftkError::FTK_OK )
  {
    ftkDeleteFrame( mFrame );
    cerr << "Cannot initialise frame" << std::endl;
    checkError( mLib, !isNotFromConsole  );
  }

  cout.setf( std::ios::fixed, std::ios::floatfield );

  // Start real time system
  mTimer = new QTimer(this);
  mTimer->setInterval(mSamplingPeriod); // default sampling rate is 5 Hz
  mTimer->moveToThread(this);
  connect(mTimer,SIGNAL(timeout()),this,SLOT(GetLastFrame()));
  mTimer->start();
  cout << "Timer started, period of " << mTimer->interval() << " ms" << std::endl;

  // Execute the thread
  exec();
}

void navAPI::StopNavigation()
{
  quit();
  cout << "Thread quitted!" << std::endl;


  if (!mFrame)
    return;

  mTimer->stop();
  ftkDeleteFrame( mFrame );
  mFrame = 0;
  delete mTimer;

  this->exit();

  cout << "Navigation stopped " << std::endl;
}

void navAPI::GetRelativePositions(unsigned int m)
{
  floatXX translation[ 3u ];
  floatXX rotation[ 3u ][ 3u ];

  for ( unsigned i = 0; i < 3u; i++ )
  {
    floatXX tmp = 0.;
    for (unsigned int k = 0u; k < 3u; ++k )
    {
      // Since the rotation is a matrix from SO(3) the inverse
      // is the transposed matrix.
      // Therefore R^-1 * t = R^T * t
      tmp += mMarker[0]->rotation[ k ][ i ] *
             ( mMarker[m]->translationMM[ k ] -
               mMarker[0]->translationMM[ k ] );
    }
    translation[i] = tmp;
  }

  for ( unsigned int i = 0u; i < 3u; ++i )
  {
    for ( unsigned int j = 0u; j < 3u; ++j )
    {
      floatXX tmp = 0.;
      for ( unsigned int k = 0u; k < 3u; ++k )
      {
        // Since the rotation is a matrix from SO(3) the inverse
        // is the transposed matrix.
        // Therefore R^-1 * R' = R^T * R'
        tmp += mMarker[0]->rotation[ k ][ i ] *
               mMarker[m]->rotation[ k ][ j ];
      }
      rotation[ i ][ j ] = tmp;
    }
  }

  // Compose transformation matrix
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  for (unsigned int i=0; i<3; i++)
  {
    for (unsigned int j=0; j<3; j++)
    {
      matrix->SetElement(i,j,rotation[i][j]);
    }
    matrix->SetElement(i,3,translation[i]);
  }
  matrix->SetElement(3,0,0.0);
  matrix->SetElement(3,1,0.0);
  matrix->SetElement(3,2,0.0);
  matrix->SetElement(3,3,1.0);

  emit MarkerRelativePosition(m,matrix);
}

void navAPI::GetLastFrame()
{
  if ( ftkGetLastFrame( mLib, mSn, mFrame, 0  ) != ftkError::FTK_OK )
      // block until next frame is available //
  {
    cout << std::endl << "." << std::endl;
    return;
  }

  if (ftkReprocessFrame(mLib, mSn, mFrame) != ftkError::FTK_OK)
    cout << std::endl << "Could not reprocess" << std::endl;

  switch ( mFrame->markersStat )
  {
  case ftkQueryStatus::QS_WAR_SKIPPED:
    ftkDeleteFrame( mFrame );
    cerr << "marker fields in the frame are not set correctly" << std::endl;
    checkError( mLib, !isNotFromConsole  );
    return;

  case ftkQueryStatus::QS_ERR_INVALID_RESERVED_SIZE:
    ftkDeleteFrame( mFrame );
    cerr << "frame -> markersVersionSize is invalid" << std::endl;
    checkError( mLib, !isNotFromConsole  );
    return;

  case ftkQueryStatus::QS_ERR_OVERFLOW:
      //ftkDeleteFrame( mFrame );
      cerr << "frame -> data is missing because buffer size is too small" << std::endl;
      //checkError( mLib, !isNotFromConsole  );
    break;

  default:
    ftkDeleteFrame( mFrame );
    cerr << "invalid status" << std::endl;
    checkError( mLib, !isNotFromConsole  );
    return;

  case ftkQueryStatus::QS_OK:
    break;
  }


  // No markers detected
  if ( mFrame->markersCount == 0u )
  {
    emit MultipleOrNullMarkersInView();
    std::vector<MarkerId> empty;
    emit MultipleMarkersInView(empty);
    return;
  }

  if ( mFrame->markersStat == ftkQueryStatus::QS_ERR_OVERFLOW )
  {
    cerr << "WARNING: marker overflow. Please increase cstMarkersCount" << std::endl;
  }


  // Reference marker, second marker and probe have to be re-detected in every frame
  mMarker[0] = 0;
  mMarker[1] = 0;
  mMarker[2] = 0;

  // Set reference marker, second marker and probe
  for ( size_t m = 0; m < mFrame->markersCount; m++ )
  {
    MarkerId type = static_cast<MarkerId>(mFrame->markers[ m ].geometryId);

    if ((type == mReferenceMarkerType) && (type != None) )
    {
      mMarker[0] = &(mFrame->markers[m]);
    }
    else if ((type == SmallTracker) || (type == MediumTracker) || (type == BigTracker))
    {
      mMarker[1] = &(mFrame->markers[m]);
    }
    else if ((type == Probe) || (type == TemporalProbe))
    {
      mMarker[2] = &(mFrame->markers[m]);
    }
  }

  // If a single marker (not probe) is in the view
  if ((mFrame->markersCount == 1) &&
      (mFrame->markers[0].geometryId != Probe) &&
      (mFrame->markers[0].geometryId != TemporalProbe))
  {
    MarkerId type = static_cast<MarkerId>(mFrame->markers[0].geometryId);
    emit SingleMarkerInView(type);
  }
  else
    emit MultipleOrNullMarkersInView();

  // Send relative positions
  for ( unsigned int m = 1; m < mMarker.size(); m++ )
  {
    if ((mMarker[m] != 0) && (mMarker[0] != 0))
      GetRelativePositions(m);
  }

  // Valid probe and reference marker
  if ((mMarker[2] != 0) && (mMarker[2]->geometryId == Probe) && (mMarker[0] != 0))
    emit ValidProbeInView();
  else
    emit InvalidProbeInView();

  // Send list of markers in view
  std::vector<MarkerId> markerList;
  for ( unsigned int m = 0; m < mFrame->markersCount; m++ )
  {
    if (mFrame->markers[m].geometryId == Probe)
      continue;

    // could use lookup table
    MarkerId type = static_cast<MarkerId>(mFrame->markers[m].geometryId);
    markerList.push_back(type);
  }
  emit MultipleMarkersInView(markerList);

  // Valid temporal probe and reference marker (calibration)
  if ((mMarker[2] != 0) && (mMarker[2]->geometryId == TemporalProbe) && (mMarker[0] != 0))
  {
    // check the existance of raw data
    if ( mFrame->threeDFiducialsStat != ftkQueryStatus::QS_OK )
    {
      //cout << "No raw data available" << std::endl;
      return;
    }

    emit ValidTemporalProbeInView();

    // get the probes fiducials
    uint32* fiducialCorrespondence = mMarker[2]->fiducialCorresp;

    for (unsigned int i=0; i<FTK_MAX_FIDUCIALS; i++)
    {
      uint32 m = fiducialCorrespondence[i];

      if (m == INVALID_ID)
        continue;
    }
  }
}
