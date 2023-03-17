/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>
#include <iomanip>

// vtk
#include <vtkSphereSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkAppendPolyData.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPlane.h>

// qt
#include <QMessageBox>
#include <QSettings>

// mitk
#include <mitkSurface.h>
#include <mitkImage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

#include "NodesManager.h"
#include "LabeledPointSetMapper3D.h"
#include "LabeledPointSetMapper2D.h"
#include "ArrowSource.h"
#include "ViewCommands.h"

using namespace std;

const string NodesManager::SURFACE_NAME = string("navCAS_planning_surface");
const unsigned int NodesManager::NUMBER_FIDUCIALS[3] = {4,4,6}; // Probe actually has 5 fiducials

NodesManager::NodesManager(mitk::DataStorage::Pointer ds) :
  mDataStorage(ds)
{
  mPrimaryPlannedPoints = mitk::PointSet::New();
  mPrimaryRealPoints = mitk::PointSet::New();
  mSecondaryPoints = mitk::PointSet::New();

  mUseMoving = false;

  mLastMovingMarkerRelativeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  // probe 2D
  for (int i = 0; i < 3; i++)
  {
    mRenderWindow.push_back(nullptr);
    mProbe2DManager.push_back(nullptr);
  }
}

NodesManager::~NodesManager()
{
	cout << "Nodes Manager destructor" << std::endl;

  // TO DO: annotation is not correctly deleted, we simply hide it
  HideProbe2D();

  // Hide everything
  ShowAllProbes(false);
  ShowAllFiducials(false);

  // We never remove from datastorage the 3D probe
}

void NodesManager::HideProbe2D()
{
  for (unsigned int i=0; i<mProbe2DManager.size(); i++)
  {
    if (mProbe2DManager[i].IsNotNull())
    {
      mProbe2DManager[i]->ShowControls(false);
      mProbe2DManager[i]->ShowCircle(false);
    }
  }
}


void NodesManager::ShowAllProbes(bool vis)
{
  if (!mProbeNode)
    return;

  mProbeNode->SetVisibility(vis);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NodesManager::ShowAllFiducials(bool vis)
{
  for (unsigned int i=0; i<mFiducials.size(); i++)
  {
    for (unsigned int n=0; n<mFiducials[i].size(); n++)
      mFiducials[i][n]->SetVisibility(vis);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void NodesManager::UpdateNavigationConfiguration()
{
  // configure navigation setup
  SetReferenceMarker(GetReferenceMarker());
  SetUseMovingMarker(GetUseMovingMarker());
  SetNavigationMode(GetNavigationMode());
}

// ** MARKERS ** //

void NodesManager::LoadMarkersGeometries()
{
  mMarkerType.clear();
  mMarkerType.push_back(navAPI::MarkerId::None);
  mMarkerType.push_back(navAPI::MarkerId::None);
  mMarkerType.push_back(navAPI::MarkerId::Probe);

  // Build fiducials by reading the geometry files
  // to do: pivot should be stored as part of the marker geometry (and should be transformed similarly to probe tip)
  mitk::Point3D smallPivot, mediumPivot, bigPivot;
  GenerateMarkerPointsFromGeometry(navAPI::MarkerId::SmallTracker,&smallPivot);
  GenerateMarkerPointsFromGeometry(navAPI::MarkerId::MediumTracker,&mediumPivot);
  GenerateMarkerPointsFromGeometry(navAPI::MarkerId::BigTracker,&bigPivot);
  GenerateMarkerPointsFromGeometry(navAPI::MarkerId::Probe);
  //GenerateMarkerPointsFromGeometry(navAPI::MarkerId::TemporalProbe);

  cout << "**************************** original probe tip: " << mProbePos[5] << std::endl;
}

void NodesManager::GenerateMarkerPointsFromGeometry(navAPI::MarkerId type, mitk::Point3D* pivot)
{
  vector<mitk::Point3D> *marker;

  switch(type)
  {
  case navAPI::MarkerId::SmallTracker:
    marker = &mSmallTrackerPos;
    break;

  case navAPI::MarkerId::MediumTracker:
    marker = &mMediumTrackerPos;
    break;

  case navAPI::MarkerId::BigTracker:
    marker = &mBigTrackerPos;
    break;

  case navAPI::MarkerId::Probe:
    marker = &mProbePos;
    break;

  case navAPI::MarkerId::TemporalProbe:
    marker = &mProbePos;
    break;

  default:
    cerr << "Wrong marker passed to function" << std::endl;
    return;
  }

  string file = navAPI::GetGeometryFileName(type);
  marker->clear();

  std::ifstream input;
  input.open( file.c_str() );

  // first line
  string line;
  getline(input,line);

  // count and id
  uint data[2];
  for (unsigned int i=0; i<2; i++)
  {
    string cell;
    getline(input,line);
    stringstream  lineStream(line);
    getline(lineStream, cell, '=');
    cout << cell << " = ";
    getline(lineStream, cell, '\n');

    data[i] = static_cast<uint>(atof(cell.c_str()));
    cout << data[i] << std::endl;
  }

  // read "count times" the fiducials coordinates
  for (uint fid=0; fid<data[0]; fid++)
  {
    getline(input, line);

    // read fiducial number
    stringstream  lineStream(line);
    string cell;
    getline(lineStream, cell, 'l');
    getline(lineStream, cell, ']'); // [fiducial0] -> from ..l to ] is the fiducial number
    int fiducial = static_cast<int>(atof(cell.c_str()));

    // read coordinates of fiducial
    mitk::Point3D point;
    for (unsigned int c=0; c<3; c++)
    {
      getline(input, line);
      stringstream  lineStream(line);
      string cell;
      getline(lineStream, cell, '=');
      getline(lineStream, cell, '\n');

      point[c] = atof(cell.c_str());
    }
    marker->push_back(point);
    cout << "Fiducial[" << fiducial << "] pushed: " << point << std::endl;
  }

  if (marker->size() != data[0])
  {
    cerr << "Error readind geometry file: marker size = " << marker->size() << std::endl;
  }

  // read pivot
  if ( (pivot != nullptr) && ((type == navAPI::MarkerId::MediumTracker) ||  (type == navAPI::MarkerId::BigTracker)) )
  {
    // read "[pivot]"
    getline(input, line);

    // read coordinates of pivot
    for (unsigned int c=0; c<3; c++)
    {
      getline(input, line);
      stringstream  lineStream(line);
      string cell;
      getline(lineStream, cell, '=');
      getline(lineStream, cell, '\n');

      pivot->SetElement(c,atof(cell.c_str()));
    }
  }
  // add probe tip
  else if ((type == navAPI::MarkerId::TemporalProbe) || (type == navAPI::MarkerId::Probe))
  {
    if (!getline(input, line))
    {
      cerr << "Error reading geometry file: probe geometry has no tip information" << std::endl;
      input.close();
      return;
    }
    mitk::Point3D tip;
    for (unsigned int c=0; c<3; c++)
    {
      getline(input, line);
      stringstream  lineStream(line);
      string cell;
      getline(lineStream, cell, '=');
      getline(lineStream, cell, '\n');

      tip[c] = atof(cell.c_str());
    }
    marker->push_back(tip);
  }

  input.close();
}

mitk::Point3D NodesManager::UpdateProbeTip(mitk::Vector3D correction)
{
  mProbePos[5] += correction;

  cout << "**************************** new probe tip: " << mProbePos[5] << std::endl;

  auto pred = mitk::NodePredicateProperty::New("navCAS.isProbeSurface",mitk::BoolProperty::New(true));
  auto so = mDataStorage->GetSubset(pred);

  if (so->Size() != 1)
  {
    cerr << "No probe 2D detected when trying to update tip position" << std::endl;
    return mProbePos[5];
  }

  // 3D arrow probe
  mitk::Vector3D normal = mProbePos[5] - mProbePos[3]; // to do: this will not necessary be the case!!
  cout << "Distance to fiducial is: " << normal.GetNorm() << std::endl;
  mProbeSource = ArrowSource::New(normal.GetDataPointer(),mProbePos[3].GetDataPointer(),normal.GetNorm());
  auto surf = dynamic_cast<mitk::Surface*>(so->Begin()->Value()->GetData());
  surf->SetVtkPolyData(mProbeSource->GetOutput());
  mProbeNode->SetData(surf);

  return mProbePos[5];
}

void NodesManager::CreateMarkers()
{
  // Markers should not be saved in the project, they have to be re-created every time

  // Check moving marker
  auto pred = mitk::NodePredicateProperty::New("navCAS.isMovingMarkerSurface",mitk::BoolProperty::New(true));
  auto so = mDataStorage->GetSubset(pred);
  if (so->Size() == 1)
    mMovingMarkerNode = so->Begin()->Value();
  else
  {
    // delete possibly existing moving markers
    for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
      mDataStorage->Remove(it->Value());

    mMovingMarkerNode = mitk::DataNode::New();
    mMovingMarkerNode->SetName("Moving marker");
    mMovingMarkerNode->SetBoolProperty("navCAS.isMovingMarkerSurface",true);
    auto surf = mitk::Surface::New();
    mMovingMarkerNode->SetData(surf);
    mMovingMarkerNode->SetBoolProperty("helper object",!GetShowHelperObjects());
    mMovingMarkerNode->SetColor(0,0,1);
    mMovingMarkerNode->SetVisibility(false);
    mDataStorage->Add(mMovingMarkerNode);
  }

  // Check probe node
  mitk::Vector3D normal = mProbePos[5] - mProbePos[3];
  mProbeSource = ArrowSource::New(normal.GetDataPointer(),mProbePos[3].GetDataPointer(),normal.GetNorm());

  pred = mitk::NodePredicateProperty::New("navCAS.isProbeSurface",mitk::BoolProperty::New(true));
  so = mDataStorage->GetSubset(pred);
  if (so->Size() == 1)
  {
    mProbeNode = so->Begin()->Value();
    auto arrowSurf = dynamic_cast<mitk::Surface*>(mProbeNode->GetData());
    arrowSurf->SetVtkPolyData(mProbeSource->GetOutput());
    mProbeNode->SetData(arrowSurf);
  }
  else
  {
    // Delete probe surface, if existing
    for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
    {
      cout << "Removed probe node: " << it->Value()->GetName() << std::endl;
      mDataStorage->Remove(it->Value());
    }

    // 3D arrow probe
    mProbeNode = mitk::DataNode::New();
    mitk::Surface::Pointer arrowSurf = mitk::Surface::New();
    arrowSurf->SetVtkPolyData(mProbeSource->GetOutput());
    mProbeNode->SetData(arrowSurf);
    mProbeNode->SetBoolProperty("navCAS.isProbeSurface",true);
    mProbeNode->SetColor(0,1,1);
    mProbeNode->SetName("ProbeNode (cyan)");
    mProbeNode->SetBoolProperty("helper object",!GetShowHelperObjects());
    mDataStorage->Add(mProbeNode);
  }


  // Check if all markers already exist and exit
  pred = mitk::NodePredicateProperty::New("navCAS.isFiducial",mitk::BoolProperty::New(true));
  so = mDataStorage->GetSubset(pred);
  if ((mFiducials.size() == 3) &&
      (so->Size() == (NUMBER_FIDUCIALS[0]+NUMBER_FIDUCIALS[1]+NUMBER_FIDUCIALS[2]))
      )
  {
    cout << "All fiducials are present" << std::endl;
    return;
  }

  bool reset = false;
  if (so->Size() == (NUMBER_FIDUCIALS[0]+NUMBER_FIDUCIALS[1]+NUMBER_FIDUCIALS[2]))
  {
    mFiducials.clear();
    for (unsigned int m=0; m<3; m++)
    {
      vector<mitk::DataNode::Pointer> marker;
      for (unsigned int i=0; i<NUMBER_FIDUCIALS[m];i++)
        marker.push_back(nullptr);

      auto pred = mitk::NodePredicateProperty::New("navCAS.fiducial.marker",mitk::IntProperty::New(m));
      auto so = mDataStorage->GetSubset(pred);

      for (auto it = so->Begin(); it != so->End(); ++it)
      {
        int fid = -1;
        if (!it->Value()->GetIntProperty("navCAS.fiducial.fid",fid))
        {
          reset=true;
          break;
        }
        marker[fid] = it->Value();
      }
      mFiducials.push_back(marker);

      if (reset)
        break;
    }
  }
  else
    reset = true;

  if (!reset)
    return;

  // Delete fiducials, if existing
  for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
    mDataStorage->Remove(it->Value());

  // Create new fiducials and store them in vector
  mFiducials.clear();

  vtkSmartPointer<vtkSphereSource> fid = vtkSmartPointer<vtkSphereSource>::New();
  fid->SetThetaResolution(8);
  fid->SetPhiResolution(8);
  fid->SetRadius(1);
  fid->Update();

  // 2 markers and 1 probe
  for (unsigned int m = 0; m<3; m++)
  {
    string type("Reference tracker");
    if (m == 1)
      type = string("Moving tracker");
    else if (m == 2)
      type = string("Probe");

    float colors[6][3] = {{1,0,0},{0,1,0},{0,0,1},{1,1,1},{1,1,1},{1,1,1}};

    vector<mitk::DataNode::Pointer> marker;
    for (unsigned int i=0; i<NUMBER_FIDUCIALS[m]; i++)
    {
      marker.push_back(mitk::DataNode::New());
      mitk::Surface::Pointer surf = mitk::Surface::New();
      surf->SetVtkPolyData(fid->GetOutput());
      marker[i]->SetData(surf);
      std::stringstream name;
      name << type << " Fiducial " << i;
      marker[i]->SetName(name.str().c_str());
      marker[i]->SetColor(colors[i]);
      marker[i]->SetBoolProperty("navCAS.isFiducial",true);
      marker[i]->SetIntProperty("navCAS.fiducial.marker",m);
      marker[i]->SetIntProperty("navCAS.fiducial.fid",i);
      marker[i]->SetBoolProperty("helper object",!GetShowHelperObjects());

      mDataStorage->Add(marker[i]);
    }
    mFiducials.push_back(marker);
  }
  // Set probe position property?
  mFiducials[2].at(5)->SetBoolProperty("navCAS.isProbePosition",true);
}


// **  SYSTEM SETUP  ** //

void NodesManager::InitializeSetup()
{
  // Delete any posible existing systemsetup node
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.systemSetup.isSetupNode",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);

  if (so->Size() == 1)
  {
    cout << "Loading last configured set up" << std::endl;
    return;
  }

  for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
    mDataStorage->Remove(it->Value());

  // create set up node with current sesion setup
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  auto data = mitk::PointSet::New();
  data->InsertPoint(mitk::Point3D(0.0));
  node->SetData(data);
  node->SetName("System Setup node");
  node->SetBoolProperty("navCAS.systemSetup.isSetupNode",true);
  node->SetBoolProperty("helper object", !GetShowHelperObjects());
  node->SetVisibility(false);
  mDataStorage->Add(node);
}

mitk::DataNode::Pointer NodesManager::GetSystemNode()
{
  if (mDataStorage.IsNull())
  {
    cerr << "Error: DataStorage is null! Can't retrieve the system node" << std::endl;
    return nullptr;
  }
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.systemSetup.isSetupNode",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);

  if (so->size() != 1)
    return nullptr;

  return so->Begin()->Value();
}

navAPI::MarkerId NodesManager::GetReferenceMarker()
{
  int markerType = navAPI::MarkerId::None;

  mitk::DataNode::Pointer node = GetSystemNode();
  if (node.IsNull())
  {
    cerr << "No system node detected!" << std::endl;
    return navAPI::MarkerId::None;
  }

  node->GetIntProperty("navCAS.systemSetup.referenceMarkerType",markerType);

  return static_cast<navAPI::MarkerId>(markerType);
}

navAPI::MarkerId NodesManager::GetMovingMarker()
{
  int markerType = navAPI::MarkerId::None;

  mitk::DataNode::Pointer node = GetSystemNode();
  if (node.IsNull())
  {
    cerr << "No system node detected!" << std::endl;
    return navAPI::MarkerId::None;
  }

  node->GetIntProperty("navCAS.systemSetup.movingMarkerType",markerType);

  return static_cast<navAPI::MarkerId>(markerType);
}



// ** NAVIGATION ** //

mitk::Point3D NodesManager::GetProbeLastPosition()
{
  return mFiducials[2].at(5)->GetData()->GetGeometry()->GetOrigin();
}

mitk::Vector3D NodesManager::GetProbeDirection()
{
  mitk::Vector3D dir = mFiducials[2].at(5)->GetData()->GetGeometry()->GetOrigin() - mFiducials[2].at(3)->GetData()->GetGeometry()->GetOrigin();
  dir.Normalize();
  return dir;
}

void NodesManager::UpdateMarker(unsigned int m, std::vector<mitk::Point3D> pos)
{
  unsigned int numberFiducials = 4;
  if (m == 2)
    numberFiducials = 6;  // actually is 5 fiducials and the tip position

  for (unsigned int i=0; i<numberFiducials; i++)
  {
    mFiducials[m][i]->GetData()->SetOrigin(pos[i]);
  }

  // update probe polydata
  if (m == 2)
  {
    mitk::Vector3D normal = pos[5] - pos[3];
    mProbeSource->SetCenter(pos[3].GetDataPointer());
    mProbeSource->SetNormal(normal.GetDataPointer());
    mProbeSource->Update();
    dynamic_cast<mitk::Surface*>(mProbeNode->GetData())->SetVtkPolyData(mProbeSource->GetOutput());

    // TO DO: only change this in update of show probe
    mProbeNode->SetVisibility(mShowProbe);

    double axis[3][3] = {{0.0}};
    axis[0][2]=-1;//axial plane
    axis[1][1]=-1;//coronal plane
    axis[2][0]=1;//sagittal plane

    // update probe in 2D
    for (unsigned int i=0; i<3; i++)
    {
      if (mProbe2DManager[i].IsNull())
      {
        cerr << "Error: Probe2dManager is null!" << std::endl;
        return;
      }
      mProbe2DManager[i]->ShowCircle(mShowProbe);

      mitk::Point2D tip;
      mRenderWindow[i]->GetRenderer()->WorldToDisplay(pos[5],tip);

      //project probe in i-axis
      double proj[3];
      vtkPlane::ProjectVector((-normal).GetDataPointer(),pos[5].GetDataPointer(),axis[i],proj);

      mitk::Vector2D dir;
      switch (i)
      {
      case 0://axial
        dir[0] = proj[0];
        dir[1] = -proj[1];
        break;
      case 1://coronal
        dir[0] = proj[0];
        dir[1] = proj[2];
        break;
      case 2://sagittal
        dir[0] = proj[1];
        dir[1] = proj[2];
        break;
      }
      mProbe2DManager[i]->UpdatePosition(tip,dir);
    }
  }
}

void NodesManager::UpdateInstrument(vtkTransform* transform)
{
  // load selected surface from datastorage
  // Polydata is already transformed according to instrument registration
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);

  if (so->size() != 1)
  {
    cout << "ERROR: Number of instruments in datastorage = " << so->size() << std::endl;
    return;
  }

  // Transform instrument surface if required
  auto instrumentNode = so->Begin()->Value();
  if  (mNavigationMode == InstrumentTracking)
  {
    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    filter->SetInputData(dynamic_cast<mitk::Surface*>(instrumentNode->GetData())->GetVtkPolyData());
    filter->SetTransform(transform);
    filter->Update();
    dynamic_cast<mitk::Surface*>(mMovingMarkerNode->GetData())->SetVtkPolyData(filter->GetOutput());
    mMovingMarkerNode->SetVisibility(true);
  }

  // get movement and angle of coil transformation
  //cout << "Coil transformation: " << std::endl;
  double wxyz[4];
  transform->GetOrientationWXYZ(wxyz);
  //cout << "Angle: " << wxyz[0] << std::endl;
  double offset[3];
  transform->GetPosition(offset);
  //cout << "Offset: " << mitk::Point3D(offset) << std::endl;

  // get moving marker registration matrix
  pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
  so = mDataStorage->GetSubset(pred);

  if (so->Size() == 1)
  {
    //vtkSmartPointer<vtkMatrix4x4> movingRegistration = GetTransformFromPatientNode(mMovingMarkerNode);
    vtkSmartPointer<vtkMatrix4x4> movingRegistration = GetTransformFromNode(so->Begin()->Value());

    if (movingRegistration != nullptr)
    {
      // compute and inform offset and ange error
      if (mNavigationMode == InstrumentTracking)
      {
        vtkSmartPointer<vtkTransform> movingTransform = vtkSmartPointer<vtkTransform>::New();
        movingTransform->SetMatrix(movingRegistration);
        movingTransform->Update();
        //cout << "Moving surface transformation: " << std::endl;
        movingTransform->GetOrientationWXYZ(wxyz);
        //cout << "Angle: " << wxyz[0] << std::endl;
        movingTransform->GetPosition(offset);
        //cout << "Offset: " << mitk::Point3D(offset) << std::endl;

        // compute total error
        vtkSmartPointer<vtkMatrix4x4> invRelativeMarker = vtkSmartPointer<vtkMatrix4x4>::New();
        vtkMatrix4x4::Invert(transform->GetMatrix(),invRelativeMarker);

        vtkSmartPointer<vtkMatrix4x4> errorMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        vtkMatrix4x4::Multiply4x4(invRelativeMarker,movingRegistration,errorMatrix);


        vtkSmartPointer<vtkTransform> errorTransform = vtkSmartPointer<vtkTransform>::New();
        errorTransform->SetMatrix(errorMatrix);
        errorTransform->Update();
        //cout << "Total error transformation: " << std::endl;
        errorTransform->GetOrientationWXYZ(wxyz);
        double angleError = abs(wxyz[0]);
        //cout << "Angle error: " << angleError << " deg" << std::endl;
        errorTransform->GetPosition(offset);
        mitk::Vector3D offsetVec(offset);
        double offsetError = offsetVec.GetNorm();
        //cout << "Offset error: " << offsetError << " mm" << std::endl;

        // change node color
        if ((offsetError < 2.0) && (angleError < 2.0))
          mMovingMarkerNode->SetColor(0,1,0);
        else if ((offsetError < 9.0) && (angleError < 8.0))
          mMovingMarkerNode->SetColor(1,1,0);
        else
          mMovingMarkerNode->SetColor(1,0,0);

        // show text labels
        emit AngleError(angleError);
        emit OffsetError(offsetError);
      }
    }
  }
}




void NodesManager::SetReferenceMarker(navAPI::MarkerId type)
{
  if (type == navAPI::MarkerId::None)
  {
    cerr << "Error: No marker was set as reference" << std::endl;
    return;
  }

  mMarkerType[0] = type;
  if (mMarkerType[1] == type)
  {
    mMarkerType[1] = navAPI::MarkerId::None;
    GetSystemNode()->SetIntProperty("navCAS.systemSetup.movingMarkerType",mMarkerType[1]);
  }

  switch(type)
    {
    case navAPI::MarkerId::SmallTracker:
      UpdateMarker(0,mSmallTrackerPos);
      break;

    case navAPI::MarkerId::MediumTracker:
      UpdateMarker(0,mMediumTrackerPos);
      break;

    case navAPI::MarkerId::BigTracker:
      UpdateMarker(0,mBigTrackerPos);
      break;

    default:
      cerr << "Error: No marker was set as reference" << std::endl;
      return;
    }

  // system setup update
  GetSystemNode()->SetIntProperty("navCAS.systemSetup.referenceMarkerType",mMarkerType[0]);
  //GetSystemNode()->SetIntProperty("navCAS.systemSetup.movingMarkerType",mMarkerType[1]);
}


void NodesManager::SetMovingMarker(navAPI::MarkerId type)
{
  mMarkerType[1] = type;

  // system setup update
  GetSystemNode()->SetIntProperty("navCAS.systemSetup.movingMarkerType",mMarkerType[1]);

  // check that moving and reference are not equal
  if (type == mMarkerType[0])
    cerr << "Error: moving and reference markers are the same geometry!" << std::endl;

  MITK_INFO << "New moving marker type: " << type;
}


void NodesManager::SetUseMovingMarker(bool use)
{
  mUseMoving = use;

  // Store in system setup
  mitk::DataNode::Pointer node = GetSystemNode();

  if (node.IsNull())
  {
    cerr << "ERROR: No system setup node detected!" << std::endl;
    return;
  }

  node->SetBoolProperty("navcas.systemSetup.useMoving",use);

  if (!use)
    return;
}

void NodesManager::ShowMovingMarker(bool vis)
{
  mMovingMarkerNode->SetVisibility(vis);
}

void NodesManager::SetNavigationMode(int mode)
{
  mNavigationMode = static_cast<NavigationMode>(mode);

  // Store in system setup
  mitk::DataNode::Pointer node = GetSystemNode();

  if (node.IsNull())
  {
    cerr << "ERROR: No system setup node detected!" << std::endl;
    return;
  }

  node->SetIntProperty("navcas.systemSetup.navigationMode",mNavigationMode);
}

void NodesManager::SetNavigationModeToTraditional()
{
  SetNavigationMode(Traditional);
}

void NodesManager::SetNavigationModeToInstrumentTracking()
{
  SetNavigationMode(InstrumentTracking);
}

int NodesManager::GetNavigationMode()
{
  // check from system setup
  mitk::DataNode::Pointer node = GetSystemNode();

  if (node.IsNull())
  {
    cerr << "ERROR: No system setup node detected!" << std::endl;
    return Invalid;
  }
  int mode = Invalid;
  node->GetIntProperty("navcas.systemSetup.navigationMode",mode);

  return mode;
}


bool NodesManager::GetUseMovingMarker()
{
  // check from system setup
  mitk::DataNode::Pointer node = GetSystemNode();

  if (node.IsNull())
  {
    cerr << "ERROR: No system setup node detected!" << std::endl;
    return false;
  }
  bool use = false;
  node->GetBoolProperty("navcas.systemSetup.useMoving",use);

  return use;
}


void NodesManager::UpdateRelativeMarker(unsigned int m, vtkMatrix4x4* matrix)
{
  // to do: use reference instead of copy
  std::vector<mitk::Point3D> pos;

  switch (mMarkerType[m])
  {
  case navAPI::SmallTracker:
    pos = mSmallTrackerPos;
    break;
  case navAPI::MediumTracker:
    pos = mMediumTrackerPos;
    break;
  case navAPI::BigTracker:
    pos = mBigTrackerPos;
    break;
  case navAPI::Probe:
    pos = mProbePos;
    break;
  default:
    return;
  }

  // store last matrix of relative position between reference and moving marker
  if (m == 1)
    mLastMovingMarkerRelativeMatrix->DeepCopy(matrix);

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(matrix);

  for (unsigned int p=0; p<pos.size(); p++)
    pos[p] = mitk::Point3D(transform->TransformDoublePoint(pos[p][0],pos[p][1],pos[p][2]));

  UpdateMarker(m,pos);

  if (mUseMoving && (mMarkerType[m] != navAPI::Probe))
    UpdateInstrument(transform);
}

// ** PLANNED SERIES ** //

void NodesManager::ShowSingleSerie(mitk::DataNode::Pointer node, bool reinit)
{
  // Set visible only the selected image
  mitk::Image* Im = dynamic_cast<mitk::Image*>(node->GetData());
  if (Im != nullptr)
  {
    node->SetVisibility(true);

    mitk::DataNode::Pointer skin = GetSkinNode(node);
    if (skin.IsNotNull())
      skin->SetVisibility(true);


    // Hide all images (and skins) in datastorage
    mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = so->Begin(); it != so->End(); ++it)
    {
      mitk::Image* Im = dynamic_cast<mitk::Image*>(it->Value()->GetData());
      if (Im == nullptr)
        continue;

      if (it->Value() == node)
        continue;

      it->Value()->SetVisibility(false);
      mitk::DataNode* skin = GetSkinNode(it->Value());

      if (skin != nullptr)
        skin->SetVisibility(false);
    }
    return;
  }

  // Set visible only the selected surface
  auto surf = dynamic_cast<mitk::Surface*>(node->GetData());
  if (surf != nullptr)
  {
    // Hide the other images/surfaces
    auto dataTypePred = mitk::NodePredicateDataType::New("Surface");
    auto helper = mitk::NodePredicateProperty::New("helper object",mitk::BoolProperty::New(true));
    auto notHelper = mitk::NodePredicateNot::New(helper);
    auto pred = mitk::NodePredicateAnd::New(dataTypePred,notHelper);

    mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = so->Begin(); it != so->End(); ++it)
      it->Value()->SetVisibility(it->Value() == node);

    // Perform reinit in selected node
    if (reinit)
      ViewCommands::Reinit(node,mDataStorage);

    // show registration planned nodes
    ShowPlannedPoints(true,node);

    node->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

mitk::DataNode::Pointer NodesManager::GetSkinNode(const mitk::DataNode::Pointer node)
{
  return mDataStorage->GetNamedDerivedNode(SURFACE_NAME.c_str(),node);
}


// ** PLANNED POINTS **//
mitk::DataNode::Pointer NodesManager::CreatePlannedPoints()
{
  // Pointset for primary registration planned points
  // Delete if already existing
  auto pred = mitk::NodePredicateProperty::New("navCAS.registration.isPrimaryPlannedPointset",mitk::BoolProperty::New(true));
  auto so = mDataStorage->GetSubset(pred);
  for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
    mDataStorage->Remove(it->Value());

  // pointset with custom mapper
  mPrimaryPointsetNode = mitk::DataNode::New();
  mPrimaryPointsetNode->SetName("Primary Registration Planned Points");
  mPrimaryPointsetNode->SetData(mPrimaryPlannedPoints);
  mPrimaryPointsetNode->SetBoolProperty("helper object",!GetShowHelperObjects());
  mPrimaryPointsetNode->SetBoolProperty("navCAS.registration.isPrimaryPlannedPointset",true);
  mPrimaryPointsetNode->SetColor(0,0.8,1);
  LabeledPointSetMapper3D::Pointer mapper = LabeledPointSetMapper3D::New();
  mPrimaryPointsetNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

  LabeledPointSetMapper2D::Pointer mapper2D = LabeledPointSetMapper2D::New();
  mPrimaryPointsetNode->SetStringProperty("default label","P");
  mPrimaryPointsetNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard2D, mapper2D);

  mDataStorage->Add(mPrimaryPointsetNode);

  return mPrimaryPointsetNode;
}

unsigned int NodesManager::ShowPlannedPoints(bool show, mitk::DataNode::Pointer imageNode)
{
  // create points if neccessary
  if ( mPrimaryPointsetNode.IsNull() || (!mDataStorage->Exists(mPrimaryPointsetNode)) )
    CreatePlannedPoints();

  mPrimaryPointsetNode->SetVisibility(show);

  if (imageNode.IsNull())
  {
    cerr << "Null node set as patient image" << endl;
    return 0;
  }

  unsigned int validPoints = 0;
  QString useProp("navCAS.registration.useP");
  // Show chosen registration points
  for (unsigned int n=1; ; n++)
  {
    // check if has point
    std::stringstream pointName;
    pointName << "navCAS.registration.p" << n;
    mitk::Point3D point;
    bool hasPoint = imageNode->GetPropertyValue(pointName.str().c_str(),point);
    if (!hasPoint)
      break;

    // check if point can be used
    bool usePoint=true;
    imageNode->GetBoolProperty((useProp+QString::number(n)).toStdString().c_str(),usePoint);
    if (!usePoint)
    {
      // remove point
      mPrimaryPlannedPoints->RemovePointIfExists(n-1);
      continue;
    }

    validPoints++;
    mPrimaryPlannedPoints->SetPoint(n-1,point);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return validPoints;
}

void NodesManager::ClearPlannedPoints()
{
  mPrimaryPlannedPoints->Clear();
}




void NodesManager::SetUsePlannedPoint(mitk::DataNode::Pointer im,unsigned int pos, bool val)
{
  stringstream name;
  name << "navCAS.registration.useP" << pos+1;
  im->SetBoolProperty(name.str().c_str(),val);
}

bool NodesManager::GetUsePlannedPoint(mitk::DataNode::Pointer im, unsigned int pos)
{
  stringstream name;
  name << "navCAS.registration.useP" << pos+1;
  bool val = false;
  im->GetBoolProperty(name.str().c_str(),val);

  return val;
}

void NodesManager::SetUseRealPoint(unsigned int pos, bool val)
{
  stringstream name;
  name << "navCAS.navreg.useP" << pos+1;
  GetSystemNode()->SetBoolProperty(name.str().c_str(),val);
}

bool NodesManager::GetUseRealPoint(unsigned int pos)
{
  stringstream name;
  name << "navCAS.navreg.useP" << pos+1;
  bool val = false;
  GetSystemNode()->GetBoolProperty(name.str().c_str(),val);

  return val;
}

// ** REAL POINTS ** //

void NodesManager::CreatePrimaryRealPoints()
{
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.navreg.isPrimaryRealNode",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);

  mUseRealPoint.clear();

  if (so->empty())
  {
    cout << "No real points found, creating them" << std::endl;

    // Pointset for primary registration real points, set using the probe
    mitk::DataNode::Pointer primaryRealNode = mitk::DataNode::New();
    primaryRealNode->SetName("Primary Registration Real Points");
    primaryRealNode->SetData(mPrimaryRealPoints);
    primaryRealNode->SetBoolProperty("helper object",!GetShowHelperObjects());
    primaryRealNode->SetBoolProperty("navCAS.navreg.isPrimaryRealNode",true);
    primaryRealNode->SetVisibility(false);
    mDataStorage->Add(primaryRealNode);

    for (unsigned int i=0; i<5; i++)
      mUseRealPoint.push_back(false);
  }
  else if (so->Size() == 1)
  {
    cout << "There already exist stored real points, no need to create them again" << std::endl;

    cout << "Node name: " << so->Begin()->Value()->GetName() << std::endl;

    // update
    mPrimaryRealPoints = dynamic_cast<mitk::PointSet*>(so->Begin()->Value()->GetData());

    cout << "size: " << mPrimaryRealPoints->GetSize() << std::endl;

    // Get only valid points and store validation
    for (int i=0; i<mPrimaryRealPoints->GetSize(); i++)
    {
      mUseRealPoint.push_back(mPrimaryRealPoints->IndexExists(i));
      if (mUseRealPoint[i])
        cout << "Real point " << i << ": " << mPrimaryRealPoints->GetPoint(i) << std::endl;
    }
  }
  else
  {
    mitkThrow() << "ERROR: several primary real points loaded in the scene!";
  }
}

void NodesManager::CreateSecondaryPoints()
{
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.navreg.isSecondaryPointsetNode",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = mDataStorage->GetSubset(pred);

  if (so->empty())
  {
    // Pointset for secondary registration points
    mSecondaryNode = mitk::DataNode::New();
    mSecondaryNode->SetName("Secondary Registration points");
    mSecondaryNode->SetData(mSecondaryPoints);
    mSecondaryNode->SetBoolProperty("helper object",!GetShowHelperObjects());
    mSecondaryNode->SetBoolProperty("navCAS.navreg.isSecondaryPointsetNode",true);
    mSecondaryNode->SetColor(0,1,1);
    mSecondaryNode->SetFloatProperty("pointsize",3.0);
    mDataStorage->Add(mSecondaryNode);
  }
  else if (so->Size() == 1)
  {
    cout << "There already exist stored secondary points, no need to create them again" << std::endl;
    mSecondaryNode = so->Begin()->Value();
  }
  else
  {
    mitkThrow() << "ERROR: several secondary points loaded in the scene!";
  }
}

void NodesManager::SetSecondaryPointsVisibility(bool vis)
{
  mSecondaryNode->SetVisibility(vis);
}

void NodesManager::SetPrimaryRealPoint(unsigned int pos, mitk::Point3D point)
{
  mPrimaryRealPoints->SetPoint(pos,point);
}

mitk::PointSet::Pointer NodesManager::GetPrimaryRealPointset()
{
  return mPrimaryRealPoints;
}

mitk::PointSet::Pointer NodesManager::GetPrimaryPlannedPointset()
{
  return mPrimaryPlannedPoints;
}

mitk::PointSet::Pointer NodesManager::GetSecondaryPointset()
{
  return mSecondaryPoints;
}

void NodesManager::ClearSecondaryPoints()
{
  int size = mSecondaryPoints->GetSize();
  for (int i=0; i<size; i++)
    mSecondaryPoints->RemovePointIfExists(i);

  mSecondaryPoints->Clear();
}

int NodesManager::InsertNewSecondaryPoint(mitk::Point3D point)
{
  // if point is near another one (5.0 mm) refuse insertion
  for (int id=0; id < mSecondaryPoints->GetSize(); id++)
  {
    mitk::Point3D cp = mSecondaryPoints->GetPoint(id);

    if (vtkMath::Distance2BetweenPoints(point.GetDataPointer(),cp.GetDataPointer()) < 25.0)
      return -1;
  }

  return mSecondaryPoints->InsertPoint(point);
}

void NodesManager::RemoveLastSecondaryPoint()
{
  if (mSecondaryPoints->GetSize() > 0)
    mSecondaryPoints->RemovePointAtEnd();
}

void NodesManager::StoreTransformInNode(const vtkMatrix4x4* transform, mitk::DataNode::Pointer node)
{
  if (node.IsNull())
    return;

  for (unsigned int i=0; i<4; i++)
  {
    mitk::Point4D line;
    for (unsigned int j=0; j<4; j++)
      line[j] = transform->GetElement(i,j);

    stringstream name;
    name << "navCAS.navreg.transformation.line";
    name << i;
    node->SetProperty(name.str().c_str(),mitk::Point4dProperty::New(line));
  }
}


vtkSmartPointer<vtkMatrix4x4> NodesManager::GetTransformFromNode(mitk::DataNode::Pointer node)
{
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();

  for (unsigned int i=0; i<4; i++)
  {
    stringstream name;
    name << "navCAS.navreg.transformation.line";
    name << i;

    mitk::Point4D line;
    if (!node->GetPropertyValue(name.str().c_str(),line))
      return nullptr;

    for (unsigned int j=0; j<4; j++)
      matrix->SetElement(i,j,line[j]);
  }

  return matrix;
}

void NodesManager::SetRenderWindow(QmitkRenderWindow* axial, QmitkRenderWindow* coronal, QmitkRenderWindow* sagittal)
{
  MITK_INFO << "Updating render window in NodesMAnager";

  mRenderWindow[0]=axial;
  mRenderWindow[1]=coronal;
  mRenderWindow[2]=sagittal;

  if (  (axial == nullptr) || (coronal == nullptr) || (sagittal == nullptr))
  {
    mProbe2DManager[0]->SetRenderWindow(nullptr);
    mProbe2DManager[1]->SetRenderWindow(nullptr);
    mProbe2DManager[2]->SetRenderWindow(nullptr);
    return;
  }

  // Create CircleOverlayControls[3] and set its corresponding QMitkRenderWindows
  Probe2DManager::Pointer axialOverlayControls = Probe2DManager::New(axial);
  mProbe2DManager[0] = axialOverlayControls;

  Probe2DManager::Pointer coronalOverlayControls = Probe2DManager::New(coronal);
  mProbe2DManager[1] = coronalOverlayControls;

  Probe2DManager::Pointer sagittalOverlayControls = Probe2DManager::New(sagittal);
  mProbe2DManager[2] = sagittalOverlayControls;

  mProbe2DManager[0]->ShowCircle(true);
  mProbe2DManager[1]->ShowCircle(true);
  mProbe2DManager[2]->ShowCircle(true);
}


void NodesManager::ErrorMessage(const char* err, const char* advice)
{
  QMessageBox msgBox;
  msgBox.setText(err);
  msgBox.setInformativeText(advice);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

bool NodesManager::GetShowHelperObjects()
{
  QSettings read("navCAS", "CAS");
  read.beginGroup("Common Settings");
  bool showHelperObjects = read.value("ShowHelperObjects").toBool();
  read.endGroup();

  return showHelperObjects;
}

void NodesManager::SetShowHelperObjects(bool show)
{
  // store qsetting
  QSettings write("navCAS", "CAS");
  write.beginGroup("Common Settings");
  write.setValue("ShowHelperObjects", show);
  write.endGroup();
}
