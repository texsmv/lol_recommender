#ifndef SPARSE_OPERATIONS_H
#define SPARSE_OPERATIONS_H

#include <fstream>
#include "cud_defs.h"

#include "structures.h"
#include <set>


template<class T>
void initialize_arr(T* arr, int n, T val){
  for (size_t i = 0; i < n; i++) {
    arr[i] = val;
  }
}


__device__ void d_desviacion(float* r1, int* col1, int s1, float* r2, int* col2, int s2, int& cardinalidad, float& desviacion){
  float sum = 0;
  cardinalidad = 0;
  int it1, it2;
  it1 = 0; it2 = 0;
  while (it1 < s1 && it2 < s2){
    if(col1[it1] == col2[it2]){
      sum += (r1[it1] - r2[it2]);
      cardinalidad++;
      it1++; it2++;
      // n++;
    }else if(col1[it1] < col2[it2]){
      it1++;
    }else{
      it2++;
    }
  }

  if(cardinalidad == 0){
    desviacion = 0;
  }

  desviacion = sum / cardinalidad;

}
__device__ float d_cosine2(float* r1, int* col1, int s1, float* r2, int* col2, int s2, float* prom){
  float sum_Rui2 = 0;
  float sum_Ruj2 = 0;
  float sum_R = 0;

  int it1 = 0;
  int it2 = 0;
  int n = 0;

  while (it1 < s1 && it2 < s2) {
    if(col1[it1] == col2[it2]){
      sum_R += (r1[it1] - prom[col1[it1]]) * (r2[it2] - prom[col2[it2]]);
      sum_Rui2+= (r1[it1] - prom[col1[it1]]) * (r1[it1] - prom[col1[it1]]);
      sum_Ruj2+= (r2[it2] - prom[col2[it2]]) * (r2[it2] - prom[col2[it2]]);
      it1++; it2++;
      // n++;
    }else if(col1[it1] < col2[it2]){
      it1++;
    }else{
      it2++;
    }
  }
  // if(std::isnan( (sum_xy / (sqrt(sum_x2)*sqrt(sum_y2)) ))){
    // cout<<"n: "<<n<<" "<<" sum_xy: "<<sum_xy<<" sum_x2: "<<sum_x2<<" sum_y2: "<<sum_y2<<endl;
  // }

  return ((sum_Rui2 == 0) || (sum_Ruj2 == 0))? 0 : (sum_R / (sqrt(sum_Rui2)*sqrt(sum_Ruj2)));
}


__global__ void one2all_adjusted_cosine(float* d_averages, float* d_item_values, int* d_item_row_ind, int* d_item_col_ind, int* d_ind_items, int* d_item_row_size, float* d_distances, int id_movie, int max_movies){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_movies){
    if(d_item_row_size[i] != 0){
      float* r1 = float_pointer(d_item_values, d_ind_items, id_movie);
      int* c1 = int_pointer(d_item_col_ind, d_ind_items, id_movie);
      float* r2 = float_pointer(d_item_values, d_ind_items, i);
      int* c2 = int_pointer(d_item_col_ind, d_ind_items, i);
      d_distances[i] = d_cosine2(r1, c1, d_item_row_size[id_movie], r2, c2, d_item_row_size[i], d_averages);
    }
  }
}

__global__ void one2all_desviacion(float* d_item_values, int* d_item_row_ind, int* d_item_col_ind, int* d_ind_items, int* d_item_row_size, float* d_distances, int* d_cardinalidad, int id_movie, int max_movies){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_movies){
    if(d_item_row_size[i] != 0){
      float* r1 = float_pointer(d_item_values, d_ind_items, id_movie);
      int* c1 = int_pointer(d_item_col_ind, d_ind_items, id_movie);
      float* r2 = float_pointer(d_item_values, d_ind_items, i);
      int* c2 = int_pointer(d_item_col_ind, d_ind_items, i);
       d_desviacion(r1, c1, d_item_row_size[id_movie], r2, c2, d_item_row_size[i], d_cardinalidad[i], d_distances[i]);
    }
  }
}


void desviacion_one2all(float*& distances, int*& cardinalidad, int id_movie, int n_ratings, int max_movies,float*& d_item_values, int*& d_item_row_ind, int*& d_item_col_ind, int*& d_ind_items, int*& d_item_row_size){
  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(max_movies / block_size), 1);

  distances = new float[max_movies];
  float* d_distances = cuda_array<float>(max_movies);
  cardinalidad = new int[max_movies];
  int* d_cardinalidad = cuda_array<int>(max_movies);

  one2all_desviacion<<<grid, block>>>(d_item_values, d_item_row_ind, d_item_col_ind, d_ind_items, d_item_row_size, d_distances, d_cardinalidad, id_movie, max_movies);
  CHECK(cudaDeviceSynchronize());
  cuda_D2H(d_distances, distances, max_movies);
  cuda_D2H(d_cardinalidad, cardinalidad, max_movies);
}




void adjusted_cosine_one2all(float*& distances, int id_movie, int n_ratings, int max_movies,float*& d_item_values, int*& d_item_row_ind, int*& d_item_col_ind, int*& d_ind_items, int*& d_item_row_size, float*& d_averages){
  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(max_movies / block_size), 1);

  distances = new float[max_movies];
  float* d_distances = cuda_array<float>(max_movies);
  one2all_adjusted_cosine<<<grid, block>>>(d_averages, d_item_values, d_item_row_ind, d_item_col_ind, d_ind_items, d_item_row_size, d_distances, id_movie, max_movies);
  CHECK(cudaDeviceSynchronize());
  cuda_D2H(d_distances, distances, max_movies);
}









#endif
