#include "reassembler.hh"
#include <iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  //cout << "the insert function is working " << data <<"  will be inserted "<< "the last tag is  " << is_last_substring<< endl ;
  int64_t t = first_index + data.size();
  if(is_last_substring) last_limited = max(last_limited, t);

  const int64_t capacity = output_.writer().available_capacity();
  //cout << "the last_limited is :" << last_limited<< endl;

  //cout << "the capacity is : "<<capacity<< "the flag is :"<< flag<< endl; 
  for(uint64_t i = 0, index = first_index; i < data.size(); i++, index++){
    if(index >= static_cast<uint64_t>(flag + capacity)) {break;}

    if(index >= char_map.size()) {char_map.resize(index + 1, '\0'); visited.resize(index + 1, 0);}
    if(visited[index] != 0) continue;
    char_map[index] = data[i];
    visited[index] = 1;
    //cout <<char_map[index] << "  has been insert into char_map"<< endl;
    count++;
    
    
  }
  

  for(int i = flag, cap = capacity; cap > 0; cap--, i++)
      {
        if(i >= static_cast<int>(char_map.size()) or visited[i] == 0) break;
        output_.writer().push(string(1, char_map[i]));
        //cout << char_map[i]<< "  has been put into stream"<< endl;
        flag++;
      }
  
  if(flag >= last_limited and last_limited != -1) output_.writer().close();
  
  
  /*for(int i = flag, j = this->bytes_pending(); j > 0; j--, i++){
    if(char_map.find(i) == char_map.end()) continue;
    cout<< "assember's Ele is :" <<i<<" "<< char_map[i].ch<< endl;
  }
  cout <<"the flag's value is :" << flag<< endl;*/
  
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return count - flag;
}
