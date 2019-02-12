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

#ifndef HOROVOD_MPI_OPERATIONS_H
#define HOROVOD_MPI_OPERATIONS_H

#include <iostream>
#include <queue>

#include "mpi.h"

#include "../common.h"
#include "../global_state.h"
#include "communication_context.h"

namespace horovod {
namespace common {

// Table for storing Tensor metadata on rank zero. This is used for error
// checking, stall checking and size calculations, as well as determining
// when a reduction is ready to be done (when all nodes are ready to do it).
using MessageTable = std::unordered_map<
    std::string,
    std::tuple<std::vector<MPIRequest>, std::chrono::steady_clock::time_point>>;

class MPIContext : public CommunicationContext {
public:
  void Allreduce(const void* buffer_data, int64_t num_elements,
                 TensorTableEntry& first_entry, const void* sendbuff,
                 Communicator comm) override;

  void Allgatherv(const void *sendbuf, int sendcount, DataType sendtype,
                  void *recvbuf, const int recvcounts[],
                  const int displs[], DataType recvtype,
                  Communicator comm) override;

  void Broadcast(const void* buffer_data, int64_t num_elements,
                 DataType dtype, int root_rank,
                 Communicator comm) override;

  void Barrier(Communicator comm) override;

  void AllocateSharedBuffer(int64_t window_size, int element_size, void* baseptr, Communicator comm) override;

  void FreeSharedBuffer() override;

  void QuerySharedBuffer(int rank, void* baseptr) override;

  void GetTypeSize(DataType dtype, int* out) override;

  inline std::string AllreduceActivity() const override {
    return MPI_ALLREDUCE;
  }

  inline std::string BroadcastActivity() const override {
    return MPI_BCAST;
  }

  MPI_Datatype GetMPIDataType(std::shared_ptr<Tensor> tensor);
  MPI_Datatype GetMPIDataType(DataType dtype);
  MPI_Comm GetMPICommunicator(Communicator comm);

  // Queue of MPI requests waiting to be sent to the coordinator node.
  std::queue<MPIRequest> message_queue;

  // MPI custom data type for float16.
  MPI_Datatype mpi_float16_t;
  MPI_Op mpi_float16_sum;

  // Private MPI communicator for Horovod to ensure no collisions with other
  // threads using MPI.
  MPI_Comm mpi_comm;

  // Node-local communicator.
  MPI_Comm local_comm;

  // Cross-node communicator for hierarchical allreduce.
  MPI_Comm cross_comm;

  // MPI Window used for shared memory allgather
  MPI_Win window;

  // Only exists on the coordinator node (rank zero). Maintains a count of
  // how many nodes are ready to allreduce every tensor (keyed by tensor
  // name) and time point when tensor started allreduce op.
  std::unique_ptr<MessageTable> message_table;
};

} // namespace common
} // namespace horovod

#endif //HOROVOD_MPI_OPERATIONS_H
