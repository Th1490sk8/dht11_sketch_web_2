#include <WiFi.h>
#include <DHT.h>

const char* ssid = "iPhone";
const char* password = "r2d2c3po";

#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  delay(2000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.readStringUntil('\n');

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Rota /data sem autenticação
    if (req.indexOf("/data") >= 0) {
      String json = "{\"humidity\":" + String(h, 2) + ",\"temperature\":" + String(t, 2) + "}";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(json);
      client.stop();
      return;
    }

    // Página HTML inspirada no CPTEC
    String html = "<!DOCTYPE html><html lang='pt-br'><head><meta charset='UTF-8'><title>Estação Meteorológica</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
    html += "<style>";
    html += "body{font-family:'Segoe UI',sans-serif;background:#f4f6f8;color:#333;margin:0;padding:20px;text-align:center;}";
    html += "header{margin-bottom:20px;}h1{font-size:24px;color:#2c3e50;}p{max-width:600px;margin:auto;font-size:14px;color:#555;}";
    html += "canvas{margin:20px auto;display:block;width:220px;height:220px;}footer{font-size:12px;color:#888;margin-top:30px;}";
    html += "</style></head><body>";
    html += "<header><h1>Estação Meteorológica Local</h1><p>Dados em tempo real - ESP32 + DHT11</p></header>";
    html += "<p><strong>Temperatura</strong> indica o grau de calor do ambiente, enquanto <strong>umidade</strong> representa a quantidade de vapor de água presente no ar. Monitorar esses dados é essencial para conforto térmico, saúde e controle de ambientes sensíveis como estufas, laboratórios e residências.</p>";
    html += "<canvas id='dhtChart'></canvas>";
    html += "<footer>Última atualização: <span id='hora'></span> | Fonte: Sensor DHT11</footer>";
    html += "<script>";
    html += "const ctx = document.getElementById('dhtChart').getContext('2d');";
    html += "const dhtChart = new Chart(ctx, {type:'doughnut',data:{labels:['Umidade (%)','Temperatura (°C)'],datasets:[{data:[0,0],backgroundColor:['#3498db','#2ecc71'],borderWidth:1}]},options:{plugins:{legend:{position:'bottom'},title:{display:true,text:'Proporção Atual'},tooltip:{callbacks:{label:function(context){return context.label + ': ' + context.formattedValue;}}}},responsive:true}});";
    html += "setInterval(() => {fetch('/data').then(res => res.json()).then(data => {dhtChart.data.datasets[0].data = [data.humidity, data.temperature]; dhtChart.update();});}, 5000);";
    html += "setInterval(() => {document.getElementById('hora').textContent = new Date().toLocaleTimeString();}, 1000);";
    html += "</script></body></html>";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);
    client.stop();
  }
}