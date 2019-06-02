#ifndef SPARSE_OPERATIONS_H
#define SPARSE_OPERATIONS_H

#include <fstream>
#include "cud_defs.h"

#include "structures.h"
#include <set>
#include "map.h"


template<class T>
void initialize_arr(T* arr, int n, T val){
  for (size_t i = 0; i < n; i++) {
    arr[i] = val;
  }
}


__device__ float d_cosine(RBTree<int, float>* rbt1, RBTree<int, float>* rbt2){
  float sum_xy = 0;
  float sum_x2 = 0;
  float sum_y2 = 0;



  bool end1, end2;
  end1 = false; end2 = false;

  Node<int, float>* it1 = rbt1->minimum(rbt1->root);
  Node<int, float>* it1_end = rbt1->maximum(rbt1->root);


  Node<int, float>* it2 = rbt2->minimum(rbt2->root);
  Node<int, float>* it2_end = rbt2->maximum(rbt2->root);
  // int n = 0;
  while (!end1 || !end2) {
    if(!end1 && !end2){
      if(it1->key == it2->key){
        sum_xy += (it1->data * it2->data);
        sum_x2+= (it1->data * it1->data);
        sum_y2+= (it2->data * it2->data);
        if(it1 == it1_end) end1 = true;
        else it1 = rbt1->successor(it1);
        if(it2 == it2_end) end2 = true;
        else it2 = rbt2->successor(it2);
        // n++;
      }else if(it1->key < it2->key){
        sum_x2+= (it1->data * it1->data);
        if(it1 == it1_end) end1 = true;
        else it1 = rbt1->successor(it1);
      }else{
        sum_y2+= (it2->data * it2->data);
        if(it2 == it2_end) end2 = true;
        else it2 = rbt2->successor(it2);
      }
    }
    else{
      if(!end1){
        sum_x2+= (it1->data * it1->data);
        if(it1 == it1_end) end1 = true;
        else it1 = rbt1->successor(it1);
      }else{
        sum_y2+= (it2->data * it2->data);
        if(it2 == it2_end) end2 = true;
        else it2 = rbt2->successor(it2);
      }
    }
  }
  return (sum_x2 == 0 || sum_y2 == 0)? 0 : (sum_xy / (sqrt(sum_x2)*sqrt(sum_y2)));
}

__global__ void one2all_cosine(RBTree<int, float >** dp, float* d_distances, int* d_row_size, int user_id, int max_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_users){
    if(d_row_size[i] != 0){
      d_distances[i] = d_cosine(dp[user_id], dp[i]);
    }
  }
}





#endif
