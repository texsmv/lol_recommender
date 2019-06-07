#include "scripts.h"
#include "structures.h"
#include "cud_defs.h"
#include "distances.h"
#include "knn.h"
#include "recomender.h"
#include <pthread.h>
#include "map.h"


int map_n_ratings, map_n_users, map_n_movies;
int n_ratings, n_users, n_movies;
int n_ratings_20, n_users_20, n_ratings_27, n_users_27, n_movies_27, n_ratings_l, n_users_l, n_movies_l, n_ratings_l2, n_users_l2, n_movies_l2, n_users_lol_17, n_ratings_lol_17;
int max_users = 8000;
int max_movies = 200;

map<int, map<int, float>* > map_users;
map<int, map<int, float>* > map_items;

float* values;
int *row_ind, * col_ind;
int * ind_users, *row_size;

float* d_values;
int *d_row_ind, * d_col_ind;
int * d_ind_users, * d_row_size;

thrust::device_vector< RBTree<int, float >* > d_map_users(max_users);




float* item_values;
int *item_row_ind, * item_col_ind;
int * ind_items, *item_row_size;

float* d_item_values;
int *d_item_row_ind, * d_item_col_ind;
int * d_ind_items, *d_item_row_size;


// para coseno ajustado
float *maxs, *mins, *averages;
float * d_averages;



vector<int> recomentar_equipo(  vector<int> ids_champions){
  RBTree<int, float>** d_n_map;
  create_map_cuda(ids_champions, d_n_map);

  reloj te;
  te.start();
  vector<int> recomendacion = knns_cosine(d_map_users, d_n_map, d_row_size, max_users, 5);
  te.stop();
  cout<<"Tiempo de prediccion: "<< te.time()<<endl;

  delete_map_cuda(d_n_map);
  return recomendacion;

}



int main(int argc, char const *argv[]) {

  vector<int> as = {1,2,3,4};
  vector<int> bs = {2,1,3,4};
  cout<<(as>bs)<<endl;


  n_ratings_27 = 27753444;
  n_users_27 = 283228;
  // n_movies_27 = 53889;

  n_ratings_20 = 20000263;
  n_users_20 = 138493;

  n_ratings_l = 49;
  n_users_l = 8;

  n_ratings_l2 = 21;
  n_users_l2 = 5;

  n_users_lol_17 = 3117;
  n_ratings_lol_17 = 31170;

  // n_ratings = n_ratings_20;
  // n_users = n_users_20;

  n_ratings = n_ratings_lol_17;
  n_users = n_users_lol_17;
  // n_movies = n_movies_27;

  // n_ratings = n_ratings_l2;
  // n_users = n_users_l2;




  // n_ratings
  n_of_users("dataset/HeroPart2.csv", n_ratings, n_users, false);
  cout<<n_ratings<<" "<<n_users<<endl;


  d_values = cuda_array<float>(n_ratings);
  d_row_ind = cuda_array<int>(n_ratings);
  d_col_ind = cuda_array<int>(n_ratings);
  d_ind_users = cuda_array<int>(max_users);
  d_row_size = cuda_array<int>(max_users);

  d_averages = cuda_array<float>(max_users);

  d_item_values = cuda_array<float>(n_ratings);
  d_item_row_ind = cuda_array<int>(n_ratings);
  d_item_col_ind = cuda_array<int>(n_ratings);
  d_ind_items = cuda_array<int>(max_movies);
  d_item_row_size = cuda_array<int>(max_movies);






  // read_ML_movies("../databases/ml-20m/movies.csv", movies_names, true);
  // read_ML_ratings("../databases/ml-20m/ratings.csv", n_ratings, n_users, true, values, row_ind, col_ind, ind_users, row_size, "27");

  // read_ML_movies("../../collaborative_filtering/databases/ml-latest/movies.csv", movies_names, true);
  // read_ML_ratings("../collaborative_filtering/databases/ml-latest/ratings.csv", n_ratings, n_users, true, values, row_ind, col_ind, ind_users, row_size, "27");

  reloj a;
  a.start();
  // read_ML_ratings("dataset/ratings.csv", n_ratings, n_users, true, values, row_ind, col_ind, ind_users, row_size, "l");
  // read_ML_ratings_items("dataset/ratings.csv", n_ratings, n_users, max_movies, true,  item_values,  item_row_ind,  item_col_ind,  ind_items, item_row_size, "l");
  read_ML_ratings("dataset/HeroPart2.csv", n_ratings, n_users, max_users, false, values, row_ind, col_ind, ind_users, row_size, "_2017");
  read_ML_ratings_items("dataset/HeroPart2.csv", n_ratings, n_users, max_movies, false,  item_values,  item_row_ind,  item_col_ind,  ind_items, item_row_size, "_2017");
  a.stop();
  cout<<"Tiempo de carga de bd: "<<a.time()<<"ms"<<endl;
  // read_ML_ratings("../collaborative_filtering/databases/libro/ratings.csv", n_ratings, n_users, true, values, row_ind, col_ind, ind_users, row_size, "l");
  // read_ML_ratings_items("../collaborative_filtering/databases/libro/ratings.csv", n_ratings, n_users, max_movies, true,  item_values,  item_row_ind,  item_col_ind,  ind_items, item_row_size, "l");




  // average_per_user(values, ind_users, row_size, maxs, mins, averages, max_users);



  cuda_H2D<float>(values, d_values, n_ratings);
  cuda_H2D<int>(row_ind, d_row_ind, n_ratings);
  cuda_H2D<int>(col_ind, d_col_ind, n_ratings);
  cuda_H2D<int>(ind_users, d_ind_users, max_users);
  cuda_H2D<int>(row_size, d_row_size, max_users);

  cuda_H2D<float>(item_values, d_item_values, n_ratings);
  cuda_H2D<int>(item_row_ind, d_item_row_ind, n_ratings);
  cuda_H2D<int>(item_col_ind, d_item_col_ind, n_ratings);
  cuda_H2D<int>(ind_items, d_ind_items, max_movies);
  cuda_H2D<int>(item_row_size, d_item_row_size, max_movies);



  size_t* t = new size_t();
  cudaDeviceGetLimit( t, cudaLimitMallocHeapSize);
  cout<<*t<<endl;
  cudaDeviceSetLimit(cudaLimitMallocHeapSize, (*t)* 100);
  CHECK(cudaDeviceSynchronize());
  cudaDeviceGetLimit( t, cudaLimitMallocHeapSize);
  cout<<*t<<endl;

  create_maps_device(d_map_users, d_values, d_row_ind, d_col_ind, d_ind_users, d_row_size, max_users);









// Esto es todo lo que se necesita para recomendar

  vector<int> ids_champions = {2, 8};


  vector<int> recomendacion = recomentar_equipo(ids_champions);

  for (size_t i = 0; i < recomendacion.size(); i++) {
    cout<<recomendacion[i]<<endl;
  }

// -----------------------------------------------
























  return 0;
}
