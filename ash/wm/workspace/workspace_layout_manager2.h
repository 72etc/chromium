// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_WORKSPACE_WORKSPACE_LAYOUT_MANAGER2_H_
#define ASH_WM_WORKSPACE_WORKSPACE_LAYOUT_MANAGER2_H_

#include <set>

#include "ash/shell_observer.h"
#include "ash/wm/base_layout_manager.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/aura/layout_manager.h"
#include "ui/aura/root_window_observer.h"
#include "ui/base/ui_base_types.h"
#include "ui/aura/window_observer.h"
#include "ui/gfx/rect.h"

namespace aura {
class RootWindow;
class Window;
}

namespace ui {
class Layer;
}

namespace ash {
namespace internal {

class Workspace2;
class WorkspaceManager2;

// LayoutManager used on the window created for a Workspace.
// TODO(sky): this code shares a fair amount of similarities with
// BaseLayoutManager, yet its different enough that subclassing is painful.
// See if I can refactor the code to make it easier to share common bits.
class ASH_EXPORT WorkspaceLayoutManager2
    : public aura::LayoutManager,
      public aura::RootWindowObserver,
      public ash::ShellObserver,
      public aura::WindowObserver {
 public:
 public:
  explicit WorkspaceLayoutManager2(Workspace2* workspace);
  virtual ~WorkspaceLayoutManager2();

  // Overridden from BaseWorkspaceLayoutManager:
  virtual void OnWindowResized() OVERRIDE {}
  virtual void OnWindowAddedToLayout(aura::Window* child) OVERRIDE;
  virtual void OnWillRemoveWindowFromLayout(aura::Window* child) OVERRIDE;
  virtual void OnWindowRemovedFromLayout(aura::Window* child) OVERRIDE;
  virtual void OnChildWindowVisibilityChanged(aura::Window* child,
                                              bool visibile) OVERRIDE;
  virtual void SetChildBounds(aura::Window* child,
                              const gfx::Rect& requested_bounds) OVERRIDE;

  // RootWindowObserver overrides:
  virtual void OnRootWindowResized(const aura::RootWindow* root,
                                   const gfx::Size& old_size) OVERRIDE;

  // ash::ShellObserver overrides:
  virtual void OnDisplayWorkAreaInsetsChanged() OVERRIDE;

  // Overriden from WindowObserver:
  virtual void OnWindowPropertyChanged(aura::Window* window,
                                       const void* key,
                                       intptr_t old) OVERRIDE;
  virtual void OnWindowDestroying(aura::Window* window) OVERRIDE;

 private:
  typedef std::set<aura::Window*> WindowSet;

  // Invoked when the show state of a window changes. |cloned_layer| is a clone
  // of the windows layer tree. |cloned_layer| is only non-NULL if a cross fade
  // should happen between the states. This method takes ownership of
  // |cloned_layer|.
  void ShowStateChanged(aura::Window* window,
                        ui::WindowShowState last_show_state,
                        ui::Layer* cloned_layer);

  // Adjusts the sizes of the windows during a screen change.
  void AdjustWindowSizesForScreenChange();

  // Adjusts the sizes of the specific window in respond to a screen change.
  void AdjustWindowSizeForScreenChange(aura::Window* window);

  // Updates the bounds of the window from a show state change.
  void UpdateBoundsFromShowState(aura::Window* window);

  // If |window| is maximized or fullscreen the bounds of the window are set and
  // true is returned. Does nothing otherwise.
  bool SetMaximizedOrFullscreenBounds(aura::Window* window);

  WorkspaceManager2* workspace_manager();

  aura::RootWindow* root_window_;

  Workspace2* workspace_;

  // Set of windows we're listening to.
  WindowSet windows_;

  // The work area. Cached to avoid unnecessarily moving windows during a
  // workspace switch.
  gfx::Rect work_area_;

  DISALLOW_COPY_AND_ASSIGN(WorkspaceLayoutManager2);
};

}  // namespace internal
}  // namespace ash

#endif  // ASH_WM_WORKSPACE_WORKSPACE_LAYOUT_MANAGER2_H_
