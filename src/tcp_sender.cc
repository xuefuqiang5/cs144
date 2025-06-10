#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

void TCPSender::push(const TransmitFunction& transmit)
{
    TCPSenderMessage msg;
    
    switch (current_state) {
    case TCPState::CLOSE:
        // 主动打开连接：发送SYN
        msg.SYN = true;
        msg.FIN = false;
        msg.seqno = this->isn_;
        msg.payload = {};
        
        // 保存到重传队列
        outstanding_segments.push_back(msg); 
        abs_number += msg.sequence_length(); // SYN占用1序列号
        current_state = TCPState::SYN_SENT;
        transmit(msg);
        /*启动计时器*/
        current_state = TCPState::ESTABLISHED;
        break;
        
    case TCPState::ESTABLISHED:
        // 正常数据传输
        uint64_t window_size = window_right - outstanding_right;
        while ((window_right - outstanding_right) > 0 && !input_.reader().is_finished()) {
            msg.SYN = false;
            
            // 计算可发送数据量
            size_t payload_size = std::min({
                window_size,
                TCPConfig::MAX_PAYLOAD_SIZE,
                input_.reader().bytes_buffered()
            });
            
            // 读取数据
            read(input_.reader(), payload_size, msg.payload);
            
            // 检查是否需要设置FIN
            if (input_.reader().is_finished() && 
                window_size > payload_size) { // FIN需要额外1序列号空间
                msg.FIN = true;
            } else {
                msg.FIN = false;
            }
            
            msg.seqno = Wrap32::wrap(abs_number, isn_);
            abs_number += msg.sequence_length(); 
            // 更新状态
            window_size -= msg.sequence_length();
            
            // 保存到重传队列并发送
            outstanding_segments.push_back(msg); 
            transmit(msg);
            /*启动计时器*/
            
            // 如果设置了FIN，进入关闭流程
            if (msg.FIN) {
                current_state = TCPState::FIN_WAIT_1;
                break;
            }
        }
        break;
        
    case TCPState::FIN_WAIT_1:
        // 已发送FIN，等待ACK
        // 重传逻辑由定时器处理
        break;
        
    case TCPState::FIN_WAIT_2:
        // 等待对端FIN
        break;
        
    case TCPState::TIME_WAIT:
        // 等待足够时间确保ACK到达
        break;
        
    default:
        break;
    }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg; 
  
  return msg;
}

void TCPSender::receive(const TCPReceiverMessage& msg)
{
    // 更新接收方窗口信息
    if (msg.ackno.has_value()) {
        // 解包确认号
        Wrap32 ackno_wrap = msg.ackno.value();
        uint64_t ackno_abs = ackno_wrap.unwrap(isn_, abs_number);
        
        // 更新窗口边界
        window_left = ackno_abs;
        window_right = window_left + msg.window_size;
        
        // 移除已确认的段
        for (auto it = outstanding_segments.rbegin(); it != outstanding_segments.rend(); ++it){
          auto seg = *it;
          if (ackno_abs == seg.seq_abs + seg.msg.sequence_length()) {
            while (it != outstanding_segments.rend()) {
              outstanding_segments.erase(--it.base());
            }
          }
        } 
        // 重置重传定时器（指数退避重置）
    } else {
        // 没有确认号，只更新窗口大小
        window_right = window_left + msg.window_size;
    }
    
    // 状态机转换逻辑
    switch (current_state) {
    case TCPState::CLOSE:
        // 被动打开：收到SYN进入SYN_RECEIVED
        current_state = TCPState::SYN_RCVD;
        break;
        
    case TCPState::
    case TCPState::ESTABLISHED:
        // 收到FIN进入CLOSE_WAIT
        if (msg.FIN) {
            current_state = TCPState::CLOSE_WAIT;
        }
        break;
        
    case TCPState::FIN_WAIT_1:
        // 情况1: 收到FIN的ACK进入FIN_WAIT_2
        if (msg.ackno && ackno_abs == next_seqno_) {
            current_state = TCPState::FIN_WAIT_2;
        }
        // 情况2: 同时收到FIN进入CLOSING
        else if (msg.FIN) {
            current_state = TCPState::CLOSING;
        }
        break;
        
    case TCPState::FIN_WAIT_2:
        // 收到FIN进入TIME_WAIT
        if (msg.FIN) {
            current_state = TCPState::TIME_WAIT;
            start_time_wait_timer();
        }
        break;
        
    case TCPState::CLOSING:
        // 收到FIN的ACK进入TIME_WAIT
        if (msg.ackno && ackno_abs == next_seqno_) {
            current_state = TCPState::TIME_WAIT;
            start_time_wait_timer();
        }
        break;
        
    case TCPState::LAST_ACK:
        // 收到FIN的ACK进入CLOSED
        if (msg.ackno && ackno_abs == next_seqno_) {
            current_state = TCPState::CLOSED;
        }
        break;
        
    case TCPState::TIME_WAIT:
        // 定时器超时后进入CLOSED（在定时器处理）
        break;
        
    case TCPState::CLOSE_WAIT:
        // 上层关闭连接，发送FIN进入LAST_ACK
        if (input_.reader().is_finished()) {
            TCPSenderMessage fin_msg;
            fin_msg.FIN = true;
            fin_msg.seqno = Wrap32::wrap(next_seqno_, isn_);
            
            outstanding_segments.push_back({
                next_seqno_,
                fin_msg.sequence_length(),
                fin_msg
            });
            
            next_seqno_ += fin_msg.sequence_length();
            current_state = TCPState::LAST_ACK;
            transmit(fin_msg);
            start_retransmission_timer();
        }
        break;
        
    default:
        break;
    }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  (void)ms_since_last_tick;
  (void)transmit;
  (void)initial_RTO_ms_;
}
