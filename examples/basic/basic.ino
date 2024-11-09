#include "IOT47_ESP_PDM_AUDIO.h"
#include "audi.h"

void setup() 
{
 Serial.begin(115200);
 pdm_init(23);
}

void loop() 
{
  if(!audio_isPlaying())
  {
    delay(1000);
    audio_play(tieng_chuong,sizeof(tieng_chuong));
  }
}
