#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "MQTTClient.h"
#include "MQTTFreertos.h"

static unsigned char sendBuf[1000];
static unsigned char readBuf[1000];

void messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}
static Network network;
static void MQTT_DemoTask(void *pvParameters) 
{
	printf("Starting ...\n");
	int rc, count = 0;
	MQTTClient client;

	NetworkInit(&network);
	printf("NetworkConnect  ...\n");
begin:	
	#if MQTT_OVER_SSL
	NetworkConnect(&network, "iot-mqtt-factory.ecoflow.com", 8883);
	#else
	NetworkConnect(&network, "10.0.0.42", 1883);
	#endif
	printf("MQTTClientInit  ...\n");
	MQTTClientInit(&client, &network, 2000, sendBuf, sizeof(sendBuf), readBuf, sizeof(readBuf));

	MQTTString clientId = MQTTString_initializer;
	#if MQTT_OVER_SSL
	clientId.cstring = "M201ZJH1ZE4L0013";
	#else
	clientId.cstring = "bearpi";
	#endif

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  	data.clientID          = clientId;
	data.willFlag          = 0;
	data.MQTTVersion       = 3;
	data.keepAliveInterval = 0;
	data.cleansession      = 1;
	#if MQTT_OVER_SSL
	data.username.cstring  = "device-90b337b0ab3b4f7e853e3cfe6545fa48";
	data.password.cstring  = "43e7657e7de640658f0b3b9e94b7447e";
	#endif

	printf("MQTTConnect  ...\n");
	rc = MQTTConnect(&client, &data);
	if (rc != 0) {
		printf("MQTTConnect: %d\n", rc);
		MQTTDisconnect(&client);
		FreeRTOS_disconnect(&network);
		vTaskDelay(200);
		goto begin;
	}

	printf("MQTTSubscribe  ...\n");
	rc = MQTTSubscribe(&client, "pubtopic", 2, messageArrived);
	if (rc != 0) {
		printf("MQTTSubscribe: %d\n", rc);
		vTaskDelay(200);
		goto begin;
	}
	while (++count)
	{
		MQTTMessage message;
		char payload[30];

		message.qos = 2;
		message.retained = 0;
		message.payload = payload;
		sprintf(payload, "message number %d", count);
		message.payloadlen = strlen(payload);
		
		if ((rc = MQTTPublish(&client, "pubtopic", &message)) != 0){
			printf("Return code from MQTT publish is %d\n", rc);
			MQTTDisconnect(&client);
			FreeRTOS_disconnect(&network); 
			goto begin; 
		}
		
		//vTaskDelay(50);	 
		MQTTYield(&client, 200); 
	}
}
void mqtt_main_entry(void)
{
    static StackType_t proc_main_stack[1024*2];
    static StaticTask_t proc_main_task;
    xTaskCreateStatic(MQTT_DemoTask, (char*)"MQTT_DemoTask", 1024*2, NULL, 25, proc_main_stack, &proc_main_task);
}
