void setupWiFi(void);
void setupSTA(void);
void setupDisplay(void);
void setupAP(void);
time_t getNtpTime(void);
void sendNTPpacket(WiFiUDP *u);