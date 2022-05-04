//#include <SPI.h>
//#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
//WiFiClientSecure is a big library. It can take a bit of time to do that first compile
const int SAMPLE_FREQ = 8000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 3;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip
const char PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\", \"speechContexts\":[{ \"phrases\":[],\"boost\":2}]}, \"audio\": {\"content\":\"";
const char SUFFIX[] = "\"}}"; //suffix to POST request
const int AUDIO_IN = 1; //pin where microphone is connected
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this
const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char*  SERVER = "speech.google.com";  // Server URL


class SpeechToText {

private:
    const char* CA_CERT = \
                        "-----BEGIN CERTIFICATE-----\n" \
                        "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
                        "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
                        "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
                        "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
                        "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
                        "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
                        "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
                        "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
                        "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
                        "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
                        "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
                        "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
                        "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
                        "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
                        "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
                        "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
                        "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
                        "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
                        "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
                        "-----END CERTIFICATE-----\n";
    //TFT_eSPI tft;// = TFT_eSPI();

    const int DELAY = 1000;

    const uint16_t RESPONSE_TIMEOUT = 6000;
    char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

    /* CONSTANTS */
    //Prefix to POST request:

    /* Global variables*/
    uint32_t time_since_sample;      // used for microsecond timing


    char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data

    uint8_t old_val;
    uint32_t timer;

    WiFiClientSecure client; //global WiFiClient Secure object

public:

    void setup() {
        timer = millis();
        client.setCACert(CA_CERT); //set cert for https
    }
    //main body of code
    char* run() {
        speech_to_text();
        Serial.printf("RS %s::\n", response);
        char res[100];
        memcpy(res, response, sizeof(res));
        return res;
    }

private:
    void speech_to_text() {
        Serial.println("listening...");
        record_audio();
        Serial.println("sending...");
        Serial.print("\nStarting connection to server...");
        delay(300);
        bool conn = false;
        for (int i = 0; i < 10; i++) {
        int val = (int)client.connect(SERVER, 443, 4000);
        Serial.print(i); Serial.print(": "); Serial.println(val);
        if (val != 0) {
            conn = true;
            break;
        }
        Serial.print(".");
        delay(300);
        }
        if (!conn) {
        Serial.println("Connection failed!");
        return;
        } else {
        Serial.println("Connected to server!");
        Serial.println(client.connected());
        int len = strlen(speech_data);
        // Make a HTTP request:
        client.print("POST /v1/speech:recognize?key="); client.print(API_KEY); client.print(" HTTP/1.1\r\n");
        client.print("Host: speech.googleapis.com\r\n");
        client.print("Content-Type: application/json\r\n");
        client.print("cache-control: no-cache\r\n");
        client.print("Content-Length: "); client.print(len);
        client.print("\r\n\r\n");
        int ind = 0;
        int jump_size = 1000;
        char temp_holder[jump_size + 10] = {0};
        Serial.println("sending data");
        while (ind < len) {
            delay(80);//experiment with this number!
            //if (ind + jump_size < len) client.print(speech_data.substring(ind, ind + jump_size));
            strncat(temp_holder, speech_data + ind, jump_size);
            client.print(temp_holder);
            ind += jump_size;
            memset(temp_holder, 0, sizeof(temp_holder));
        }
        client.print("\r\n");
        //Serial.print("\r\n\r\n");
        Serial.println("Through send...");
        unsigned long count = millis();
        while (client.connected()) {
            Serial.println("IN!");
            String line = client.readStringUntil('\n');
            Serial.print(line);
            if (line == "\r") { //got header of response
            Serial.println("headers received");
            break;
            }
            if (millis() - count > RESPONSE_TIMEOUT) break;
        }
        Serial.println("");
        Serial.println("Response...");
        count = millis();
        while (!client.available()) {
            delay(100);
            Serial.print(".");
            if (millis() - count > RESPONSE_TIMEOUT) break;
        }
        Serial.println();
        Serial.println("-----------");
        memset(response, 0, sizeof(response));
        while (client.available()) {
            char_append(response, client.read(), OUT_BUFFER_SIZE);
        }
        //Serial.println(response); //comment this out if needed for debugging
        char* trans_id = strstr(response, "transcript");
        if (trans_id != NULL) {
            char* foll_coll = strstr(trans_id, ":");
            char* starto = foll_coll + 2; //starting index
            char* endo = strstr(starto + 1, "\""); //ending index
            int transcript_len = endo - starto + 1;
            char transcript[100] = {0};
            strncat(transcript, starto, transcript_len);
            Serial.println(transcript);
            memcpy(response, transcript, sizeof(transcript));
        }
        Serial.println("-----------");
        client.stop();
        Serial.println("done");
        }
    }

    //function used to record audio at sample rate for a fixed nmber of samples
    void record_audio() {
        timer = millis();
        int sample_num = 0;    // counter for samples
        int enc_index = strlen(PREFIX) - 1;  // index counter for encoded samples
        float time_between_samples = 1000000 / SAMPLE_FREQ;
        int value = 0;
        char raw_samples[3];   // 8-bit raw sample data array
        memset(speech_data, 0, sizeof(speech_data));
        sprintf(speech_data, "%s", PREFIX);
        char holder[5] = {0};
        Serial.println("starting");
        uint32_t text_index = enc_index;
        uint32_t start = millis();
        time_since_sample = micros();
        while (sample_num < NUM_SAMPLES && millis()-timer<5000) { //read in NUM_SAMPLES worth of audio data
            value = analogRead(AUDIO_IN);  //make measurement
            raw_samples[sample_num % 3] = mulaw_encode(value - 1800); //remove 1.5ishV offset (from 12 bit reading)
            sample_num++;
            if (sample_num % 3 == 0) {
            base64_encode(holder, raw_samples, 3);
            strncat(speech_data + text_index, holder, 4);
            text_index += 4;
            }
            // wait till next time to read
            while (micros() - time_since_sample <= time_between_samples); //wait...
            time_since_sample = micros();
        }
        Serial.println(millis() - start);
        sprintf(speech_data + strlen(speech_data), "%s", SUFFIX);
        Serial.println("out");
    }


    int8_t mulaw_encode(int16_t sample){
        const uint16_t MULAW_MAX = 0x1FFF;
        const uint16_t MULAW_BIAS = 33;
        uint16_t mask = 0x1000;
        uint8_t sign = 0;
        uint8_t position = 12;
        uint8_t lsb = 0;
        if (sample < 0)
        {
            sample = -sample;
            sign = 0x80;
        }
        sample += MULAW_BIAS;
        if (sample > MULAW_MAX)
        {
            sample = MULAW_MAX;
        }
        for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
                ;
        lsb = (sample >> (position - 4)) & 0x0f;
        return (~(sign | ((position - 5) << 4) | lsb));
    }



    // ========================= SUPPORT FUNCTIONS ==============================
    /* 'Private' declarations */
    /*inline void a3_to_a4(unsigned char * a4, unsigned char * a3);
    inline void a4_to_a3(unsigned char * a3, unsigned char * a4);
    inline unsigned char b64_lookup(char c);*/

    int base64_encode(char *output, char *input, int inputLen) {
    int i = 0, j = 0;
    int encLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    while(inputLen--) {
        a3[i++] = *(input++);
        if(i == 3) {
        a3_to_a4(a4, a3);

        for(i = 0; i < 4; i++) {
            output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
        }

        i = 0;
        }
    }

    if(i) {
        for(j = i; j < 3; j++) {
        a3[j] = '\0';
        }

        a3_to_a4(a4, a3);

        for(j = 0; j < i + 1; j++) {
        output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
        }

        while((i++ < 3)) {
        output[encLen++] = '=';
        }
    }
    //  output[encLen] = '\0';
    return encLen;
    }

    int base64_decode(char * output, char * input, int inputLen) {
    int i = 0, j = 0;
    int decLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];


    while (inputLen--) {
        if(*input == '=') {
        break;
        }

        a4[i++] = *(input++);
        if (i == 4) {
        for (i = 0; i <4; i++) {
            a4[i] = b64_lookup(a4[i]);
        }

        a4_to_a3(a3,a4);

        for (i = 0; i < 3; i++) {
            output[decLen++] = a3[i];
        }
        i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
        a4[j] = '\0';
        }

        for (j = 0; j <4; j++) {
        a4[j] = b64_lookup(a4[j]);
        }

        a4_to_a3(a3,a4);

        for (j = 0; j < i - 1; j++) {
        output[decLen++] = a3[j];
        }
    }
    output[decLen] = '\0';
    return decLen;
    }

    int base64_enc_len(int plainLen) {
    int n = plainLen;
    return (n + 2 - ((n + 2) % 3)) / 3 * 4;
    }

    int base64_dec_len(char * input, int inputLen) {
    int i = 0;
    int numEq = 0;
    for(i = inputLen - 1; input[i] == '='; i--) {
        numEq++;
    }

    return ((6 * inputLen) / 8) - numEq;
    }

    inline void a3_to_a4(unsigned char * a4, unsigned char * a3) {
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
    }

    inline void a4_to_a3(unsigned char * a3, unsigned char * a4) {
    a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
    }

    inline unsigned char b64_lookup(char c) {
    if(c >='A' && c <='Z') return c - 'A';
    if(c >='a' && c <='z') return c - 71;
    if(c >='0' && c <='9') return c + 4;
    if(c == '+') return 62;
    if(c == '/') return 63;
    return -1;
    }


    uint8_t char_append(char* buff, char c, uint16_t buff_size) {
    int len = strlen(buff);
    if (len > buff_size) return false;
    buff[len] = c;
    buff[len + 1] = '\0';
    return true;
    }
};