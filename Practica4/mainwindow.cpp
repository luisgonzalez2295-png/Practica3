#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTimeEdit>

static const char *FUENTE = "simulada";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // REGLA ANTIPLAGIO: Título con Nombre y Código[cite: 1]
    this->setWindowTitle("Practica 4 | Luis Yafett Gonzalez Montes - 223822954");
    const QString ALUMNO = "Luis Yafett Gonzalez Montes - 223822954";
    setFocusPolicy(Qt::StrongFocus);
    if (centralWidget()) centralWidget()->setFocusPolicy(Qt::StrongFocus);

    // WebSocket
    connect(&socket, &QWebSocket::connected, this, &MainWindow::alConectar);
    connect(&socket, &QWebSocket::disconnected, this, &MainWindow::alDesconectar);
    connect(&socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);

    cronometro = new QTimer(this);
    connect(cronometro, &QTimer::timeout, this, &MainWindow::loop);
    cronometro->start(1000);

    // Gráfica Distancia (P2)
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

    // =========================================================
    // ESTRUCTURA PESTAÑA TEMPERATURA (PRÁCTICA 4)
    // =========================================================
    QWidget *centralRobot = centralWidget();
    auto *pestanas = new QTabWidget(this);
    pestanas->addTab(centralRobot, "Robot");

    auto *pagTemp = new QWidget(this);
    m_capaTemp = new QVBoxLayout(pagTemp);

    // Regla antiplagio visual[cite: 1]
    QLabel *lblAlumno = new QLabel(ALUMNO, pagTemp);
    lblAlumno->setStyleSheet("font-size: 12px; font-weight: bold; color: #1976D2; padding: 2px;");
    lblAlumno->setAlignment(Qt::AlignCenter);

    // E2: Etiqueta dedicada para mostrar "Base nueva creada" / "Base encontrada"
    m_lblEstadoBase = new QLabel("Inicializando SQLite...", pagTemp);
    m_lblEstadoBase->setStyleSheet("font-size: 11px; font-weight: bold; color: #C62828; background: #FFEBEE; padding: 4px; border-radius: 4px;");
    m_lblEstadoBase->setWordWrap(true);
    m_lblEstadoBase->setAlignment(Qt::AlignCenter);

    // E1: Controles de Lectura
    QPushButton *btnLeerTemp = new QPushButton("Leer Temperatura", pagTemp);
    m_lblTemperatura = new QLabel("--.- °C", pagTemp);
    m_lblTemperatura->setStyleSheet("font-size: 26px; font-weight: bold; color: #2E7D32;");
    m_lblTemperatura->setAlignment(Qt::AlignCenter);

    // E3: Guardado automático cada 10s
    m_chkAutoGuardar = new QCheckBox("Guardar cada 10 s", pagTemp);
    m_timerAutoGuardar = new QTimer(this);

    // E4: Tabla e Historial
    m_tabla = new QTableWidget(0, 4, pagTemp);
    m_tabla->setHorizontalHeaderLabels({"ID", "Fecha/Hora", "Temp (°C)", "Nota"});
    m_tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_lblTotal = new QLabel("Mediciones guardadas: 0", pagTemp);
    QPushButton *btnCargarHistorial = new QPushButton("Cargar historial", pagTemp);

    // E5: UPDATE / Notas
    m_spnIdNota = new QSpinBox(pagTemp);
    m_spnIdNota->setRange(0, 99999);
    m_spnIdNota->setSpecialValueText("-");
    m_txtNota = new QLineEdit(pagTemp);
    m_txtNota->setPlaceholderText("Escribe una nota...");
    QPushButton *btnGuardarNota = new QPushButton("Guardar nota", pagTemp);

    QHBoxLayout *capaNotas = new QHBoxLayout();
    capaNotas->addWidget(new QLabel("ID:"));
    capaNotas->addWidget(m_spnIdNota);
    capaNotas->addWidget(m_txtNota);
    capaNotas->addWidget(btnGuardarNota);

    // Estadísticas previas
    QPushButton *btnStats = new QPushButton("Calcular Estadísticas", pagTemp);
    m_lblStatsE7 = new QLabel("Mín: -- | Máx: -- | Prom: --", pagTemp);
    m_lblStatsE7->setStyleSheet("font-weight: bold; color: #1565C0;");

    QHBoxLayout *capaStats = new QHBoxLayout();
    capaStats->addWidget(btnStats);
    capaStats->addWidget(m_lblStatsE7);

    // =========================================================
    // E7: EXTRA PROPIO - FILTRADO AVANZADO POR RANGO DE FECHAS
    // =========================================================
    m_dtDesde = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1), pagTemp);
    m_dtDesde->setDisplayFormat("yyyy-MM-dd hh:mm");

    m_dtHasta = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), pagTemp);
    m_dtHasta->setDisplayFormat("yyyy-MM-dd hh:mm");

    QPushButton *btnFiltrarFechas = new QPushButton("Filtrar Rango (E7)", pagTemp);
    btnFiltrarFechas->setStyleSheet("font-weight: bold; background-color: #E8EAF6; color: #283593;");

    QHBoxLayout *capaFiltro = new QHBoxLayout();
    capaFiltro->addWidget(new QLabel("Desde:"));
    capaFiltro->addWidget(m_dtDesde);
    capaFiltro->addWidget(new QLabel("Hasta:"));
    capaFiltro->addWidget(m_dtHasta);
    capaFiltro->addWidget(btnFiltrarFechas);

    // Construcción del Layout
    m_capaTemp->addWidget(lblAlumno);
    m_capaTemp->addWidget(m_lblEstadoBase);
    m_capaTemp->addWidget(btnLeerTemp);
    m_capaTemp->addWidget(m_lblTemperatura);
    m_capaTemp->addWidget(m_chkAutoGuardar);
    m_capaTemp->addWidget(btnCargarHistorial);
    m_capaTemp->addWidget(m_tabla);
    m_capaTemp->addWidget(m_lblTotal);
    m_capaTemp->addLayout(capaNotas);
    m_capaTemp->addLayout(capaStats);
    m_capaTemp->addLayout(capaFiltro); // <--- LAYOUT AGREGADO PARA E7

    pestanas->addTab(pagTemp, "Temperatura");
    setCentralWidget(pestanas);

    // Conexiones de Eventos
    connect(btnLeerTemp, &QPushButton::clicked, this, [this](){
        enviarJSON("{\"tipo\":\"leer_dht\"}");
    });

    connect(m_chkAutoGuardar, &QCheckBox::toggled, this, [this](bool active){
        if (active) {
            m_timerAutoGuardar->start(10000);
        } else {
            m_timerAutoGuardar->stop();
        }
    });

    connect(m_timerAutoGuardar, &QTimer::timeout, this, [this](){
        enviarJSON("{\"tipo\":\"leer_dht\"}");
    });

    connect(btnCargarHistorial, &QPushButton::clicked, this, &MainWindow::cargarHistorialMediciones);
    connect(btnGuardarNota, &QPushButton::clicked, this, &MainWindow::guardarNotaMedicion);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::calcularEstadisticasE7);
    connect(btnFiltrarFechas, &QPushButton::clicked, this, &MainWindow::filtrarPorFechasE7);

    connect(m_tabla, &QTableWidget::cellClicked, this, [this](int fila, int) {
        if (m_tabla->item(fila, 0)) {
            m_spnIdNota->setValue(m_tabla->item(fila, 0)->text().toInt());
        }
    });

    // INICIALIZACIÓN DE LA BASE DE DATOS E2
    inicializarBaseDatos();

    conectarWebSocket("ws://10.42.0.169:81");
}

MainWindow::~MainWindow()
{
    socket.close();
    delete ui;
}

// =========================================================
// IMPLEMENTACIÓN BLOQUE BASE DE DATOS (EVIDENCIA E2 A E6)
// =========================================================

void MainWindow::inicializarBaseDatos() {
    const QString carpeta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(carpeta);

    // REGLA ANTIPLAGIO: temperatura_2954.db[cite: 1]
    const QString ruta = carpeta + "/temperatura_2954.db";
    const bool yaExistia = QFile::exists(ruta);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(ruta);

    if (!m_db.open()) {
        QString err = "No se abrió la base: " + m_db.lastError().text();
        m_lblEstadoBase->setText(err);
        ui->statusbar->showMessage(err);
        return;
    }

    QSqlQuery consulta(m_db);
    if (!consulta.exec("CREATE TABLE IF NOT EXISTS mediciones ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "fecha_hora TEXT NOT NULL, "
                       "temperatura REAL NOT NULL, "
                       "fuente TEXT NOT NULL, "
                       "nota TEXT)")) {
        QString err = "No se creó la tabla: " + consulta.lastError().text();
        m_lblEstadoBase->setText(err);
        ui->statusbar->showMessage(err);
        return;
    }

    QString msgBase = (yaExistia ? "Base encontrada: " : "Base nueva creada: ") + ruta;
    m_lblEstadoBase->setText(msgBase);

    cargarHistorialMediciones();
}

void MainWindow::guardarTemperatura(double celsius) {
    double c1 = qRound(celsius * 10.0) / 10.0;
    QSqlQuery consulta(m_db);
    consulta.prepare("INSERT INTO mediciones (fecha_hora, temperatura, fuente) "
                     "VALUES (:fecha_hora, :temperatura, :fuente)");
    consulta.bindValue(":fecha_hora", QDateTime::currentDateTime().toString(Qt::ISODate));
    consulta.bindValue(":temperatura", c1);
    consulta.bindValue(":fuente", QString::fromUtf8(FUENTE));

    if (!consulta.exec()) {
        ui->statusbar->showMessage("INSERT falló: " + consulta.lastError().text());
        return;
    }

    ui->statusbar->showMessage(QString("Guardada %1 °C con id %2")
                                   .arg(c1, 0, 'f', 1)
                                   .arg(consulta.lastInsertId().toString()));
    cargarHistorialMediciones();
}

void MainWindow::cargarHistorialMediciones() {
    QSqlQuery consulta(m_db);
    consulta.exec("SELECT id, fecha_hora, temperatura, nota FROM mediciones ORDER BY id DESC LIMIT 50");

    m_tabla->setRowCount(0);
    while (consulta.next()) {
        int fila = m_tabla->rowCount();
        m_tabla->insertRow(fila);
        m_tabla->setItem(fila, 0, new QTableWidgetItem(consulta.value(0).toString()));
        m_tabla->setItem(fila, 1, new QTableWidgetItem(consulta.value(1).toString()));
        m_tabla->setItem(fila, 2, new QTableWidgetItem(QString::number(consulta.value(2).toDouble(), 'f', 1)));
        m_tabla->setItem(fila, 3, new QTableWidgetItem(consulta.value(3).toString()));
    }
    m_tabla->resizeColumnsToContents();

    QSqlQuery total("SELECT COUNT(*) FROM mediciones", m_db);
    if (total.next()) {
        m_lblTotal->setText("Mediciones guardadas: " + total.value(0).toString());
    }
}

void MainWindow::guardarNotaMedicion() {
    int idSel = m_spnIdNota->value();
    if (idSel == 0) {
        ui->statusbar->showMessage("Primero toca una fila o ingresa un ID válido");
        return;
    }

    QSqlQuery consulta(m_db);
    consulta.prepare("UPDATE mediciones SET nota = :nota WHERE id = :id");
    consulta.bindValue(":nota", m_txtNota->text());
    consulta.bindValue(":id", idSel);

    if (!consulta.exec()) {
        ui->statusbar->showMessage("UPDATE falló: " + consulta.lastError().text());
        return;
    }

    int afectadas = consulta.numRowsAffected();
    ui->statusbar->showMessage(QString("UPDATE ejecutado: %1 fila(s) modificada(s)").arg(afectadas));
    cargarHistorialMediciones();
}

void MainWindow::calcularEstadisticasE7() {
    QSqlQuery q("SELECT MIN(temperatura), MAX(temperatura), AVG(temperatura) "
                "FROM mediciones", m_db);
    if (q.next()) {
        double minT = q.value(0).toDouble();
        double maxT = q.value(1).toDouble();
        double avgT = q.value(2).toDouble();

        m_lblStatsE7->setText(QString("Mín: %1 °C | Máx: %2 °C | Prom: %3 °C")
                                  .arg(minT, 0, 'f', 1)
                                  .arg(maxT, 0, 'f', 1)
                                  .arg(avgT, 0, 'f', 1));
    }
}

// =========================================================
// IMPLEMENTACIÓN EXTRA PROPIO (EVIDENCIA E7)
// =========================================================

void MainWindow::filtrarPorFechasE7() {
    QString inicio = m_dtDesde->dateTime().toString(Qt::ISODate);
    QString fin = m_dtHasta->dateTime().toString(Qt::ISODate);

    QSqlQuery query(m_db);
    query.prepare("SELECT id, fecha_hora, temperatura, nota FROM mediciones "
                  "WHERE fecha_hora BETWEEN :inicio AND :fin "
                  "ORDER BY id DESC");
    query.bindValue(":inicio", inicio);
    query.bindValue(":fin", fin);

    if (query.exec()) {
        m_tabla->setRowCount(0);
        int filas = 0;

        while (query.next()) {
            int fila = m_tabla->rowCount();
            m_tabla->insertRow(fila);
            m_tabla->setItem(fila, 0, new QTableWidgetItem(query.value(0).toString()));
            m_tabla->setItem(fila, 1, new QTableWidgetItem(query.value(1).toString()));
            m_tabla->setItem(fila, 2, new QTableWidgetItem(QString::number(query.value(2).toDouble(), 'f', 1)));
            m_tabla->setItem(fila, 3, new QTableWidgetItem(query.value(3).toString()));
            filas++;
        }
        m_tabla->resizeColumnsToContents();
        ui->statusbar->showMessage(QString("Filtro aplicado: %1 registros dentro del rango").arg(filas));
    } else {
        ui->statusbar->showMessage("Error al filtrar: " + query.lastError().text());
    }
}

// RESTO DE FUNCIONALIDADES EXISTENTES (P1, P2, P3)
void MainWindow::conectarWebSocket(const QString &urlStr) {
    QString urlLimpia = urlStr;
    urlLimpia.remove(" ");
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
    ui->label->setStyleSheet("");
}

void MainWindow::alDesconectar() {
    ui->statusbar->showMessage("Estado: DESCONECTADO");
    ui->label->setText("Estado : DESCONECTADO");
    ui->label->setStyleSheet("");

    if (m_chkAutoGuardar->isChecked()) {
        m_chkAutoGuardar->setChecked(false);
    }
}

void MainWindow::enviarJSON(const QString &mensaje) {
    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage(mensaje);
}

void MainWindow::alRecibirMensaje(const QString &mensaje) {
    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();

    if (obj.contains("d")) {
        float distancia = obj["d"].toDouble();
        ui->labelDistancia->setText(QString::number(distancia, 'f', 1) + " cm");

        numeroMuestra++;
        series->append(numeroMuestra, distancia);
        if (numeroMuestra > 30) {
            axisX->setRange(numeroMuestra - 30, numeroMuestra);
        }
    }

    if (obj.contains("tipo") && obj["tipo"].toString() == "alerta_robo") {
        ui->label->setText("⚠️ ¡ALERTA DE ROBO DETECTADA! ⚠️");
        ui->label->setStyleSheet("QLabel { background-color : red; color : white; font-weight: bold; font-size: 14px; }");
    }

    if (obj.contains("tipo") && obj["tipo"].toString() == "dht_lectura") {
        bool ok = obj["dht_ok"].toBool();
        QJsonValue t = obj["temperatura"];

        if (!ok || !t.isDouble()) {
            ui->statusbar->showMessage("Lectura inválida: no se guarda");
            return;
        }

        double temp = t.toDouble();
        m_lblTemperatura->setText(QString::number(temp, 'f', 1) + " °C");
        guardarTemperatura(temp);
    }

    if (obj.contains("tipo") && obj["tipo"].toString() == "alerta") {
        ui->statusbar->showMessage("Aviso: " + obj["msj"].toString());
    }
}

void MainWindow::on_pushButton_3_clicked() { enviarJSON("{\"tipo\":\"mover\", \"accion\":\"avanzar\", \"ms\": 100}"); }
void MainWindow::on_pushButton_4_clicked() { enviarJSON("{\"tipo\":\"mover\", \"accion\":\"retroceder\", \"ms\": 100}"); }
void MainWindow::on_pushButton_2_clicked() { enviarJSON("{\"tipo\":\"mover\", \"accion\":\"derecha\", \"ms\": 100}"); }
void MainWindow::on_pushButton_clicked() { enviarJSON("{\"tipo\":\"mover\", \"accion\":\"izquierda\", \"ms\": 100}"); }
void MainWindow::on_pushButton_5_clicked() { conectarWebSocket(ui->lineEdit->text()); }

void MainWindow::on_sliderServo_valueChanged(int value)
{
    ui->labelAngulo->setText(QString::number(value) + "°");
    enviarJSON("{\"tipo\":\"servo\", \"angulo\":" + QString::number(value) + "}");
}

void MainWindow::on_btn45_clicked() { ui->sliderServo->setValue(45); }
void MainWindow::on_btn90_clicked() { ui->sliderServo->setValue(90); }
void MainWindow::on_btn135_clicked() { ui->sliderServo->setValue(135); }
void MainWindow::on_btnMedirDistancia_clicked() { enviarJSON("{\"tipo\":\"leer_distancia\"}"); }
void MainWindow::on_btnClaxon_clicked() { enviarJSON("{\"tipo\":\"claxon\"}"); }

void MainWindow::on_btnCentinela_clicked() {
    static bool armado = false;
    armado = !armado;

    if (armado) {
        ui->btnCentinela->setText("DESARMAR CENTINELA");
        ui->btnCentinela->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold;");
        enviarJSON("{\"tipo\":\"centinela_on\"}");
    } else {
        ui->btnCentinela->setText("ARMAR MODO CENTINELA");
        ui->btnCentinela->setStyleSheet("");
        ui->label->setText("Estado : CONECTADO");
        ui->label->setStyleSheet("");
        enviarJSON("{\"tipo\":\"centinela_off\"}");
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Up: on_pushButton_3_clicked(); break;
    case Qt::Key_Down: on_pushButton_4_clicked(); break;
    case Qt::Key_Left: on_pushButton_clicked(); break;
    case Qt::Key_Right: on_pushButton_2_clicked(); break;
    default: QMainWindow::keyPressEvent(event); break;
    }
}