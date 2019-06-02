#ifndef STRUCTURES_H
#define STRUCTURES_H

// #define n_ratings 27753444
// #define n_users 283228
enum Measures {
MANHATTAN, EUCLIDEAN, PEARSON, COSINE
};


__host__ __device__ float* float_pointer(float* values, int* ind_users, int pos_user){
  return &(values[ind_users[pos_user]]);
}

__host__ __device__ int* int_pointer(int* values, int* ind_users, int pos_user){
  return &(values[ind_users[pos_user]]);
}






#endif
