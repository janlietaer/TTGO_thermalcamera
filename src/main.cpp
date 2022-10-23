#include <Arduino.h>
#include <Adafruit_MLX90640.h>

#include <TFT_eSPI.h>

//#include "MLX90640_API.h"
//#include "MLX90640_I2C_Driver.h"

Adafruit_MLX90640 mlx;

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

float frame[32 * 24]; // buffer for full frame of temperatures
byte red, green, blue;
float minTemp = 22.0;
float maxTemp = 41.0;
float intPoint, val, a, b, c, d, ii;
String waarde;
int i, j;
// uncomment *one* of the below
#define PRINT_TEMPERATURES

#define EMMISIVITY 0.95
#define INTERPOLATE false

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
/**
#define C_BLUE tft.color565(0,0,255)
#define C_RED tft.color565(255,0,0)
#define C_GREEN tft.color565(0,255,0)
#define C_WHITE tft.color565(255,255,255)
#define C_BLACK tft.color565(0,0,0)
#define C_LTGREY tft.color565(200,200,200)
#define C_DKGREY tft.color565(80,80,80)
#define C_GREY tft.color565(127,127,127)
**/

// Get color for temp value.
uint16_t getColor(float val)
{
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed simple linear equations
    again a 5-6-5 color display will not need accurate temp to R G B color calculation
    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html
  */

  red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

  if ((val > minTemp) & (val < a))
  {
    green = constrain(255.0 / (a - minTemp) * val - (255.0 * minTemp) / (a - minTemp), 0, 255);
  }
  else if ((val >= a) & (val <= c))
  {
    green = 255;
  }
  else if (val > c)
  {
    green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
  }
  else if ((val > d) | (val < a))
  {
    green = 0;
  }

  if (val <= b)
  {
    blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
  }
  else if ((val > b) & (val <= d))
  {
    blue = 0;
  }
  else if (val > d)
  {
    blue = constrain(240.0 / (maxTemp - d) * val - (d * 240.0) / (maxTemp - d), 0, 240);
  }

  // use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
  return tft.color565(red, green, blue);
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  delay(100);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  if (TFT_BL > 0)
  {                                         // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }
  tft.setSwapBytes(true);
  delay(100);
  tft.setRotation(1);
  int i = 1;
  while (i--)
  {
    tft.fillScreen(TFT_RED);
    delay(100);
    tft.fillScreen(TFT_BLUE);
    delay(100);
    tft.fillScreen(TFT_GREEN);
    delay(100);
  }

  tft.fillScreen(TFT_BLACK);

  tft.setRotation(0);

  tft.setTextDatum(0);

  Serial.println("Adafruit MLX90640 Simple Test");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
  {
    Serial.println("MLX90640 not found!");
    while (1)
      delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  // mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS)
  {
    Serial.println("Chess");
  }
  else
  {
    Serial.println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC_16BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res)
  {
  case MLX90640_ADC_16BIT:
    Serial.println("16 bit");
    break;
  case MLX90640_ADC_17BIT:
    Serial.println("17 bit");
    break;
  case MLX90640_ADC_18BIT:
    Serial.println("18 bit");
    break;
  case MLX90640_ADC_19BIT:
    Serial.println("19 bit");
    break;
  }

  mlx.setRefreshRate(MLX90640_4_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate)
  {
  case MLX90640_0_5_HZ:
    Serial.println("0.5 Hz");
    break;
  case MLX90640_1_HZ:
    Serial.println("1 Hz");
    break;
  case MLX90640_2_HZ:
    Serial.println("2 Hz");
    break;
  case MLX90640_4_HZ:
    Serial.println("4 Hz");
    break;
  case MLX90640_8_HZ:
    Serial.println("8 Hz");
    break;
  case MLX90640_16_HZ:
    Serial.println("16 Hz");
    break;
  case MLX90640_32_HZ:
    Serial.println("32 Hz");
    break;
  case MLX90640_64_HZ:
    Serial.println("64 Hz");
    break;
  }
}

void setTempScale()
{
  minTemp = 255;
  maxTemp = 0;

  for (i = 0; i < 768; i++)
  {
    minTemp = min(minTemp, frame[i]);
    maxTemp = max(maxTemp, frame[i]);
  }
}

void setAbcd()
{
  a = minTemp + (maxTemp - minTemp) * 0.2121;
  b = minTemp + (maxTemp - minTemp) * 0.3182;
  c = minTemp + (maxTemp - minTemp) * 0.4242;
  d = minTemp + (maxTemp - minTemp) * 0.8182;
}

// Draw a legend.
void drawLegend()
{
  float inc = (maxTemp - minTemp) / 180;
  j = 0;
  for (ii = minTemp; ii < maxTemp; ii += inc)
  {
    tft.drawFastVLine( j++, 220,20, getColor(ii));
  }

 

  // tft.drawRightString(waarde, 45, 200, 4);
  // waarde = String(maxTemp, 1);
  //   tft.drawRightString("       ", 130, 200, 4);
  // tft.drawRightString(waarde, 130, 200, 4);

   tft.setTextFont(4);
  // tft.setTextSize(4);
   tft.setCursor(130, 165);
   tft.setTextColor(TFT_WHITE, TFT_BLACK,true);
   tft.print(String(minTemp,1));


 
  // tft.setTextSize(4);
   tft.setCursor(60, 190);
   tft.setTextColor(TFT_WHITE, TFT_BLACK,true);
   tft.print(String(maxTemp,1));

  // tft.setTextFont(NULL);
}

void loop()
{
 
  if (mlx.getFrame(frame) != 0)
  {
    Serial.println("Failed");
    return;
  }
  setTempScale();
  setAbcd();
  drawLegend();

  for (uint8_t h = 0; h < 24; h++)
  {
    for (uint8_t w = 0; w < 32; w++)
    {
      float t = frame[h * 32 + w];

  tft.fillRect( 6 + h * 5,6 + w * 5, 5, 5, getColor(t));
    }
    Serial.println();
  }
}