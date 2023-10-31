/*
DanVi
BuiltIn(ATMEGA32U4-MU)
PCB_ver
*/


// Gyro
// MPU6050_light
#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
unsigned long timer = 0;

// 디지털핀 설계
#define led_pin 13                    // 13번 디지털핀(led_pin)                 :: LED 출력
#define buzzer_pin 6                  // 6번 디지털핀(buzzer_pin)               :: BUZZER 출력

#define F_left_A_pin 4                  // 4번 디지털핀(left_A_pin)               :: MOTER 출력
#define F_left_B_pin 7                  // 7번 디지털핀(left_B_pin)               :: MOTER 출력
#define F_right_A_pin 12                // 12번 디지털핀(right_A_pin)             :: MOTER 출력
#define F_right_B_pin 8                 // 8번 디지털핀(right_B_pin)              :: MOTER 출력
#define B_left_A_pin 14                 // 14번 디지털핀(left_A_pin)               :: MOTER 출력
#define B_left_B_pin 15                 // 15번 디지털핀(left_B_pin)               :: MOTER 출력
#define B_right_A_pin 23                // 23번 디지털핀(right_A_pin)             :: MOTER 출력
#define B_right_B_pin 16                // 16번 디지털핀(right_B_pin)              :: MOTER 출력

// 아날로그핀 설계
#define button_pin A0                 // A0번 아날로그핀(button_pin)             :: 푸쉬스위치 입력
#define line_sensor_L_pin A1          // A1번 아날로그핀(line_sensor_L_pin)      :: 라인센서 입력
#define line_sensor_R_pin A2          // A2번 아날로그핀(line_sensor_R_pin)      :: 라인센서 입력

// 버튼상태 변수 선언
int button_state = 0;
int button_state_cnt = 0;
int led_blink_speed = 50;

// 블루투스 받는 문자열
String bluetooth_string = "";

// 라인센서 변수선언
int line_sensor_L, line_sensor_R;
int line_sensor_limit = 100;

// Gyro
int run_yaw_angle;
int tarket_yaw_angle = 9999;


//-----
// 부팅시 만 실행
void setup()
{
  // 시리얼통신 초기화(9600)bps(bits per second)
  Serial.begin(9600);

  //블루투스시리얼[하드웨어] 초기화(9600)bps(bits per second)
  Serial1.begin(9600);  

  // Gyro
  Wire.begin();
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");  

  // 디지털핀 초기화(OUTPUT)
  pinMode(led_pin, OUTPUT);                   // led_pin
  pinMode(buzzer_pin, OUTPUT);                // buzzer_pin

  pinMode(F_left_A_pin, OUTPUT);                // left_A_pin
  pinMode(F_left_B_pin, OUTPUT);                // left_B_pin
  pinMode(F_right_A_pin, OUTPUT);               // right_A_pin
  pinMode(F_right_B_pin, OUTPUT);               // right_B_pin
  pinMode(B_left_A_pin, OUTPUT);                // left_A_pin
  pinMode(B_left_B_pin, OUTPUT);                // left_B_pin
  pinMode(B_right_A_pin, OUTPUT);               // right_A_pin
  pinMode(B_right_B_pin, OUTPUT);               // right_B_pin  

  // LED 모드선택(최초 부팅시)
  blink_fn(button_state_cnt);          
}
// 부팅시 만 실행


//-----
// 메인(반복루프)
void loop()
{
  // gyro_getAngle_fn
  gyro_getAngle_fn();

  // 모드스위치(푸쉬스위치) 눌림 512이상 입력값 생성 :: 아날로그 입력값[0~1023]
  // 블루투스시리얼 0 => ASCII["48"]
  if ((analogRead(button_pin) > 512) || (bluetooth_string == "48"))
  {
    // 블루투스 받는 문자열 초기화
    bluetooth_string = "";

    // 모드 선택시
    // LED OFF
    digitalWrite(led_pin, LOW);    
    // button_state_cnt 누적 더하기
    button_state_cnt += 1;

    // 모드스위치(푸쉬스위치) 누르고 기다리는 시간
    delay(500);

    // 모드 처음부터 다시 시작
    if (button_state_cnt == 4)
    {
      button_state_cnt = 0;
    }
    // 시리얼 프린터 알림
    Serial.println(button_state_cnt);
    // 블루투스시리얼[하드웨어] 프린터 알림
    Serial1.println(button_state_cnt);    
    // LED 깜박임
    blink_fn(button_state_cnt);
  }

  // 블루투스시리얼[하드웨어] 명령 수신
  if (Serial1.available())
  { // Serial1[하드웨어] 값이 있으면
    bluetooth_string = Serial1.read();  // 블루투스측 내용 저장
    // 0 => ASCII["48"]
    // Serial.println(bluetooth_string);
  }

  // mode setting:
  mode_setting_fn(button_state_cnt);
}
// 메인 끝


//-----
// 모드 셋팅 함수
// mode_setting_fn
void mode_setting_fn(int button_state_cnt)
{
  // mode setting
  if (button_state_cnt == 0)
  {
    // BLE_rx_tx_fn
    BLE_rx_tx_fn();
  }
  else if (button_state_cnt == 1)
  {
    // line_sensor_fn:
    line_sensor_fn();
  }
  else if (button_state_cnt == 2)
  {

  }
  else if (button_state_cnt == 3)
  {

  }
}


//-----
// gyro_getAngle_fn
int gyro_getAngle_fn()
{
  mpu.update();
  
  if ((millis() - timer) > 10) // print data every 10ms
  {
    // Serial.print("X : ");
    // Serial.print(mpu.getAngleX());
    // Serial.print("\tY : ");
    // Serial.print(mpu.getAngleY());
    // Serial.print("\tZ : ");
    // Serial.println(mpu.getAngleZ());

    run_yaw_angle = mpu.getAngleZ();

    timer = millis();  
  }

  // 목표앵글 도달
  if (run_yaw_angle == tarket_yaw_angle)
  {
    Serial.println("중지");

    F_left_moter_stop_fn();
    F_right_moter_stop_fn();
    B_left_moter_stop_fn();
    B_right_moter_stop_fn();

    tarket_yaw_angle = 9999;
  }
}


//-----
// line_sensor_fn
int line_sensor_fn()
{
  // Line_Sensor:
  line_sensor_L = analogRead(line_sensor_L_pin);
  line_sensor_R = analogRead(line_sensor_R_pin);
  // print out the value you read:
  Serial.print("line_sensor_L : ");
  Serial.println(line_sensor_L);
  Serial.print("line_sensor_R : ");
  Serial.println(line_sensor_R);
  // 블루투스시리얼[하드웨어] 프린터 알림
  Serial1.print(line_sensor_L);
  Serial1.print(" / ");
  Serial1.println(line_sensor_R);  

  // 라인센서 아날로그 신호(백색:~100, 흑색:200~)  :: 평균값임 조도에 따라서 변화 심함
  if (line_sensor_L < line_sensor_limit && line_sensor_R < line_sensor_limit)
  {
    Serial.println("전진");

    F_left_moter_F_fn();
    F_right_moter_F_fn();
    B_left_moter_F_fn();
    B_right_moter_F_fn();
  }

  else if (line_sensor_L < line_sensor_limit && line_sensor_R > line_sensor_limit)
  {
    Serial.println("좌회전");

    F_left_moter_R_fn();
    F_right_moter_F_fn();
    B_left_moter_R_fn();
    B_right_moter_F_fn();
  }

  else if (line_sensor_L > line_sensor_limit && line_sensor_R < line_sensor_limit)
  {
    Serial.println("우회전");

    F_left_moter_F_fn();
    F_right_moter_R_fn();
    B_left_moter_F_fn();
    B_right_moter_R_fn();
  }

  else if (line_sensor_L > line_sensor_limit && line_sensor_R > line_sensor_limit)
  {
    Serial.println("중지");

    F_left_moter_stop_fn();
    F_right_moter_stop_fn();
    B_left_moter_stop_fn();
    B_right_moter_stop_fn();
  }
}


//-----
// BLE_rx_tx_fn
void BLE_rx_tx_fn()
{
  // 블루투스시리얼[하드웨어] 명령 수신
  if (Serial1.available())
  { // Serial1[하드웨어] 값이 있으면
    Serial.write(Serial1.read());       //블루투스측 내용을 시리얼모니터에 출력
  }  

  // Serial.println("bluetooth_string : " + bluetooth_string);       //시리얼모니터에 bluetooth_string 값 출력
  if (bluetooth_string != "")         //bluetooth_string 값이 있다면
  { 
    // 실행(1~9)(전화버튼 기준):: ASCII

    // 2 = "50"
    if (bluetooth_string == "50")
    {
      Serial.println("전진");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_F_fn();
      F_right_moter_F_fn();
      B_left_moter_F_fn();
      B_right_moter_F_fn();      
    }

    // 4 = "52"
    else if (bluetooth_string == "52")
    {
      Serial.println("좌측이동");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_R_fn();
      F_right_moter_F_fn();
      B_left_moter_F_fn();
      B_right_moter_R_fn();      
    }

    // 5 = "53"
    else if (bluetooth_string == "53")
    {
      Serial.println("중지");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_stop_fn();
      F_right_moter_stop_fn();
      B_left_moter_stop_fn();      
      B_right_moter_stop_fn();
    }
    
    // 6 = "54"
    else if (bluetooth_string == "54")
    {
      Serial.println("우측이동");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_F_fn();
      F_right_moter_R_fn();
      B_left_moter_R_fn();
      B_right_moter_F_fn();      
    }

    // 8 = "56"
    else if (bluetooth_string == "56")
    {
      Serial.println("후진");

      // BUZZER ON/OFF 함수
      button_state = 1;

      F_left_moter_R_fn();
      F_right_moter_R_fn();
      B_left_moter_R_fn();
      B_right_moter_R_fn();      
    }

    // 1 = "49"
    else if (bluetooth_string == "49")
    {
      Serial.println("좌회전");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_R_fn();
      F_right_moter_F_fn();
      B_left_moter_stop_fn();
      B_right_moter_stop_fn();      
    }

    // 3 = "51"
    else if (bluetooth_string == "51")
    {
      Serial.println("우회전");

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_F_fn();
      F_right_moter_R_fn();
      B_left_moter_stop_fn();
      B_right_moter_stop_fn();      
    }

    // 7 = "55"
    else if (bluetooth_string == "55")
    {
      Serial.println("좌90");

      //자이로 계산(목표 yaw)
      tarket_yaw_angle = run_yaw_angle + 90;

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_R_fn();
      F_right_moter_F_fn();
      B_left_moter_R_fn();
      B_right_moter_F_fn();

      // bluetooth_string초기화
      bluetooth_string = "";               
    }

    // 9 = "57"
    else if (bluetooth_string == "57")
    {
      Serial.println("우90");

      //자이로 계산(목표 yaw)
      tarket_yaw_angle = run_yaw_angle - 90;      

      // BUZZER ON/OFF 함수
      button_state = 0;

      F_left_moter_F_fn();
      F_right_moter_R_fn();
      B_left_moter_F_fn();
      B_right_moter_R_fn();      

      // bluetooth_string초기화
      bluetooth_string = "";         
    }                

    // BUZZER ON/OFF 함수
    buzzer_fn(button_state);          
  }

/*
"No line ending [9600 baud]"
AT  (OK)
AT+NAME이름 (OK+Set:이름)
*/

  // 시리얼통신 명령 수신
  if (Serial.available())
  {
    Serial1.write(Serial.read());  // 시리얼모니터 내용을 블루투스측에 출력
  }    
}


//-----
// MOTER 구동 함수
// F_left_moter_F_fn
void F_left_moter_F_fn()
{
  digitalWrite(F_left_A_pin, HIGH);
  digitalWrite(F_left_B_pin, LOW);
}
// F_left_moter_R_fn
void F_left_moter_R_fn()
{
  digitalWrite(F_left_A_pin, LOW);
  digitalWrite(F_left_B_pin, HIGH);
}
// F_right_moter_F_fn
void F_right_moter_F_fn()
{
  digitalWrite(F_right_A_pin, HIGH);
  digitalWrite(F_right_B_pin, LOW);
}
// F_right_moter_R_fn
void F_right_moter_R_fn()
{
  digitalWrite(F_right_A_pin, LOW);
  digitalWrite(F_right_B_pin, HIGH);
}

// B_left_moter_F_fn
void B_left_moter_F_fn()
{
  digitalWrite(B_left_A_pin, HIGH);
  digitalWrite(B_left_B_pin, LOW);
}
// B_left_moter_R_fn
void B_left_moter_R_fn()
{
  digitalWrite(B_left_A_pin, LOW);
  digitalWrite(B_left_B_pin, HIGH);
}
// B_right_moter_F_fn
void B_right_moter_F_fn()
{
  digitalWrite(B_right_A_pin, HIGH);
  digitalWrite(B_right_B_pin, LOW);
}
// B_right_moter_R_fn
void B_right_moter_R_fn()
{
  digitalWrite(B_right_A_pin, LOW);
  digitalWrite(B_right_B_pin, HIGH);
}

// moter_stop_fn
void F_left_moter_stop_fn()
{
  digitalWrite(F_left_A_pin, LOW);
  digitalWrite(F_left_B_pin, LOW);
}
void F_right_moter_stop_fn()
{
  digitalWrite(F_right_A_pin, LOW);
  digitalWrite(F_right_B_pin, LOW);
}
void B_left_moter_stop_fn()
{
  digitalWrite(B_left_A_pin, LOW);
  digitalWrite(B_left_B_pin, LOW);
}
void B_right_moter_stop_fn()
{
  digitalWrite(B_right_A_pin, LOW);
  digitalWrite(B_right_B_pin, LOW);
}


//-----
// BUZZER ON/OFF 함수
// buzzer_fn
void buzzer_fn(int button_state)
{
    if (button_state == 0)
    {
      // BUZZER OFF
      digitalWrite(buzzer_pin, LOW);
      Serial.println("BUZZER OFF");
    }
    else if (button_state == 1)
    {
      // BUZZER ON
      digitalWrite(buzzer_pin, HIGH);
      Serial.println("BUZZER ON");    
    }   
}


//-----
// LED ON/OFF 함수
// blink_fn
void blink_fn(int button_state_cnt)
{
  for (int i = 0; i < button_state_cnt; i++)
  {
    // LED ON      
    digitalWrite(led_pin, HIGH);
    delay(led_blink_speed);   
    // LED OFF
    digitalWrite(led_pin, LOW);
    delay(led_blink_speed);   
  }
  // LED ON    
  digitalWrite(led_pin, HIGH);
}