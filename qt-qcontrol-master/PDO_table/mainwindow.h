#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCanBusFrame>
#include<QCoreApplication>
#include<QFileDialog>
#include<QDesktopServices>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>
#include<QObject>
#include <QTableWidget>
#include <QMessageBox>
#include <QAxObject>
#include <QtGui>
#include <QtCore>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHBoxLayout>
#include<QStackedWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
     QString BintoHex(QString data);
     uint Get_NodeID(QString data);
     QString Process_ECU_value(QString Torque,QString Speed);
     QString Process_Current_value(QString data);
     QString Process_SDO_value(QString data);
     QString Process_Read_SDO_Msg(QString Index,QString Subindex);
     QString Process_Write_SDO_Msg(QString Index,QString Subindex,QString Data,QString Datatype);
     QString HextoBin(QString data);
     QString HextoDec(QString data);
     QString DectoHex(QString data);
     QString FloattoHex(QString data);
     static void ReadExcel(const QString& filePath);
     QString HextoFloat(QString data);


protected:
    void timerEvent(QTimerEvent* event);

private slots:

    void on_Pre_operational_clicked();

    void on_Operational_clicked();

    void on_Reset_Node_clicked();

    void on_Reset_communication_clicked();

    void on_Stop_clicked();

    void on_Torque_control_clicked();

    void on_Speed_control_clicked();

    void on_Cartesian_clicked();

    void on_Polar_clicked();

    void processReceivedFrames();

    void on_checkBox_stateChanged(int arg1);

    void on_Apply_ECU_value_clicked();

    void on_Send_Current_value_clicked();

    void on_checkBox_NodeGuard_stateChanged(int arg1);

    void on_NodeGuard_Interval_textChanged(const QString &arg1);

    void on_SYNC_interval_textChanged(const QString &arg1);

    void on_Clear_Send_clicked();

    void on_Clear_Received_clicked();

    void on_checkBox_SYNC_stateChanged(int arg1);

    void on_choose_file_clicked();

    void on_Clear_error_clicked();

    void on_Disable_clicked();

    void on_Enable_clicked(bool checked);

    void on_send_interval_textChanged(const QString &arg1);

    void on_Connect_clicked();

    void on_Node_textChanged(const QString &arg1);

    void pluginChanged(const QString &plug);

    void interfaceChanged(const QString &interf);

signals:

    void page(int a);
public slots:

    void getTreeClicked(const QModelIndex index);
    void HasChild(QStandardItem *item,QStackedWidget *stackedWidget,QSignalMapper* mSignalMapper);
    void Read_all_child_value(QStandardItem *item);
    void Write_all_child_value(QStandardItem *item);
    void Write_SDO();
    void Write_Multi_SDO();
    void Read_SDO();
    void Read_Multi_SDO();
    void Store_SDO();
    void Add_table_content(QString data,QStandardItemModel* model,QCanBusFrame frame);
    void Add_Receive_content(QString data,QStandardItemModel* model,QCanBusFrame frame);

    void Add_clear_row(QString comment,QStandardItemModel* model);
private:
    Ui::MainWindow *ui;
    QHBoxLayout *hLayout;
    int Timer1;
    int Timer2;
    int Timer3;
    QList<QCanBusDeviceInfo> m_interfaces;
  //  void slotCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous,QStandardItemModel* model);
};
#endif // MAINWINDOW_H
