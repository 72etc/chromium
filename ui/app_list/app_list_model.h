// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_APP_LIST_APP_LIST_MODEL_H_
#define UI_APP_LIST_APP_LIST_MODEL_H_
#pragma once

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/app_list/app_list_export.h"
#include "ui/base/models/list_model.h"

namespace app_list {

class AppListItemModel;
class SearchBoxModel;
class SearchResult;

// Master model of app list that consists of three sub models: Apps,
// SearchBoxModel and SearchResults. The Apps sub model owns a list of
// AppListItemModel and is displayed in the grid view. SearchBoxModel is
// the model for SearchBoxView. SearchResults owns a list of SearchResult.
class APP_LIST_EXPORT AppListModel {
 public:
  typedef ui::ListModel<AppListItemModel> Apps;
  typedef ui::ListModel<SearchResult> SearchResults;

  AppListModel();
  ~AppListModel();

  Apps* apps() { return apps_.get(); }
  SearchBoxModel* search_box() { return search_box_.get(); }
  SearchResults* results() { return results_.get(); }

 private:
  scoped_ptr<Apps> apps_;

  scoped_ptr<SearchBoxModel> search_box_;
  scoped_ptr<SearchResults> results_;

  DISALLOW_COPY_AND_ASSIGN(AppListModel);
};

}  // namespace app_list

#endif  // UI_APP_LIST_APP_LIST_MODEL_H_
