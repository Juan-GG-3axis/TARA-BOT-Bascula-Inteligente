#include <SPI.h>
#include <TFT_eSPI.h> 
#include "HX711.h"

TFT_eSPI tft = TFT_eSPI(); 

const int LOADCELL_DOUT_PIN = 21;
const int LOADCELL_SCK_PIN = 22;
HX711 scale;

void setup() {
  // Iniciar pantalla
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM); 

  tft.setTextColor(TFT_WHITE);
  tft.drawString("Iniciando...", 120, 120, 4);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(2000);

  // Instrucción 1: Tara
  tft.fillScreen(TFT_BLACK);
  tft.drawString("1. Vacia la", 120, 100, 4);
  tft.drawString("bascula", 120, 140, 4);
  
  delay(3000); 
  scale.set_scale(); 
  scale.tare();      


  tft.fillScreen(TFT_BLACK);
  tft.drawString("2. Pon un peso", 120, 100, 4);
  tft.drawString("conocido", 120, 140, 4);
  delay(3000);
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Valor crudo:", 120, 60, 2);
}

void loop() {
  // Promedia 10 lecturas para mayor estabilidad
  long rawValue = scale.get_units(10);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  String valorPantalla = String(rawValue) + "    ";

  tft.drawString(valorPantalla, 120, 120, 4);
  
  delay(300);
}