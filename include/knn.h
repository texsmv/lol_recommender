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

bool compare_greater(const pair<float, int>&i, const pair<float, int>&j)
{
  if(abs(i.first - j.first) < 0.00001){
    return i.second < j.second;
  }
  return j.first < i.first;
}



vector<pair<int, float> > knn_less_map(float* distances, int* row_size, int max_users, int user_id, int k){
  vector<pair<int, float> > knns(k);

  // set<pair<float, int>, decltype(&compare_greater)> mapa(&compare_greater);
  map<float, pair<float, int>> mapa;
  for (size_t i = 0; i < max_users; i++) {
    if((row_size[i] != 0) && i != user_id){
      mapa.insert(make_pair((distances[i]), make_pair(distances[i], i)));
      // mapa[distances[i]] = dis
      // cout<<"i: "<<i<<" -> "<<distances[i]<<endl;

    }
  }


  auto it = mapa.rbegin();
  int i = 0;
  for (; it != mapa.rend() && (i < k); it++) {
    knns[i] = make_pair(it->second.second, it->second.first);
    i++;
    // cout<<it->second<<" "<<it->first<<endl;
  }
  return knns;
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


vector<pair<int, float> > knn_greater_map(float* distances, int* item_row_size, int max_movies, int movie_id, int k){
  vector<pair<int, float> > knns(k);

  vector< pair<float, int> >v;

  set<pair<float, int>, decltype(&compare_greater)> mapa(&compare_greater);
  // set<pair<float, int>, less<pair<float, int> > > mapa;
  for (size_t i = 0; i < max_movies; i++) {
    if((item_row_size[i] != 0) ){
      if (i == 1)
        cout<<"asddddddddddd "<<distances[i]<<" "<<endl;
      mapa.insert(make_pair(distances[i], i));
      // v.push_back(make_pair(distances[i], i));
      // mapa[distances[i]] = dis
      // cout<<"i: "<<i<<" -> "<<distances[i]<<endl;
    }
  }

  // sort(v.begin(),v.end(), compare_greater);
  cout<<"mapa size: "<<mapa.size()<<endl;
  cout<<"mapa size: "<<v.size()<<endl;

  // float* dists = new float[v.size()];
  // int* ids = new int[v.size()];
  //
  // dists[0] = v[0].first;
  // ids[0] = v[0].second;
  //
  // float t_dist = dists[0];
  // int n = 1;
  // float* pos_dist = &(dists[0]);
  // int* pos_id = &(ids[0]);

  // for (size_t i = 1; i < v.size(); i++) {
  //   if (t_dist != v[i].first && n != 1) {
  //     // cout<<i<<endl;
  //     cuda_order_ascending<int, float>(pos_id, pos_dist, n);
  //     pos_dist = &(dists[i]);
  //     pos_id = &(ids[i]);
  //     t_dist = dists[i];
  //     n = 0;
  //   }
  //   n++;
  //   dists[i] = v[i].first;
  //   ids[i] = v[i].second;
  // }

  // for (size_t i = 0; i < 10; i++) {
  //   cout<<"Hola: "<<ids[i]<<" -> "<<dists[i]<<endl;
  // }


  int i = 0;
  for (auto it = mapa.begin(); it != mapa.end() && (i < k); it++) {
    knns[i] = make_pair(it->second, it->first);
    // knns[i] = make_pair(v[i].second, v[i].first);
    cout<<it->second<<"  ->  "<<it->first<<endl;
    i++;
    // cout<<it->second<<" "<<it->first<<endl;
  }
  return knns;
}


#endif
