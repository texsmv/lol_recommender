#ifndef SCRIPTS_H
#define SCRIPTS_H
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <map>
#include <fstream>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/copy.h>

#include "structures.h"
#include  "distances.h"
#include "map.h"

using namespace std;


vector<string> split(const string& s, char delimiter){
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}


class reloj{
public:
  void start(){c_start = clock();}
  void stop(){c_stop = clock();}
  double time(){return (double)(c_stop - c_start) * 1000.0 / CLOCKS_PER_SEC;}
  clock_t c_start, c_stop;
  double elapsed;
  reloj(){

  }
  ~reloj(){

  }

};

bool fexists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

template<class T>
void write_array(T*& arr, int size, string nombre){
  ofstream out(nombre, ios::out | ios::binary);
  if(!out) {
    cout << "Cannot open file.";
    return;
  }
  cout<<"Writing: "<<sizeof(T) * size<<" bytes"<<endl;
  out.write(&(((char*)arr)[0]), sizeof(T) * size);
  out.close();
}

template<class T>
void read_array(T*& arr, int size, string nombre){
  ifstream in(nombre, ios::in | ios::binary);
  in.read(&(((char*)arr)[0]), sizeof(T) * size);
  in.close();
}

void n_of_users(string path, int& n_ratings, int& n_users, bool header){
  ifstream infile(path);
  string line;
  if(header) getline(infile, line);
  std::vector<string> tokens;

  int id_user, curr_id_user, users_counter, ratings_counter;

  ratings_counter = 0;  users_counter = 0;  id_user = -1;

  while (getline(infile, line)) {
    if(ratings_counter % 1000000 == 0){
      cout<<ratings_counter<<endl;
    }
    tokens = split(line, ',');
    curr_id_user = atoi(tokens[0].c_str());
    if(id_user < curr_id_user){
      users_counter++;
      id_user = curr_id_user;
    }
    ratings_counter++;
  }
  n_ratings = ratings_counter;
  n_users = users_counter;
  cout<<n_ratings<<" "<<n_users<<endl;
}


void read_ML_ratings(string path, int n_ratings, int n_users, int max_users, bool header, float*& values, int*& row_ind, int*& col_ind, int*& ind_users, int*& row_size, string version){
  cout<<"Lectura de users"<<endl;

  string path_b = "binarios/";
  // int max_users = 300000;
  values = new float[n_ratings]; //ratings
  row_ind = new int[n_ratings]; //id_users
  col_ind = new int[n_ratings]; //id_items
  ind_users = new int[max_users]; // indice users
  row_size = new int[max_users]; //indice items



  if(fexists(path_b + "values_" + version) && fexists(path_b + "row_ind_" + version) && fexists(path_b + "col_ind_" + version) && fexists(path_b + "ind_users_" + version) && fexists(path_b + "row_size_" + version)){
    cout<<"Reading values"<<endl;
    read_array<float>(values, n_ratings, path_b + "values_" + version);
    cout<<"Reading row_ind"<<endl;
    read_array<int>(row_ind, n_ratings, path_b + "row_ind_" + version);
    cout<<"Reading col_ind"<<endl;
    read_array<int>(col_ind, n_ratings, path_b + "col_ind_" + version);
    cout<<"Reading ind_users"<<endl;
    read_array<int>(ind_users, max_users, path_b + "ind_users_" + version);
    cout<<"Reading row_size"<<endl;
    read_array<int>(row_size, max_users, path_b + "row_size_" + version);
  }
  else{
    initialize_arr<float>(values, n_ratings, 0);
    initialize_arr<int>(row_ind, n_ratings, 0);
    initialize_arr<int>(col_ind, n_ratings, 0);
    initialize_arr<int>(ind_users, max_users, 0);
    initialize_arr<int>(row_size, max_users, 0);

    ifstream infile(path);
    string line;
    if(header) getline(infile, line);
    std::vector<string> tokens;

    int id_user, curr_id_user, curr_id_item, users_counter, ratings_counter, n_r;
    float curr_rating;
    ratings_counter = 0;  users_counter = 0;  id_user = -1; n_r = 0;

    while (getline(infile, line)) {
      if(ratings_counter % 1000000 == 0)
      cout<<ratings_counter<<endl;
      // cout << id_user << ", " << curr_id_user << endl;
      tokens = split(line, ',');
      curr_id_user = atoi(tokens[0].c_str());
      curr_id_item = atoi(tokens[1].c_str());
      curr_rating = atof(tokens[2].c_str());
      // cout<<curr_id_user<<" "<<curr_id_item<<" "<<curr_rating<<endl;

      if(id_user < curr_id_user){
        if(id_user != -1)
          row_size[id_user] = n_r;

        n_r = 0;
        ind_users[curr_id_user] = ratings_counter;
        id_user = curr_id_user;
        users_counter++;
      }

      values[ratings_counter] = curr_rating;
      row_ind[ratings_counter] = curr_id_user;
      col_ind[ratings_counter] = curr_id_item;
      n_r ++;
      ratings_counter++;
    }
    cout<<ratings_counter<<" - "<<users_counter<<endl;
    // row_size[n_users - 1] = n_r;
    row_size[curr_id_user] = n_r;

    cout<<"Writing values"<<endl;
    write_array<float>(values, n_ratings, path_b + "values_" + version);
    cout<<"Writing row_ind"<<endl;
    write_array<int>(row_ind, n_ratings, path_b + "row_ind_" + version);
    cout<<"Writing col_ind"<<endl;
    write_array<int>(col_ind, n_ratings, path_b + "col_ind_" + version);
    cout<<"Writing ind_users"<<endl;
    write_array<int>(ind_users, max_users, path_b + "ind_users_" + version);
    cout<<"Writing row_size"<<endl;
    write_array<int>(row_size, max_users, path_b + "row_size_" + version);

  }

}


void read_ML_ratings_items(string path,
        int n_ratings,
        int n_users,
        int max_movies,

        bool header,
        float*& item_values,
        int*& item_row_ind,
        int*& item_col_ind,
        int*& ind_items,
        int*& item_row_size,
        string version
        ){

  cout<<"Lectura de items"<<endl;
  string path_b = "binarios/";

  item_values = new float[n_ratings];
  item_row_ind = new int[n_ratings];
  item_col_ind = new int[n_ratings];
  ind_items = new int[max_movies];
  item_row_size = new int[max_movies];

  // pos_movies = new int[n_ids_movies];

  if(fexists(path_b + "item_values_" + version) &&
    fexists(path_b + "item_row_ind_" + version) &&
    fexists(path_b + "item_col_ind_" + version) && fexists(path_b + "ind_item_" + version) && fexists(path_b + "item_row_size_" + version)){

    cout<<"Reading item_values"<<endl;
    read_array<float>(item_values, n_ratings, path_b + "item_values_" + version);
    cout<<"Reading item_row_ind"<<endl;
    read_array<int>(item_row_ind, n_ratings, path_b + "item_row_ind_" + version);
    cout<<"Reading item_col_ind"<<endl;
    read_array<int>(item_col_ind, n_ratings, path_b + "item_col_ind_" + version);
    cout<<"Reading ind_items"<<endl;
    read_array<int>(ind_items, max_movies, path_b + "ind_item_" + version);
    cout<<"Reading item_row_size"<<endl;
    read_array<int>(item_row_size, max_movies, path_b + "item_row_size_" + version);
    // cout<<"Reading pos_movies"<<endl;
    // read_array<int>(pos_movies, n_ids_movies, path_b + "pos_movies_" + version);
  }else{
  cout<<"say hi"<<endl;

    initialize_arr<float>(item_values, n_ratings, 0);
    initialize_arr<int>(item_row_ind, n_ratings, 0);
    initialize_arr<int>(item_col_ind, n_ratings, 0);
    initialize_arr<int>(ind_items, max_movies, 0);
    initialize_arr<int>(item_row_size, max_movies, 0);

    ifstream infile(path);
    string line;
    // int n_movies;
    if(header) getline(infile, line);
    std::vector<string> tokens;


    map<int, map<int, float>* > map_movies_items;
    int cont = 0;
    while (getline(infile, line)) {
      if(cont % 1000000 == 0)
        cout<<cont<<endl;
      tokens = split(line, ',');
      auto it = map_movies_items.find(atoi(tokens[1].c_str()));
      if(it == map_movies_items.end()){
        map<int, float> * mapa = new map<int, float>();
        map_movies_items[atoi(tokens[1].c_str())] = mapa;
      }
      (*(map_movies_items[atoi(tokens[1].c_str())]))[atoi(tokens[0].c_str())] = atof(tokens[2].c_str());
      cont++;
    }

    // n_movies = map_movies_items.size();
    // cout<<"Numero de peliculas: "<<n_movies<<endl;

    int i = 0;

    auto it = map_movies_items.begin();
    while (it != map_movies_items.end()) {
      // pos_movies[it->first] = j;
      auto ite = it->second->begin();
      ind_items[it->first] = i;
      // cout<<it->first<<endl;
      item_row_size[it->first] = it->second->size();
      while (ite != it->second->end()) {
        item_values[i] = ite->second;
        item_row_ind[i] = it->first;
        item_col_ind[i] = ite->first;
        i++;
        ite++;
      }
      it++;
    }

    cout<<"Writing item_values"<<endl;
    write_array<float>(item_values, n_ratings, path_b + "item_values_" + version);
    cout<<"Writing item_row_ind"<<endl;
    write_array<int>(item_row_ind, n_ratings, path_b + "item_row_ind_" + version);
    cout<<"Writing item_col_ind"<<endl;
    write_array<int>(item_col_ind, n_ratings, path_b + "item_col_ind_" + version);
    cout<<"Writing ind_items"<<endl;
    write_array<int>(ind_items, max_movies, path_b + "ind_item_" + version);
    cout<<"Writing item_row_size"<<endl;
    write_array<int>(item_row_size, max_movies, path_b + "item_row_size_" + version);
    // cout<<"Writing pos_movies"<<endl;
    // write_array<int>(pos_movies, n_ids_movies, path_b + "pos_movies_" + version);


  }

}

void average_per_user(float *&values, int *&ind_users, int *&row_size, float*&maxs, float*&mins, float*&averages, int max_users){
  cout<<"hallando mins, maxs y promedios\n";
  averages = new float[max_users];
  maxs = new float[max_users];
  mins = new float[max_users];
  float sum;
  float max_,min_;
  for(size_t i=0; i<max_users; i++){
    max_ = 0; min_ = 5;
    sum = 0;
    if(row_size[i] != 0){
      // cout<<i<<endl;
      // cout<<row_size[i]<<" "<<ind_users[i]<<endl;
      for(size_t j=ind_users[i];j < ind_users[i]+row_size[i];j++){
        sum += values[j];
        if(max_< values[j]) max_= values[j];
        if(min_> values[j]) min_ = values[j];
      }
      averages[i]=sum/row_size[i];
      maxs[i] = max_;
      mins[i] = min_;
    }
  }

}

void read_ML_movies(string path, map<int, string>& movie_names, bool header){
  ifstream infile(path);
  string line;
  if(header) getline(infile, line);
  std::vector<string> tokens;
  int movies_counter = 0;
  int id;
  while (getline(infile, line) ) {
    // if(movies_counter % 10000 == 0)
    // cout<<movies_counter<<"  "<<line<<endl;
    tokens = split(line, ',');
    id = atoi(tokens[0].c_str());
    movie_names[id] = tokens[1];
    // cout<<tokens[0]<<endl;
    movies_counter++;
  }
  cout<<"Numero de peliculas: "<<movies_counter<<endl;
}



__global__ void initialize_maps(RBTree<int, float >** dp, int* d_row_size, int max_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_users){
    if(d_row_size[i] > 0){
      dp[i] = new RBTree<int, float>();
    }
  }
}

__global__ void fill_maps(RBTree<int, float >** dp, float* d_values, int *d_row_ind, int* d_col_ind, int * d_ind_users, int* d_row_size, int max_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_users){
    if(d_row_size[i] != 0){
      float* r = float_pointer(d_values, d_ind_users, i);
      int* c = int_pointer(d_col_ind, d_ind_users, i);
      for (size_t it = 0; it < d_row_size[i]; it++) {
        dp[i]->insert(c[it], r[it]);
        // dp[i]->insert(it, it);
      }
      // dp[i]->maximum(dp[i]->root);

    }
  }
}

__global__ void test_maps(RBTree<int, float >** dp, float* d_test, int* d_row_size, int max_users){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < max_users){
    if(d_row_size[i] > 0){
      d_test[i] = (dp[i]->maximum(dp[i]->root))->key;
      // dp[i]->maximum(dp[i]->root);
      // d_test[i] = 100;
    }
  }

}


__global__ void create_map_and_fill(RBTree<int, float >** dp, int* d_ids_champions, int n){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < 1){
    dp[0] = new RBTree<int, float>();
    for (size_t it = 0; it < n; it++) {
      dp[0]->insert(d_ids_champions[it], 1);
    }
  }
}

__global__ void delete_map(RBTree<int, float >** dp){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < 1){
    delete(dp[0]);
  }
}

void delete_map_cuda(RBTree<int, float>**& d_n_map){

  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(1 / block_size), 1);

  delete_map<<<grid, block>>>(d_n_map);
  CHECK(cudaDeviceSynchronize());
  cudaFree(d_n_map);
}


void create_map_cuda(std::vector<int> ids_champions, RBTree<int, float>**& d_n_map){
  d_n_map = cuda_array<RBTree<int, float>* >(1);

  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(1 / block_size), 1);

  int n = ids_champions.size();
  int* d_ids_champions = cuda_array<int>(n);
  cuda_H2D<int>(ids_champions.data(), d_ids_champions, n);
  create_map_and_fill<<<grid, block>>>(d_n_map, d_ids_champions, n);
  CHECK(cudaDeviceSynchronize());
}



void create_maps_device(thrust::device_vector< RBTree<int, float >* >& d_map_users, float* d_values, int *d_row_ind, int* d_col_ind, int * d_ind_users, int* d_row_size, int max_users){
  float block_size = 256;
  dim3 block =  dim3(block_size, 1, 1);
  dim3 grid =  dim3(ceil(max_users / block_size), 1);
  RBTree<int, float >** dp = thrust::raw_pointer_cast(d_map_users.data());
  // RBTree<int, float >** dp = cuda_array<RBTree<int, float >*>(max_users);

  initialize_maps<<<grid, block>>>(dp, d_row_size, max_users);
  CHECK(cudaDeviceSynchronize());

  fill_maps<<<grid, block>>>(dp, d_values, d_row_ind, d_col_ind, d_ind_users, d_row_size, max_users);
  CHECK(cudaDeviceSynchronize());

  // float* d_tests = cuda_array<float>(max_users);
  // float* tests = new float[max_users];
  //
  // test_maps<<<grid, block>>>(dp, d_tests, d_row_size, max_users);
  // CHECK(cudaDeviceSynchronize());
  //
  //
  // cuda_D2H<float>(d_tests, tests, max_users);
  // CHECK(cudaDeviceSynchronize());
  //
  // for (size_t i = 0; i < max_users; i++) {
  //   if(tests[i] != 0)
  //     cout<<i<<" -> "<<tests[i]<<endl;
  // }


}

#endif
