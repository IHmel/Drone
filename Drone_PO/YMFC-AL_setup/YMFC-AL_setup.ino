#include <Wire.h>               
#include <EEPROM.h>             


byte last_channel_1, last_channel_2, last_channel_3, last_channel_4;
byte lowByte, highByte, type, gyro_address, error, clockspeed_ok;
byte channel_1_assign, channel_2_assign, channel_3_assign, channel_4_assign;
byte roll_axis, pitch_axis, yaw_axis;
byte receiver_check_byte, gyro_check_byte;
volatile int receiver_input_channel_1, receiver_input_channel_2, receiver_input_channel_3, receiver_input_channel_4;
int center_channel_1, center_channel_2, center_channel_3, center_channel_4;
int high_channel_1, high_channel_2, high_channel_3, high_channel_4;
int low_channel_1, low_channel_2, low_channel_3, low_channel_4;
int address, cal_int;
unsigned long timer, timer_1, timer_2, timer_3, timer_4, current_time;
float gyro_pitch, gyro_roll, gyro_yaw;
float gyro_roll_cal, gyro_pitch_cal, gyro_yaw_cal;


void setup() {
  pinMode(12, OUTPUT);
  PCICR |= (1 << PCIE0);    
  PCMSK0 |= (1 << PCINT0);  
  PCMSK0 |= (1 << PCINT1);  
  PCMSK0 |= (1 << PCINT2);  
  PCMSK0 |= (1 << PCINT3);  
  Wire.begin();             
  Serial.begin(57600);      
  delay(250);               
}

void loop() {
  
  intro();

  Serial.println(F(""));
  Serial.println(F("==================================================="));
  Serial.println(F("Проверка системы"));
  Serial.println(F("==================================================="));
  delay(1000);
  Serial.println(F("Проверка I2C тактовой чистоты."));
  delay(1000);

  TWBR = 12;                 

#if F_CPU == 16000000L         
  clockspeed_ok = 1;         
#endif                         

  if (TWBR == 12 && clockspeed_ok) {
    Serial.println(F("I2C тактовая частота корректно установлена на 400kHz."));
  }
  else {
    Serial.println(F("I2C тактовая частота не установлена на 400kHz. (Ошибка 8)"));
    error = 1;
  }

  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F("==================================================="));
    Serial.println(F("Проверка передатчика"));
    Serial.println(F("==================================================="));
    delay(1000);
    Serial.print(F("Проверка подключен ли приемник к Ардуино ( должен быть подключен к питанию + и - )"));
    wait_for_receiver();
    Serial.println(F(""));
  }

  if (error == 0) {
    delay(2000);
    Serial.println(F("Поместите все стики в центр (в том числе стик газа) в течении 10 секунд."));
    for (int i = 9; i > 0; i--) {
      delay(1000);
      Serial.print(i);
      Serial.print(" ");
    }
    Serial.println(" ");
    center_channel_1 = receiver_input_channel_1;
    center_channel_2 = receiver_input_channel_2;
    center_channel_3 = receiver_input_channel_3;
    center_channel_4 = receiver_input_channel_4;
    Serial.println(F(""));
    Serial.println(F("Центральные точки сохранены."));
    Serial.print(F("Цифровой вход 08 = "));
    Serial.println(receiver_input_channel_1);
    Serial.print(F("Цифровой вход 09 = "));
    Serial.println(receiver_input_channel_2);
    Serial.print(F("Цифровой вход 10 = "));
    Serial.println(receiver_input_channel_3);
    Serial.print(F("Цифровой вход 11 = "));
    Serial.println(receiver_input_channel_4);
    Serial.println(F(""));
    Serial.println(F(""));
  }
  if (error == 0) {
    Serial.println(F("Передвиньте стик газа вверх (полный газ) и затем верните обратно в центр"));
    Serial.println(F("Стандартно [стик левый вверх] и в центр"));
    check_receiver_inputs(1);
    Serial.print(F("Газ (Throttle) подключен к цифровому входу "));
    Serial.println((channel_3_assign & 0b00000111) + 7);
    if (channel_3_assign & 0b10000000)Serial.println(F("Перевернутый канал = ДА"));
    else Serial.println(F("Перевернутый канал = НЕТ"));
    wait_sticks_zero();

    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Стик наклона коптера вправо (КРЕН - ROLL) затем верните в центр"));
    Serial.println(F("Стандартно [стик правый вправо] и в центр"));
    check_receiver_inputs(2);
    Serial.print(F("Крен (Roll) подключен к цифровому входу "));
    Serial.println((channel_1_assign & 0b00000111) + 7);
    if (channel_1_assign & 0b10000000)Serial.println(F("Перевернутый канал = ДА"));
    else Serial.println(F("Перевернутый канал = НЕТ"));
    wait_sticks_zero();
  }
  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Стик для наклона коптера назад (Тангаж - Pitch) затем верните в центр"));
    Serial.println(F("Стандартно [стик правый вниз] и в центр"));
    check_receiver_inputs(3);
    Serial.print(F("Тангаж (Pitch) подключен к цифровому входу  "));
    Serial.println((channel_2_assign & 0b00000111) + 7);
    if (channel_2_assign & 0b10000000)Serial.println(F("Перевернутый канал = ДА"));
    else Serial.println(F("Перевернутый канал = НЕТ"));
    wait_sticks_zero();
  }
  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Стик для рысканье коптера вправо (Рысканье - Yaw) затем верните в центр"));
    Serial.println(F("Стандартно [стик левый вправо] и в центр"));
    check_receiver_inputs(4);
    Serial.print(F("Рысканье (Yaw) подключен к цифровому входу  "));
    Serial.println((channel_4_assign & 0b00000111) + 7);
    if (channel_4_assign & 0b10000000)Serial.println(F("Перевернутый канал = ДА"));
    else Serial.println(F("Перевернутый канал = НЕТ"));
    wait_sticks_zero();
  }
  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Аккуратно оба стика вращайте по кругу в разные стороны"));
    Serial.println(F("Когда завершите верните их в центральное положение"));
    register_min_max();
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Максимальные, минимальные и центральные значения найдены во время настройки"));
    Serial.print(F("Значение цифрового входа 08:"));
    Serial.print(low_channel_1);
    Serial.print(F(" - "));
    Serial.print(center_channel_1);
    Serial.print(F(" - "));
    Serial.println(high_channel_1);
    Serial.print(F("Значение цифрового входа 09:"));
    Serial.print(low_channel_2);
    Serial.print(F(" - "));
    Serial.print(center_channel_2);
    Serial.print(F(" - "));
    Serial.println(high_channel_2);
    Serial.print(F("Значение цифрового входа 10:"));
    Serial.print(low_channel_3);
    Serial.print(F(" - "));
    Serial.print(center_channel_3);
    Serial.print(F(" - "));
    Serial.println(high_channel_3);
    Serial.print(F("Значение цифрового входа 11:"));
    Serial.print(low_channel_4);
    Serial.print(F(" - "));
    Serial.print(center_channel_4);
    Serial.print(F(" - "));
    Serial.println(high_channel_4);
    Serial.println(F("Передвиньте правый стик вниз и верните в центр для продолжения"));
    check_to_continue();
  }

  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F("==================================================="));
    Serial.println(F("Поиск гироскопа"));
    Serial.println(F("==================================================="));
    delay(2000);

    Serial.println(F("Поиск MPU-6050 по адресу 0x68/104"));
    delay(1000);
    if (search_gyro(0x68, 0x75) == 0x68) {
      Serial.println(F("MPU-6050 найден по адресу 0x68"));
      type = 1;
      gyro_address = 0x68;
    }

    if (type == 0) {
      Serial.println(F("Поиск MPU-6050 по адресу 0x69/105"));
      delay(1000);
      if (search_gyro(0x69, 0x75) == 0x68) {
        Serial.println(F("MPU-6050 найден по адресу 0x69"));
        type = 1;
        gyro_address = 0x69;
      }
    }

    if (type == 0) {
      Serial.println(F("Поиск L3G4200D по адресу 0x68/104"));
      delay(1000);
      if (search_gyro(0x68, 0x0F) == 0xD3) {
        Serial.println(F("L3G4200D найден по адресу 0x68"));
        type = 2;
        gyro_address = 0x68;
      }
    }

    if (type == 0) {
      Serial.println(F("Поиск L3G4200D по адресу 0x69/105"));
      delay(1000);
      if (search_gyro(0x69, 0x0F) == 0xD3) {
        Serial.println(F("L3G4200D найден по адресу 0x69"));
        type = 2;
        gyro_address = 0x69;
      }
    }

    if (type == 0) {
      Serial.println(F("Поиск L3GD20H по адресу 0x6A/106"));
      delay(1000);
      if (search_gyro(0x6A, 0x0F) == 0xD7) {
        Serial.println(F("L3GD20H найден по адресу 0x6A"));
        type = 3;
        gyro_address = 0x6A;
      }
    }

    if (type == 0) {
      Serial.println(F("Поиск L3GD20H по адресу 0x6B/107"));
      delay(1000);
      if (search_gyro(0x6B, 0x0F) == 0xD7) {
        Serial.println(F("L3GD20H найден по адресу 0x6B"));
        type = 3;
        gyro_address = 0x6B;
      }
    }

    if (type == 0) {
      Serial.println(F("Гироскоп не найден, возможно вы забыли его подключить или сделали это не правильно!!! (Ошибка 3)"));
      Serial.println(F("Проверьте SDA => SDA, SCL => SCL или SDA => A4, SCL => A5 и конечно питание +5V (VCC) и GND"));
      error = 1;
    }

    else {
      delay(3000);
      Serial.println(F(""));
      Serial.println(F("==================================================="));
      Serial.println(F("Настройка гироскопа..."));
      Serial.println(F("==================================================="));
      start_gyro(); 
    }
  }

  if (error == 0) {
    delay(3000);
    Serial.println(F(""));
    Serial.println(F("==================================================="));
    Serial.println(F("Калибровка гироскопа..."));
    Serial.println(F("==================================================="));
    Serial.println(F("Внимание не двигайте квадрокоптер!!!"));
    Serial.println(F("Калибровка начнется через 3 сек..."));
    delay(3000);
    Serial.println(F("Калибровка гироскопа, это займет +/- 8 секунд"));
    Serial.print(F("Пожалуйста подождите..."));
    for (cal_int = 0; cal_int < 2000; cal_int++) {              
      if (cal_int % 100 == 0)Serial.print(F("."));            
      gyro_signalen();                                        
      gyro_roll_cal += gyro_roll;                             
      gyro_pitch_cal += gyro_pitch;                           
      gyro_yaw_cal += gyro_yaw;                               
      delay(4);                                               
    }
    
    gyro_roll_cal /= 2000;                                      
    gyro_pitch_cal /= 2000;                                     
    gyro_yaw_cal /= 2000;                                       

    Serial.println(F(""));
    Serial.print(F("Ось 1 смещение="));
    Serial.println(gyro_roll_cal);
    Serial.print(F("Осьis 2 смещение="));
    Serial.println(gyro_pitch_cal);
    Serial.print(F("Ось 3 смещение="));
    Serial.println(gyro_yaw_cal);
    Serial.println(F(""));

    Serial.println(F("==================================================="));
    Serial.println(F("Конфигурирование осей гироскопа"));
    Serial.println(F("Определяем где передняя часть квадрокоптера, а где задняя"));
    Serial.println(F("==================================================="));

    Serial.println(F("Наклоните квадрокоптер вправо на 45 градусов в течении 10 секунд"));
    Serial.println(F("[Левую часть вверх]"));
    check_gyro_axes(1);
    if (error == 0) {
      Serial.println(F("ХОРОШО!"));
      Serial.print(F("Угол обнаружен = "));
      Serial.println(roll_axis & 0b00000011);
      if (roll_axis & 0b10000000)Serial.println(F("Инверсия оси = ДА"));
      else Serial.println(F("Инверсия оси = НЕТ"));
      Serial.println(F("Верните квадрокоптер в прежнее положение"));
      Serial.println(F("Передвиньте правый стик вниз и верните в центр для продолжения"));
      check_to_continue();

      Serial.println(F(""));
      Serial.println(F(""));
      Serial.println(F("Наклоните квадрокоптер назад на 45 градусов в течении 10 секунд"));
      Serial.println(F("[Носовую часть вверх]"));
      check_gyro_axes(2);
    }
    if (error == 0) {
      Serial.println(F("ХОРОШО!"));
      Serial.print(F("Угол обнаружен = "));
      Serial.println(pitch_axis & 0b00000011);
      if (pitch_axis & 0b10000000)Serial.println(F("Инверсия оси = ДА"));
      else Serial.println(F("Инверсия оси = НЕТ"));
      Serial.println(F("Верните квадрокоптер в прежнее положение"));
      Serial.println(F("Передвиньте правый стик вниз и верните в центр для продолжения"));
      check_to_continue();


      Serial.println(F(""));
      Serial.println(F(""));
      Serial.println(F("Поверните квадрокптер вправо вокруг оси на 45 градусов в течении 10 секунд"));
      check_gyro_axes(3);
    }
    if (error == 0) {
      Serial.println(F("ХОРОШО!"));
      Serial.print(F("Угол обнаружен = "));
      Serial.println(yaw_axis & 0b00000011);
      if (yaw_axis & 0b10000000)Serial.println(F("Инверсия оси = ДА"));
      else Serial.println(F("Инверсия оси = НЕТ"));
      Serial.println(F("Верните квадрокоптер в прежнее положение"));
      Serial.println(F("Передвиньте правый стик вниз и верните в центр для продолжения"));
      check_to_continue();
    }
  }
  if (error == 0) {
    Serial.println(F(""));
    Serial.println(F("==================================================="));
    Serial.println(F("Тест светодиода"));
    Serial.println(F("==================================================="));
    digitalWrite(12, HIGH);
    Serial.println(F("Светодиод должен загорется"));
    Serial.println(F("Передвиньте правый стик вниз и верните в центр для продолжения"));
    check_to_continue();
    digitalWrite(12, LOW);
  }

  Serial.println(F(""));

  if (error == 0) {
    Serial.println(F("==================================================="));
    Serial.println(F("Последние настройки"));
    Serial.println(F("==================================================="));
    delay(1000);
    if (receiver_check_byte == 0b00001111) {
      Serial.println(F("Каналы передатчика впорядке"));
    }
    else {
      Serial.println(F("Ошибка каналов передатчика проверьте кабели и питание!!! (ОШИБКА 6)"));
      error = 1;
    }
    delay(1000);
    if (gyro_check_byte == 0b00000111) {
      Serial.println(F("Оси гироскопа впорядке"));
    }
    else {
      Serial.println(F("Ошибка определения осей гироскопа!!! (ОШИБКА 7)"));
      error = 1;
    }
  }

  if (error == 0) {

    Serial.println(F(""));
    Serial.println(F("==================================================="));
    Serial.println(F("Сохранение EEPROM данных"));
    Serial.println(F("==================================================="));
    Serial.println(F("Запись EEPROM"));
    delay(1000);
    Serial.println(F("Готово!"));
    EEPROM.write(0, center_channel_1 & 0b11111111);
    EEPROM.write(1, center_channel_1 >> 8);
    EEPROM.write(2, center_channel_2 & 0b11111111);
    EEPROM.write(3, center_channel_2 >> 8);
    EEPROM.write(4, center_channel_3 & 0b11111111);
    EEPROM.write(5, center_channel_3 >> 8);
    EEPROM.write(6, center_channel_4 & 0b11111111);
    EEPROM.write(7, center_channel_4 >> 8);
    EEPROM.write(8, high_channel_1 & 0b11111111);
    EEPROM.write(9, high_channel_1 >> 8);
    EEPROM.write(10, high_channel_2 & 0b11111111);
    EEPROM.write(11, high_channel_2 >> 8);
    EEPROM.write(12, high_channel_3 & 0b11111111);
    EEPROM.write(13, high_channel_3 >> 8);
    EEPROM.write(14, high_channel_4 & 0b11111111);
    EEPROM.write(15, high_channel_4 >> 8);
    EEPROM.write(16, low_channel_1 & 0b11111111);
    EEPROM.write(17, low_channel_1 >> 8);
    EEPROM.write(18, low_channel_2 & 0b11111111);
    EEPROM.write(19, low_channel_2 >> 8);
    EEPROM.write(20, low_channel_3 & 0b11111111);
    EEPROM.write(21, low_channel_3 >> 8);
    EEPROM.write(22, low_channel_4 & 0b11111111);
    EEPROM.write(23, low_channel_4 >> 8);
    EEPROM.write(24, channel_1_assign);
    EEPROM.write(25, channel_2_assign);
    EEPROM.write(26, channel_3_assign);
    EEPROM.write(27, channel_4_assign);
    EEPROM.write(28, roll_axis);
    EEPROM.write(29, pitch_axis);
    EEPROM.write(30, yaw_axis);
    EEPROM.write(31, type);
    EEPROM.write(32, gyro_address);
    EEPROM.write(33, 'J');
    EEPROM.write(34, 'M');
    EEPROM.write(35, 'B');


    Serial.println(F("Проверка EEPROM данных"));
    delay(1000);
    if (center_channel_1 != ((EEPROM.read(1) << 8) | EEPROM.read(0)))error = 1;
    if (center_channel_2 != ((EEPROM.read(3) << 8) | EEPROM.read(2)))error = 1;
    if (center_channel_3 != ((EEPROM.read(5) << 8) | EEPROM.read(4)))error = 1;
    if (center_channel_4 != ((EEPROM.read(7) << 8) | EEPROM.read(6)))error = 1;

    if (high_channel_1 != ((EEPROM.read(9) << 8) | EEPROM.read(8)))error = 1;
    if (high_channel_2 != ((EEPROM.read(11) << 8) | EEPROM.read(10)))error = 1;
    if (high_channel_3 != ((EEPROM.read(13) << 8) | EEPROM.read(12)))error = 1;
    if (high_channel_4 != ((EEPROM.read(15) << 8) | EEPROM.read(14)))error = 1;

    if (low_channel_1 != ((EEPROM.read(17) << 8) | EEPROM.read(16)))error = 1;
    if (low_channel_2 != ((EEPROM.read(19) << 8) | EEPROM.read(18)))error = 1;
    if (low_channel_3 != ((EEPROM.read(21) << 8) | EEPROM.read(20)))error = 1;
    if (low_channel_4 != ((EEPROM.read(23) << 8) | EEPROM.read(22)))error = 1;

    if (channel_1_assign != EEPROM.read(24))error = 1;
    if (channel_2_assign != EEPROM.read(25))error = 1;
    if (channel_3_assign != EEPROM.read(26))error = 1;
    if (channel_4_assign != EEPROM.read(27))error = 1;

    if (roll_axis != EEPROM.read(28))error = 1;
    if (pitch_axis != EEPROM.read(29))error = 1;
    if (yaw_axis != EEPROM.read(30))error = 1;
    if (type != EEPROM.read(31))error = 1;
    if (gyro_address != EEPROM.read(32))error = 1;

    if ('J' != EEPROM.read(33))error = 1;
    if ('M' != EEPROM.read(34))error = 1;
    if ('B' != EEPROM.read(35))error = 1;

    if (error == 1)Serial.println(F("EEPROM ошибка проверки!!! (ОШИБКА 5)"));
    else Serial.println(F("Проверка завершена"));
  }


  if (error == 0) {
    Serial.println(F("Настройка завершена."));
    Serial.println(F("Теперь вы можете перейти к калибровке ESC (Моторов и регуляторов скорости)."));
    Serial.println(F("Следующим шагом запустите YMFC-AL_esc_calibrate.ino"));
  }
  else {
    Serial.println(F("Это установка отменена из-за ошибки."));
    Serial.println(F("Check the Q and A page of the YMFC-AL project on:"));
    Serial.println(F("www.brokking.net for more information about this error."));
  }
  while (1);
}

byte search_gyro(int gyro_address, int who_am_i) {
  Wire.beginTransmission(gyro_address);
  Wire.write(who_am_i);
  Wire.endTransmission();
  Wire.requestFrom(gyro_address, 1);
  timer = millis() + 100;
  while (Wire.available() < 1 && timer > millis());
  lowByte = Wire.read();
  address = gyro_address;
  return lowByte;
}

void start_gyro() {

  if (type == 2 || type == 3) {
    Wire.beginTransmission(address);                            
    Wire.write(0x20);                                           
    Wire.write(0x0F);                                           
    Wire.endTransmission();                                     

    Wire.beginTransmission(address);                            
    Wire.write(0x20);                                           
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 1);                               
    while (Wire.available() < 1);                               
    Serial.print(F("Register 0x20 is set to:"));
    Serial.println(Wire.read(), BIN);

    Wire.beginTransmission(address);                            
    Wire.write(0x23);                                           
    Wire.write(0x90);                                           
    Wire.endTransmission();                                     

    Wire.beginTransmission(address);                            
    Wire.write(0x23);                                           
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 1);                               
    while (Wire.available() < 1);                               
    Serial.print(F("Register 0x23 is set to:"));
    Serial.println(Wire.read(), BIN);

  }
  if (type == 1) {

    Wire.beginTransmission(address);                            
    Wire.write(0x6B);                                           
    Wire.write(0x00);                                           
    Wire.endTransmission();                                     

    Wire.beginTransmission(address);                            
    Wire.write(0x6B);                                           
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 1);                               
    while (Wire.available() < 1);                               
    Serial.print(F("Register 0x6B is set to:"));
    Serial.println(Wire.read(), BIN);

    Wire.beginTransmission(address);                            
    Wire.write(0x1B);                                           
    Wire.write(0x08);                                           
    Wire.endTransmission();                                     

    Wire.beginTransmission(address);                            
    Wire.write(0x1B);                                           
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 1);                               
    while (Wire.available() < 1);                               
    Serial.print(F("Register 0x1B is set to:"));
    Serial.println(Wire.read(), BIN);

  }
}

void gyro_signalen() {
  if (type == 2 || type == 3) {
    Wire.beginTransmission(address);                            
    Wire.write(168);                                            
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 6);                               
    while (Wire.available() < 6);                               
    lowByte = Wire.read();                                      
    highByte = Wire.read();                                     
    gyro_roll = ((highByte << 8) | lowByte);                    
    if (cal_int == 2000)gyro_roll -= gyro_roll_cal;             
    lowByte = Wire.read();                                      
    highByte = Wire.read();                                     
    gyro_pitch = ((highByte << 8) | lowByte);                   
    if (cal_int == 2000)gyro_pitch -= gyro_pitch_cal;           
    lowByte = Wire.read();                                      
    highByte = Wire.read();                                     
    gyro_yaw = ((highByte << 8) | lowByte);                     
    if (cal_int == 2000)gyro_yaw -= gyro_yaw_cal;               
  }
  if (type == 1) {
    Wire.beginTransmission(address);                            
    Wire.write(0x43);                                           
    Wire.endTransmission();                                     
    Wire.requestFrom(address, 6);                               
    while (Wire.available() < 6);                               
    gyro_roll = Wire.read() << 8 | Wire.read();                 
    if (cal_int == 2000)gyro_roll -= gyro_roll_cal;             
    gyro_pitch = Wire.read() << 8 | Wire.read();                
    if (cal_int == 2000)gyro_pitch -= gyro_pitch_cal;           
    gyro_yaw = Wire.read() << 8 | Wire.read();                  
    if (cal_int == 2000)gyro_yaw -= gyro_yaw_cal;               
  }
}

void check_receiver_inputs(byte movement) {
  byte trigger = 0;
  int pulse_length;
  timer = millis() + 30000;
  while (timer > millis() && trigger == 0) {
    delay(250);
    if (receiver_input_channel_1 > 1750 || receiver_input_channel_1 < 1250) {
      trigger = 1;
      receiver_check_byte |= 0b00000001;
      pulse_length = receiver_input_channel_1;
    }
    if (receiver_input_channel_2 > 1750 || receiver_input_channel_2 < 1250) {
      trigger = 2;
      receiver_check_byte |= 0b00000010;
      pulse_length = receiver_input_channel_2;
    }
    if (receiver_input_channel_3 > 1750 || receiver_input_channel_3 < 1250) {
      trigger = 3;
      receiver_check_byte |= 0b00000100;
      pulse_length = receiver_input_channel_3;
    }
    if (receiver_input_channel_4 > 1750 || receiver_input_channel_4 < 1250) {
      trigger = 4;
      receiver_check_byte |= 0b00001000;
      pulse_length = receiver_input_channel_4;
    }
  }
  if (trigger == 0) {
    error = 1;
    Serial.println(F("За последние 30 секунд не обнаружено движение стиков на пульте!!! (ERROR 2)"));
  }

  else {
    if (movement == 1) {
      channel_3_assign = trigger;
      if (pulse_length < 1250)channel_3_assign += 0b10000000;
    }
    if (movement == 2) {
      channel_1_assign = trigger;
      if (pulse_length < 1250)channel_1_assign += 0b10000000;
    }
    if (movement == 3) {
      channel_2_assign = trigger;
      if (pulse_length < 1250)channel_2_assign += 0b10000000;
    }
    if (movement == 4) {
      channel_4_assign = trigger;
      if (pulse_length < 1250)channel_4_assign += 0b10000000;
    }
  }
}

void check_to_continue() {
  byte continue_byte = 0;
  while (continue_byte == 0) {
    if (channel_2_assign == 0b00000001 && receiver_input_channel_1 > center_channel_1 + 150)continue_byte = 1;
    if (channel_2_assign == 0b10000001 && receiver_input_channel_1 < center_channel_1 - 150)continue_byte = 1;
    if (channel_2_assign == 0b00000010 && receiver_input_channel_2 > center_channel_2 + 150)continue_byte = 1;
    if (channel_2_assign == 0b10000010 && receiver_input_channel_2 < center_channel_2 - 150)continue_byte = 1;
    if (channel_2_assign == 0b00000011 && receiver_input_channel_3 > center_channel_3 + 150)continue_byte = 1;
    if (channel_2_assign == 0b10000011 && receiver_input_channel_3 < center_channel_3 - 150)continue_byte = 1;
    if (channel_2_assign == 0b00000100 && receiver_input_channel_4 > center_channel_4 + 150)continue_byte = 1;
    if (channel_2_assign == 0b10000100 && receiver_input_channel_4 < center_channel_4 - 150)continue_byte = 1;
    delay(100);
  }
  wait_sticks_zero();
}


void wait_sticks_zero() {
  byte zero = 0;
  while (zero < 15) {
    if (receiver_input_channel_1 < center_channel_1 + 20 && receiver_input_channel_1 > center_channel_1 - 20)zero |= 0b00000001;
    if (receiver_input_channel_2 < center_channel_2 + 20 && receiver_input_channel_2 > center_channel_2 - 20)zero |= 0b00000010;
    if (receiver_input_channel_3 < center_channel_3 + 20 && receiver_input_channel_3 > center_channel_3 - 20)zero |= 0b00000100;
    if (receiver_input_channel_4 < center_channel_4 + 20 && receiver_input_channel_4 > center_channel_4 - 20)zero |= 0b00001000;
    delay(100);
  }
}


void wait_for_receiver() {
  byte zero = 0;
  timer = millis() + 10000;
  while (timer > millis() && zero < 15) {
    if (receiver_input_channel_1 < 2100 && receiver_input_channel_1 > 900)zero |= 0b00000001;
    if (receiver_input_channel_2 < 2100 && receiver_input_channel_2 > 900)zero |= 0b00000010;
    if (receiver_input_channel_3 < 2100 && receiver_input_channel_3 > 900)zero |= 0b00000100;
    if (receiver_input_channel_4 < 2100 && receiver_input_channel_4 > 900)zero |= 0b00001000;
    delay(500);
    Serial.print(F("."));
  }
  if (zero == 0) {
    error = 1;
    Serial.println(F("."));
    Serial.println(F("Не найден сигнал приемника!!! (ERROR 1)"));
  }
  else Serial.println(F(" OK"));
}


void register_min_max() {
  byte zero = 0;
  low_channel_1 = receiver_input_channel_1;
  low_channel_2 = receiver_input_channel_2;
  low_channel_3 = receiver_input_channel_3;
  low_channel_4 = receiver_input_channel_4;
  while (receiver_input_channel_1 < center_channel_1 + 20 && receiver_input_channel_1 > center_channel_1 - 20)delay(250);
  Serial.println(F("Измерение конечных точек...."));
  while (zero < 15) {
    if (receiver_input_channel_1 < center_channel_1 + 20 && receiver_input_channel_1 > center_channel_1 - 20)zero |= 0b00000001;
    if (receiver_input_channel_2 < center_channel_2 + 20 && receiver_input_channel_2 > center_channel_2 - 20)zero |= 0b00000010;
    if (receiver_input_channel_3 < center_channel_3 + 20 && receiver_input_channel_3 > center_channel_3 - 20)zero |= 0b00000100;
    if (receiver_input_channel_4 < center_channel_4 + 20 && receiver_input_channel_4 > center_channel_4 - 20)zero |= 0b00001000;
    if (receiver_input_channel_1 < low_channel_1)low_channel_1 = receiver_input_channel_1;
    if (receiver_input_channel_2 < low_channel_2)low_channel_2 = receiver_input_channel_2;
    if (receiver_input_channel_3 < low_channel_3)low_channel_3 = receiver_input_channel_3;
    if (receiver_input_channel_4 < low_channel_4)low_channel_4 = receiver_input_channel_4;
    if (receiver_input_channel_1 > high_channel_1)high_channel_1 = receiver_input_channel_1;
    if (receiver_input_channel_2 > high_channel_2)high_channel_2 = receiver_input_channel_2;
    if (receiver_input_channel_3 > high_channel_3)high_channel_3 = receiver_input_channel_3;
    if (receiver_input_channel_4 > high_channel_4)high_channel_4 = receiver_input_channel_4;
    delay(100);
  }
}


void check_gyro_axes(byte movement) {
  byte trigger_axis = 0;
  float gyro_angle_roll, gyro_angle_pitch, gyro_angle_yaw;
  //Reset all axes
  gyro_angle_roll = 0;
  gyro_angle_pitch = 0;
  gyro_angle_yaw = 0;
  gyro_signalen();
  timer = millis() + 10000;
  while (timer > millis() && gyro_angle_roll > -30 && gyro_angle_roll < 30 && gyro_angle_pitch > -30 && gyro_angle_pitch < 30 && gyro_angle_yaw > -30 && gyro_angle_yaw < 30) {
    gyro_signalen();
    if (type == 2 || type == 3) {
      gyro_angle_roll += gyro_roll * 0.00007;              
      gyro_angle_pitch += gyro_pitch * 0.00007;
      gyro_angle_yaw += gyro_yaw * 0.00007;
    }
    if (type == 1) {
      gyro_angle_roll += gyro_roll * 0.0000611;          
      gyro_angle_pitch += gyro_pitch * 0.0000611;
      gyro_angle_yaw += gyro_yaw * 0.0000611;
    }

    delayMicroseconds(3700); 
  }
  
  if ((gyro_angle_roll < -30 || gyro_angle_roll > 30) && gyro_angle_pitch > -30 && gyro_angle_pitch < 30 && gyro_angle_yaw > -30 && gyro_angle_yaw < 30) {
    gyro_check_byte |= 0b00000001;
    if (gyro_angle_roll < 0)trigger_axis = 0b10000001;
    else trigger_axis = 0b00000001;
  }
  if ((gyro_angle_pitch < -30 || gyro_angle_pitch > 30) && gyro_angle_roll > -30 && gyro_angle_roll < 30 && gyro_angle_yaw > -30 && gyro_angle_yaw < 30) {
    gyro_check_byte |= 0b00000010;
    if (gyro_angle_pitch < 0)trigger_axis = 0b10000010;
    else trigger_axis = 0b00000010;
  }
  if ((gyro_angle_yaw < -30 || gyro_angle_yaw > 30) && gyro_angle_roll > -30 && gyro_angle_roll < 30 && gyro_angle_pitch > -30 && gyro_angle_pitch < 30) {
    gyro_check_byte |= 0b00000100;
    if (gyro_angle_yaw < 0)trigger_axis = 0b10000011;
    else trigger_axis = 0b00000011;
  }

  if (trigger_axis == 0) {
    error = 1;
    Serial.println(F("За последние 10 секунд не обнаружено углового движения!!! (ERROR 4)"));
  }
  else
    if (movement == 1)roll_axis = trigger_axis;
  if (movement == 2)pitch_axis = trigger_axis;
  if (movement == 3)yaw_axis = trigger_axis;

}


ISR(PCINT0_vect) {
  current_time = micros();
  
  if (PINB & B00000001) {                                       
    if (last_channel_1 == 0) {                                
      last_channel_1 = 1;                                   
      timer_1 = current_time;                               
    }
  }
  else if (last_channel_1 == 1) {                               
    last_channel_1 = 0;                                       
    receiver_input_channel_1 = current_time - timer_1;        
  }
  
  if (PINB & B00000010) {                                       
    if (last_channel_2 == 0) {                                
      last_channel_2 = 1;                                   
      timer_2 = current_time;                               
    }
  }
  else if (last_channel_2 == 1) {                               
    last_channel_2 = 0;                                       
    receiver_input_channel_2 = current_time - timer_2;        
  }
  
  if (PINB & B00000100) {                                       
    if (last_channel_3 == 0) {                                
      last_channel_3 = 1;                                   
      timer_3 = current_time;                               
    }
  }
  else if (last_channel_3 == 1) {                               
    last_channel_3 = 0;                                       
    receiver_input_channel_3 = current_time - timer_3;        

  }
  
  if (PINB & B00001000) {                                       
    if (last_channel_4 == 0) {                                
      last_channel_4 = 1;                                   
      timer_4 = current_time;                               
    }
  }
  else if (last_channel_4 == 1) {                               
    last_channel_4 = 0;                                       
    receiver_input_channel_4 = current_time - timer_4;        
  }
}

void intro() {
  Serial.println(F("==================================================="));
  delay(1500);
  Serial.println(F(""));
  Serial.println(F("Ваш"));
  delay(500);
  Serial.println(F("  Квадрокоптер"));
  delay(500);
  Serial.println(F("    Полетный"));
  delay(500);
  Serial.println(F("      Контроллер"));
  delay(1000);
  Serial.println(F(""));
  Serial.println(F("YMFC-AL программа установки"));
  Serial.println(F(""));
  Serial.println(F("==================================================="));
  delay(1500);
  Serial.println(F("Для поддержки и вопросов: www.brokking.net"));
  Serial.println(F(""));
  Serial.println(F("Удачных полетов, перевод выполнен Cool Easy Diy YouTube Канал! https://cediy.ru"));
}
