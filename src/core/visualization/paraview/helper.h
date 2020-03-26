// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_VISUALIZATION_PARAVIEW_HELPER_H_
#define CORE_VISUALIZATION_PARAVIEW_HELPER_H_

#include <set>
#include <string>
#include <unordered_map>

#include <vtkCPDataDescription.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>

#include "core/shape.h"

class TClass;

namespace bdm {

static constexpr char const* kSimulationInfoJson = "simulation_info.json";

class SimObject;
struct PopulateDataArraysFunctor;

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `ParaviewAdaptor` to visualize simulation objects.
struct VtkSoGrid {
  VtkSoGrid(const char* type_name, vtkCPDataDescription* data_description);

  ~VtkSoGrid();

  void UpdateMappedDataArrays(uint64_t tid, const std::vector<SimObject*>* sim_objects, uint64_t start, uint64_t end);

  std::string name_;
  TClass* tclass_;
  std::vector<vtkUnstructuredGrid*> data_;
  Shape shape_;

 private:
  TClass* GetTClass();
  void InitializeDataMembers(SimObject* so,
                             std::vector<std::string>* data_members);
};

/// Adds additional data members to the `vtkImageData` required by
/// `ParaviewAdaptor` to visualize diffusion grid.
struct VtkDiffusionGrid {
  VtkDiffusionGrid(const std::string& name,
                   vtkCPDataDescription* data_description);

  ~VtkDiffusionGrid();

  void Init();

  bool used_ = false;
  std::string name_;
  vtkImageData* data_ = nullptr;
  vtkDoubleArray* concentration_ = nullptr;
  vtkDoubleArray* gradient_ = nullptr;
};

/// If the user selects the visualiation option export, we need to pass the
/// information on the C++ side to a python script which generates the
/// ParaView state file. The Json file is generated inside this function
/// \see GenerateParaviewState
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids);

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_HELPER_H_
