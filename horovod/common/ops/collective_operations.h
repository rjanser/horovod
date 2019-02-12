// Copyright 2016 The TensorFlow Authors. All Rights Reserved.
// Modifications copyright (C) 2019 Uber Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#ifndef HOROVOD_COLLECTIVE_OPERATIONS_H
#define HOROVOD_COLLECTIVE_OPERATIONS_H

#include <iostream>

#include "../common.h"
#include "../global_state.h"
#include "communication_context.h"

namespace horovod {
namespace common {

class HorovodOp {
public:
  HorovodOp(CommunicationContext* comm_context, HorovodGlobalState* global_state);
  virtual Status Execute(std::vector<TensorTableEntry>& entries, const HorovodResponse& response) = 0;

protected:
  CommunicationContext* comm_context_;
  HorovodGlobalState* global_state_;
};

class AllreduceOp : public HorovodOp {
public:
  AllreduceOp(CommunicationContext* comm_context, HorovodGlobalState* global_state);
  virtual ~AllreduceOp()=default;

  virtual Status Execute(std::vector<TensorTableEntry>& entries, const HorovodResponse& response);

protected:
  virtual void MemcpyInFusionBuffer(void* buffer_data_at_offset, TensorTableEntry& e,
                                    std::vector<TensorTableEntry>& entries);
  virtual void MemcpyOutFusionBuffer(void* buffer_data_at_offset, TensorTableEntry& e,
                                     std::vector<TensorTableEntry>& entries);
  virtual void StreamSynchronize(std::vector<TensorTableEntry>& entries);
};

class AllgatherOp : public HorovodOp {
public:
  AllgatherOp(CommunicationContext* comm_context, HorovodGlobalState* global_state);
  virtual ~AllgatherOp()=default;

  virtual Status Execute(std::vector<TensorTableEntry>& entries, const HorovodResponse& response);

protected:
  virtual void DoAllgather(std::vector<TensorTableEntry>& entries, int* recvcounts, int* displcmnts,
                           int64_t** entry_component_offsets, int64_t** entry_component_sizes,
                           int64_t total_size, int element_size);
};

class BroadcastOp : public HorovodOp {
public:
  BroadcastOp(CommunicationContext* comm_context, HorovodGlobalState* global_state);
  virtual ~BroadcastOp()=default;

  virtual Status Execute(std::vector<TensorTableEntry>& entries, const HorovodResponse& response);
};

class ErrorOp : public HorovodOp {
public:
  ErrorOp(CommunicationContext* comm_context, HorovodGlobalState* global_state);
  virtual ~ErrorOp()=default;

  virtual Status Execute(std::vector<TensorTableEntry>& entries, const HorovodResponse& response);
};

class HierarchicalAllgather : public AllgatherOp {
public:
  HierarchicalAllgather(CommunicationContext* comm_context,
                        HorovodGlobalState* global_state);

  virtual void DoAllgather(std::vector<TensorTableEntry>& entries, int* recvcounts, int* displcmnts,
                           int64_t** entry_component_offsets, int64_t** entry_component_sizes,
                           int64_t total_size, int element_size) override;
};

} // namespace common
} // namespace horovod

#endif //HOROVOD_COLLECTIVE_OPERATIONS_H
