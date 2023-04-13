#include <qapplication.h>
#include <qtimer.h>
#include <qthread.h>
#include <Windows.h>

#include "PlotWidget.h"
#include "ui_MaintDRONE.h"
#include "MAINT.h"

#define __CSVFILE__ "log.csv"

QApplication* app;
QMainWindow* mainWindow;
QTimer* timer;
QThread* thread;
QObject* receiver;
Ui_MainWindow ui;


void writeCsv(maint_data_t* data)
{
    FILE* f = fopen(__CSVFILE__, "a");
    fprintf(f, "%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %lld, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %d, %d, %d, %d, %d\n",
        data->acc[0], data->acc[1], data->acc[2],
        data->gyro[0], data->gyro[1], data->gyro[2],
        data->magn[0], data->magn[1], data->magn[2],
        data->attitude[0], data->attitude[1], data->attitude[2],
        0.0f, 0.0f, 0.0f,
        data->loop_time,
        data->throttle,
        data->command[0], data->command[1], data->command[2],
        data->PID[0], data->PID[1], data->PID[2],
        data->motors_armed,
        data->motors_speed[0], data->motors_speed[1], data->motors_speed[2], data->motors_speed[3]);

    fclose(f);
 }

void plot(QComboBox* combo, PlotWidget* widget, PlotTrack track, maint_data_t* data)
{
    if (combo->currentText().toLower().contains("accx"))
    {
        widget->addValue(track, data->acc[0]);
    }
    else if (combo->currentText().toLower().contains("accy"))
    {
        widget->addValue(track, data->acc[1]);
    }
    else if (combo->currentText().toLower().contains("accz"))
    {
        widget->addValue(track, data->acc[2]);
    }
    else if (combo->currentText().toLower().contains("gyrox"))
    {
        widget->addValue(track, data->gyro[0]);
    }
    else if (combo->currentText().toLower().contains("gyroy"))
    {
        widget->addValue(track, data->gyro[1]);
    }
    else if (combo->currentText().toLower().contains("gyroz"))
    {
        widget->addValue(track, data->gyro[2]);
    }
    else if (combo->currentText().toLower().contains("magnx"))
    {
        widget->addValue(track, data->magn[0]);
    }
    else if (combo->currentText().toLower().contains("magny"))
    {
        widget->addValue(track, data->magn[1]);
    }
    else if (combo->currentText().toLower().contains("magnz"))
    {
        widget->addValue(track, data->magn[2]);
    }
    else if (combo->currentText().toLower().contains("pid_roll"))
    {
        widget->addValue(track, data->PID[0]);
    }
    else if (combo->currentText().toLower().contains("pid_pitch"))
    {
        widget->addValue(track, data->PID[1]);
    }
    else if (combo->currentText().toLower().contains("pid_yaw"))
    {
        widget->addValue(track, data->PID[2]);
    }
    else if (combo->currentText().toLower().contains("roll"))
    {
        widget->addValue(track, data->attitude[0]);
    }
    else if (combo->currentText().toLower().contains("pitch"))
    {
        widget->addValue(track, data->attitude[1]);
    }
    else if (combo->currentText().toLower().contains("yaw"))
    {
        widget->addValue(track, data->attitude[2]);
    }
    else if (combo->currentText().toLower().contains("motor1"))
    {
        widget->addValue(track, data->motors_speed[0]);
    }
    else if (combo->currentText().toLower().contains("motor2"))
    {
        widget->addValue(track, data->motors_speed[1]);
    }
    else if (combo->currentText().toLower().contains("motor3"))
    {
        widget->addValue(track, data->motors_speed[2]);
    }
    else if (combo->currentText().toLower().contains("motor4"))
    {
        widget->addValue(track, data->motors_speed[3]);
    }
    else if (combo->currentText().toLower().contains("loop_time"))
    {
        widget->addValue(track, data->loop_time);
    }
}

void updateGui(maint_data_t* data)
{
    ui.lineSwVer->setText(QString("%1.%2-%3").arg((char)data->sw_ver[0]).arg((char)data->sw_ver[1]).arg((char)data->sw_ver[2]));

    ui.lineAccX->setText(QString::number(data->acc[0]));
    ui.lineAccY->setText(QString::number(data->acc[1]));
    ui.lineAccZ->setText(QString::number(data->acc[2]));

    ui.lineGyroX->setText(QString::number(data->gyro[0]));
    ui.lineGyroY->setText(QString::number(data->gyro[1]));
    ui.lineGyroZ->setText(QString::number(data->gyro[2]));

    ui.lineMagnX->setText(QString::number(data->magn[0]));
    ui.lineMagnY->setText(QString::number(data->magn[1]));
    ui.lineMagnZ->setText(QString::number(data->magn[2]));

    ui.checkBWFilterEnabled->setChecked(data->avg_filter > 0);

    ui.lineKfRoll->setText(QString::number(data->attitude[0]));
    ui.lineKfPitch->setText(QString::number(data->attitude[1]));
    ui.lineKfYaw->setText(QString::number(data->attitude[2]));

    ui.lineLoopTime->setText(QString::number(data->loop_time));

    ui.lineCmdRoll->setText(QString::number(data->command[0]));
    ui.lineCmdPitch->setText(QString::number(data->command[1]));
    ui.lineCmdYaw->setText(QString::number(data->command[2]));
    ui.lineCmdThrottle->setText(QString::number(data->throttle));

    ui.linePidRoll->setText(QString::number(data->PID[0]));
    ui.linePidPitch->setText(QString::number(data->PID[1]));
    ui.linePidYaw->setText(QString::number(data->PID[2]));

    ui.checkMotorsArmed->setChecked(data->motors_armed > 0);
    ui.lineM1->setText(QString::number(data->motors_speed[0]));
    ui.lineM2->setText(QString::number(data->motors_speed[1]));
    ui.lineM3->setText(QString::number(data->motors_speed[2]));
    ui.lineM4->setText(QString::number(data->motors_speed[3]));

    plot(ui.comboPlotTrack1, ui.plot, PlotTrack::PLOT_TRACK_1, data);
    plot(ui.comboPlotTrack2, ui.plot, PlotTrack::PLOT_TRACK_2, data);
    plot(ui.comboPlotTrack3, ui.plot, PlotTrack::PLOT_TRACK_3, data);
    ui.plot->repaint();
    app->processEvents();
}


void initGui()
{
    mainWindow = new QMainWindow();

    ui.setupUi(mainWindow);
    int r, g, b;
    ui.plot->trackColors[0].getRgb(&r, &g, &b); //cyan
    ui.comboPlotTrack1->setStyleSheet(QString("background-color:rgb(%1, %2, %3); ").arg(r).arg(g).arg(b));
    ui.plot->trackColors[1].getRgb(&r, &g, &b); //magenta
    ui.comboPlotTrack2->setStyleSheet(QString("background-color:rgb(%1, %2, %3); ").arg(r).arg(g).arg(b));
    ui.plot->trackColors[2].getRgb(&r, &g, &b); //green
    ui.comboPlotTrack3->setStyleSheet(QString("background-color:rgb(%1, %2, %3); ").arg(r).arg(g).arg(b));

    QObject::connect(ui.horizontalSlider1, &QSlider::valueChanged, mainWindow, [](int value)
        {
            ui.plot->setRange(PlotTrack::PLOT_TRACK_1, value);
        });
    QObject::connect(ui.horizontalSlider2, &QSlider::valueChanged, mainWindow, [](int value)
        {
            ui.plot->setRange(PlotTrack::PLOT_TRACK_2, value);
        });
    QObject::connect(ui.horizontalSlider3, &QSlider::valueChanged, mainWindow, [](int value)
        {
            ui.plot->setRange(PlotTrack::PLOT_TRACK_3, value);
        });
    mainWindow->setVisible(true);
}


void rxThread(const char* port)
{
    HANDLE hComm = CreateFileA(port,    //PORT NAME
        GENERIC_READ | GENERIC_WRITE,   //READ/WRITE
        0,                              //NO SHARING
        NULL,                           //NO SECURITY
        OPEN_EXISTING,                  //OPEN EXISTING PORT ONLY
        0,                              //NON OVERLAPPED I/O
        NULL);                          //NULL FOR COMM DEVICES

    DCB commStateConfig;
    GetCommState(hComm, &commStateConfig);
    commStateConfig.BaudRate = 115200;
    commStateConfig.ByteSize = 8;
    SetCommState(hComm, &commStateConfig);

    maint_data_t* packet = MAINT_Get();

    enum rx_fsm_state
    {
        WAIT_77 = 0,
        WAIT_FE,
        WAIT_BA,
        WAIT_B0,
        WAIT_DATA
    };

    rx_fsm_state rxState = WAIT_77;
    while (1)
    {
        DWORD size;
        BOOL res = 0;
        uint8_t byteIn;
        switch (rxState)
        {
            case WAIT_77:
            {
                res = ReadFile(hComm, &byteIn, 1, &size, 0);
                if (res && byteIn == 0x77)
                {
                    rxState = WAIT_FE;
                }
                else rxState = WAIT_77;
                break;
            }
            case WAIT_FE:
            {
                res = ReadFile(hComm, &byteIn, 1, &size, 0);
                if (res && byteIn == 0xFE)
                {
                    rxState = WAIT_BA;
                }
                else rxState = WAIT_77;
                break;
            }
            case WAIT_BA:
            {
                res = ReadFile(hComm, &byteIn, 1, &size, 0);
                if (res && byteIn == 0xBA)
                {
                    rxState = WAIT_B0;
                }
                else rxState = WAIT_77;
                break;
            }
            case WAIT_B0:
            {
                res = ReadFile(hComm, &byteIn, 1, &size, 0);
                if (res && byteIn == 0xB0)
                {
                    rxState = WAIT_DATA;
                }
                else rxState = WAIT_77;
                break;
            }
            default: break;
        }

        if (rxState == WAIT_DATA)
        {
            rxState = WAIT_77;
            res = ReadFile(hComm, &packet->sw_ver, sizeof(maint_data_t) - 4, &size, 0);
            if (res)
            {
                writeCsv(packet);
                updateGui(packet);
            }
        }

    }
}


void initSerialReceiver(const char* port)
{
    timer = new QTimer();
    timer->setInterval(10);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setSingleShot(true);

    thread = new QThread();
    timer->moveToThread(thread);

    receiver = new QObject();

    QObject::connect(timer, &QTimer::timeout, receiver, [port]()
    {
        rxThread(port);
    });

    QObject::connect(thread, SIGNAL(started()), timer, SLOT(start()));
    thread->start();
}


int main(int argc, char** argv)
{
    FILE* f = fopen(__CSVFILE__, "w");
    fprintf(f, "acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z, magn.x, magn.y, magn.z, roll, pitch, yaw, roll_var, pitch_var, yaw_var, kf_dt, throttle, command_roll, command_pitch, command_yaw, pid_roll, pid_pitch, pid_yaw, motors_armed, m1_speed, m2_speed, m3_speed, m4_speed\n");
    fclose(f);

    app = new QApplication(argc, argv);
    initGui();
    initSerialReceiver(argv[1]);

    return app->exec();
}