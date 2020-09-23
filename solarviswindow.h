#ifndef PARTVISWINDOW_H
#define PARTVISWINDOW_H

#include <QMainWindow>
#include <QGLWidget>
#include <QDebug>
#include <QTimer>

#include <cmath>

#include <vtkParticleReader.h>
#include <vtkPolyDataMapper.h>

#include <vtkCylinderSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMath.h>
#include <vtkVertexGlyphFilter.h>

/*Bao's define - BEGIN*/
#define N_TRAJECTORY_SEGMENT 500
#define N_PLANET 100
#define REAL_G 0.185E-11 //still, not a real value of G
/*Bao's define - BEGIN*/

#include "solarsystem.h"

namespace Ui {
class SolarVisWindow;
}

class SolarVisWindow : public QMainWindow
{
    Q_OBJECT

private:
    vtkSmartPointer<vtkPoints> m_points;
    /*Bao's declaration - BEGIN*/
    vtkSmartPointer<vtkPoints> b_pointsArray[N_PLANET];//static allocation - NEED IMPROVEMENT
    unsigned int n_planet;//actual number of planets
    unsigned int counter = 0; //used in onRedrawTimer function to slow down updating rate of drawing trajectory
    bool playing = true;
    /*Bao's declaration - END*/
    vtkSmartPointer<vtkPolyData> m_polyData;
    vtkSmartPointer<vtkSphereSource> m_sphereSource;
    vtkSmartPointer<vtkGlyph3D> m_glyph3d;
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;
    vtkSmartPointer<vtkActor>  m_actor;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkVertexGlyphFilter> m_glyphFilter;

    vtkSmartPointer<vtkActor> sunActor;

    SolarSystem* m_solarSystem;

    QTimer* m_redrawTimer;

    //Remember the speed up value, to display on the slider
    unsigned int solarSpeedUpValue = 1;


public:
    explicit SolarVisWindow(QWidget *parent = 0);
    ~SolarVisWindow();
    void startBasicMode(unsigned int, unsigned int, unsigned int);
    void changeSunSize (unsigned int);
    void updateInterval (unsigned int);
    void speedUpRealMode (unsigned int);
    void createNewSolarSystemFromFile (QString);

private slots:
    void onRedrawTimer();

    void on_actionOpen_data_triggered();

    void on_actionPause_triggered();

    void on_actionAbout_triggered();

    void on_actionManual_triggered();

    void on_actionBasic_Mode_triggered();

    void on_actionReal_Mode_triggered();

private:
    Ui::SolarVisWindow *ui;
};



#endif // PARTVISWINDOW_H
