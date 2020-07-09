
//create stream used to encode the message with nanopb
pb_ostream_t stream = pb_ostream_from_buffer(proto_buffer_tx, 
                                            sizeof(proto_buffer_tx));

//read encoders
float left_wheel = left_encoder.GetLinearVelocity();
float right_wheel = right_encoder.GetLinearVelocity();

//calculate odometry values and put data in the message
odom.FromWheelVelToOdom(left_wheel, right_wheel);

status_msg.linear_velocity = odom.GetLinearVelocity();
status_msg.angular_velocity = odom.GetAngularVelocity();

//read time and put data in the message
float current_tx_millis = HAL_GetTick();
status_msg.delta_millis = current_tx_millis - previous_tx_millis;
previous_tx_millis = current_tx_millis;

//set status inside the message
status_msg.status = 3;

//encode message
pb_encode(&stream, StatusMessage_fields, &status_msg);

//start DMA transmission
HAL_UART_Transmit_DMA(&huart6, (uint8_t*) &proto_buffer_tx, 
                      StatusMessage_size);
