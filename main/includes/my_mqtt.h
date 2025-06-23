typedef int (*formatter_t)(char *buf, size_t buf_len, const void *value);

int format_float(char *buf, size_t buf_len, const void *value);
int format_uint16(char *buf, size_t buf_len, const void *value);

void mqtt_init();
void sendMQTTpayload(const char *topic, const void *value, formatter_t formatter);