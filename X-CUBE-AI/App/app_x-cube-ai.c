#include "app_x-cube-ai.h"
#include "main.h"
#include "math.h"
#include "ai_datatypes_defines.h"
#include "sine_model.h"
#include "sine_model_data.h"

/* IO buffers */
AI_ALIGNED(4) ai_i8 data_in_1[AI_SINE_MODEL_IN_1_SIZE_BYTES];
ai_i8* data_ins[AI_SINE_MODEL_IN_NUM] = { data_in_1 };

AI_ALIGNED(4) ai_i8 data_out_1[AI_SINE_MODEL_OUT_1_SIZE_BYTES];
ai_i8* data_outs[AI_SINE_MODEL_OUT_NUM] = { data_out_1 };

/* Activations buffers */
AI_ALIGNED(32) static uint8_t pool0[AI_SINE_MODEL_DATA_ACTIVATION_1_SIZE];
ai_handle data_activations0[] = { pool0 };

/* AI objects */
static ai_handle sine_model = AI_HANDLE_NULL;
static ai_buffer* ai_input;
static ai_buffer* ai_output;

extern UART_HandleTypeDef huart2;  // Usa huart2

int angle=0;

static void ai_log_err(const ai_error err, const char *fct)
{
  char buf[100];
  int buf_len = sprintf(buf, "Error (%s) - type=0x%02x code=0x%02x\r\n", fct, err.type, err.code);
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);  // Usa huart2 per la trasmissione
}

static int ai_boostrap(ai_handle *act_addr)
{
  ai_error err;

  err = ai_sine_model_create_and_init(&sine_model, act_addr, NULL);
  if (err.type != AI_ERROR_NONE) {
    ai_log_err(err, "ai_sine_model_create_and_init");
    return -1;
  }

  ai_input = ai_sine_model_inputs_get(sine_model, NULL);
  ai_output = ai_sine_model_outputs_get(sine_model, NULL);

  for (int idx = 0; idx < AI_SINE_MODEL_IN_NUM; idx++) {
    ai_input[idx].data = data_ins[idx];
  }

  for (int idx = 0; idx < AI_SINE_MODEL_OUT_NUM; idx++) {
    ai_output[idx].data = data_outs[idx];
  }

  return 0;
}

static int ai_run(void)
{
  ai_i32 batch;
  batch = ai_sine_model_run(sine_model, ai_input, ai_output);
  if (batch != 1) {
    ai_log_err(ai_sine_model_get_error(sine_model), "ai_sine_model_run");
    return -1;
  }
  return 0;
}

int acquire_and_process_data(ai_i8* data[])
{

  float radians = angle * (M_PI / 180.0);
  ((ai_float*)data[0])[0] = radians;

  angle = (angle + 1) % 360;
  return 0;
}

int post_process(ai_i8* data[])
{
  float y_val = ((ai_float*)data[0])[0];
  char buf[50];
  int buf_len = sprintf(buf, "Seno(%d gradi) = %f\r\n", angle, y_val);
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);  // Usa huart2
  return 0;
}

void MX_X_CUBE_AI_Init(void)
{
  printf("AI Model Initialization\r\n");
  ai_boostrap(data_activations0);
}

void MX_X_CUBE_AI_Process(void)
{
  int res = -1;

  if (sine_model) {
//    do {
      res = acquire_and_process_data(data_ins);  // Prepara i dati di input
      if (res == 0)
        res = ai_run();  // Esegue l'inferenza
      if (res == 0)
        res = post_process(data_outs);  // Elabora i risultati

      // Incrementa l'angolo per il prossimo ciclo
          angle += 1;  // Incrementa di 30 gradi (o usa 0.1 rad per incrementi in radianti)
          if (angle >= 360) {
            angle = 0;  // Resetta l'angolo dopo 360 gradi
          }

//    } while (res == 0);
  }

  if (res) {
    ai_error err = {AI_ERROR_INVALID_STATE, AI_ERROR_CODE_NETWORK};
    ai_log_err(err, "Process has FAILED");
  }
}
