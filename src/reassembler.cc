#include "reassembler.hh"
#include <iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  //cout << "the insert function is working " << data <<"  will be inserted "<< "the last tag is  " << is_last_substring<< endl ;
  int64_t t = first_index + data.size();
  if(is_last_substring) last_limited = max(last_limited, t);
  //cout << "the last_limited is :" << last_limited<< endl;
  for(uint64_t i = 0, index = first_index; i < data.size(); i++, index++){
    if(index >= flag + output_.writer().available_capacity()) break;
    if (char_map.find(index) != char_map.end()) 
    {
      char_map[index].is_the_last = is_last_substring;
      continue;
    }
    char_map[index] = Ichar(data[i], is_last_substring);
  }
  

  for(int i = flag, cap = output_.writer().available_capacity(); cap > 0; cap--, i++)
      {
        if(char_map.find(i) == char_map.end()) break;
        output_.writer().push(string(1, char_map[i].ch));
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
  return char_map.size() - flag;
}
