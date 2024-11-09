#ifndef __IOT47_DPM_AUDIO_
#define __IOT47_DPM_AUDIO_

/*thư viện dpm audio
 Người viết: Đào nguyện iot47
 Email: daonguyen20798@gmail.com
 Website: iot47.com
*/

#include "esp_system.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "esp_log.h"

int pdm_pin = 23;
int play_audio = 0;
const uint8_t *audio_data;
uint32_t audio_data_length;
uint32_t startAudioCount;
hw_timer_t *My_timer = NULL;

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (0) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (150000*2) // Frequency in Hertz. Set frequency at 5 kHz

void ledCEnablePin()
{
   ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel     = LEDC_CHANNEL;
    ledc_channel.timer_sel     = LEDC_TIMER;
    ledc_channel.intr_type     = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num     = pdm_pin;
    ledc_channel.duty     = 0;
    ledc_channel.hpoint     = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}
void ledCDisablePin()
{
    ledc_stop(LEDC_MODE,LEDC_CHANNEL,0);
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = 1ULL<<pdm_pin;
    io_conf.mode = GPIO_MODE_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}
void IRAM_ATTR timer_group_isr_callback()
{
  if(play_audio==1)
  {
    uint8_t data=audio_data[startAudioCount];
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, data);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    startAudioCount++;
    if(startAudioCount>=audio_data_length)
    {
      play_audio=0;
      ledCDisablePin();
      timerAlarmDisable(My_timer);
    }
  }
}
void pdm_init(int pin)
{
  pdm_pin = pin;

  ledc_timer_config_t ledc_timer;
  ledc_timer.speed_mode       = LEDC_MODE;
  ledc_timer.timer_num       = LEDC_TIMER;
  ledc_timer.duty_resolution       = LEDC_DUTY_RES;
  ledc_timer.freq_hz       = LEDC_FREQUENCY;
  ledc_timer.clk_cfg       = LEDC_AUTO_CLK;
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledCEnablePin();

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &timer_group_isr_callback, true);
  timerAlarmWrite(My_timer, 62, true);

  ledCDisablePin();
}
void audio_play(const uint8_t *wav,uint32_t len)
{
  if(play_audio)timerAlarmDisable(My_timer);
  
  startAudioCount=0;
  audio_data = wav;
  audio_data_length = len;
  play_audio=1;
  ledCEnablePin();
  timerAlarmEnable(My_timer);
}

int audio_isPlaying()
{
    return play_audio;
}

#endif