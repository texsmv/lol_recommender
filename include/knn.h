#ifndef KNN_H
#define KNN_H

#include <set>
#include "distances.h"
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/copy.h>
#include <thrust/reduce.h>
#include <algorithm>
#include <map>


using namespace std;

template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

template<typename A, typename B>
std::multimap<B,A> flip_map(const std::map<A,B> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}



template<class A, class B>
void cuda_order_ascending(A* keys, B* values, int tam){
  thrust::host_vector<A> t_keys(tam);
  thrust::host_vector<B> t_values(tam);
  thrust::device_vector<A> d_t_keys;
  thrust::device_vector<B> d_t_values;

  for (size_t i = 0; i < tam; i++) {
    t_values[i] = values[i];
    t_keys[i] = keys[i];
  }
  d_t_values = t_values;
  d_t_keys = t_keys;

  thrust::sort_by_key(d_t_keys.begin(), d_t_keys.begin() + tam, d_t_values.begin());
  thrust::copy(d_t_values.begin(), d_t_values.end(), t_values.begin());
  thrust::copy(d_t_keys.begin(), d_t_keys.end(), t_keys.begin());

  for (size_t i = 0; i < tam; i++) {
    values[i] = t_values[i];
    keys[i] = t_keys[i];
  }
}

template<class A, class B>
void cuda_order_descending(A* keys, B* values, int tam){
  thrust::host_vector<A> t_keys(tam);
  thrust::host_vector<B> t_values(tam);
  thrust::device_vector<A> d_t_keys;
  thrust::device_vector<B> d_t_values;

  for (size_t i = 0; i < tam; i++) {
    t_values[i] = values[i];
    t_keys[i] = keys[i];
  }
  d_t_values = t_values;
  d_t_keys = t_keys;

  thrust::sort_by_key(d_t_keys.begin(), d_t_keys.begin() + tam, d_t_values.begin());
  thrust::copy(d_t_values.begin(), d_t_values.end(), t_values.begin());
  thrust::copy(d_t_keys.begin(), d_t_keys.end(), t_keys.begin());

  for (size_t i = 0; i < tam; i++) {
    values[i] = t_values[tam - i - 1];
    keys[i] = t_keys[tam - i - 1];
  }
}


__global__ void get_map_counts(RBTree<int, float >** dp, int* d_counts_maps, int* ids_users, int n_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < n_users){
    d_counts_maps[i] = dp[ids_users[i]]->size;
  }
}

__global__ void copy_map_keys(RBTree<int, float >** dp, int* d_counts_maps, int* d_indices, int* ids_users, int* d_keys, int n_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < n_users){
    Node<int, float>* it1 = dp[ids_users[i]]->minimum(dp[ids_users[i]]->root);
    Node<int, float>* it2 = dp[ids_users[i]]->maximum(dp[ids_users[i]]->root);
    for (size_t it = d_indices[i]; it < (d_indices[i] + d_counts_maps[i]) ; it++) {
      d_keys[it] = it1->key;
      if(it1 != it2)
        it1 = dp[ids_users[i]]->successor(it1);
    }
  }
}



vector<int> knns_cosine(thrust::device_vector< RBTree<int, float >* >& d_map_users, RBTree<int, float>** d_n_map, int* d_row_size, int max_users, int k){
  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(max_users / block_size), 1);
  RBTree<int, float >** dp = thrust::raw_pointer_cast(d_map_users.data());

  float* d_distances = cuda_array<float>(max_users);
  float* distances = new float[max_users];

  reloj t_dist;
  t_dist.start();
  // one2all_cosine<<<grid, block>>>(dp, d_distances, d_row_size, user_id, max_users);
  one2all_cosine_gpointer<<<grid, block>>>(dp, d_n_map, d_distances, d_row_size, max_users);
  CHECK(cudaDeviceSynchronize());
  t_dist.stop();
  cout<<"Tiempo de distancias uno a todos: "<<t_dist.time()<<"ms"<<endl;

  cuda_D2H(d_distances, distances, max_users);

  int* ids = new int[max_users];
  for (size_t i = 0; i < max_users; i++) {
    ids[i] = i;
  }

  cuda_order_descending<float, int>(distances, ids, max_users);
  for (size_t i = 0; i < 20; i++) {
    cout<<" dist: "<<distances[i]<< " id: "<<ids[i];
  }

  int* counts_maps = new int[k];
  int* d_counts_maps = cuda_array<int>(k);
  int* d_ids = cuda_array<int>(k);

  cuda_H2D<int>(ids, d_ids, k);

  dim3 grid2 =  dim3(ceil(k / block_size), 1);

  get_map_counts<<<grid2, block>>>(dp, d_counts_maps, d_ids, k);
  CHECK(cudaDeviceSynchronize());

  cuda_D2H<int>(d_counts_maps, counts_maps, k);
  CHECK(cudaDeviceSynchronize());

  int* indices = new int[k];
  int* d_indices = cuda_array<int>(k);
  indices[0] = 0;
  for (size_t i = 1; i < k; i++) {
    indices[i] = indices[i - 1] + counts_maps[i - 1];
    // cout<<"ind: "<<indices[i]<<" "<<counts_maps[i]<<endl;
  }
  cuda_H2D<int>(indices, d_indices, k);

  int total = thrust::reduce(counts_maps, counts_maps + k);
  int* keys = new int[total];
  int* d_keys = cuda_array<int>(total);

  copy_map_keys<<<grid2, block>>>(dp, d_counts_maps, d_indices, d_ids, d_keys, k);
  CHECK(cudaDeviceSynchronize());

  cuda_D2H<int>(d_keys, keys, total);
  CHECK(cudaDeviceSynchronize());






  map <vector<int>, int > mset;
  // vector<int> t = {1,2,4,5,6,7,8};
  // mset[t] = 1;
  for (size_t i = 0; i < k; i++) {
    vector<int> temp;
    temp.assign(&(keys[indices[i]]),&(keys[indices[i]]) + counts_maps[i]);
    auto it = mset.find(temp);
    if(it == mset.end())
      mset.insert(make_pair(temp, 1));
    else{
      it->second = it->second + 1;
    }
  }

  // int i = 0;
  // for (auto it = mset.begin() ; it != mset.end(); ++it) {
  //   cout<<i<<" -> "<<it->second<<endl;
  //   i++;
  // }
  // cout<<endl;

  std::multimap<int, vector<int>> dst = flip_map(mset);

  auto it = dst.rbegin();
  cout<<"Coincidencias: "<<it->first<<endl;

  vector<int> recomendacion(5);

  for (size_t i = 0; i < it->second.size(); i++) {
    recomendacion[i] = it->second[i];
  }
  return recomendacion;
}

#endif
