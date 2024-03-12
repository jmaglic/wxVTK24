/*=========================================================================

  Program:   Visualization Toolkit
  Language:  C++

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even 
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE.  See the above copyright notice for more information.

===========================================================================

  This software has been modified in accordance with the VTK license.

  Copyright: (c) 2024 Jasmin B. Maglic

=========================================================================*/ 

#include "wxVTKRenderWindowInteractor.h"
#include <vtkCommand.h>
#include <vtkDebugLeaks.h>
#include <assert.h>

#define WX_USE_X_CAPTURE 1
#define ID_wxVTKRenderWindowInteractor_TIMER 1001

IMPLEMENT_DYNAMIC_CLASS(wxVTKRenderWindowInteractor, wxWindow)
BEGIN_EVENT_TABLE(wxVTKRenderWindowInteractor, wxWindow)
//refresh window by doing a Render
  EVT_PAINT(wxVTKRenderWindowInteractor::OnPaint)
  EVT_ERASE_BACKGROUND(wxVTKRenderWindowInteractor::OnEraseBackground)
  EVT_MOTION(wxVTKRenderWindowInteractor::OnMotion)

//Bind the events to the event converters
  EVT_LEFT_DOWN(wxVTKRenderWindowInteractor::OnButtonDown)
  EVT_MIDDLE_DOWN(wxVTKRenderWindowInteractor::OnButtonDown)
  EVT_RIGHT_DOWN(wxVTKRenderWindowInteractor::OnButtonDown)
  EVT_LEFT_UP(wxVTKRenderWindowInteractor::OnButtonUp)
  EVT_MIDDLE_UP(wxVTKRenderWindowInteractor::OnButtonUp)
  EVT_RIGHT_UP(wxVTKRenderWindowInteractor::OnButtonUp)
  EVT_ENTER_WINDOW(wxVTKRenderWindowInteractor::OnEnter)
  EVT_LEAVE_WINDOW(wxVTKRenderWindowInteractor::OnLeave)
  EVT_MOUSEWHEEL(wxVTKRenderWindowInteractor::OnMouseWheel)
  EVT_KEY_DOWN(wxVTKRenderWindowInteractor::OnKeyDown)
  EVT_KEY_UP(wxVTKRenderWindowInteractor::OnKeyUp)
  EVT_CHAR(wxVTKRenderWindowInteractor::OnChar)
  EVT_TIMER(ID_wxVTKRenderWindowInteractor_TIMER, wxVTKRenderWindowInteractor::OnTimer)
  EVT_SIZE(wxVTKRenderWindowInteractor::OnSize)
END_EVENT_TABLE()

wxVTKRenderWindowInteractor::wxVTKRenderWindowInteractor() : wxWindow(), vtkRenderWindowInteractor()
{
  this->RenderWindow = NULL;
  this->SetRenderWindow(vtkRenderWindow::New());
  this->RenderWindow->Delete();
}

wxVTKRenderWindowInteractor::wxVTKRenderWindowInteractor(wxWindow *parent,
    wxWindowID id,
    const wxPoint &pos,
    const wxSize &size,
    long style,
    const wxString &name) : wxWindow(parent, id, pos, size, style, name), vtkRenderWindowInteractor()

  , timer(this, ID_wxVTKRenderWindowInteractor_TIMER)
  , ActiveButton(wxEVT_NULL)
  , Stereo(0)
  , Handle(0)
  , Created(true)
  , RenderWhenDisabled(1)
  , UseCaptureMouse(0)
{
#ifdef VTK_DEBUG_LEAKS
  vtkDebugLeaks::ConstructClass("wxVTKRenderWindowInteractor");
#endif
  this->RenderWindow = NULL;
  this->SetRenderWindow(vtkRenderWindow::New());
  this->RenderWindow->Delete();

}


wxVTKRenderWindowInteractor::~wxVTKRenderWindowInteractor()
{
  SetRenderWindow(NULL);
  SetInteractorStyle(NULL);
}


wxVTKRenderWindowInteractor * wxVTKRenderWindowInteractor::New()
{

  return new wxVTKRenderWindowInteractor;
}


void wxVTKRenderWindowInteractor::Initialize()
{
  int *size = RenderWindow->GetSize();
  Enable();
  Size[0] = size[0];
  Size[1] = size[1];
  Initialized = 1;
}


void wxVTKRenderWindowInteractor::Enable()
{

  if (Enabled)
    return;
  Enabled = 1;

  Modified();
}


bool wxVTKRenderWindowInteractor::Enable(bool enable)
{
  return wxWindow::Enable(enable);
}


void wxVTKRenderWindowInteractor::Disable()
{
  if (!Enabled)
    return;
  Enabled = 0;
  Modified();
}

void wxVTKRenderWindowInteractor::Start()
{
  // the interactor cannot control the event loop
  vtkErrorMacro( << "wxVTKRenderWindowInteractor::Start() "
           "interactor cannot control event loop.");
}


void wxVTKRenderWindowInteractor::UpdateSize(int x, int y)
{
  if( RenderWindow )
  {
    if ( x != Size[0] || y != Size[1] )
    {
      Size[0] = x;
      Size[1] = y;
      RenderWindow->SetSize(x, y);
      this->Refresh();

    }
  }
}

int wxVTKRenderWindowInteractor::CreateTimer(int WXUNUSED(timertype))
{

  if (!timer.Start(10, TRUE))
    return 0;

  return 1;

}

int wxVTKRenderWindowInteractor::InternalCreateTimer(int timerId, int timerType, unsigned long duration)
{
  if (!timer.Start(duration, timerType == OneShotTimer))
    return 0;

  return ID_wxVTKRenderWindowInteractor_TIMER;
}


int wxVTKRenderWindowInteractor::InternalDestroyTimer(int platformTimerId)
{
  timer.Stop();
  return 1;
}

int wxVTKRenderWindowInteractor::DestroyTimer()
{
  return 1;
}


void wxVTKRenderWindowInteractor::OnTimer(wxTimerEvent& WXUNUSED(event))
{
  if (!Enabled)
    return;

  int timerId = this->GetCurrentTimerId();
  this->InvokeEvent(vtkCommand::TimerEvent, &timerId);

}

long wxVTKRenderWindowInteractor::GetHandleHack()
{

  long handle_tmp = 0;
  handle_tmp = (long long)this->GetHandle();
  return handle_tmp;
}


void wxVTKRenderWindowInteractor::OnPaint(wxPaintEvent& WXUNUSED(event))
{

  wxPaintDC pDC(this);

  if(!Handle)
  {
    Handle = GetHandleHack();
    RenderWindow->SetWindowId(reinterpret_cast<void *>(Handle));
    RenderWindow->SetParentId(reinterpret_cast<void *>(this->GetParent()->GetHandle()));
    this->RenderWindow->SetDisplayId(this->RenderWindow->GetGenericDisplayId());
  }
  Render();
}

void wxVTKRenderWindowInteractor::OnEraseBackground(wxEraseEvent &event)
{
  event.Skip(false);
}

void wxVTKRenderWindowInteractor::OnSize(wxSizeEvent& WXUNUSED(event))
{
  int w, h;
  GetClientSize(&w, &h);
  UpdateSize(w, h);

  if (!Enabled)
  {
    return;
  }


  InvokeEvent(vtkCommand::ConfigureEvent, NULL);

}

void wxVTKRenderWindowInteractor::OnMotion(wxMouseEvent &event)
{
  if (!Enabled) {return;}

  SetEventInformationCentralize(event.GetX()-PrevX, event.GetY()-PrevY,
      event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  InvokeEvent(vtkCommand::MouseMoveEvent, NULL);

}

void wxVTKRenderWindowInteractor::OnEnter(wxMouseEvent &event)
{
  if (!Enabled)
  {
    return;
  }
  SetEventInformationCentralize(event.GetX(), event.GetY(), event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  InvokeEvent(vtkCommand::EnterEvent, NULL);

}

void wxVTKRenderWindowInteractor::OnLeave(wxMouseEvent &event)
{
  if (!Enabled)
  {
    return;
  }
  SetEventInformationCentralize(event.GetX(), event.GetY(), event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  InvokeEvent(vtkCommand::LeaveEvent, NULL);

}

void wxVTKRenderWindowInteractor::OnKeyDown(wxKeyEvent &event)
{
  if (!Enabled)
  {
    return;
  }
  int keycode = event.GetKeyCode();
  char key = '\0';
  if (((unsigned int)keycode) < 256)
  {
    key = (char)keycode;
  }
  wxPoint mousePos = ScreenToClient(wxGetMousePosition());
  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), key, 0, NULL);
  InvokeEvent(vtkCommand::KeyPressEvent, NULL);
  event.Skip();
}

void wxVTKRenderWindowInteractor::OnKeyUp(wxKeyEvent &event)
{
  if (!Enabled)
  {
    return;
  }

  int keycode = event.GetKeyCode();
  char key = '\0';
  if (((unsigned int)keycode) < 256)
  {
    key = (char)keycode;
  }

  wxPoint mousePos = ScreenToClient(wxGetMousePosition());
  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), key, 0, NULL);
  InvokeEvent(vtkCommand::KeyReleaseEvent, NULL);
  event.Skip();
}


void wxVTKRenderWindowInteractor::OnChar(wxKeyEvent &event)
{
  if (!Enabled)
  {
    return;
  }

  int keycode = event.GetKeyCode();
  char key = '\0';

  if (((unsigned int)keycode) < 256)
  {
    key = (char)keycode;
  }

  wxPoint mousePos = ScreenToClient(wxGetMousePosition());
  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), key, 0, NULL);
  InvokeEvent(vtkCommand::CharEvent, NULL);
  event.Skip();
}

void wxVTKRenderWindowInteractor::OnButtonDown(wxMouseEvent &event)
{
  if (!Enabled || (ActiveButton != wxEVT_NULL))
  {
    return;
  }
  ActiveButton = event.GetEventType();
  this->SetFocus();

  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  PrevX = event.GetX();
  PrevY = event.GetY();

  if(event.RightDown())
  {
    InvokeEvent(vtkCommand::RightButtonPressEvent, NULL);
  }
  else if(event.LeftDown())
  {
    InvokeEvent(vtkCommand::LeftButtonPressEvent, NULL);
  }
  else if(event.MiddleDown())
  {
    InvokeEvent(vtkCommand::MiddleButtonPressEvent, NULL);
  }
  if ((ActiveButton != wxEVT_NULL) && WX_USE_X_CAPTURE && UseCaptureMouse)
  {
    CaptureMouse();
  }
}

void wxVTKRenderWindowInteractor::OnButtonUp(wxMouseEvent &event)
{

  if (!Enabled || (ActiveButton != (event.GetEventType()-1)))
  {
    return;
  }

  this->SetFocus();

  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  PrevX = 0;
  PrevY = 0;
  
  if(ActiveButton == wxEVT_RIGHT_DOWN)
  {
    InvokeEvent(vtkCommand::RightButtonReleaseEvent, NULL);
  }
  else if(ActiveButton == wxEVT_LEFT_DOWN)
  {

    InvokeEvent(vtkCommand::LeftButtonReleaseEvent, NULL);
  }
  else if(ActiveButton == wxEVT_MIDDLE_DOWN)
  {
    InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, NULL);
  }

  if ((ActiveButton != wxEVT_NULL) && WX_USE_X_CAPTURE && UseCaptureMouse)
  {
    ReleaseMouse();
  }
  ActiveButton = wxEVT_NULL;
}


void wxVTKRenderWindowInteractor::OnMouseWheel(wxMouseEvent& event)
{

  SetEventInformationCentralize(0, 0, event.ControlDown(), event.ShiftDown(), '\0', 0, NULL);
  if(event.GetWheelRotation() > 0)
  {
    InvokeEvent(vtkCommand::MouseWheelForwardEvent, NULL);
  }
  else
  {
    InvokeEvent(vtkCommand::MouseWheelBackwardEvent, NULL);
  }

}


void wxVTKRenderWindowInteractor::Render()
{
  int renderAllowed = 1;
  if (renderAllowed && !RenderWhenDisabled)
  {
    wxWindow *topParent = wxGetTopLevelParent(this);
    if (topParent)
    {
      renderAllowed = topParent->IsEnabled();
    }
  }

  if (renderAllowed)
  {
    if(Handle && (Handle == GetHandleHack()) )
    {
      RenderWindow->Render();
    }
    else if(GetHandleHack())
    {
      Handle = GetHandleHack();
      RenderWindow->SetNextWindowId(reinterpret_cast<void *>(Handle));
      RenderWindow->WindowRemap();
      RenderWindow->Render();
    }
  }
}

void wxVTKRenderWindowInteractor::SetRenderWhenDisabled(int newValue)
{
  RenderWhenDisabled = (bool)newValue;
}

void wxVTKRenderWindowInteractor::SetStereo(int capable)
{
  if (Stereo != capable)
  {
    Stereo = capable;
    RenderWindow->StereoCapableWindowOn();
    RenderWindow->SetStereoTypeToCrystalEyes();
    Modified();
  }
}

void wxVTKRenderWindowInteractor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
