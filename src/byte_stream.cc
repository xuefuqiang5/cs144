#include "byte_stream.hh"
#include <string_view>
#include <deque>
#include <iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) :byte_pip(), capacity_( capacity ) {}
bool ByteStream::is_fulled() const{
  if(this->current_size == capacity_) return true;
  else return false;
}
bool ByteStream::is_empty() const{
  if(this->current_size == 0) return true;
  else return false;
 }
bool Writer::is_closed() const
{
  
  // Your code here.
  return this->pip_shut_down;
}

void Writer::push( string data )
{
  // Your code here.
  if(is_closed()) return;
  
  if(this->available_capacity() < data.size()) {
    size_t count = available_capacity();
    for(size_t i = 0; i < count; i++){
      byte_pip.push_back(data[i]);
      this->written_count++;
      this->current_size++;
    }
    return;
  }
  
  for (char ch : data){
    this->byte_pip.push_back(ch);
    this->written_count++;
    this->current_size++;
  }

  
  //cout << sub_data << endl;
  //for(char ch :byte_pip) cout << ch<<endl;
  
  

  return;  
  
}

void Writer::close()
{
  // Your code here.
  this->pip_shut_down = true;

}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  uint64_t rest_capacity = this->capacity_ - current_size; 

  return rest_capacity;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return this->written_count;
}

bool Reader::is_finished() const
{
  // Your code here.
  return this->pip_shut_down && (this->is_empty());
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return this->poped_count;
}

string_view Reader::peek() const
{
  // Your code here.
  //cout << "this fun run !"<<"  "<< "the value is "<<string_view(&byte_pip.front(), 1)<< endl;
  if(is_empty()) return string_view{};
  return string_view(&byte_pip.front(), 1);
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  
  if(len > current_size) 
  {
    this->byte_pip.clear(); 
    poped_count += current_size; 
    current_size = 0;
    return;
  }
  for(size_t i = 0; i < len; i++){
    this->byte_pip.pop_front();
    poped_count++;
    current_size--;
  }



  //cout<<"i'm run!"<< endl;
  
  
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.

  return this->current_size;
}
