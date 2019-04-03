#include "btree.h"
#include <map>

void clear_cache() {
  // Remove cache
  int size = 256*1024*1024;
  char *garbage = new char[size];
  for(int i=0;i<size;++i)
    garbage[i] = i;
  for(int i=100;i<size;++i)
    garbage[i] += garbage[i-100];
  delete[] garbage;
}

// MAIN
int main(int argc, char** argv)
{
  // Parsing arguments
  int num_data = 1, delete_key_start = 1, num_delete_keys = 1;
  int n_threads = 1;
  int num_threads = 1;
  float selection_ratio = 0.0f;
  char *input_path = (char *)std::string("../sample.txt").data();
  bool perform_delete = false;

  int c;
  while((c = getopt(argc, argv, "n:w:t:s:i:d:m:")) != -1) {
    switch(c) {
      case 'n':
        num_data = atoi(optarg);
        break;
      case 'w':
        write_latency_in_ns = atol(optarg);
        break;
      case 'd':
	perform_delete = true;
	delete_key_start = atoi(optarg);
	break;
      case 'm':
	num_delete_keys = atoi(optarg);
	break;
      case 't':
        n_threads = atoi(optarg);
        break;
      case 's':
        selection_ratio = atof(optarg);
      case 'i':
        input_path = optarg;
      default:
        break;
    }
  }

  btree *bt;
  bt = new btree();

  struct timespec start, end;

  // Reading data
  entry_key_t* keys = new entry_key_t[num_data];

  ifstream ifs;
  ifs.open(input_path);
  cout << input_path << endl;
  if(!ifs) {
    cout << "input loading error!" << endl;

    delete[] keys;
    exit(-1);
  }

  // hashmap of inserted keys to facilitate deletes and search after crash
  map<int, char*> keysInserted;
  map<int, char*>::iterator it;

  for(int i=0; i<num_data; ++i)
    ifs >> keys[i]; 

  ifs.close();

    clock_gettime(CLOCK_MONOTONIC,&start);
    
    bool success;
    for(int i = 0; i < num_data; i++) {
      success = bt->btree_insert(keys[i], (char *)keys[i]); 
      if (success){
	keysInserted[keys[i]] = (char *)keys[i];
      }
    }

    clock_gettime(CLOCK_MONOTONIC,&end);

    long long elapsed_time = 
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    elapsed_time /= 1000;

    //printf("INSERT elapsed_time: %ld, Avg: %f\n", elapsed_time,
    //    (double)elapsed_time / num_data);

    int inserted = keysInserted.size();
    int deleted = 0;


   if (perform_delete) {
     // delele <num_delete_keys> keys starting at <delete_key_start>
     // <delete_key_start is the start index of the key in the input keylist
     for (int i = delete_key_start; i < delete_key_start + num_delete_keys; i++) {
        it = keysInserted.find(keys[i]);
        if ( it != keysInserted.end()){
		success = bt->btree_delete(keys[i]);
		if (success){
			deleted ++;
			keysInserted.erase(it);
		}
	}
      }
    }

  // delelet keys 15-28
  /*int start_key = 15;
  entry_key_t* keys_to_del = new entry_key_t[100];
  for (int j=0; j<14; j++){
	keys_to_del[j] = start_key;
        //cout << "Delete key " << keys_to_del[j] << endl;
        bt->btree_delete(keys_to_del[j]);
	start_key++;
  }*/


  clear_cache();

  {
    clock_gettime(CLOCK_MONOTONIC,&start);

    /*for(int i = 0; i < num_data; ++i) {
      bt->btree_search(keys[i]);
    }*/

    //char *val;
    for (it = keysInserted.begin(); it != keysInserted.end(); it++){
	bt->btree_search(it->first);
	//if (val != it->second)
	//	cout << "Value MISMATCH for " << it->first << endl;
    }

    clock_gettime(CLOCK_MONOTONIC,&end);

    long long elapsed_time = 
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    elapsed_time /= 1000;

    //printf("SEARCH elapsed_time: %ld, Avg: %f\n", elapsed_time,
    //    (double)elapsed_time / num_data);
    bt->printAll();
   
    cout << "Total keys inserted = " << inserted << endl;
    cout << "Total keys deleted = " << deleted << endl;
    cout << "Total keys searched = " << keysInserted.size() << endl;
   cout << "-----------------------------------------------" << endl;
  }

  delete bt;
  delete[] keys;

  return 0;
}


