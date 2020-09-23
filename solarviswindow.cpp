
#include "solarviswindow.h"
#include "ui_solarviswindow.h"

#include <qdebug.h>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkStringArray.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkLabeledDataMapper.h>
#include <vtkActor2D.h>
#include <vtkCharArray.h>
#include <vtkCellData.h>

/*Bao's include - BEGIN*/

#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkTransform.h>
#include <vtkAxesActor.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "basicmode.h"
#include "realmode.h"
#include <QtMath>

/*Bao's include - END*/

/*Bao's TODO:   the three methods: constructor, on_actionOpen_data_triggered() and createNewSolarSystemFromFile(QString) contain

                similar lengthy code that should be cut out and put into a new method to improve readability                */


SolarVisWindow::SolarVisWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SolarVisWindow)
{
    // Flag determining visualisation type

    bool useSpheres = true;

    // Initialise user interface

    ui->setupUi(this);

    // Create solarsystem simulation
    n_planet = N_PLANET;
    m_solarSystem = new SolarSystem(n_planet);

    // Create a set of points for representing our planet bodies

    m_points = vtkSmartPointer<vtkPoints>::New();

    for (int i=0;i<m_solarSystem->size();i++)
    {
        m_points->InsertNextPoint(
                    m_solarSystem->at(i)->location().x(),
                    m_solarSystem->at(i)->location().y(),
                    m_solarSystem->at(i)->location().z()
                    );
    }

    // Map points to a vtkPolyData

    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);

    // Create a scalar array to hold the planet masses to be able to visualise this

    vtkSmartPointer<vtkDoubleArray> radiusArray = vtkSmartPointer<vtkDoubleArray>::New();
    radiusArray->SetNumberOfValues(m_solarSystem->size());

    for (int i=0;i<m_solarSystem->size();i++)
        radiusArray->SetValue(i, m_solarSystem->at(i)->mass());

    m_polyData->GetPointData()->SetScalars(radiusArray);

    // This is our glyph that will represent the planet bodies

    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetRadius(0.5);

    if (useSpheres)
    {
        // Create Glyph3D objects

        m_glyph3d = vtkSmartPointer<vtkGlyph3D>::New();
        m_glyph3d->SetSourceConnection(m_sphereSource->GetOutputPort());
        m_glyph3d->SetInputData(m_polyData);
        m_glyph3d->SetScaleModeToScaleByScalar();
        m_glyph3d->SetColorModeToColorByScalar();
        m_glyph3d->ScalingOn();
        m_glyph3d->Update();
    }
    else
    {
        // Create GlyphFilter

        m_glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        m_glyphFilter->SetInputData(m_polyData);
        m_glyphFilter->Update();
    }

    // Create mapper

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    if (useSpheres)
    {
        m_mapper->SetInputConnection(m_glyph3d->GetOutputPort());
        m_mapper->SetScalarRange(0.1, 2);
        m_mapper->ScalarVisibilityOn();
    }
    else
    {
        m_mapper->SetInputConnection(m_glyphFilter->GetOutputPort());
    }

    // Create sun

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(2);
    sphereSource->SetThetaResolution(30);
    sphereSource->SetPhiResolution(30);
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    sunActor = vtkSmartPointer<vtkActor>::New();
    sunActor->SetMapper(mapper);
    sunActor->GetProperty()->SetColor(1, 0.8, 0.133);

    // Create actor

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetPointSize(2);

    // Create a VTK renderer

    m_renderer = vtkSmartPointer<vtkRenderer>::New();

    /*Bao's code - BEGIN - Label the planets*/

    // Add a label to each point
    vtkSmartPointer<vtkStringArray> pointNames =
      vtkSmartPointer<vtkStringArray>::New();
    pointNames->SetNumberOfComponents(1);
    pointNames->SetName("Name of planets");

    for (vtkIdType i = 0; i < m_solarSystem->size(); i++)
        pointNames->InsertNextValue(std::to_string(i+1));

    m_polyData->GetPointData()->AddArray(pointNames);


    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
      vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Masses");
    labelMapper->SetInputData(m_polyData);
    labelMapper->SetLabelModeToLabelFieldData();
    labelMapper->SetFieldDataName ("Name of planets");
    //labelMapper->SetLabelFormat("%6.2f");
    // Create label actor

    vtkSmartPointer<vtkActor2D> labelActor =
        vtkSmartPointer<vtkActor2D>::New();
      labelActor->SetMapper( labelMapper );


    /*Bao's code - END - Label the planets*/


    /*Bao's code - BEGIN - Add trajectories*/

    // Since we have many trajectories, we have to create arrays for everything, each element of each array is for a trajectory
    double originArray[n_planet][3];
    vtkSmartPointer<vtkPolyLine> b_polyLineArray[n_planet];
    vtkSmartPointer<vtkCellArray> b_cellArrayArray[n_planet];
    vtkSmartPointer<vtkPolyData> b_polyDataArray[n_planet];
    vtkSmartPointer<vtkPolyDataMapper> b_polyDataMapperArray[n_planet];
    vtkSmartPointer<vtkActor> b_vtkActorArray[n_planet];
    for (unsigned int i = 0; i < n_planet; i++){
        // Create the origin point.
        originArray[i][0] = m_solarSystem->at(i)->location().x();
        originArray[i][1] = m_solarSystem->at(i)->location().y();
        originArray[i][2] = m_solarSystem->at(i)->location().z();
        // Create vtkPoints objects and store N_TRAJECTORY_SEGMENT points in them
        b_pointsArray[i] = vtkSmartPointer<vtkPoints>::New();
        for (unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j ++)//just store N_TRAJECTORY_SEGMENT identical points (initilization)
            b_pointsArray[i]->InsertNextPoint(originArray[i]);
        // Create vtkPolyLines to store the trajectories
        b_polyLineArray[i] = vtkSmartPointer<vtkPolyLine>::New();
        b_polyLineArray[i]->GetPointIds()->SetNumberOfIds(N_TRAJECTORY_SEGMENT);
        for(unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j++)
        {
            b_polyLineArray[i]->GetPointIds()->SetId(j,j);
        }
        // Create cell array array to store the lines in and add the lines to it
        b_cellArrayArray[i] = vtkSmartPointer<vtkCellArray>::New();
        b_cellArrayArray[i]->InsertNextCell(b_polyLineArray[i]);
        // Create a polydata to store everything in
        b_polyDataArray[i] = vtkSmartPointer<vtkPolyData>::New();
        // Add the points to the dataset
        b_polyDataArray[i]->SetPoints(b_pointsArray[i]);
        // Add the lines to the dataset
        b_polyDataArray[i]->SetLines(b_cellArrayArray[i]);
        // Setup actor and mapper
        b_polyDataMapperArray[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
        b_polyDataMapperArray[i]->SetInputData(b_polyDataArray[i]);
        b_vtkActorArray[i] = vtkSmartPointer<vtkActor>::New();
        b_vtkActorArray[i]->SetMapper(b_polyDataMapperArray[i]);
        m_renderer->AddActor(b_vtkActorArray[i]);
    }

    /*Bao's code - END - Add trajectories*/

    /*Bao's code - BEGIN - Add axes*/

    vtkSmartPointer<vtkTransform> transform =
      vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0.0, 0.0, -30.0);

    vtkSmartPointer<vtkAxesActor> axes =
      vtkSmartPointer<vtkAxesActor>::New();

    // The axes are positioned with a user transform
    axes->SetUserTransform(transform);

    // properties of the axes labels can be set as follows
    // this sets the x axis label to red
    // axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1,0,0);

    // the actual text of the axis label can be changed:
    // axes->SetXAxisLabelText("test");

    m_renderer->AddActor(axes);

    /*Bao's code - END - Add axes*/

    // Attach actor

    m_renderer->AddActor(m_actor);
    m_renderer->AddActor(sunActor);
    /*Bao's code - BEGIN*/
    m_renderer->AddActor(labelActor);
    /*Bao's code - END*/
    m_renderer->SetBackground(0.0, 0.0, 0.0);

    // Camera

    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera->SetPosition(0, 100, 0);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(1, 0, 0);

    m_renderer->SetActiveCamera(camera);

    // Connect to Qt widget

    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

    // Create frame update timer (50 Hz)

    m_redrawTimer = new QTimer(this);
    this->connect(m_redrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawTimer()));
    m_redrawTimer->start(1000.0/50);
}

SolarVisWindow::~SolarVisWindow()
{
    delete m_solarSystem;
    delete ui;
}

void SolarVisWindow::onRedrawTimer()
{
    // Simulation update

    m_solarSystem->update(1.0/50.0);

    // Update VTK data

    for (int i=0;i<m_solarSystem->size();i++)
    {
        m_points->SetPoint(i,
                           m_solarSystem->at(i)->location().x(),
                           m_solarSystem->at(i)->location().y(),
                           m_solarSystem->at(i)->location().z()
                           );

    }


    // Tell VTK data is modified

    m_points->Modified();

    /*Bao's code - BEGIN*/
    counter = (counter + 1) % 10;
    // Only update trajectory after onRedrawTimer has ran 10 times
    if (counter == 0){
        for (unsigned int i = 0; i < n_planet; i++){
            double tempPoint[3];
            for (int j=0;j<N_TRAJECTORY_SEGMENT;j++)
            {
                if (j != (N_TRAJECTORY_SEGMENT - 1))
                    b_pointsArray[i]->GetPoint(j+1, tempPoint);
                else
                    m_points->GetPoint(i, tempPoint);
                b_pointsArray[i]->SetPoint(j,
                                 tempPoint[0],
                                 tempPoint[1],
                                 tempPoint[2]
                                 );

            }
            b_pointsArray[i]->Modified();
        }
    }
    /*Bao's code - END*/

    // Redraw the qvtkWidget

    this->ui->qvtkWidget->update();
}

void SolarVisWindow::on_actionOpen_data_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "/home/cppdev/solar_vtk",
                                                    "CSV Files (*.csv)");
    solarSpeedUpValue = 1; //reset the speed
    createNewSolarSystemFromFile (fileName);
}

void SolarVisWindow::createNewSolarSystemFromFile (QString fileName){
    QFile f1(fileName);
    QStringList planetNames, planetMassesStr, planetXStr, planetYStr, planetZStr;
    f1.open(QIODevice::ReadOnly);
    QTextStream s1(&f1);
    unsigned int temp_n_planet = 0;
    while (!s1.atEnd()){
        temp_n_planet ++;
        QString s=s1.readLine(); // reads line from file
        planetNames.append(s.split(",").first()); // appends first column to list, ',' is separator
        planetMassesStr.append(s.split(",")[1]);
        planetXStr.append(s.split(",")[2]);
        planetYStr.append(s.split(",")[3]);
        planetZStr.append(s.split(",")[4]);
    }
    temp_n_planet -= 2; //not counting the first row and sun's row
    qDebug()<<planetNames;
    f1.close();

    // Update the Solar system

    bool useSpheres = true;

    // Create solarsystem simulation
    n_planet = temp_n_planet;

    delete m_solarSystem;

    double sunMass;
    double planetMasses[n_planet];
    double planetX[n_planet];
    double planetY[n_planet];
    double planetZ[n_planet];

    sunMass = planetMassesStr.at(1).toDouble();

    for (unsigned int i=0; i<n_planet; i++)
    {
        planetMasses[i] = planetMassesStr.at(i+2).toDouble();
        planetX[i]      = planetXStr.at(i+2).toDouble();
        planetY[i]      = planetYStr.at(i+2).toDouble();
        planetZ[i]      = planetZStr.at(i+2).toDouble();
    }

    m_solarSystem = new SolarSystem(n_planet, sunMass, planetMasses, planetX, planetY, planetZ, REAL_G);

    // Create a set of points for representing our planet bodies

    m_points = vtkSmartPointer<vtkPoints>::New();//dont need to delete vtksmartpointer

    for (int i=0;i<m_solarSystem->size();i++)
    {
        m_points->InsertNextPoint(
                    m_solarSystem->at(i)->location().x(),
                    m_solarSystem->at(i)->location().y(),
                    m_solarSystem->at(i)->location().z()
                    );
    }

    // Map points to a vtkPolyData

    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);

    // Create a scalar array to hold the planet masses to be able to visualise this

    vtkSmartPointer<vtkDoubleArray> radiusArray = vtkSmartPointer<vtkDoubleArray>::New();
    radiusArray->SetNumberOfValues(m_solarSystem->size());

    for (int i=0;i<m_solarSystem->size();i++)
        radiusArray->SetValue(i, m_solarSystem->at(i)->mass());

    m_polyData->GetPointData()->SetScalars(radiusArray);

    // This is our glyph that will represent the planet bodies

    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetRadius(0.5);

    if (useSpheres)
    {
        // Create Glyph3D objects

        m_glyph3d = vtkSmartPointer<vtkGlyph3D>::New();
        m_glyph3d->SetSourceConnection(m_sphereSource->GetOutputPort());
        m_glyph3d->SetInputData(m_polyData);
        m_glyph3d->SetScaleModeToScaleByScalar();
        m_glyph3d->SetColorModeToColorByScalar();
        m_glyph3d->ScalingOn();
        m_glyph3d->Update();
    }
    else
    {
        // Create GlyphFilter

        m_glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        m_glyphFilter->SetInputData(m_polyData);
        m_glyphFilter->Update();
    }

    // Create mapper

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    if (useSpheres)
    {
        m_mapper->SetInputConnection(m_glyph3d->GetOutputPort());
        m_mapper->SetScalarRange(0.1, 2);
        m_mapper->ScalarVisibilityOn();
    }
    else
    {
        m_mapper->SetInputConnection(m_glyphFilter->GetOutputPort());
    }

    // Create sun

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(2);
    sphereSource->SetThetaResolution(30);
    sphereSource->SetPhiResolution(30);
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    sunActor = vtkSmartPointer<vtkActor>::New();
    sunActor->SetMapper(mapper);
    sunActor->GetProperty()->SetColor(1, 0.8, 0.133);

    // Create actor

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetPointSize(2);

    // Remove renderer before re-initialize renderer

    this->ui->qvtkWidget->GetRenderWindow()->RemoveRenderer(m_renderer);

    // Create a VTK renderer

    m_renderer = vtkSmartPointer<vtkRenderer>::New();

    /*Bao's code - BEGIN - Label the planets*/

    // Add a label to each point
    vtkSmartPointer<vtkStringArray> pointNames =
      vtkSmartPointer<vtkStringArray>::New();
    pointNames->SetNumberOfComponents(1);
    pointNames->SetName("Name of planets");

    for (vtkIdType i = 0; i < m_solarSystem->size(); i++)
        pointNames->InsertNextValue(planetNames.at(i+2).toStdString()); //not counting the first row and sun's row

    m_polyData->GetPointData()->AddArray(pointNames);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
      vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Masses");
    labelMapper->SetInputData(m_polyData);
    labelMapper->SetLabelModeToLabelFieldData();
    labelMapper->SetFieldDataName ("Name of planets");
    //labelMapper->SetLabelFormat("%6.2f");
    // Create label actor

    vtkSmartPointer<vtkActor2D> labelActor =
        vtkSmartPointer<vtkActor2D>::New();
      labelActor->SetMapper( labelMapper );


    /*Bao's code - END - Label the planets*/


    /*Bao's code - BEGIN - Add trajectories*/

    // Since we have many trajectories, we have to create arrays for everything, each element of each array is for a trajectory
    double originArray[n_planet][3];
    vtkSmartPointer<vtkPolyLine> b_polyLineArray[n_planet];
    vtkSmartPointer<vtkCellArray> b_cellArrayArray[n_planet];
    vtkSmartPointer<vtkPolyData> b_polyDataArray[n_planet];
    vtkSmartPointer<vtkPolyDataMapper> b_polyDataMapperArray[n_planet];
    vtkSmartPointer<vtkActor> b_vtkActorArray[n_planet];
    for (unsigned int i = 0; i < n_planet; i++){
        // Create the origin point.
        originArray[i][0] = m_solarSystem->at(i)->location().x();
        originArray[i][1] = m_solarSystem->at(i)->location().y();
        originArray[i][2] = m_solarSystem->at(i)->location().z();
        // Create vtkPoints objects and store N_TRAJECTORY_SEGMENT points in them
        b_pointsArray[i] = vtkSmartPointer<vtkPoints>::New();
        for (unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j ++)//just store N_TRAJECTORY_SEGMENT identical points (initilization)
            b_pointsArray[i]->InsertNextPoint(originArray[i]);
        // Create vtkPolyLines to store the trajectories
        b_polyLineArray[i] = vtkSmartPointer<vtkPolyLine>::New();
        b_polyLineArray[i]->GetPointIds()->SetNumberOfIds(N_TRAJECTORY_SEGMENT);
        for(unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j++)
        {
            b_polyLineArray[i]->GetPointIds()->SetId(j,j);
        }
        // Create cell array array to store the lines in and add the lines to it
        b_cellArrayArray[i] = vtkSmartPointer<vtkCellArray>::New();
        b_cellArrayArray[i]->InsertNextCell(b_polyLineArray[i]);
        // Create a polydata to store everything in
        b_polyDataArray[i] = vtkSmartPointer<vtkPolyData>::New();
        // Add the points to the dataset
        b_polyDataArray[i]->SetPoints(b_pointsArray[i]);
        // Add the lines to the dataset
        b_polyDataArray[i]->SetLines(b_cellArrayArray[i]);
        // Setup actor and mapper
        b_polyDataMapperArray[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
        b_polyDataMapperArray[i]->SetInputData(b_polyDataArray[i]);
        b_vtkActorArray[i] = vtkSmartPointer<vtkActor>::New();
        b_vtkActorArray[i]->SetMapper(b_polyDataMapperArray[i]);
        m_renderer->AddActor(b_vtkActorArray[i]);
    }

    /*Bao's code - END - Add trajectories*/

    /*Bao's code - BEGIN - Add axes*/

    vtkSmartPointer<vtkTransform> transform =
      vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0.0, 0.0, -30.0);

    vtkSmartPointer<vtkAxesActor> axes =
      vtkSmartPointer<vtkAxesActor>::New();

    // The axes are positioned with a user transform
    axes->SetUserTransform(transform);

    m_renderer->AddActor(axes);

    /*Bao's code - END - Add axes*/

    // Attach actor

    m_renderer->AddActor(m_actor);
    m_renderer->AddActor(sunActor);
    /*Bao's code - BEGIN*/
    m_renderer->AddActor(labelActor);
    /*Bao's code - END*/
    m_renderer->SetBackground(0.0, 0.0, 0.0);

    // Camera

    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera->SetPosition(0, 100, 0);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(1, 0, 0);

    m_renderer->SetActiveCamera(camera);

    // Connect to Qt widget

    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);
}

void SolarVisWindow::on_actionPause_triggered()
{
    if (playing == true)
        this->disconnect(m_redrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawTimer()));
    else
        this->connect(m_redrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawTimer()));
    playing = !playing;
}

void SolarVisWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"),
      "Solar System Simulator 2015 by Bao Ho\nA project within the Advanced Programming in Science and Technology course\nSkeleton code by Jonas Lindemann");
}

void SolarVisWindow::on_actionManual_triggered()
{
    QMessageBox::information(this, tr("Manual"),
                             "There are two modes.\n"
                             "   In 'Basic Mode' you can change update interval,\n"
                             "sun size or create a new solar system.\n"
                             "   In 'Real Mode' you can load a .csv file and let\n"
                             "the simulator build the solar system with your\n"
                             "data. Please look at the sample file 'planet_config.csv'.\n"
                             "You can also speed up the simulation.\n"
                             "   The 'File->Load Config' menu is the same as in the\n"
                             "Real Mode, I just set it up for demonstration.\n"
                             "   The 'Real Mode' is still not 'real' since it\n"
                             "has problem with the scale of parameters.\n"
                             "Nevertheless, hope you have fun!");
}

void SolarVisWindow::on_actionBasic_Mode_triggered()
{
    BasicMode basicPopup;
    basicPopup.setModal(true);
    basicPopup.exec();
    qDebug() << basicPopup.getNumberOfPlanetFromUser()<<basicPopup.getUpdateIntervalFromUser()<<basicPopup.getSunSizeFromUser();
    if (basicPopup.getbasicModeAction() == BASIC_MODE_UPDATE_ALL)
        startBasicMode (basicPopup.getNumberOfPlanetFromUser(), basicPopup.getUpdateIntervalFromUser(), basicPopup.getSunSizeFromUser());
    else if (basicPopup.getbasicModeAction() == BASIC_MODE_UPDATE_SIZE)
        changeSunSize(basicPopup.getSunSizeFromUser());
    else if (basicPopup.getbasicModeAction() == BASIC_MODE_UPDATE_INTERVAL)
        updateInterval(basicPopup.getUpdateIntervalFromUser());
}

void SolarVisWindow::updateInterval(unsigned int interval){
    // Create frame update timer (1000/updateInterval Hz)
    m_redrawTimer->setInterval(interval);
}

void SolarVisWindow::changeSunSize(unsigned int sunSize){
    m_renderer->RemoveActor(sunActor);
    // Create sun
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(sunSize);
    sphereSource->SetThetaResolution(30);
    sphereSource->SetPhiResolution(30);
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    sunActor = vtkSmartPointer<vtkActor>::New();
    sunActor->SetMapper(mapper);
    sunActor->GetProperty()->SetColor(1, 0.8, 0.133);

    m_renderer->AddActor(sunActor);
}

void SolarVisWindow::startBasicMode(unsigned int noOfPlanets, unsigned int updateInterval, unsigned int sunSize)
{
    // Update the Solar system

    bool useSpheres = true;

    // Create solarsystem simulation
    n_planet = noOfPlanets;
    m_solarSystem = new SolarSystem(n_planet);

    // Create a set of points for representing our planet bodies

    m_points = vtkSmartPointer<vtkPoints>::New();

    for (int i=0;i<m_solarSystem->size();i++)
    {
        m_points->InsertNextPoint(
                    m_solarSystem->at(i)->location().x(),
                    m_solarSystem->at(i)->location().y(),
                    m_solarSystem->at(i)->location().z()
                    );
    }

    // Map points to a vtkPolyData

    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);

    // Create a scalar array to hold the planet masses to be able to visualise this

    vtkSmartPointer<vtkDoubleArray> radiusArray = vtkSmartPointer<vtkDoubleArray>::New();
    radiusArray->SetNumberOfValues(m_solarSystem->size());

    for (int i=0;i<m_solarSystem->size();i++)
        radiusArray->SetValue(i, m_solarSystem->at(i)->mass());

    m_polyData->GetPointData()->SetScalars(radiusArray);

    // This is our glyph that will represent the planet bodies

    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetRadius(0.5);

    if (useSpheres)
    {
        // Create Glyph3D objects

        m_glyph3d = vtkSmartPointer<vtkGlyph3D>::New();
        m_glyph3d->SetSourceConnection(m_sphereSource->GetOutputPort());
        m_glyph3d->SetInputData(m_polyData);
        m_glyph3d->SetScaleModeToScaleByScalar();
        m_glyph3d->SetColorModeToColorByScalar();
        m_glyph3d->ScalingOn();
        m_glyph3d->Update();
    }
    else
    {
        // Create GlyphFilter

        m_glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        m_glyphFilter->SetInputData(m_polyData);
        m_glyphFilter->Update();
    }

    // Create mapper

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    if (useSpheres)
    {
        m_mapper->SetInputConnection(m_glyph3d->GetOutputPort());
        m_mapper->SetScalarRange(0.1, 2);
        m_mapper->ScalarVisibilityOn();
    }
    else
    {
        m_mapper->SetInputConnection(m_glyphFilter->GetOutputPort());
    }

    // Create sun

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(sunSize);
    sphereSource->SetThetaResolution(30);
    sphereSource->SetPhiResolution(30);
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    sunActor = vtkSmartPointer<vtkActor>::New();
    sunActor->SetMapper(mapper);
    sunActor->GetProperty()->SetColor(1, 0.8, 0.133);

    // Create actor

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetPointSize(2);

    // Create a VTK renderer

    m_renderer = vtkSmartPointer<vtkRenderer>::New();

    /*Bao's code - BEGIN - Label the planets*/

    // Add a label to each point
    vtkSmartPointer<vtkStringArray> pointNames =
      vtkSmartPointer<vtkStringArray>::New();
    pointNames->SetNumberOfComponents(1);
    pointNames->SetName("Name of planets");

    for (vtkIdType i = 0; i < m_solarSystem->size(); i++)
        pointNames->InsertNextValue(std::to_string(i+1));

    m_polyData->GetPointData()->AddArray(pointNames);


    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
      vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Masses");
    labelMapper->SetInputData(m_polyData);
    labelMapper->SetLabelModeToLabelFieldData();
    labelMapper->SetFieldDataName ("Name of planets");
    //labelMapper->SetLabelFormat("%6.2f");
    // Create label actor

    vtkSmartPointer<vtkActor2D> labelActor =
        vtkSmartPointer<vtkActor2D>::New();
      labelActor->SetMapper( labelMapper );


    /*Bao's code - END - Label the planets*/


    /*Bao's code - BEGIN - Add trajectories*/

    // Since we have many trajectories, we have to create arrays for everything, each element of each array is for a trajectory
    double originArray[n_planet][3];
    vtkSmartPointer<vtkPolyLine> b_polyLineArray[n_planet];
    vtkSmartPointer<vtkCellArray> b_cellArrayArray[n_planet];
    vtkSmartPointer<vtkPolyData> b_polyDataArray[n_planet];
    vtkSmartPointer<vtkPolyDataMapper> b_polyDataMapperArray[n_planet];
    vtkSmartPointer<vtkActor> b_vtkActorArray[n_planet];
    for (unsigned int i = 0; i < n_planet; i++){
        // Create the origin point.
        originArray[i][0] = m_solarSystem->at(i)->location().x();
        originArray[i][1] = m_solarSystem->at(i)->location().y();
        originArray[i][2] = m_solarSystem->at(i)->location().z();
        // Create vtkPoints objects and store N_TRAJECTORY_SEGMENT points in them
        b_pointsArray[i] = vtkSmartPointer<vtkPoints>::New();
        for (unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j ++)//just store N_TRAJECTORY_SEGMENT identical points (initilization)
            b_pointsArray[i]->InsertNextPoint(originArray[i]);
        // Create vtkPolyLines to store the trajectories
        b_polyLineArray[i] = vtkSmartPointer<vtkPolyLine>::New();
        b_polyLineArray[i]->GetPointIds()->SetNumberOfIds(N_TRAJECTORY_SEGMENT);
        for(unsigned int j = 0; j < N_TRAJECTORY_SEGMENT; j++)
        {
            b_polyLineArray[i]->GetPointIds()->SetId(j,j);
        }
        // Create cell array array to store the lines in and add the lines to it
        b_cellArrayArray[i] = vtkSmartPointer<vtkCellArray>::New();
        b_cellArrayArray[i]->InsertNextCell(b_polyLineArray[i]);
        // Create a polydata to store everything in
        b_polyDataArray[i] = vtkSmartPointer<vtkPolyData>::New();
        // Add the points to the dataset
        b_polyDataArray[i]->SetPoints(b_pointsArray[i]);
        // Add the lines to the dataset
        b_polyDataArray[i]->SetLines(b_cellArrayArray[i]);
        // Setup actor and mapper
        b_polyDataMapperArray[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
        b_polyDataMapperArray[i]->SetInputData(b_polyDataArray[i]);
        b_vtkActorArray[i] = vtkSmartPointer<vtkActor>::New();
        b_vtkActorArray[i]->SetMapper(b_polyDataMapperArray[i]);
        m_renderer->AddActor(b_vtkActorArray[i]);
    }

    /*Bao's code - END - Add trajectories*/

    /*Bao's code - BEGIN - Add axes*/

    vtkSmartPointer<vtkTransform> transform =
      vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0.0, 0.0, -30.0);

    vtkSmartPointer<vtkAxesActor> axes =
      vtkSmartPointer<vtkAxesActor>::New();

    // The axes are positioned with a user transform
    axes->SetUserTransform(transform);

    // properties of the axes labels can be set as follows
    // this sets the x axis label to red
    // axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1,0,0);

    // the actual text of the axis label can be changed:
    // axes->SetXAxisLabelText("test");

    m_renderer->AddActor(axes);

    /*Bao's code - END - Add axes*/

    // Attach actor

    m_renderer->AddActor(m_actor);
    m_renderer->AddActor(sunActor);
    /*Bao's code - BEGIN*/
    m_renderer->AddActor(labelActor);
    /*Bao's code - END*/
    m_renderer->SetBackground(0.0, 0.0, 0.0);

    // Camera

    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera->SetPosition(0, 100, 0);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(1, 0, 0);

    m_renderer->SetActiveCamera(camera);

    // Connect to Qt widget

    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

    // Create frame update timer (1000/updateInterval Hz)
    m_redrawTimer->setInterval(updateInterval);
}

void SolarVisWindow::on_actionReal_Mode_triggered()
{
    RealMode realPopup;
    realPopup.setModal(true);
    realPopup.setSlider(solarSpeedUpValue);
    realPopup.exec();
    qDebug() << realPopup.getFileNameFromUI()
             << realPopup.getSpeedValueFromUI()
             << realPopup.getRealModeAction();
    solarSpeedUpValue = realPopup.getSpeedValueFromUI();
    if (realPopup.getRealModeAction() == REAL_MODE_CHANGE_SPEED)
        speedUpRealMode (realPopup.getSpeedValueFromUI());
    else if (realPopup.getRealModeAction() == REAL_MODE_CREATE_NEW){
        createNewSolarSystemFromFile(realPopup.getFileNameFromUI());
        speedUpRealMode (realPopup.getSpeedValueFromUI());
    }
}

void SolarVisWindow::speedUpRealMode(unsigned int speed){
    double dSpeed, newG;
    dSpeed = speed;

    // Reset G to the real G
    m_solarSystem->getSun()->setG(REAL_G);
    double x, z, v, vx, vz, a, ax, az, dist;
    for (int i = 0; i < m_solarSystem->size(); i++){
        // First calculate real acceleration and velocity of each planet
        x = m_solarSystem->at(i)->getX();
        z = m_solarSystem->at(i)->getZ();
        dist = qSqrt (x*x + 0*0 + z*z);

        a  = - m_solarSystem->getSun()->getG()*m_solarSystem->getSun()->getM()/(dist*dist);
        ax = a * (x-m_solarSystem->getSun()->getX()) / dist;
        az = a * (z-m_solarSystem->getSun()->getZ()) / dist;

        v  = qSqrt(-a*dist);
        vx = v * -(z-m_solarSystem->getSun()->getZ()) / dist;//initial v is perpendicular to initial a
        vz = v * (x-m_solarSystem->getSun()->getX()) / dist;
        // Then update each planet to the velocity and acceleration according to the new speed
        m_solarSystem->at(i)->setAx(ax*dSpeed*dSpeed);
        m_solarSystem->at(i)->setAz(az*dSpeed*dSpeed);
        m_solarSystem->at(i)->setVx(vx*dSpeed);
        m_solarSystem->at(i)->setVz(vz*dSpeed);
    }
    // Update G to the new G
    newG = REAL_G*dSpeed*dSpeed;
    m_solarSystem->getSun()->setG(newG);
}

