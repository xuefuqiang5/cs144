#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here. 
  if(message.SYN) {
    trigger = 1;
    this->zero_point = message.seqno;
  }


  this->checkpoint += message.sequence_length();
  uint64_t abs = message.seqno.unwrap(zero_point, this->checkpoint);
  
  if(abs == 0 && message.SYN == false) return ;

  this->reassembler_.insert((abs == 0) ? 0:abs - 1, message.payload, message.FIN);
       
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage message ;
  uint64_t cap = this->writer().available_capacity();
  message.window_size = (cap > UINT16_MAX) ? UINT16_MAX : cap;
  
  if(!trigger) return message;
  uint64_t abs = this->writer().bytes_pushed() + 1ULL;
  if(this->writer().is_closed()) abs += 1ULL;
  Wrap32 ackno = Wrap32::wrap(abs, zero_point);

  message.ackno = ackno;
  

  return message;
}
