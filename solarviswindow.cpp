#include "solarviswindow.h"
#include "ui_solarviswindow.h"

#include <qdebug.h>
#include <vector>

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
#include "utils.h"
#include <QtMath>

#include <vtkLookupTable.h>

#define DISTANCE_JUPITER_SUN 778.6// *1e9 m
#define DISTANCE_MARS_SUN 227.9//*1e9 m

#define LENGTH_SCALE DISTANCE_MARS_SUN
#define DEFAULT_CAMERA_HEIGHT LENGTH_SCALE
#define N_TRAJECTORY_SEGMENT 600
#define N_PLANET 100
#define REAL_G 6.674e-3 //in scaled unit
#define UPDATE_INTERVAL 10 //10 miliseconds
#define DIAMETER_SCALE  10

#define SUN_MASS 19890000 //*1e23 kg
#define JUPITER_MASS 18980 //*1e23 kg
#define PLUTO_MASS 0.146//*1e23 kg
#define EARTH_MASS 59.7 //1e23 kg

#define SUN_DIAMETER 1.3927 //*1e9 m
#define JUPITER_DIAMETER 0.142984//*1e9 m
#define PLUTO_DIAMETER 0.002370 //*1e9 m
#define EARTH_DIAMETER 0.012756 //*1e9 m

#define RED  255*256*256 //red
#define BLACK 0
#define WHITE 256*256*256-1

#define PLANET_RESOLUTION 20

/*Bao's include - END*/

/*Bao's TODO:   the three methods: constructor, on_actionOpen_data_triggered() and createNewSolarSystemFromFile(QString) contain

                similar lengthy code that should be cut out and put into a new method to improve readability                */


SolarVisWindow::SolarVisWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SolarVisWindow)
{

    // Initialise user interface
    ui->setupUi(this);

    // Initialise timer
    m_redrawTimer = new QTimer(this);
    this->connect(m_redrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawTimer()));

    // Trick to avoid delete unallocated memory
    m_solarSystem = NULL;

    startBasicMode(N_PLANET, UPDATE_INTERVAL, DIAMETER_SCALE);
}

SolarVisWindow::~SolarVisWindow()
{
    delete m_solarSystem;
    delete ui;
}

void SolarVisWindow::onRedrawTimer()
{
    // Simulation update

    m_solarSystem->update(0.01);//update every 0.01s (10 ms)

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
    // Only update trajectory after onRedrawTimer has run 10 times
    if (counter == 0)
    {
        for (unsigned int i = 0; i < n_planet; i++)
        {
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "../Solar_Final", "CSV Files (*.csv)");
    speedUpFactor = 1e6; //reset the speed
    createNewSolarSystemFromFile (fileName);
}

void SolarVisWindow::createNewSolarSystemFromFile (QString fileName){
    QFile f1(fileName);
    QStringList planetNames, planetMassesStr, planetXStr, planetYStr, planetZStr;
    QStringList planetDiameterStr, planetColorStr;
    f1.open(QIODevice::ReadOnly);
    QTextStream s1(&f1);
    while (!s1.atEnd()){
        QString s=s1.readLine(); // reads line from file
        planetNames.append(s.split(",").first()); // appends first column to list, ',' is separator
        planetMassesStr.append(s.split(",")[1]);
        planetXStr.append(s.split(",")[2]);
        planetYStr.append(s.split(",")[3]);
        planetZStr.append(s.split(",")[4]);
        planetDiameterStr.append(s.split(",")[5]);
        planetColorStr.append(s.split(",")[6]);
    }
    f1.close();

    planetNames.removeFirst();
    planetNames.removeFirst();//remove two first names, which are "Objects" and "Sun"

    // Create solarsystem simulation
    n_planet = planetNames.size();

    delete m_solarSystem;

    std::vector<double> planetMasses;
    std::vector<double> planetX;
    std::vector<double> planetY;
    std::vector<double> planetZ;
    std::vector<double> planetDiameter;
    std::vector<unsigned int>    planetColor;
    bool ok;

    double sunMass          = planetMassesStr.at(1).toDouble();
    double sunDiameter      = planetDiameterStr.at(1).toDouble();
    unsigned int sunColor   = planetColorStr.at(1).toUInt(&ok, 16);

    for (unsigned int i=0; i<n_planet; i++)
    {
        planetMasses.push_back(planetMassesStr.at(i+2).toDouble());
        planetX.push_back(planetXStr.at(i+2).toDouble());
        planetY.push_back(planetYStr.at(i+2).toDouble());
        planetZ.push_back(planetZStr.at(i+2).toDouble());
        planetDiameter.push_back(planetDiameterStr.at(i+2).toDouble());
        planetColor.push_back(planetColorStr.at(i+2).toUInt(&ok, 16));
    }

    m_solarSystem = new SolarSystem(sunMass, sunDiameter, sunColor,
                                    planetMasses, planetNames, planetDiameter, planetColor,
                                    planetX, planetY, planetZ, REAL_G);

    draw (UPDATE_INTERVAL, DIAMETER_SCALE);
}

void SolarVisWindow::on_actionPause_triggered()
{
    if (m_redrawTimer->isActive())
        m_redrawTimer->stop();
    else
        m_redrawTimer->start();
    ui->actionPause->setChecked(!m_redrawTimer->isActive());
}

void SolarVisWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"),
      "Solar System Simulator 2020 by Bao Ho\nSkeleton code by Jonas Lindemann");
}

void SolarVisWindow::on_actionManual_triggered()
{
    QMessageBox::information(this, tr("Manual"),
                             "There are two modes.\n"
                             "   In 'Basic Mode' you can create a new solar system\n"
                             "by setting #planets, update interval, and sun/planet size.\n"
                             "   In 'Real Mode' you can load a .csv file and let\n"
                             "the simulator build the solar system from your\n"
                             "data. Please look at the sample file 'planet_config.csv'.\n"
                             "You can also speed up the simulation.\n"
                             "   The 'File->Load Config' menu is the same as in the\n"
                             "Real Mode, I just set it up for demonstration.\n");
}

void SolarVisWindow::on_actionBasic_Mode_triggered()
{
    BasicMode basicPopup;
    basicPopup.setModal(true);
    basicPopup.exec();
    startBasicMode (basicPopup.getNumberOfPlanetFromUser(), basicPopup.getUpdateIntervalFromUser(), basicPopup.getDiameterScaleFromUser());
}

void SolarVisWindow::on_actionReal_Mode_triggered()
{
    RealMode realPopup;
    realPopup.setModal(true);
    realPopup.setSlider(speedUpFactor);
    realPopup.exec();
//    qDebug() << realPopup.getFileNameFromUI()
//             << realPopup.getSpeedValueFromUI()
//             << realPopup.getRealModeAction();
    speedUpFactor = realPopup.getSpeedValueFromUI();
    if (realPopup.getRealModeAction() == REAL_MODE_CREATE_NEW)
        createNewSolarSystemFromFile(realPopup.getFileNameFromUI());    
    speedUpRealMode (speedUpFactor);
}

void SolarVisWindow::speedUpRealMode(unsigned int speedFactor){
    double dSpeedFactor, newG;
    dSpeedFactor = speedFactor/1e6;

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
        m_solarSystem->at(i)->setAx(ax*dSpeedFactor*dSpeedFactor);
        m_solarSystem->at(i)->setAz(az*dSpeedFactor*dSpeedFactor);
        m_solarSystem->at(i)->setVx(vx*dSpeedFactor);
        m_solarSystem->at(i)->setVz(vz*dSpeedFactor);
    }
    // Update G to the new G
    newG = REAL_G*dSpeedFactor*dSpeedFactor;
    m_solarSystem->getSun()->setG(newG);
}

void SolarVisWindow::startBasicMode(unsigned int noOfPlanets, unsigned int updateInterval, double diameterScale)
{
    // Create solarsystem simulation
    n_planet = noOfPlanets;
    /* initialize random seed: */
    srand (time(NULL));
    std::vector<double> planetMass, planetDiameter, locX, locY, locZ;
    std::vector<unsigned int> planetColor;
    QStringList planetName;
    for (int i=0; i<n_planet; i++)
    {
        planetMass.push_back(random(EARTH_MASS, JUPITER_MASS));
        locX.push_back(randomAbs(LENGTH_SCALE/10, LENGTH_SCALE));
        locY.push_back(0);
        locZ.push_back(0);
        planetName.append(QString::number(i+1));
        planetDiameter.push_back(random(EARTH_DIAMETER, JUPITER_DIAMETER));
        planetColor.push_back(random(BLACK, WHITE));
    }
    delete m_solarSystem;
    m_solarSystem = new SolarSystem(SUN_MASS, SUN_DIAMETER, RED,
                                    planetMass, planetName, planetDiameter, planetColor,
                                    locX, locY, locZ, REAL_G);

    draw (updateInterval, diameterScale);
}

void SolarVisWindow::draw (unsigned int updateInterval, double diameterScale)
{
    // Flag determining visualisation type
    bool useSpheres = true;

    // Create a VTK renderer

    m_renderer = vtkSmartPointer<vtkRenderer>::New();

    b_pointsArray.clear();

    // Create a set of points for representing our planet bodies
    m_points        = vtkSmartPointer<vtkPoints>::New();
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
                    radiusArray->SetName("radius");

                    //TEST CODE
                    // setup color label
                    vtkSmartPointer<vtkDoubleArray> col = vtkSmartPointer<vtkDoubleArray>::New();
                    col->SetName("color");



                    // setup lookupTable and add some colors
                    vtkSmartPointer<vtkLookupTable> colors = vtkSmartPointer<vtkLookupTable>::New();
                    colors->SetNumberOfTableValues(m_solarSystem->size());
                    Vec3d color;
                    for (int i = 0; i < colors->GetNumberOfTableValues(); i++)
                    {
                        color = convertIntToRGB(m_solarSystem->at(i)->getColor());
                        colors->SetTableValue(i, color.x(), color.y(), color.z(), 1.0);
                        // the last double value is for opacity (1->max, 0->min)
                    }

    double sd, pd;
    sd = m_solarSystem->getSun()->getDiameter();
    for (int i=0;i<m_solarSystem->size();i++)
    {
                    pd = m_solarSystem->at(i)->getDiameter();
                    radiusArray->SetValue(i, sd/(1.5*log10(sd/pd))/2*diameterScale);
                    col->InsertNextValue(i);

    }
                    m_polyData->GetPointData()->AddArray(radiusArray);
                    m_polyData->GetPointData()->SetActiveScalars("radius"); //Set radius
                    m_polyData->GetPointData()->AddArray(col);
    // m_polyData->GetPointData()->SetScalars(radiusArray);

    // This is our glyph that will represent the planet bodies

    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetRadius(1);
    m_sphereSource->SetThetaResolution(PLANET_RESOLUTION);
    m_sphereSource->SetPhiResolution(PLANET_RESOLUTION);
    m_sphereSource->Update();

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
        //m_mapper->SetScalarRange(0.1, 2);
        //m_mapper->ScalarVisibilityOn();

                //TEST CODE
                m_mapper->SetScalarModeToUsePointFieldData(); // without, color are displayed regarding radius and not color label
                m_mapper->SetScalarRange(0, colors->GetNumberOfTableValues()-1); // to scale color label (without, col should be between 0 and 1)
                m_mapper->SelectColorArray("color"); // !!!to set color (nevertheless you will have nothing)
                m_mapper->SetLookupTable(colors);
    }
    else
    {
        m_mapper->SetInputConnection(m_glyphFilter->GetOutputPort());
    }

    // Create sun

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(sd/2*diameterScale);
    sphereSource->SetThetaResolution(30);
    sphereSource->SetPhiResolution(30);
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    sunActor = vtkSmartPointer<vtkActor>::New();
    sunActor->SetMapper(mapper);

    Vec3d sunColor = convertIntToRGB(m_solarSystem->getSun()->getColor());
    sunActor->GetProperty()->SetColor(sunColor.x(), sunColor.y(), sunColor.z());

                    //Translation

                    vtkSmartPointer<vtkTransform> sunLabelTranslation = vtkSmartPointer<vtkTransform>::New();
                    sunLabelTranslation->Translate(-sd/8*diameterScale, -sd/2*diameterScale, -sd/2*diameterScale);

                    // Create label for the sun
                    vtkSmartPointer<vtkPoints> sunPoint = vtkSmartPointer<vtkPoints>::New();
                    sunPoint->InsertNextPoint(0, 0, 0);
                    vtkSmartPointer<vtkPolyData> sunPolyData = vtkSmartPointer<vtkPolyData>::New();
                    sunPolyData->SetPoints(sunPoint);
                    vtkSmartPointer<vtkStringArray> sunPointName = vtkSmartPointer<vtkStringArray>::New();
                    sunPointName->SetNumberOfComponents(1);
                    sunPointName->SetName("Sun name");
                    sunPointName->InsertNextValue("Sun");
                    sunPolyData->GetPointData()->AddArray(sunPointName);

                    // Set mapper and actor
                    vtkSmartPointer<vtkLabeledDataMapper> sunLabelMapper = vtkSmartPointer<vtkLabeledDataMapper>::New();
                    sunLabelMapper->SetInputData(sunPolyData);
                    sunLabelMapper->SetLabelModeToLabelFieldData();
                    sunLabelMapper->SetFieldDataName ("Sun name");
                    sunLabelMapper->SetTransform(sunLabelTranslation);
                    vtkSmartPointer<vtkActor2D> sunLabelActor = vtkSmartPointer<vtkActor2D>::New();
                    sunLabelActor->SetMapper (sunLabelMapper);
                    m_renderer->AddActor(sunLabelActor);



    // Create planet actor

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetPointSize(2);

    // Remove renderer before re-initialize renderer
    if (this->ui->qvtkWidget->GetRenderWindow()->HasRenderer(m_renderer))
        this->ui->qvtkWidget->GetRenderWindow()->RemoveRenderer(m_renderer);



    /*Bao's code - BEGIN - Label the planets*/

    // Add a label to each point
    vtkSmartPointer<vtkStringArray> pointNames =
      vtkSmartPointer<vtkStringArray>::New();
    pointNames->SetNumberOfComponents(1);
    pointNames->SetName("Name of planets");

    for (vtkIdType i = 0; i < m_solarSystem->size(); i++)
        pointNames->InsertNextValue(m_solarSystem->at(i)->getName().toStdString());

    m_polyData->GetPointData()->AddArray(pointNames);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper = vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetInputData(m_polyData);
    labelMapper->SetLabelModeToLabelFieldData();
    labelMapper->SetFieldDataName ("Name of planets");


    vtkSmartPointer<vtkTransform> planetLabelTranslation = vtkSmartPointer<vtkTransform>::New();
    planetLabelTranslation->Translate(-sd/8*diameterScale, -sd/4*diameterScale, -sd/4*diameterScale);
    labelMapper->SetTransform(planetLabelTranslation);
    //labelMapper->SetLabelFormat("%6.2f");
    // Create label actor

    vtkSmartPointer<vtkActor2D> labelActor =
        vtkSmartPointer<vtkActor2D>::New();
    labelActor->SetMapper (labelMapper);

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
        b_pointsArray.push_back(vtkSmartPointer<vtkPoints>::New());
                //b_pointsArray[i] = vtkSmartPointer<vtkPoints>::New();
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
        b_vtkActorArray[i]->GetProperty()->SetColor(1,1,1);
        m_renderer->AddActor(b_vtkActorArray[i]);
    }

    /*Bao's code - END - Add trajectories*/

    /*Bao's code - BEGIN - Add axes*/

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0.0, 0.0, 0);

    vtkSmartPointer<vtkAxesActor> axes =
      vtkSmartPointer<vtkAxesActor>::New();

    // The axes are positioned with a user transform
    axes->SetUserTransform(transform);

    // properties of the axes labels can be set as follows
    // this sets the x axis label to red
    // axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1,0,0);

    // the actual text of the axis label can be changed:
    // axes->SetXAxisLabelText("test");

    //m_renderer->AddActor(axes);

    /*Bao's code - END - Add axes*/

    // Attach actor

    m_renderer->AddActor(m_actor);
    m_renderer->AddActor(sunActor);
    m_renderer->AddActor(labelActor);
    m_renderer->SetBackground(0.0, 0.0, 0.0);

    // Camera

    //vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    vtkSmartPointer<vtkCamera> camera = m_renderer->GetActiveCamera();
    double cameraHeight = 0;
    for (int i = 1; i < m_solarSystem->size(); i++)
    {
        if (cameraHeight < std::abs(m_solarSystem->at(i)->getX()))
        {
            cameraHeight = std::abs(m_solarSystem->at(i)->getX());
        }
    }
    cameraHeight *= 2.5;
    camera->SetPosition(0, -cameraHeight, cameraHeight);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 1);

    camera->SetClippingRange(0.1, cameraHeight*2);

    m_renderer->SetActiveCamera(camera);

    // Connect to Qt widget
    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

    // Create frame update timer (1000/updateInterval Hz)
    m_redrawTimer->setInterval(updateInterval);
    m_redrawTimer->start();
    ui->actionPause->setChecked(!m_redrawTimer->isActive());
}

Vec3d SolarVisWindow::convertIntToRGB (unsigned int colorAsInt)
{
    Vec3d colorAsVec3d;
    unsigned int R = colorAsInt/(256*256);
    unsigned int G = colorAsInt%(256*256)/256;
    unsigned int B = colorAsInt%256;
    colorAsVec3d.setComponents(R/255.0,G/255.0,B/255.0);
    return colorAsVec3d;
}
