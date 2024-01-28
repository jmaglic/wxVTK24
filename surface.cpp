
// Custom library
#include "wxVTKRenderWindowInteractor.h"

// Data
#include "mondrian.xpm"

// wxWidgets
#include <wx/wx.h>
#include <wx/version.h>

// VTK
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkSampleFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>

// Standard library
#include <stdlib.h>
#include <numeric> // std::iota

class MyApp;
class MyFrame;

class MyApp : public wxApp
{
public:
  virtual bool OnInit();
};


class MyFrame : public wxFrame
{
public:
  MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  ~MyFrame();
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  //Declaring Variables
  vtkSmartPointer<vtkImageData> imageData;
  vtkSmartPointer<vtkVolumeProperty> volumeProperty;
  vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity;
  vtkSmartPointer<vtkColorTransferFunction> color;
  vtkSmartPointer<vtkVolume> volume;
  vtkSmartPointer<vtkSmartVolumeMapper> mapper;
  vtkSmartPointer<vtkActor> actor;
  vtkSmartPointer<vtkRenderer> renderer;
  //vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkSmartPointer<vtkRenderWindow> renderWindow;

  //Assigning Values , Allocating Memory
  int X1 = 6;
  int X2 = 6;
  int X3 = 6;
  int X1X2X3 = X1 * X2 * X3;
  std::vector<int> I;

protected:
  void ConstructVTK();
  void ConfigureVTK();
  void DestroyVTK();

private:
  wxVTKRenderWindowInteractor *m_pVTKWindow;
  vtkSmartPointer<vtkRenderer> pRenderer;
  vtkSmartPointer<vtkRenderWindow> pRenderWindow;
  vtkSmartPointer<vtkPolyDataMapper> pConeMapper;
  vtkSmartPointer<vtkActor> pConeActor;
  vtkSmartPointer<vtkConeSource> pConeSource;
private:
  DECLARE_EVENT_TABLE()
};

enum
{
  Minimal_Quit = 1,
  Minimal_About
};

#define MY_FRAME    101
#define MY_VTK_WINDOW 102

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
  EVT_MENU(Minimal_About, MyFrame::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
  MyFrame *frame = new MyFrame(_T("wxWidgets-VTK App"), wxPoint(50, 50), wxSize(450, 450));
  frame->Show(TRUE);
  return TRUE;
}


MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{

  SetIcon(wxICON(mondrian));
  wxMenu *menuFile = new wxMenu(_T(""), wxMENU_TEAROFF);
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(Minimal_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
  menuFile->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Append(menuFile, _T("&File"));
  menuBar->Append(helpMenu, _T("&Help"));
  SetMenuBar(menuBar);
  CreateStatusBar(2);
  wxString mystring;
  mystring << "WxWidgets Version: ";
  mystring << wxMAJOR_VERSION;
  mystring << ".";
  mystring << wxMINOR_VERSION;
  mystring << ".";
  mystring << wxRELEASE_NUMBER;
  mystring << ".";
  mystring << wxSUBRELEASE_NUMBER;
  SetStatusText(mystring,1);
  m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW);
  m_pVTKWindow->UseCaptureMouseOn(); // TODO: Not sure what this does
  ConstructVTK();
  ConfigureVTK();
}

MyFrame::~MyFrame()
{
  if(m_pVTKWindow) m_pVTKWindow->Delete();
  DestroyVTK();
}

void MyFrame::ConstructVTK()
{
  imageData = vtkSmartPointer<vtkImageData>::New();
  imageData->SetDimensions(X1, X2, X3);
  imageData->AllocateScalars(VTK_INT, 1);
  volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
  color = vtkSmartPointer<vtkColorTransferFunction>::New();
  volume = vtkSmartPointer<vtkVolume>::New();
  mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  actor = vtkSmartPointer<vtkActor>::New();
  renderer = vtkSmartPointer<vtkRenderer>::New();


}

void MyFrame::ConfigureVTK()
{
  // Here we get the render window from our custom interactor class
  renderWindow = m_pVTKWindow->GetRenderWindow();
  
  // Adding a renderer 
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(800, 800);
  
  // Adding the cube
  renderer->AddViewProp(volume);
  renderer->SetBackground(0.5, 0.5, 0.5);
  
  // Setting up cube
  volume->SetProperty(volumeProperty); 
  volume->SetMapper(mapper);

  // Setting volume properties
  volumeProperty->SetInterpolationType(0);
  volumeProperty->SetColor(color);
  volumeProperty->SetScalarOpacity(compositeOpacity);
  volumeProperty->ShadeOff();

  // Setting up mapper
  mapper->SetBlendModeToComposite();
  mapper->SetRequestedRenderModeToRayCast();
  mapper->SetInputData(imageData);
  
  // Setting up image data
  imageData->AllocateScalars(VTK_INT, 1); 
  imageData->UpdateCellGhostArrayCache();
  
  //I is supposed to store the 3D data which has to be shown as volume visualization. This 3D data is stored 
  //as a 1D array in which the order of iteration over 3 dimensions is x->y->z, this leads to the following 
  //3D to 1D index conversion farmula index1D =  i + X1*j + X1*X2*k   
  I.resize(X1X2X3); // No need to use int* I = new int[X1X2X3] //Vectors are good
  std::iota(&I[0], &I[0] + X1X2X3, 1); //Creating dummy data as 1,2,3...X1X2X3

  //Setting Voxel Data and Its Properties
  for (int k = 0; k < X3; k++) {
    for (int j = 0; j < X2; j++) {
      for (int i = 0; i < X1; i++) {

        // Here we access the individual voxels inside image data and set their value 
        int* voxel = static_cast<int*>(imageData->GetScalarPointer(i, j, k));

        //copying data from I to imagedata voxel
        *voxel = I[i + X1 * j + X1 * X2 * k];

      }
    }
  }

  //Setting Up Display Properties
  for (int i = 1; i < X1X2X3; i++)
  {
    compositeOpacity->AddPoint(i, 1);
    color->AddRGBPoint(i, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX);
  }

 }

void MyFrame::DestroyVTK()
{
  if (pRenderer != 0)
    pRenderer->Delete();
  if (pConeMapper != 0)
    pConeMapper->Delete();
  if (pConeActor != 0)
    pConeActor->Delete();
  if (pConeSource != 0)
    pConeSource->Delete();
}


void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxString msg;
  msg.Printf( _T("This is the about dialog of wx-vtk sample.\n"));
  wxMessageBox(msg, _T("About wx-vtk"), wxOK | wxICON_INFORMATION, this);
}
