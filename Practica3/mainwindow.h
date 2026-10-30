#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QTimer>
#include <QKeyEvent>

// Módulo para la gráfica de la Práctica 2
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

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

    // Botones de la Práctica 1
    void on_pushButton_3_clicked(); // Adelante
    void on_pushButton_4_clicked(); // Atrás
    void on_pushButton_2_clicked(); // Derecha
    void on_pushButton_clicked();   // Izquierda
    void on_pushButton_5_clicked(); // Conectar

    // Nuevos controles Práctica 2 (Servo y Medición)
    void on_sliderServo_valueChanged(int value);
    void on_btn45_clicked();
    void on_btn90_clicked();
    void on_btn135_clicked();
    void on_btnMedirDistancia_clicked();

    void conectarWebSocket(const QString &urlStr);
    void on_btnClaxon_clicked();
    //Funcion centinela practica 3
    void on_btnCentinela_clicked();
private:
    Ui::MainWindow *ui;
    QWebSocket socket;
    QTimer *cronometro;

    // Componentes de la gráfica
    QLineSeries *series;
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    int numeroMuestra = 0;
};
#endif // MAINWINDOW_H