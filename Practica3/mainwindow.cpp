#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // REGLA ANTIPLAGIO: Título de ventana con Nombre y Código
    this->setWindowTitle("Practica 3 | Luis Yafett Gonzalez Montes - 223822954");
    const QString ALUMNO = "Luis Yafett Gonzalez Montes - 223822964";
    setFocusPolicy(Qt::StrongFocus);
    if (centralWidget()) centralWidget()->setFocusPolicy(Qt::StrongFocus);

    // Conexión de señales de WebSocket
    connect(&socket, &QWebSocket::connected, this, &MainWindow::alConectar);
    connect(&socket, &QWebSocket::disconnected, this, &MainWindow::alDesconectar);
    connect(&socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);

    // Timer para latido (ping/heartbeat) cada segundo
    cronometro = new QTimer(this);
    connect(cronometro, &QTimer::timeout, this, &MainWindow::loop);
    cronometro->start(1000);

    // Inicialización de la gráfica de distancia
    series = new QLineSeries();
    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Distancia Ultrasonido (HC-SR04)");
    chart->legend()->hide();

    axisX = new QValueAxis();
    axisX->setTitleText("Muestras");
    axisX->setRange(0, 30);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setTitleText("Distancia (cm)");
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    // Dirección IP y Puerto WebSocket por defecto
    conectarWebSocket("ws://10.42.0.169:81");
}

MainWindow::~MainWindow()
{
    socket.close();
    delete ui;
}

void MainWindow::conectarWebSocket(const QString &urlStr) {
    QString urlLimpia = urlStr;
    urlLimpia.remove(" "); // Limpia espacios si el cuadro de texto los tiene
    ui->statusbar->showMessage("Conectando a: " + urlLimpia + "...");
    socket.open(QUrl(urlLimpia));
}

void MainWindow::loop() {
    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"latido\"}");
}

void MainWindow::alConectar() {
    ui->statusbar->showMessage("Estado: CONECTADO | URL: " + socket.requestUrl().toString());
    ui->label->setText("Estado : CONECTADO");
    ui->label->setStyleSheet(""); // Restablece el estilo por defecto al conectar
}

void MainWindow::alDesconectar() {
    ui->statusbar->showMessage("Estado: DESCONECTADO");
    ui->label->setText("Estado : DESCONECTADO");
    ui->label->setStyleSheet("");
}

void MainWindow::enviarJSON(const QString &mensaje) {
    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage(mensaje);
}

void MainWindow::alRecibirMensaje(const QString &mensaje) {
    ui->statusbar->showMessage("Estado: CONECTADO | Servidor: " + mensaje);

    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();

    // 1. Lectura y graficación de la distancia del ultrasonido
    if (obj.contains("d")) {
        float distancia = obj["d"].toDouble();
        ui->labelDistancia->setText(QString::number(distancia, 'f', 1) + " cm");

        numeroMuestra++;
        series->append(numeroMuestra, distancia);
        if (numeroMuestra > 30) {
            axisX->setRange(numeroMuestra - 30, numeroMuestra);
        }
    }

    // 2. Recepción de Alerta de Robo desde la ESP32-S3 (Sensor SW-420)
    if (obj.contains("tipo") && obj["tipo"].toString() == "alerta_robo") {
        ui->label->setText("⚠️ ¡ALERTA DE ROBO DETECTADA! ⚠️");
        ui->label->setStyleSheet("QLabel { background-color : red; color : white; font-weight: bold; font-size: 14px; }");
    }

    // 3. Confirmaciones o alertas normales
    if (obj.contains("tipo") && obj["tipo"].toString() == "alerta") {
        ui->statusbar->showMessage("Aviso: " + obj["msj"].toString());
    }
}

// Botón "Adelante" (pushButton_3)
void MainWindow::on_pushButton_3_clicked()
{
    enviarJSON("{\"tipo\":\"mover\", \"accion\":\"avanzar\", \"ms\": 100}");
}

// Botón "Atrás" (pushButton_4)
void MainWindow::on_pushButton_4_clicked()
{
    enviarJSON("{\"tipo\":\"mover\", \"accion\":\"retroceder\", \"ms\": 100}");
}

// Botón "Derecha" (pushButton_2)
void MainWindow::on_pushButton_2_clicked()
{
    enviarJSON("{\"tipo\":\"mover\", \"accion\":\"derecha\", \"ms\": 100}");
}

// Botón "Izquierda" (pushButton)
void MainWindow::on_pushButton_clicked()
{
    enviarJSON("{\"tipo\":\"mover\", \"accion\":\"izquierda\", \"ms\": 100}");
}

// Botón "Conectar" (pushButton_5)
void MainWindow::on_pushButton_5_clicked()
{
    conectarWebSocket(ui->lineEdit->text());
}

// --- CONTROLES DE SERVO Y MEDIDOR ---
void MainWindow::on_sliderServo_valueChanged(int value)
{
    ui->labelAngulo->setText(QString::number(value) + "°");
    enviarJSON("{\"tipo\":\"servo\", \"angulo\":" + QString::number(value) + "}");
}

void MainWindow::on_btn45_clicked()
{
    ui->sliderServo->setValue(45);
}

void MainWindow::on_btn90_clicked()
{
    ui->sliderServo->setValue(90);
}

void MainWindow::on_btn135_clicked()
{
    ui->sliderServo->setValue(135);
}

void MainWindow::on_btnMedirDistancia_clicked()
{
    enviarJSON("{\"tipo\":\"leer_distancia\"}");
}

// Botón "Claxon" (SFM-27)
void MainWindow::on_btnClaxon_clicked() {
    enviarJSON("{\"tipo\":\"claxon\"}");
}

// ====== MODO CENTINELA (EXTRA P3) ======
void MainWindow::on_btnCentinela_clicked() {
    static bool armado = false;
    armado = !armado;

    if (armado) {
        ui->btnCentinela->setText("DESARMAR CENTINELA");
        ui->btnCentinela->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold;");
        enviarJSON("{\"tipo\":\"centinela_on\"}");
    } else {
        ui->btnCentinela->setText("ARMAR MODO CENTINELA");
        ui->btnCentinela->setStyleSheet(""); // Restablece estilo por defecto
        ui->label->setText("Estado : CONECTADO");
        ui->label->setStyleSheet("");
        enviarJSON("{\"tipo\":\"centinela_off\"}");
    }
}

// Control mediante teclado para los movimientos
void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Up:
        on_pushButton_3_clicked();
        break;
    case Qt::Key_Down:
        on_pushButton_4_clicked();
        break;
    case Qt::Key_Left:
        on_pushButton_clicked();
        break;
    case Qt::Key_Right:
        on_pushButton_2_clicked();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}