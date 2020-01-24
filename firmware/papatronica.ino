#include <BluetoothSerial.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif


BluetoothSerial SerialBT;
MPU6050 mpu6050(Wire, 0.1, 0.9);

const uint8_t scl = 16;// Select SDA and SCL pins for I2C communication
const uint8_t sda = 17;
double MagnitudUmbral = 0.0 ;
double Magnitud = 0 ;
long timeL, timeS = 0;

float maximunAccMeasure = 3;
float redRange = 0.4;
float yelowAlarm = 0.4;
float greenRange = 0.4;
bool ledState = false;

double Ax, Ay, Az, aux_Magnitud ;
float IMU_Temperature;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("PapaTronica"); //Bluetooth device name
  Wire.begin(sda, scl);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(false);
  mpu6050.writeMPU6050(MPU6050_ACCEL_CONFIG, 0x30);
  mpu6050.writeMPU6050(MPU6050_PWR_MGMT_1, 0x01);
  mpu6050.update();
}

void loop()
{
  mpu6050.update();
  
  Ax = mpu6050.getAccX();
  Ay = mpu6050.getAccY();
  Az = mpu6050.getAccZ();
  aux_Magnitud = std::abs(sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az)) - 1.0);

  if (aux_Magnitud > MagnitudUmbral && aux_Magnitud > Magnitud )
  {
    Magnitud = aux_Magnitud;
    SerialBT.println((String)"lvl "+Magnitud*40);
    SerialBT.println((String)"plot "+Magnitud*40);
    SerialBT.println((String)"txt "+Magnitud); 
    timeL = millis();
  }
  if (millis() - timeL > 1000)
  { 
    timeL = millis();
    ledState = !ledState;
    IMU_Temperature = ((float)mpu6050.getRawTemp()/333.87) + 11.0;        //340.0)+36.53; // You have to divide the temperature reading by 340 and then add +36.53.  The result is Celsius.
  //  SerialBT.println((String)"txt "+IMU_Temperature); 
    SerialBT.println(ledState ? "pled 1\n" : "pled 0\n");//+ledState);
    SerialBT.println((String)"lvl "+MagnitudUmbral*40); 
    Magnitud = 0;     
  }
  if (millis() - timeS > 100)
  { 
    SerialBT.println((String)"plot "+(aux_Magnitud*40));      
    timeS = millis();    
  }
  
  if (SerialBT.available()) 
  {
    Serial.write(SerialBT.read());
  }
}
