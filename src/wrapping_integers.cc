#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  

  
  return Wrap32 { zero_point + static_cast<uint32_t>(n)};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
 
  
  uint64_t cur_value = static_cast<uint64_t>(this->raw_value_ - zero_point.raw_value_);
  
  
  if(checkpoint < cur_value) return cur_value;
  
  uint64_t checkpoint1 = checkpoint;
  checkpoint1 = checkpoint1 - cur_value;
  uint32_t low_32bit = checkpoint1;
  uint32_t high_32bit = checkpoint1 >> 32;
 
  if(low_32bit > static_cast<uint32_t>(1 << 31)) 
  cur_value += (static_cast<uint64_t>(high_32bit + 1) << 32);
  else cur_value += static_cast<uint64_t>(high_32bit) << 32;
  

  
  return cur_value;
}
