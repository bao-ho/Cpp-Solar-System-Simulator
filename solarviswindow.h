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
    std::vector<vtkSmartPointer<vtkPoints>> b_pointsArray;
    unsigned int n_planet;//actual number of planets
    unsigned int counter = 0; //used in onRedrawTimer function to slow down updating rate of drawing trajectory
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
    unsigned int speedUpFactor = 1e6;


public:
    explicit SolarVisWindow(QWidget *parent = 0);
    ~SolarVisWindow();
    void startBasicMode(unsigned int, unsigned int, double);
    void speedUpRealMode (unsigned int speedFactor);
    void createNewSolarSystemFromFile (QString);    
    Vec3d convertIntToRGB(unsigned int colorAsInt);
    void draw (unsigned int updateInterval, double diameterScale);

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
