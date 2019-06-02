#ifndef KNN_H
#define KNN_H

#include <set>
#include "distances.h"
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/copy.h>
#include <algorithm>

using namespace std;


void knns_cosine(thrust::device_vector< RBTree<int, float >* >& d_map_users, int* d_row_size, int user_id, int max_users){
  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(max_users / block_size), 1);
  RBTree<int, float >** dp = thrust::raw_pointer_cast(d_map_users.data());

  float* d_distances = cuda_array<float>(max_users);
  float* distances = new float[max_users];

  one2all_cosine<<<grid, block>>>(dp, d_distances, d_row_size, user_id, max_users);
  CHECK(cudaDeviceSynchronize());
  cuda_D2H(d_distances, distances, max_users);

  for (size_t i = 0; i < 10; i++) {
    cout<<i<<" -> "<<distances[i]<<endl;
  }

}

#endif
