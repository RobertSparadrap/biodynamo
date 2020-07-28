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

#ifndef CORE_SIM_OBJECT_SO_POINTER_H_
#define CORE_SIM_OBJECT_SO_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

class SimObject;

/// Simulation object pointer. Required to point to a simulation object with
/// throughout the whole simulation. Raw pointers cannot be used, because
/// a sim object might be copied to a different NUMA domain, or if it resides
/// on a different address space in case of a distributed runtime.
/// Benefit compared to SoHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations.
/// @tparam TSimObject simulation object type
template <typename TSimObject>
class SoPointer {
 public:
  explicit SoPointer(const SoUid& uid) : uid_(uid) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  SoPointer(const SoPointer& other)
    : uid_(other.uid_)
    , cache_so_(other.cache_so_)
    , cache_id_(other.cache_id_)
    {}

  virtual ~SoPointer() {}

  uint64_t GetUid() const { return uid_; }

  /// Equals operator that enables the following statement `so_ptr == nullptr;`
  bool operator==(std::nullptr_t) const { return uid_ == SoUid(); }

  /// Not equal operator that enables the following statement `so_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const SoPointer& other) const { return uid_ == other.uid_; }

  bool operator!=(const SoPointer& other) const { return uid_ != other.uid_; }

  template <typename TSo>
  bool operator==(const TSo* other) const {
    return uid_ == other->GetUid();
  }

  template <typename TSo>
  bool operator!=(const TSo* other) const {
    return !this->operator==(other);
  }
  
  SoPointer& operator=(const SoPointer& other) {
    if (this != &other) {
    std::lock_guard<Spinlock> guard(lock_);
      uid_ = other.uid_;
      // reset to invalid cache state to avoid race conditions in op->
      cache_so_ = nullptr;
      cache_id_ = -1;
    }
    return *this;
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `so_ptr = nullptr;`
  SoPointer& operator=(std::nullptr_t) {
    std::lock_guard<Spinlock> guard(lock_);
    uid_ = SoUid();
    cache_so_ = nullptr;
    cache_id_ = -1;
    return *this;
  }

  TSimObject* operator->() {
    assert(*this != nullptr);
    auto* local_cache_so = cache_so_; 
    auto local_cache_id = cache_id_; 
    
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    if (local_cache_so && ctxt->IsCacheValid(local_cache_id)) {
      return local_cache_so;
    }
    std::lock_guard<Spinlock> guard(lock_);
    auto* so =  Cast<SimObject, TSimObject>(ctxt->GetSimObject(uid_, &cache_id_));
    cache_so_ = so;
    return so;
  }

  const TSimObject* operator->() const {
    assert(*this != nullptr);
    auto* local_cache_so = cache_so_; 
    auto local_cache_id = cache_id_; 
    
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    if (local_cache_so && ctxt->IsCacheValid(local_cache_id)) {
      return const_cast<const TSimObject*>(local_cache_so);
    }
    std::lock_guard<Spinlock> guard(lock_);
    auto* so =  Cast<const SimObject, const TSimObject>(ctxt->GetConstSimObject(uid_, &cache_id_));
    cache_so_ = const_cast<TSimObject*>(so); 
    return so;
  }

  friend std::ostream& operator<<(std::ostream& str, const SoPointer& so_ptr) {
    str << "{ uid: " << so_ptr.uid_ << "}";
    return str;
  }

  TSimObject& operator*() { return *(this->operator->()); }

  const TSimObject& operator*() const { return *(this->operator->()); }

  operator bool() const { return *this != nullptr; }

  TSimObject* Get() { return this->operator->(); }

  const TSimObject* Get() const { return this->operator->(); }

 private:
  SoUid uid_;
  mutable TSimObject* cache_so_ = nullptr;
  mutable int64_t cache_id_ = -1;
  mutable Spinlock lock_;

  template <typename TFrom, typename TTo>
  typename std::enable_if<std::is_base_of<TFrom, TTo>::value, TTo*>::type Cast(
      TFrom* so) const {
    return static_cast<TTo*>(so);
  }

  template <typename TFrom, typename TTo>
  typename std::enable_if<!std::is_base_of<TFrom, TTo>::value, TTo*>::type Cast(
      TFrom* so) const {
    return dynamic_cast<TTo*>(so);
  }

  BDM_CLASS_DEF(SoPointer, 2);
};

template <typename T>
struct is_so_ptr {
  static constexpr bool value = false;  // NOLINT
};

template <typename T>
struct is_so_ptr<SoPointer<T>> {
  static constexpr bool value = true;  // NOLINT
};

namespace detail {

struct ExtractUid {
  template <typename T>
  static typename std::enable_if<is_so_ptr<T>::value, uint64_t>::type GetUid(
      const T& t) {
    return t.GetUid();
  }

  template <typename T>
  static typename std::enable_if<!is_so_ptr<T>::value, uint64_t>::type GetUid(
      const T& t) {
    return 0;  // std::numeric_limits<uint64_t>::max();
  }
};

}  // namespace detail

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_POINTER_H_
