#pragma once

#include <unordered_map>
#include <stddef.h>

#include "../utility/assign_utils.h"
#include "../conduit/Duct.h"

#include "../distributed/mpi_utils.h"
#include "mesh_utils.h"

namespace uit {

template<typename T, size_t N=DEFAULT_BUFFER>
class Mesh {

  using node_id_t = size_t;
  using pipe_id_t = size_t;

  mesh_t<T, N> mesh;

  // pipe_id -> node_id
  std::unordered_map<pipe_id_t, node_id_t> input_registry;
  std::unordered_map<pipe_id_t, node_id_t> output_registry;

  const std::function<thread_id_t(node_id_t)> thread_assignment;
  const std::function<proc_id_t(node_id_t)> proc_assignment;

  void InitializeRegistries() {
    for (node_id_t node_id = 0; node_id < mesh.size(); ++node_id) {
      auto & node = mesh[node_id];
      auto & [inputs, outputs] = node;

      for (auto & link : inputs) {
        emp_assert(input_registry.count(link) == 0);
        input_registry[link.GetPipeID()] = node_id;
      }

      for (auto & link : outputs) {
        emp_assert(output_registry.count(link) == 0);
        output_registry[link.GetPipeID()] = node_id;
      }

    }
  }

  void InitializeInterThreadPipes() {

    for (node_id_t node_id = 0; node_id < mesh.size(); ++node_id) {
      auto & node = mesh[node_id];
      auto & [inputs, outputs] = node;
      const thread_id_t my_thread = thread_assignment(node_id);

      for (auto & link : inputs) {
        const node_id_t their_node = output_registry[link];
        const thread_id_t their_thread = thread_assignment(their_node);

        if (my_thread != their_thread) {
          link.GetInput().template EmplaceDuct<
            ThreadDuct<T, N>
          >();
        }
      }

    }

  }

  void InitializeInterProcPipes() {

    for (node_id_t node_id = 0; node_id < mesh.size(); ++node_id) {
      auto & node = mesh[node_id];
      auto & [inputs, outputs] = node;
      // TODO rename
      const proc_id_t my_proc = proc_assignment(node_id);

      for (auto & link : inputs) {
        const node_id_t their_node = output_registry[link];
        const proc_id_t their_proc = proc_assignment(their_node);

        if (my_proc != their_proc) {
          link.GetInput().template SplitDuct<
            ProcOutletDuct<T, N>
          >(
            my_proc,
            their_proc,
            link.GetPipeID()
          );
        }
      }

      for (auto & link : outputs) {
        const node_id_t their_node = input_registry[link];
        const proc_id_t their_proc = proc_assignment(their_node);

        if (my_proc != their_proc) {
          link.GetOutput().template SplitDuct<
            ProcInletDuct<T, N>
          >(
            my_proc,
            their_proc,
            link.GetPipeID()
          );
        }
      }

    }

  }

public:

  using value_type = io_bundle_t<T, N>;

  Mesh(
    const mesh_t<T, N> & mesh_,
    const std::function<thread_id_t(node_id_t)> thread_assignment_,
    const std::function<proc_id_t(node_id_t)> proc_assignment_
      =uit::AssignIntegrated<proc_id_t>{}
  )
  : mesh(mesh_)
  , thread_assignment(thread_assignment_)
  , proc_assignment(proc_assignment_) {
    InitializeRegistries();
    InitializeInterThreadPipes();
    InitializeInterProcPipes();
  }

  size_t GetSize() const { return mesh.size(); }

  value_type & GetNode(const node_id_t node_id) { return mesh[node_id]; }

  value_type & operator[](const node_id_t node_id) { return GetNode(node_id); }

  typename mesh_t<T, N>::iterator begin() { return std::begin(mesh); }

  typename mesh_t<T, N>::iterator end() { return std::end(mesh); }

  typename mesh_t<T, N>::const_iterator begin() const {
    return std::begin(mesh);
  }

  typename mesh_t<T, N>::const_iterator end() const {
    return std::end(mesh);
  }

  mesh_t<T, N> GetSubmesh(
    const thread_id_t tid,
    const proc_id_t pid=get_proc_id()
  ) {
    mesh_t<T, N> res;
    for (node_id_t node_id = 0; node_id < mesh.size(); ++node_id) {
      if (
        thread_assignment(node_id) == tid
        && proc_assignment(node_id) == pid
      ) res.push_back(
        mesh[node_id]
      );
    }
    return res;
  }

};

}
