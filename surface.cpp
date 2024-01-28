
// Custom library
#include "wxVTKRenderWindowInteractor.h"

// wxWidgets
#include <wx/wx.h>
#include <wx/version.h>

// VTK
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkNamedColors.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>
#include <vtkVoxelModeller.h>
#include <vtkMarchingCubes.h>

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
  vtkSmartPointer<vtkNamedColors> colors;
  vtkSmartPointer<vtkImageData> volume;
  vtkSmartPointer<vtkSphereSource> sphereSource;
  vtkSmartPointer<vtkVoxelModeller> voxelModeller;
  vtkSmartPointer<vtkMarchingCubes> surface;
  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkRenderWindow> renderWindow;
  vtkSmartPointer<vtkPolyDataMapper> mapper;
  vtkSmartPointer<vtkActor> actor;

protected:
  void ConstructVTK();
  void ConfigureVTK();
  void DestroyVTK();

private:
  wxVTKRenderWindowInteractor* m_pVTKWindow;

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
  colors = vtkSmartPointer<vtkNamedColors>::New(); 
  volume = vtkSmartPointer<vtkImageData>::New();
  sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  voxelModeller = vtkSmartPointer<vtkVoxelModeller>::New();
  surface = vtkSmartPointer<vtkMarchingCubes>::New();
  renderer = vtkSmartPointer<vtkRenderer>::New();
  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  actor = vtkSmartPointer<vtkActor>::New();

}

void MyFrame::ConfigureVTK()
{
  double isoValue = 0.5;

  // Here we get the renderer window from wxVTK
  renderWindow = m_pVTKWindow->GetRenderWindow();
  renderWindow->AddRenderer(renderer);

  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
  renderer->AddActor(actor);

  actor->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
  actor->SetMapper(mapper);

  // Define surface
  sphereSource->SetPhiResolution(20);
  sphereSource->SetThetaResolution(20);
  sphereSource->Update();

  double bounds[6];
  sphereSource->GetOutput()->GetBounds(bounds);
  for (unsigned int i = 0; i < 6; i += 2)
  {
    double range = bounds[i + 1] - bounds[i];
    bounds[i] = bounds[i] - 0.1 * range;
    bounds[i + 1] = bounds[i + 1] + 0.1 * range;
  }

  voxelModeller->SetInputConnection(sphereSource->GetOutputPort());
  voxelModeller->SetSampleDimensions(50, 50, 50);
  voxelModeller->SetModelBounds(bounds);
  voxelModeller->SetScalarTypeToFloat();
  voxelModeller->SetMaximumDistance(0.1);
  voxelModeller->Update();

  surface->SetInputData(volume);
  surface->ComputeNormalsOn();
  surface->SetValue(0, isoValue);

  volume->DeepCopy(voxelModeller->GetOutput());

  // The mapper requires our vtkMarchingCubes object
  mapper->SetInputConnection(surface->GetOutputPort());
  mapper->ScalarVisibilityOff();

}

void MyFrame::DestroyVTK(){}


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
