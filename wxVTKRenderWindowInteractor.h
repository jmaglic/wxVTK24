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

#pragma once
#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/dcclient.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkVersionMacros.h>

// wx forward declarations
class wxPaintEvent;
class wxMouseEvent;
class wxTimerEvent;
class wxKeyEvent;
class wxSizeEvent;

class wxVTKRenderWindowInteractor : public wxWindow, public vtkRenderWindowInteractor{
  DECLARE_DYNAMIC_CLASS(wxVTKRenderWindowInteractor)
  public:
  wxVTKRenderWindowInteractor();

  wxVTKRenderWindowInteractor(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxWANTS_CHARS | wxNO_FULL_REPAINT_ON_RESIZE,
    const wxString& name = wxPanelNameStr);

  static wxVTKRenderWindowInteractor* New();
  void PrintSelf(ostream& os, vtkIndent indent);
  ~wxVTKRenderWindowInteractor();

  void Initialize();
  void Enable();
  bool Enable(bool enable);
  void Disable();
  void Start();
  void UpdateSize(int x, int y);
  int CreateTimer(int timertype);
  int DestroyTimer();
  void TerminateApp() {};

  void OnPaint(wxPaintEvent &event);
  void OnEraseBackground (wxEraseEvent& event);
  void OnMotion(wxMouseEvent &event);

  void OnButtonDown(wxMouseEvent &event);
  void OnButtonUp(wxMouseEvent &event);

  void OnEnter(wxMouseEvent &event);
  void OnLeave(wxMouseEvent &event);
  void OnMouseWheel(wxMouseEvent& event);
  void OnKeyDown(wxKeyEvent &event);
  void OnKeyUp(wxKeyEvent &event);
  void OnChar(wxKeyEvent &event);

  void OnTimer(wxTimerEvent &event);
  void OnSize(wxSizeEvent &event);

  void Render();
  void SetRenderWhenDisabled(int newValue);
  vtkGetMacro(Stereo,int);
  vtkBooleanMacro(Stereo,int);
  virtual void SetStereo(int capable);
  vtkSetMacro(UseCaptureMouse,int);
  vtkBooleanMacro(UseCaptureMouse,int);

  protected:
  wxTimer timer;
  int ActiveButton;
  long GetHandleHack();
  int Stereo;
  virtual int InternalCreateTimer(int timerId, int timerType, unsigned long duration);
  virtual int InternalDestroyTimer(int platformTimerId);

  private:
  long Handle;
  bool Created;
  int RenderWhenDisabled;
  int UseCaptureMouse;

  DECLARE_EVENT_TABLE()
};

