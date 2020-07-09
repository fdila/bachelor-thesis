//create stream use to decode the message with nanopb
pb_istream_t stream = pb_istream_from_buffer(proto_buffer_rx,
                                            VelocityCommand_size);
//decode message
bool decode_status = pb_decode(&stream, VelocityCommand_fields, 
                                &vel_cmd);

//if message was decoded successfully
if (decode_status) {
  
  //read message
  float linear_velocity = vel_cmd.linear_velocity;
  float angular_velocity = vel_cmd.angular_velocity;
  
  //calculate setpoints
  odom.FromCmdVelToSetpoint(linear_velocity, angular_velocity);
  left_setpoint = odom.GetLeftVelocity();
  right_setpoint = odom.GetRightVelocity();

  //update pid setpoints
  left_pid.Set(left_setpoint);
  right_pid.Set(right_setpoint);
  float cross_setpoint = left_setpoint - right_setpoint;
  cross_pid.Set(cross_setpoint);

}

//configure DMA to reaceive another message
HAL_UART_Receive_DMA(&huart6, (uint8_t*) &proto_buffer_rx,
                                VelocityCommand_size);