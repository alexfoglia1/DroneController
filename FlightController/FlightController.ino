
#include <Servo.h>
#include <Wire.h>
#include <NeoSWSerial.h>

#include "PID.h"
#include "MAINT.h"
#include "Utils.h"
#include "NanoUnoIF.h"

#define MAJOR_VERSION '1'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'B'


#define MAX_MOTOR_SIGNAL 1500
#define MIN_MOTOR_SIGNAL 1000

#define MAX_RADIO_SIGNAL 2000
#define MIN_RADIO_SIGNAL 1000

#define MOTOR1_PIN 4
#define MOTOR2_PIN 7
#define MOTOR3_PIN 6
#define MOTOR4_PIN 5

#define CHANNEL_1_PIN 12
#define CHANNEL_2_PIN 11
#define CHANNEL_3_PIN 10
#define CHANNEL_4_PIN  9
#define CHANNEL_5_PIN  8

#define SW_UART_RX 2
#define SW_UART_TX 3

#define RED_LED_PIN  13

#define MOTORS_ARM_THRESHOLD 100 // motors start spinning at MIN_SIGNAL + x
#define CHANNEL(N)(N-1)
#define MOTOR(N)(N-1)
#define THROTTLE_CHANNEL   CHANNEL(3)
#define ROLL_CHANNEL       CHANNEL(1)
#define PITCH_CHANNEL      CHANNEL(2)
#define MOTORS_ARM_CHANNEL CHANNEL(5)

#define READ_COMMAND_THRESHOLD 0
#define DISPLAY_THRESHOLD 50


typedef enum
{
  WAIT_SYNC,
  WAIT_DATA  
} nano_proto_state_t;


Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;
NeoSWSerial uart_nano(SW_UART_RX, SW_UART_TX);

UNO2NANO_Message message_out;
NANO2UNO_Message message_in;
uint8_t rx_buffer[256];
int n_bytes_in;

nano_proto_state_t rx_status;
int count_to_command;
int count_to_disp;
float channels[5];
float channels_dead_center_zones[5][2];
bool motors_armed;

void update_fsm(uint8_t byte_in)
{
  switch (rx_status)
  {
    case WAIT_SYNC:
    {
      if (byte_in == 0xFF)
      {
        rx_buffer[0] = 0xFF;
        n_bytes_in = 1;
        rx_status = WAIT_DATA;
      }
      break;
    }

    case WAIT_DATA:
    {
      rx_buffer[n_bytes_in] = byte_in;
      n_bytes_in += 1;


      if (n_bytes_in == sizeof(NANO2UNO_Message))
      {
        //Serial.print("RX Buffer: [\t");
        //for (int i = 0; i < sizeof(NANO2UNO_Message); i++)
        //{
           //Serial.print(rx_buffer[i]);
           //Serial.print("\t");
        //}
        //Serial.println("]");

        NANO2UNO_Message* received = (NANO2UNO_Message*)(&rx_buffer[0]);
        uint8_t rx_cks   = received->checksum;
        uint8_t calc_cks = NANO2UNO_Cks((uint8_t*) received);
        //Serial.print("RX CHECKSUM: ");
        //Serial.print(rx_cks);
        //Serial.print(" CALC CHECKSUM: ");
        //Serial.println(calc_cks);
        if (rx_cks == calc_cks)
        { 
          memcpy(&message_in, received, sizeof(NANO2UNO_Message));
        }
        rx_status = WAIT_SYNC;
        n_bytes_in = 0;
        for (int i = 0; i < 256; i++)
        {
          rx_buffer[i] = 0;
        }

      }
      break;
    }
  }


}

void setup(void)
{
  // HW Initialisation
  pinMode(RED_LED_PIN,  OUTPUT);

  pinMode(CHANNEL_1_PIN, INPUT);
  pinMode(CHANNEL_2_PIN, INPUT);
  pinMode(CHANNEL_3_PIN, INPUT);
  pinMode(CHANNEL_4_PIN, INPUT);
  pinMode(CHANNEL_5_PIN, INPUT);
  
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);

  motor1.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor2.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor3.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor4.writeMicroseconds(MIN_MOTOR_SIGNAL);

  digitalWrite(RED_LED_PIN, LOW);
  Serial.begin(115200);
  // SW Initialisation
  uart_nano.begin(38400);
  message_out.sync = 0xFF;
  message_out.motors_armed = 0x00;

  message_in.sync = 0x00;
  message_in.lsm9ds1_found = 0;
  message_in.roll = 0;
  message_in.pitch = 0;
  message_in.yaw = 0;
  message_in.loop_time = 0;
  for (int i = 0; i < 256; i++)
  {
    rx_buffer[i] = 0;
  }
  n_bytes_in = 0;
  rx_status = WAIT_SYNC;
  
  motors_armed = false;
  
  MAINT_Init(MAJOR_VERSION, MINOR_VERSION, STAGE_VERSION);

  count_to_disp = 0;
  count_to_command = 0;
  
  for (int i = CHANNEL(1); i <= CHANNEL(5); i++)
  {
    channels[i] = 0.0f;
    channels_dead_center_zones[i][0] = 0.0f;
    channels_dead_center_zones[i][1] = 0.0f;
  }

  channels_dead_center_zones[ROLL_CHANNEL][0] = 0.35f;
  channels_dead_center_zones[ROLL_CHANNEL][1] = 0.45f;
  channels_dead_center_zones[PITCH_CHANNEL][0] = 0.35f;
  channels_dead_center_zones[PITCH_CHANNEL][1] = 0.45f;

  channels[MOTORS_ARM_CHANNEL] = 0.0f;
  channels[THROTTLE_CHANNEL] = 0.0f;
  channels[PITCH_CHANNEL] = 0.4f;
  channels[ROLL_CHANNEL] = 0.4f;
}

void loop(void)
{ 
  uint16_t motors_speed[4] = {MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL};
// -------------------------------------------- UPDATE MESSAGE IN --------------------------------------------------
  while (uart_nano.available())
  {
    update_fsm(uart_nano.read());
  }

// -----------------------------------------------------------------------------------------------------------------

  if (message_in.sync != 0xFF)
  {
    digitalWrite(RED_LED_PIN, HIGH);
  }
  else if (message_in.lsm9ds1_found)
  {
    digitalWrite(RED_LED_PIN, LOW);
// -------------------------------------------- READ ATTITUDE FROM NANO --------------------------------------------
   float attitude[3] = {message_in.roll, message_in.pitch, message_in.yaw};
   MAINT_UpdateAHRS(attitude);
// -----------------------------------------------------------------------------------------------------------------

// -------------------------------------------- READ COMMAND FROM RADIO --------------------------------------------
    if (count_to_command == READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(1)] = normalizedPulseIn(CHANNEL_1_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
    }

    if (count_to_command == 1 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(2)] = normalizedPulseIn(CHANNEL_2_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
    }

    if (count_to_command == 2 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(3)] = normalizedPulseIn(CHANNEL_3_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL); 
    }

    if (count_to_command == 3 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(5)] = normalizedPulseIn(CHANNEL_5_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
      count_to_command = 0;
    }
    else
    {
      count_to_command += 1;
    }

    motors_armed = (channels[MOTORS_ARM_CHANNEL] > 0.5f);
    float roll_sp  = (channels[ROLL_CHANNEL]  >= channels_dead_center_zones[ROLL_CHANNEL][0]  && channels[ROLL_CHANNEL]  <= channels_dead_center_zones[ROLL_CHANNEL][1])  ? 0.0f : toRange(channels[ROLL_CHANNEL],  0.0f, 0.85f, -2.0f, 2.0f);
    float pitch_sp = (channels[PITCH_CHANNEL] >= channels_dead_center_zones[PITCH_CHANNEL][0] && channels[PITCH_CHANNEL] <= channels_dead_center_zones[PITCH_CHANNEL][1]) ? 0.0f : toRange(0.85f - channels[PITCH_CHANNEL], 0.0f, 0.85f, -2.0f, 2.0f);

    float attitude_sp[3] = {roll_sp, pitch_sp, 0.0f};
    MAINT_UpdateCMD(channels[THROTTLE_CHANNEL], attitude_sp);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- PID UPDATE --------------------------------------------------------- 
    PID_Update(attitude_sp, attitude, message_in.loop_time * 1e-6);
    //float* PID = PID_Get();
    float PID[3] = {0, 0, 0};
    MAINT_UpdatePID(PID);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- APPLY SPEED CORRECTION --------------------------------------------- 
     //[M1]     [M2]    (X)^
     //    \   /           |
     //     \ /            | 
     //      X            (Z)- - ->(Y)
     //     / \
     //    /   \
     //[M4]     [M3]
      
     // motors_speed[0] = M1 speed = TOP LEFT
     // motors_speed[1] = M2 speed = TOP RIGHT
     // motors_speed[2] = M3 speed = BOTTOM RIGHT
     // motors_speed[3] = M4 speed = BOTTOM LEFT

     // If I want to roll LEFT  I shall give more thrust to RIGHT motors M2,M3
     // If I want to roll RIGHT I shall give more thrust to LEFT  motors M1,M4

     // If I want to PITCH DOWN I shall give more thrust to BACK motors  M4,M3
     // If I want to PITCH UP   I shall give more thrust to FRONT motors M1,M2
     motors_speed[MOTOR(1)] = minMax(
                                       toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) + PID[ROLL] + PID[PITCH],
                                       MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                       MAX_MOTOR_SIGNAL);
     motors_speed[MOTOR(2)] = minMax(
                                       toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) - PID[ROLL] + PID[PITCH],
                                       MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                       MAX_MOTOR_SIGNAL);
     motors_speed[MOTOR(3)] = minMax(
                                      toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) - PID[ROLL] - PID[PITCH],
                                      MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                      MAX_MOTOR_SIGNAL);
     motors_speed[MOTOR(4)] = minMax(
                                      toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) + PID[ROLL] - PID[PITCH],
                                      MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                      MAX_MOTOR_SIGNAL);
       
     MAINT_UpdateMOTORS(motors_armed, motors_speed);

  }
  else
  {
    digitalWrite(RED_LED_PIN, HIGH);
  }

// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- UPDATE MESSAGE OUT ------------------------------------------------- 
  //message_out.motors_armed = motors_armed ? 0x01 : 0x00;
  //uart_nano.write((uint8_t*)&message_out, sizeof(UNO2NANO_Message));
// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MOTORS CONTROL ----------------------------------------------------- 
  motor1.writeMicroseconds(motors_armed ? motors_speed[MOTOR(1)] : MIN_MOTOR_SIGNAL);
  motor2.writeMicroseconds(motors_armed ? motors_speed[MOTOR(2)] : MIN_MOTOR_SIGNAL);
  motor3.writeMicroseconds(motors_armed ? motors_speed[MOTOR(3)] : MIN_MOTOR_SIGNAL);
  motor4.writeMicroseconds(motors_armed ? motors_speed[MOTOR(4)] : MIN_MOTOR_SIGNAL);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MAINTENANCE  -------------------------------------------------------
  MAINT_UpdateLoopTime(message_in.loop_time);
  Serial.write((uint8_t*)MAINT_Get(), sizeof(maint_data_t));
// -----------------------------------------------------------------------------------------------------------------
}
