#include <WiFi.h>
#include <ESP_Mail_Client.h>

// Thông tin WiFi
#define WIFI_SSID "Hằng"
#define WIFI_PASSWORD "12345678"

// Thông tin Gmail SMTP
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "truongphuc01573@gmail.com"
#define AUTHOR_PASSWORD "hisx exye uwkv fnou"
#define RECIPIENT_EMAIL "truongphuc1573@gmail.com"

// Khai báo chân ESP32
#define FLAME_AO_PIN   35    // Cảm biến lửa
#define MQ135_PIN      34    // Cảm biến khói MQ135
#define RELAY1_PIN     25    // Relay còi
#define RELAY2_PIN     26    // Relay quạt hút
#define RELAY3_PIN     27    // Relay ngắt điện

const int FLAME_THRESHOLD = 1000;  // Ngưỡng phát hiện lửa
const int SMOKE_THRESHOLD = 200;   // Ngưỡng phát hiện khói

const unsigned long EMAIL_COOLDOWN = 60000; // 60s cooldown
unsigned long lastEmailSent = 0;

SMTPSession smtp;

void setup() {
  Serial.begin(115200);
  pinMode(FLAME_AO_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, LOW); // Ban đầu tắt còi
  digitalWrite(RELAY2_PIN, LOW); // Ban đầu tắt quạt
  digitalWrite(RELAY3_PIN, LOW); // Ban đầu không ngắt điện

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Wi-Fi đã kết nối!");

  Serial.println("==========================================");
  Serial.println(" HỆ THỐNG CẢNH BÁO LỬA VÀ KHÓI - ESP32");
  Serial.println(" Đang khởi động hệ thống...");
  Serial.print("  FLAME_THRESHOLD = ");
  Serial.println(FLAME_THRESHOLD);
  Serial.print("  SMOKE_THRESHOLD = ");
  Serial.println(SMOKE_THRESHOLD);
  Serial.println("==========================================\n");
  delay(5000);
}

void loop() {
  int flameAnalog = analogRead(FLAME_AO_PIN);
  int smokeAnalog = analogRead(MQ135_PIN);

  Serial.println("------------------------------------------");
  Serial.print("Giá trị cảm biến lửa (AO): ");
  Serial.println(flameAnalog);
  Serial.print("Giá trị cảm biến khói (AO): ");
  Serial.println(smokeAnalog);

  bool fireDetected = flameAnalog < FLAME_THRESHOLD;
  bool smokeDetected = smokeAnalog > SMOKE_THRESHOLD;

  if (fireDetected) {
    // Khi có lửa
    Serial.println(" Phát hiện LỬA!");
    digitalWrite(RELAY1_PIN, HIGH);  // Bật còi
    digitalWrite(RELAY2_PIN, LOW);   // Tắt quạt
    digitalWrite(RELAY3_PIN, HIGH);  // Ngắt điện

  } else if (smokeDetected) {
    // Khi chỉ có khói
    Serial.println("Phát hiện KHÓI!");
    digitalWrite(RELAY1_PIN, HIGH);  // Bật còi
    digitalWrite(RELAY2_PIN, HIGH);  // Bật quạt hút
    digitalWrite(RELAY3_PIN, LOW);   // Không ngắt điện
  } else {
    // Không có khói hoặc lửa
    Serial.println(" Không có lửa hoặc khói. Hệ thống an toàn.");
    digitalWrite(RELAY1_PIN, LOW);   // Tắt còi
    digitalWrite(RELAY2_PIN, LOW);   // Tắt quạt
    digitalWrite(RELAY3_PIN, LOW);   // Không ngắt điện
  }

  // Gửi email nếu cần
  if ((fireDetected || smokeDetected) && (millis() - lastEmailSent >= EMAIL_COOLDOWN)) {
    String message = fireDetected ? "PHÁT HIỆN LỬA!" : "PHÁT HIỆN KHÓI!";
    if (WiFi.status() == WL_CONNECTED) {
      sendEmail(message);
    } else {
      Serial.println(" Không gửi email được vì mất WiFi.");
    }
    lastEmailSent = millis();
  }

  delay(1500);
}

void sendEmail(String messageContent) {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message email;
  email.sender.name = "Hệ thống cảnh báo ESP32";
  email.sender.email = AUTHOR_EMAIL;
  email.subject = "CẢNH BÁO: " + messageContent;
  email.addRecipient("Người nhận", RECIPIENT_EMAIL);
  email.text.content = "Hệ thống ESP32 vừa phát hiện: " + messageContent + ". Địa chỉ: KM 10 Trần Phú, Mộ Lao, Hà Đông. Hãy kiểm tra ngay!";

  if (smtp.connect(&session)) {
    if (MailClient.sendMail(&smtp, &email)) {
      Serial.println(" Gửi email thành công!");
    } else {
      Serial.print(" Gửi email thất bại: ");
      Serial.println(smtp.errorReason());
    }
    smtp.closeSession();
  }
}
