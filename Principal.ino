#include <SPI.h>
#include <TFT_eSPI.h> 
#include "HX711.h"
#include <Preferences.h>

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite spr = TFT_eSprite(&tft);

uint16_t colorPiel;
uint16_t colorSombra;
uint16_t colorRojoOscuro;

const int LOADCELL_DOUT_PIN = 21;
const int LOADCELL_SCK_PIN = 22;
HX711 scale;

Preferences preferencias;
#define BTN_PIN 32

struct MarcaFilamento {
  String nombre;
  int peso;
};
// ==========================================
// TARAS CONOCIDAS 
// ==========================================
MarcaFilamento taras[] = {
  {"Sin Tara", 0},
  {"Sunlu", 147},     // <--- Modifica el nombre y peso del carrete vacío en gramos
  {"Creality", 150},
  {"eSUN", 220},      // <--- A partir de aquí puedes agregar más marcas, siguiendo esta estructura {"Marca", Peso(g)},
  {"Generico", 200}
};

int totalTaras = sizeof(taras) / sizeof(taras[0]);
int indiceTara = 0;
int taraGuardada = 0;

enum Estado { MENU_TARA, MODO_PESAJE, DORMIDO };
Estado estadoActual = MENU_TARA;

unsigned long tiempoPresionado = 0;
unsigned long ultimoClic = 0;
unsigned long tiempoEnCero = 0;
unsigned long ultimaLecturaPeso = 0;
bool botonEstabaPresionado = false;

float pesoNetoActual = 0;

void mostrarMenuTara();
void actualizarPesaje();
void ejecutarModoDormido();
void leerPesoPeriodico();



void dibujarOjoGrande(int x, int y) {
  int ancho = 36;
  int alto = 85;
  int radio = 18;
  spr.fillRoundRect(x, y, ancho, alto, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 5, alto - 5, radio - 1, colorPiel);
}

void dibujarOjoCerradoGrande(int x, int y) {
  int ancho = 36;
  int altoLinea = 12; 
  int radio = 6;
  int centroY = y + 40; 
  spr.fillRoundRect(x, centroY, ancho, altoLinea, radio, colorSombra);
  spr.fillRoundRect(x, centroY, ancho - 4, altoLinea - 4, radio - 1, colorPiel);
}

void dibujarOjoLuna(int x, int y) {
  int ancho = 36;
  int alto = 50;  
  int radio = 18; 
  spr.fillRoundRect(x, y, ancho, alto, radio, colorPiel);
  int radioCorte = 13; 
  int centroX = x + 16; 
  int centroY = y + alto - 8; 
  spr.fillCircle(centroX, centroY, radioCorte, colorSombra);
  spr.fillCircle(centroX, centroY + 4, radioCorte, TFT_BLACK);
  spr.fillRect(x - 2, y + alto, ancho + 10, 20, TFT_BLACK);
}

void dibujarOjoVariable(int x, int y, int alto) {
  int ancho = 36;
  int radio = (alto < 36) ? (alto / 2) : 18; 
  spr.fillRoundRect(x, y, ancho, alto, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 5, alto - 5, radio - 1, colorPiel);
}

void dibujarOjoPreocupado(int x, int y, bool esIzquierdo) {
  int ancho = 36;
  int alto = 85;
  int radio = 18;
  spr.fillRoundRect(x, y, ancho, alto, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 5, alto - 5, radio - 1, colorPiel);
  int caida = 30; 
  if (esIzquierdo) {
    spr.fillTriangle(x - 10, y - 10, x + ancho + 10, y - 10, x - 10, y + caida, TFT_BLACK);
  } else {
    spr.fillTriangle(x - 10, y - 10, x + ancho + 10, y - 10, x + ancho + 10, y + caida, TFT_BLACK);
  }
}

void dibujarOjoSerio(int x, int y) {
  int ancho = 36;
  int alto = 85;
  int radio = 18;
  spr.fillRoundRect(x, y, ancho, alto, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 5, alto - 5, radio - 1, colorPiel);
  int corteAltura = 35; 
  spr.fillRect(x - 5, y - 5, ancho + 10, corteAltura, TFT_BLACK);
}

void dibujarOjoTriste(int x, int y, bool esIzquierdo) {
  int ancho = 48; 
  int alto = 42;  
  int radio = 18;
  x = x - 6; y = y + 22; 
  spr.fillRoundRect(x, y, ancho, alto, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 5, alto - 5, radio - 1, colorPiel);
  int caida = 32; 
  if (esIzquierdo) {
    spr.fillTriangle(x - 10, y - 10, x + ancho + 15, y - 10, x - 10, y + caida, TFT_BLACK);
  } else {
    spr.fillTriangle(x - 15, y - 10, x + ancho + 10, y - 10, x + ancho + 10, y + caida, TFT_BLACK);
  }
}

void dibujarOjoCerradoTriste(int x, int y) {
  int ancho = 48; 
  int altoLinea = 12; 
  int radio = 6;
  x = x - 6; y = y + 22; 
  int centroY = y + 15; 
  spr.fillRoundRect(x, centroY, ancho, altoLinea, radio, colorSombra);
  spr.fillRoundRect(x, centroY, ancho - 4, altoLinea - 4, radio - 1, colorPiel);
}

void dibujarOjoEquis(int x, int y) {
  int tamano = 30; 
  int grosor = 5; 
  spr.drawWedgeLine(x, y, x + tamano, y + tamano, grosor, grosor, colorRojoOscuro);
  spr.drawWedgeLine(x + tamano, y, x, y + tamano, grosor, grosor, colorRojoOscuro);
  spr.drawWedgeLine(x - 3, y - 3, x + tamano - 3, y + tamano - 3, grosor, grosor, TFT_RED);
  spr.drawWedgeLine(x + tamano - 3, y - 3, x - 3, y + tamano - 3, grosor, grosor, TFT_RED);
}

void dibujarOjoDormido(int x, int y) {
  int ancho = 52; 
  int altoLinea = 12; 
  int radio = 6;
  spr.fillRoundRect(x, y, ancho, altoLinea, radio, colorSombra);
  spr.fillRoundRect(x, y, ancho - 4, altoLinea - 4, radio - 1, colorPiel);
}

void dibujarZGeometrica(int x, int y, int escala) {
  int ancho = 5 * escala;
  int alto = 6 * escala;
  int grosor = 1 + (1 * escala);
  int off = 2; 
  
  spr.drawWedgeLine(x + off, y + off, x + ancho + off, y + off, grosor, grosor, colorSombra);
  spr.drawWedgeLine(x + ancho + off, y + off, x + off, y + alto + off, grosor, grosor, colorSombra);
  spr.drawWedgeLine(x + off, y + alto + off, x + ancho + off, y + alto + off, grosor, grosor, colorSombra);
  
  spr.drawWedgeLine(x, y, x + ancho, y, grosor, grosor, colorPiel);
  spr.drawWedgeLine(x + ancho, y, x, y + alto, grosor, grosor, colorPiel);
  spr.drawWedgeLine(x, y + alto, x + ancho, y + alto, grosor, grosor, colorPiel);
}



void dibujarFaseFeliz(int offsetX, int offsetY) {
  unsigned long tiempo = millis();
  bool parpadeo = (tiempo % 4000) < 150;
  bool esLuna = (tiempo % 30000) >= 15000;
  int baseOjoIzqX = 63, baseOjoDerX = 141, baseOjoY = 65, baseLunaY = 75; 
  if (esLuna) {
    int vibracionX = ((tiempo % 4000) < 400) ? (((tiempo % 120) < 60) ? 3 : -3) : 0;
    dibujarOjoLuna(baseOjoIzqX + offsetX + vibracionX, baseLunaY + offsetY);
    dibujarOjoLuna(baseOjoDerX + offsetX + vibracionX, baseLunaY + offsetY);
  } else {
    if (parpadeo) {
      dibujarOjoCerradoGrande(baseOjoIzqX + offsetX, baseOjoY + offsetY);
      dibujarOjoCerradoGrande(baseOjoDerX + offsetX, baseOjoY + offsetY);
    } else {
      dibujarOjoGrande(baseOjoIzqX + offsetX, baseOjoY + offsetY);
      dibujarOjoGrande(baseOjoDerX + offsetX, baseOjoY + offsetY);
    }
  }
}

void dibujarFaseMolesto(int offsetX, int offsetY) {
  unsigned long tiempo = millis();
  bool ticDerecho = ((tiempo / 15000) % 2) == 0;
  int baseOjoIzqX = 63, baseOjoDerX = 141, baseOjoY = 65, altoOjoGrande = 85; 
  int altoTic = 50; 
  if ((tiempo % 3000) < 400 && (tiempo % 100) < 50) altoTic = 25; 
  int yCentrado = baseOjoY + ((altoOjoGrande - altoTic) / 2);
  if (ticDerecho) {
    dibujarOjoGrande(baseOjoIzqX + offsetX, baseOjoY + offsetY);
    dibujarOjoVariable(baseOjoDerX + offsetX, yCentrado + offsetY, altoTic);
  } else {
    dibujarOjoVariable(baseOjoIzqX + offsetX, yCentrado + offsetY, altoTic);
    dibujarOjoGrande(baseOjoDerX + offsetX, baseOjoY + offsetY);
  }
}

void dibujarFasePreocupado(int offsetX, int offsetY) {
  unsigned long tiempo = millis();
  bool parpadeo = (tiempo % 4000) < 150; 
  int baseOjoIzqX = 63, baseOjoDerX = 141, baseOjoY = 65; 
  if (parpadeo) {
    dibujarOjoCerradoGrande(baseOjoIzqX + offsetX, baseOjoY + offsetY);
    dibujarOjoCerradoGrande(baseOjoDerX + offsetX, baseOjoY + offsetY);
  } else {
    dibujarOjoPreocupado(baseOjoIzqX + offsetX, baseOjoY + offsetY, true);
    dibujarOjoPreocupado(baseOjoDerX + offsetX, baseOjoY + offsetY, false);
  }
}

void dibujarFaseSerio(int offsetX, int offsetY) {
  unsigned long tiempo = millis();
  bool parpadeo = (tiempo % 4000) < 150; 
  int baseOjoIzqX = 63, baseOjoDerX = 141, baseOjoY = 65; 
  if (parpadeo) {
    dibujarOjoCerradoGrande(baseOjoIzqX + offsetX, baseOjoY + offsetY);
    dibujarOjoCerradoGrande(baseOjoDerX + offsetX, baseOjoY + offsetY);
  } else {
    dibujarOjoSerio(baseOjoIzqX + offsetX, baseOjoY + offsetY);
    dibujarOjoSerio(baseOjoDerX + offsetX, baseOjoY + offsetY);
  }
}

void dibujarFaseTriste(int offsetX, int offsetY) {
  unsigned long tiempo = millis();
  bool parpadeo = (tiempo % 4000) < 150; 
  int baseOjoIzqX = 63, baseOjoDerX = 141, baseOjoY = 65; 
  if (parpadeo) {
    dibujarOjoCerradoTriste(baseOjoIzqX + offsetX, baseOjoY + offsetY);
    dibujarOjoCerradoTriste(baseOjoDerX + offsetX, baseOjoY + offsetY);
  } else {
    dibujarOjoTriste(baseOjoIzqX + offsetX, baseOjoY + offsetY, true);
    dibujarOjoTriste(baseOjoDerX + offsetX, baseOjoY + offsetY, false);
  }
}

void dibujarFaseMuerto() {
  unsigned long tiempo = millis();
  bool encendido = (tiempo % 1000) < 600; 
  int baseOjoIzqX = 60, baseOjoDerX = 138, baseOjoY = 85; 
  if (encendido) {
    dibujarOjoEquis(baseOjoIzqX, baseOjoY);
    dibujarOjoEquis(baseOjoDerX, baseOjoY);
  }
}

void dibujarFaseDormido() {
  int baseOjoIzqX = 52; 
  int baseOjoDerX = 136; 
  int baseOjoY = 120; 
  
  dibujarOjoDormido(baseOjoIzqX, baseOjoY);
  dibujarOjoDormido(baseOjoDerX, baseOjoY);
  
  unsigned long tiempo = millis();
  int pasoAnimacion = (tiempo / 600) % 4; 
  
  if (pasoAnimacion >= 1) {
    dibujarZGeometrica(110, 85, 1);
  }
  if (pasoAnimacion >= 2) {
    dibujarZGeometrica(135, 50, 2);
  }
  if (pasoAnimacion >= 3) {
    dibujarZGeometrica(165, 10, 3);
  }
}



void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  tft.init();
  tft.setRotation(0); 
  tft.fillScreen(TFT_BLACK);

  colorPiel = tft.color565(250, 175, 130);   
  colorSombra = tft.color565(210, 100, 40);  
  colorRojoOscuro = tft.color565(120, 0, 0); 

  spr.setColorDepth(8);
  spr.createSprite(240, 240);
  spr.setTextDatum(MC_DATUM); 

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(430.3); // <--- CAMBIA EL 430.3 POR TU FACTOR DE CALIBRACIÓN
  
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE);
  spr.drawString("Calibrando...", 120, 120, 4); 
  spr.pushSprite(0, 0);
  
  delay(1000);
  scale.tare(); 

  preferencias.begin("bascula", false);
  taraGuardada = preferencias.getInt("tara", -1); 

  if (taraGuardada != -1) {
    estadoActual = MODO_PESAJE;
  } else {
    taraGuardada = 0;
    mostrarMenuTara();
  }
}


void loop() {
  bool lectura = (digitalRead(BTN_PIN) == LOW);

  if (lectura && !botonEstabaPresionado && (millis() - ultimoClic > 150)) {
    tiempoPresionado = millis();
    botonEstabaPresionado = true;
    
    if (estadoActual == DORMIDO) {
      estadoActual = MODO_PESAJE;
      tiempoEnCero = 0;
    }
  } 
  else if (!lectura && botonEstabaPresionado && (millis() - tiempoPresionado > 50)) {
    unsigned long duracion = millis() - tiempoPresionado;
    botonEstabaPresionado = false;
    ultimoClic = millis();

    if (duracion < 500 && estadoActual == MENU_TARA) {
      indiceTara = (indiceTara + 1) % totalTaras;
      mostrarMenuTara();
    } 
    else if (duracion > 1500 && (estadoActual == MODO_PESAJE || estadoActual == DORMIDO)) {
      estadoActual = MENU_TARA;
      indiceTara = 0;
      ultimoClic = millis();
      preferencias.putInt("tara", -1);
      mostrarMenuTara();
    }
  }

  switch (estadoActual) {
    case MENU_TARA:
      if (millis() - ultimoClic > 3000 && ultimoClic != 0) {
        taraGuardada = taras[indiceTara].peso;
        preferencias.putInt("tara", taraGuardada);
        estadoActual = MODO_PESAJE;
        tiempoEnCero = 0;
      }
      break;

    case MODO_PESAJE:
      actualizarPesaje();
      break;

    case DORMIDO:
      ejecutarModoDormido();
      break;
  }
}



void leerPesoPeriodico() {
  if (millis() - ultimaLecturaPeso > 200) {
    if (scale.is_ready()) {
      float pesoBruto = scale.get_units(1);
      pesoNetoActual = (pesoNetoActual * 0.6) + ((pesoBruto - taraGuardada) * 0.4);
    }
    ultimaLecturaPeso = millis();
  }
}

void mostrarMenuTara() {
  spr.fillSprite(TFT_BLACK);
  
  spr.setTextColor(TFT_YELLOW);
  spr.drawString("Rollo:", 120, 70, 4);
  
  spr.setTextColor(TFT_WHITE);
  spr.drawString(taras[indiceTara].nombre, 120, 120, 4);
  
  spr.setTextColor(TFT_CYAN);
  spr.drawString(String(taras[indiceTara].peso) + "g", 120, 170, 4);
  
  spr.pushSprite(0, 0);
}

void actualizarPesaje() {
  leerPesoPeriodico();
  
  int pesoRedondeado = round(pesoNetoActual);
  int pesoLimitado = constrain(pesoRedondeado, 0, 1000);
  
  spr.fillSprite(TFT_BLACK);

  unsigned long tiempoActual = millis();
  unsigned long faseMirada = tiempoActual % 15000;
  int offsetX = 0;
  int offsetY = 0;

  if (faseMirada < 3000)       { offsetX = 0;   offsetY = 0; } 
  else if (faseMirada < 4500)  { offsetX = 30;  offsetY = 0; } 
  else if (faseMirada < 6000)  { offsetX = -30; offsetY = 0; } 
  else if (faseMirada < 7500)  { offsetX = -25; offsetY = 30; } 
  else if (faseMirada < 9000)  { offsetX = 0;   offsetY = 35; } 
  else if (faseMirada < 10500) { offsetX = 20;  offsetY = -25; } 
  else if (faseMirada < 12000) { offsetX = -20; offsetY = -25; } 
  else                         { offsetX = 0;   offsetY = 0; }

  if (pesoLimitado >= 500) {
    dibujarFaseFeliz(offsetX, offsetY);       // 1000g a 500g
  } else if (pesoLimitado >= 400) {
    dibujarFaseMolesto(offsetX, offsetY);     // 499g a 400g
  } else if (pesoLimitado >= 300) {
    dibujarFasePreocupado(offsetX, offsetY);  // 399g a 300g
  } else if (pesoLimitado >= 200) {
    dibujarFaseSerio(offsetX, offsetY);       // 299g a 200g
  } else if (pesoLimitado >= 51) {
    dibujarFaseTriste(offsetX, offsetY);      // 199g a 51g
  } else {
    dibujarFaseMuerto();                      // 50g a 0g
  }


  int grados = map(pesoLimitado, 0, 1000, 0, 359);
  uint16_t colorBarra = (pesoLimitado < 200) ? TFT_RED : TFT_CYAN;

  if (grados > 0) {
    int anguloFinal = (180 + grados) % 360;
    spr.drawSmoothArc(120, 120, 120, 105, 180, anguloFinal, colorBarra, TFT_BLACK, true);
  }

  spr.setTextColor(TFT_WHITE);
  int pesoMostrar = (abs(pesoRedondeado) <= 2) ? 0 : pesoRedondeado;
  spr.drawString(String(pesoMostrar) + "g", 120, 202, 4);

  spr.pushSprite(0, 0);

  if (abs(pesoNetoActual) <= 2.0) {
    if (tiempoEnCero == 0) {
      tiempoEnCero = millis();
    } else if (millis() - tiempoEnCero > 180000UL) {
      estadoActual = DORMIDO;
    }
  } else {
    tiempoEnCero = 0;
  }
}

void ejecutarModoDormido() {
  leerPesoPeriodico();

  if (abs(pesoNetoActual) > 5.0) {
    estadoActual = MODO_PESAJE;
    tiempoEnCero = 0;
    return;
  }

  spr.fillSprite(TFT_BLACK);
  
  dibujarFaseDormido();
  
  spr.pushSprite(0, 0);
}