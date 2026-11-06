#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QTimer>
#include <QKeyEvent>

// Qt Charts (P2)
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

// SQLite y UI (P4)
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void alConectar();
    void alDesconectar();
    void enviarJSON(const QString &mensaje);
    void loop();
    void alRecibirMensaje(const QString &mensaje);

    // Controles Práctica 1 y 2
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_5_clicked();
    void on_sliderServo_valueChanged(int value);
    void on_btn45_clicked();
    void on_btn90_clicked();
    void on_btn135_clicked();
    void on_btnMedirDistancia_clicked();
    void conectarWebSocket(const QString &urlStr);
    void on_btnClaxon_clicked();
    void on_btnCentinela_clicked();

    // Métodos SQLite (P4)[cite: 6, 7, 8, 9]
    void guardarTemperatura(double celsius);
    void cargarHistorialMediciones();
    void guardarNotaMedicion();
    void calcularEstadisticasE7();
    // Extra practica 4
    void filtrarPorFechasE7();
private:
    Ui::MainWindow *ui;
    QWebSocket socket;
    QTimer *cronometro;

    // Componentes Gráfica P2
    QLineSeries *series;
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    int numeroMuestra = 0;

    // UI Pestaña Temperatura (P4)[cite: 5, 6, 8, 9]
    QVBoxLayout *m_capaTemp;
    QLabel *m_lblEstadoBase; // ETIQUETA E2 PARA MOSTRAR BASE NUEVA / ENCONTRADA
    QLabel *m_lblTemperatura;
    QLabel *m_lblTotal;
    QLabel *m_lblStatsE7;
    QTableWidget *m_tabla;
    QCheckBox *m_chkAutoGuardar;
    QTimer *m_timerAutoGuardar;
    QSpinBox *m_spnIdNota;
    QLineEdit *m_txtNota;

    // Manejador de la BD SQLite
    QSqlDatabase m_db;
    void inicializarBaseDatos();
    //Extra practica4
    QDateTimeEdit *m_dtDesde;
    QDateTimeEdit *m_dtHasta;
};
#endif // MAINWINDOW_H