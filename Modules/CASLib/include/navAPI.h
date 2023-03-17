/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVAPI_H
#define NAVAPI_H

#include <QThread>
#include <QTimer>

#include <vtkMatrix4x4.h>

#include <mitkPoint.h>

#include <ftkInterface.h>

#include <CASLibExports.h>

class CASLib_EXPORT navAPI : public QThread
{
    Q_OBJECT
public:
    navAPI();
    ~navAPI();

    enum MarkerId{None=0, SmallTracker=1, MediumTracker=2, BigTracker=4, Probe=123, TemporalProbe=124};

    static std::string GetGeometryFileName(MarkerId);

    bool DetectCamera();
    bool InitializeCamera();
    bool SetModeToStandard();
    bool CloseCamera();
    bool AddGeometry(const MarkerId geom);
    void StopNavigation();

    inline void SetSamplingPeriod(int sp){mSamplingPeriod = sp;}

    /*  Refrence marker only depends on the geometry of the marker.
        Every dissociated volume should be assigned a differente geometry
        Probe must not be used as reference marker.*/
    void SetReferenceMarker(const MarkerId geom){mReferenceMarkerType = geom;}

    void run();

signals:
    void MarkerPosition(unsigned int,std::vector<mitk::Point3D>);
    void FiducialsRawData(std::vector<mitk::Point3D>);
    void SingleMarkerInView(navAPI::MarkerId);
    void MultipleMarkersInView(std::vector<navAPI::MarkerId>);  // must leave the namespace for correct qt conection
    void ValidProbeInView();
    void ValidTemporalProbeInView();
    void InvalidProbeInView();
    void MultipleOrNullMarkersInView();
    void MarkerRelativePosition(unsigned int,vtkMatrix4x4*);

protected slots:
    void GetLastFrame();

protected:

    /*  Compute relative positions of m marker in mMarker vector*/
    void GetRelativePositions(unsigned int m);

private:
    QTimer*                 mTimer;
    ftkLibrary              mLib;
    uint64                  mSn;
    ftkFrameQuery*          mFrame;
    MarkerId                mReferenceMarkerType=None;

    /*  mMarker[0] = Reference marker
        mMarker[1] = Moving marker
        mMarker[2] = Probe*/
    std::vector<ftkMarker*> mMarker;

    /// sampling perior in ms
    int                   mSamplingPeriod;
};


#endif // NAVAPI_H
