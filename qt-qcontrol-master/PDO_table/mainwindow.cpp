#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QModbusPdu>
#include <QModbusRtuSerialMaster>
#include <QCanBusFrame>
#include <QCheckBox>
#include <QHBoxLayout>
#include<QSignalMapper>
QList<QCanBusDeviceInfo> m_interfaces;
QCanBusDevice *m_canDevice = nullptr;
QString plugin;
QString Interface;
QString errorString;
bool nodeguard_flag = 0;
bool SYNC_flag = 0;
bool Send_flag = 0;
QCanBusFrame NodeGuard_frame;
QCanBusFrame SYNC_frame;
QCanBusFrame ECU_frame;
QCanBusFrame Iq_frame;
QCanBusFrame Id_frame;
QCanBusFrame frame;
int tab_index;
int start = 10000;
QStringList lines;
QStandardItemModel* model;
QStandardItemModel* Send_data_model =new QStandardItemModel();;
QStandardItemModel* Receive_data_model=new QStandardItemModel();;
QStackedWidget *stackedWidget;
int Value;
int ready;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->checkBox_NodeGuard->setEnabled(false);
    ui->checkBox_SYNC->setEnabled(false);
    ui->Iq_Value_Ca->setEnabled(true);
    ui->Id_Value_Ca->setEnabled(true);
    ui->lineEdit_8->setEnabled(false);
    ui->lineEdit_10->setEnabled(false);
    ui->radioButton_5->setEnabled(false);
    ui->radioButton_6->setEnabled(false);
    ui->lineEdit_6->setEnabled(false);
    ui->lineEdit->setEnabled(true);
    ui->lineEdit_5->setEnabled(true);
    ui->lineEdit_2->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit_4->setEnabled(false);
    ui->Enable->setEnabled(false);    
    ui->Pre_operational->setEnabled(false);
    ui->Operational->setEnabled(false);
    ui->Reset_Node->setEnabled(false);
    ui->Reset_communication->setEnabled(false);
    ui->Stop->setEnabled(false);

    Send_data_model->setHorizontalHeaderLabels(QStringList()<<QStringLiteral("CAN_ID")<< QStringLiteral("Length")<< QStringLiteral("Data")<< QStringLiteral("Cycle time")<< QStringLiteral("Comment"));
    Receive_data_model->setHorizontalHeaderLabels(QStringList()<<QStringLiteral("CAN_ID")<< QStringLiteral("Length")<< QStringLiteral("Data")<< QStringLiteral("Cycle time")<< QStringLiteral("Comment"));

    ui->tableView->setModel(Send_data_model);
    ui->tableView_2->setModel(Receive_data_model);

    //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->tableView->resizeColumnToContents(4);

    ui->tableView->setColumnWidth(0,150);
    ui->tableView->setColumnWidth(1,100);
    ui->tableView->setColumnWidth(2,350);
    ui->tableView->setColumnWidth(3,150);
    ui->tableView->setColumnWidth(4,500);

    ui->tableView_2->setColumnWidth(0,150);
    ui->tableView_2->setColumnWidth(1,100);
    ui->tableView_2->setColumnWidth(2,350);
    ui->tableView_2->setColumnWidth(3,150);
    ui->tableView_2->setColumnWidth(4,500);
    ui->comboBox->addItems(QCanBus::instance()->plugins());
    connect(ui->comboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::pluginChanged);
    connect(ui->comboBox_2, &QComboBox::currentTextChanged,
            this, &MainWindow::interfaceChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pluginChanged(const QString &plug)
{
    plugin = plug;
    ui->comboBox_2->clear();
    m_interfaces = QCanBus::instance()->availableDevices(plug);
    for (const QCanBusDeviceInfo &info : qAsConst(m_interfaces))
               ui->comboBox_2->addItem(info.name());
}

void MainWindow::interfaceChanged(const QString &interf)
{
    Interface = interf;
}

//Connect device
void MainWindow::on_Connect_clicked()
{
     bool connectflag = false;
    //If detected can device
     if(!m_interfaces.empty())
     {
         if(ui->Connect->text()=="Open Port")
         {
             m_canDevice = QCanBus::instance()->createDevice(plugin, Interface, &errorString);
             //Bit Rate is set to 500000
             m_canDevice->setConfigurationParameter(QCanBusDevice::BitRateKey,QVariant("500000"));
             //Connect can Device
             connectflag= m_canDevice->connectDevice();
             if (!connectflag)
             {
                 ui->textEdit_2->setText("Connect failed");
                 ui->textEdit_2->append(errorString);
             }
             else
             {
                 ui->textEdit_2->setText("Connect success");
                 connect(m_canDevice, &QCanBusDevice::framesReceived, this, &MainWindow::processReceivedFrames);
                 // connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &MainWindow::processErrors);
                 ui->Connect->setText("Disconnect");
             }

        }
         //Disconnect the device
         if(ui->Connect->text()=="Disconnect")
         {
             m_canDevice->disconnectDevice();
             ui->Connect->setText("Disconnect");
         }
     }
     else
     {
      ui->textEdit_2->setText("Do not find available interface for "+plugin);
     }
}

//Pre-operation send 2 byte data: 80, nodeID
void MainWindow::on_Pre_operational_clicked()
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {
        //COBID
        QString COBID = "000";

        //Data bytes
        QString data="80"+ui->Node->text();

        //frame
        const QByteArray payload = QByteArray::fromHex(data.toLatin1());
        QCanBusFrame frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
       // ui->textEdit_2->setText(frame.toString());        
        Add_clear_row("NMT-Pre_operational",Send_data_model);
        Add_table_content("NMT-Pre_operational",Send_data_model,frame);

        //Send message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(frame);
    }
}
//Operation send 2 byte data: 01, nodeID
void MainWindow::on_Operational_clicked()
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {
        //COBID
        QString COBID = "000";
        //Data bytes
        QString data="01"+ui->Node->text();
        //frame
        const QByteArray payload = QByteArray::fromHex(data.toLatin1());
        QCanBusFrame frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
        //ui->textEdit_2->append(frame.toString());
        Add_clear_row("NMT-Operational",Send_data_model);
        Add_table_content("NMT-Operational",Send_data_model,frame);

        //Send message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(frame);       
    }
}
//Reset_node send 2 byte data: 81, nodeID
void MainWindow::on_Reset_Node_clicked()
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {
        //COBID
        QString COBID = "000";
        //Data bytes
        QString data="81"+ui->Node->text();
        //frame
        const QByteArray payload = QByteArray::fromHex(data.toLatin1());
        QCanBusFrame frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
        //ui->textEdit_2->append(frame.toString());
        Add_clear_row("NMT-ResetNode",Send_data_model);
        Add_table_content("NMT-ResetNode",Send_data_model,frame);

        //Send message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(frame);
    }
}
//Reset_communication send 2 byte data: 82, nodeID
void MainWindow::on_Reset_communication_clicked()
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {
        //COBID
        QString COBID ="000";
        //Data bytes
        QString data="82"+ui->Node->text();
        //frame
        const QByteArray payload = QByteArray::fromHex(data.toLatin1());
        QCanBusFrame frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
        //ui->textEdit_2->append(frame.toString());
        Add_clear_row("NMT-ResetComms",Send_data_model);
        Add_table_content("NMT-ResetComms",Send_data_model,frame);

        //Send message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(frame);
    }
}
//Stop send 2 byte data: 02, nodeID
void MainWindow::on_Stop_clicked()
{    
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {       
        //COBID
        QString COBID = "000";
        //Data bytes
        QString data="02"+ui->Node->text();
        //frame
        const QByteArray payload = QByteArray::fromHex(data.toLatin1());
        QCanBusFrame frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
        //ui->textEdit_2->append(frame.toString());
        Add_clear_row("NMT-Stop",Send_data_model);
        Add_table_content("NMT-Stop",Send_data_model,frame);

        //Send message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(frame);
    }
}
//Clear error frame
void MainWindow::on_Clear_error_clicked()
{
    QString COBID = "67A";
    QString Data = "2F03200001000000";
    const QByteArray payload = QByteArray::fromHex(Data.toLatin1());
    QCanBusFrame Clear_error_frame = QCanBusFrame(COBID.toUInt(nullptr, 16), payload);
    Add_clear_row("Clear Error",Send_data_model);
    Add_table_content("Clear Error",Send_data_model,Clear_error_frame);

    //Send SDO message
    if (!m_canDevice)
        return;
    else
      m_canDevice->writeFrame(Clear_error_frame);
}
//Send disable message to stop the ECU/Current regulator
void MainWindow::on_Disable_clicked()
{
    Add_clear_row("Disable",Send_data_model);
    Send_flag = false;
    //Stop timer
    killTimer(Timer3);
    ui->Enable->setEnabled(false);
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        //Disable frame
        const uint COBID = Get_NodeID("ECUID");
        int tab_index = ui->tabWidget->currentIndex();
        QString Data = "0000000000000000";
        const QByteArray payload = QByteArray::fromHex(Data.toLatin1());

        if (!m_canDevice)
            return;
        else
        {
            //Disable ECU configuration
            if(tab_index==0)
            {
                ECU_frame = QCanBusFrame(COBID, payload);
                Add_table_content("Disable",Send_data_model,ECU_frame);
                m_canDevice->writeFrame(ECU_frame);
            }
            //Disable Current regulator
            else if(tab_index==1)
            {
                Iq_frame = QCanBusFrame(COBID, payload);
                Id_frame = QCanBusFrame(COBID, payload);
                Add_table_content("Disable",Send_data_model,Iq_frame);
                m_canDevice->writeFrame(Iq_frame);
            }
        }
    }
}

//Change button state
void MainWindow::on_Node_textChanged(const QString &arg1)
{
    ui->Pre_operational->setEnabled(true);
    ui->Operational->setEnabled(true);
    ui->Reset_Node->setEnabled(true);
    ui->Reset_communication->setEnabled(true);
    ui->Stop->setEnabled(true);
}
void MainWindow::on_send_interval_textChanged(const QString &arg1)
{
    if ( !arg1.isEmpty() )
    {
        ui->Enable->setEnabled(true);
    }
    else
    {
        ui->Enable->setEnabled(false);
    }
}
void MainWindow::on_Torque_control_clicked()
{
    const bool Asymmetric = ui->checkBox->isChecked();
    if(Asymmetric)
    {
         ui->lineEdit_6->setEnabled(true);
         ui->lineEdit->setEnabled(true);
         ui->lineEdit_5->setEnabled(true);
         ui->lineEdit_2->setEnabled(false);
         ui->lineEdit_3->setEnabled(false);
         ui->lineEdit_4->setEnabled(false);
    }
    else{
        ui->lineEdit_6->setEnabled(false);
        ui->lineEdit->setEnabled(true);
        ui->lineEdit_5->setEnabled(true);
        ui->lineEdit_2->setEnabled(false);
        ui->lineEdit_3->setEnabled(false);
        ui->lineEdit_4->setEnabled(false);
    }

}
void MainWindow::on_Speed_control_clicked()
{
    const bool Asymmetric = ui->checkBox->isChecked();
    if(Asymmetric)
    {
        ui->lineEdit_4->setEnabled(true);
        ui->lineEdit_2->setEnabled(true);
        ui->lineEdit_6->setEnabled(false);
        ui->lineEdit_5->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->lineEdit_3->setEnabled(true);
    }
    else
    {
        ui->lineEdit_4->setEnabled(true);
        ui->lineEdit_2->setEnabled(true);
        ui->lineEdit_6->setEnabled(false);
        ui->lineEdit_5->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->lineEdit_3->setEnabled(false);
    }
}
void MainWindow::on_Cartesian_clicked()
{
    ui->Iq_Value_Ca->setEnabled(true);
    ui->Id_Value_Ca->setEnabled(true);
    ui->lineEdit_8->setEnabled(false);
    ui->lineEdit_10->setEnabled(false);
    ui->radioButton_5->setEnabled(false);
    ui->radioButton_6->setEnabled(false);
}
void MainWindow::on_Polar_clicked()
{
    ui->Iq_Value_Ca->setEnabled(false);
    ui->Id_Value_Ca->setEnabled(false);
    ui->lineEdit_8->setEnabled(true);
    ui->lineEdit_10->setEnabled(true);
    ui->radioButton_5->setEnabled(true);
    ui->radioButton_6->setEnabled(true);   
}
void MainWindow::on_checkBox_stateChanged(int arg1)
{
    const bool Asymmetric = arg1;
    const bool Torque = ui->Torque_control->isChecked();
    const bool Speed = ui->Speed_control->isChecked();
    if(Asymmetric)//If Asymmetric has been clicked
    {
        if(Torque)
        {
            ui->lineEdit_6->setEnabled(true);
        }
        else if(Speed)
        {
            ui->lineEdit_3->setEnabled(true);
        }
    }
    else
    {
        ui->lineEdit_6->setEnabled(false);
        ui->lineEdit_3->setEnabled(false);
    }
}

//Receive Data
void MainWindow::processReceivedFrames()
{
    int length;
    bool ok;
    if (!m_canDevice)
    {
        ui->textEdit_2->setText("No CAN device");
        return;
    }
    //While receive frame from server
    while (m_canDevice->framesAvailable())
    {
        const QCanBusFrame frame = m_canDevice->readFrame();
        QString view;
        QString zero = QString::number(0,16);
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            view = m_canDevice->interpretErrorFrame(frame);
        else
            view = frame.toString();

        const QString time = QString::fromLatin1("%1.%2  ")
                .arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
                .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0'));
        quint32 ID = frame.frameId();
        QString PDO_number = QString::number(ID, 16);
        int ID_type = ID - ui->Node->text().toUInt(nullptr, 16);

        //Receive Tx-PDO1 message
        if(ID_type == 180)
        {
            Add_table_content("TXPDO1",Receive_data_model,frame);
            //Mode of operation:
            QString byte0 = HextoBin(frame.toString().mid(16,2));
            for(int length = byte0.size();length<8;length++)
            {
                byte0 = "0"+byte0;
            }

            if(byte0.mid(7,1).toInt() == 1)
                ui->Operation_mode->setText("OFF");

            if(byte0.mid(6,1).toInt() == 1)
                ui->Operation_mode->setText("Vector control");

            if(byte0.mid(5,1).toInt() == 1)
                ui->Operation_mode->setText("BLDC control");

            if(byte0.mid(4,1).toInt() == 1)
                ui->Operation_mode->setText("Dicharge capacitor");

            if(byte0.mid(3,1).toInt() == 1)
                ui->Operation_mode->setText("Open rotating mode");

            if(byte0.mid(2,1).toInt() == 1)
                ui->Operation_mode->setText("Stationary vector mode");

            if(byte0.mid(1,1).toInt() == 1)
                ui->Operation_mode->setText("Reserved");

            if(byte0.mid(0,1).toInt() == 1)
                ui->Operation_mode->setText("Switch off PWM");

            //Power Module Error (Hardware detected):
            QString byte1 = HextoBin(frame.toString().mid(19,2));
            QString zero = QString::number(0,2);
            for(length=byte1.size();length<8;length++)
            {
                byte1=zero+byte1;
            }
            if(byte1.midRef(7,1).toInt()==1)
                ui->Power_module_error->setText("Short circuit,Vce desatuation");

            if(byte1.midRef(6,1).toInt()==1)
                ui->Power_module_error->setText("5V supply under voltage");

            if(byte1.midRef(5,1).toInt()==1)
                ui->Power_module_error->setText("Inverter over temp");

            if(byte1.midRef(4,1).toInt()==1)
                ui->Power_module_error->setText("Auxiliary supply under voltage");

            if(byte1.midRef(3,1).toInt()==1)
                ui->Power_module_error->setText("Over current I1,I2 or I3");

            if(byte1.midRef(0,1).toInt()==1)
                ui->Power_module_error->setText("DC link over voltage");

            //System Error 2:
            QString byte2 = HextoBin(frame.toString().mid(22,2));
            for(length=byte2.size();length<8;length++)
            {
                byte2=zero+byte2;
            }
            if(byte2.mid(5,1).toInt()==1)
                ui->System_error->append("Motor temperature sensor fault");

            if(byte2.mid(4,1).toInt()==1)
                ui->System_error->append("Error in vector control loop");

            if(byte2.mid(2,1).toInt()==1)
                ui->System_error->append("Digital input MP_DIC1 error");

            if(byte2.mid(1,1).toInt()==1)
                ui->System_error->append("Encoder error");

            //System Error 1:
            QString byte3 = HextoBin(frame.toString().mid(25,2));
            for(length=byte3.size();length<8;length++)
            {
                byte3=zero+byte3;
            }
            if(byte3.midRef(7,1).toInt()==1)
                ui->System_error->append("Electronic over temp");

            if(byte3.midRef(6,1).toInt()==1)
                ui->System_error->append("DC link under/over voltage");

            if(byte3.midRef(5,1).toInt()==1)
                ui->System_error->append("Motor over temperature");

            if(byte3.midRef(4,1).toInt()==1)
                ui->System_error->append("Over current");

            if(byte3.midRef(3,1).toInt()==1)
                ui->System_error->append("Watchdog");

            if(byte3.midRef(2,1).toInt()==1)
                ui->System_error->append("Over speed");

            if(byte3.midRef(1,1).toInt()==1)
                ui->System_error->append("Application error");

            if(byte3.midRef(0,1).toInt()==1)
                ui->System_error->append("Communication error");

            //System Warning:
            QString byte4 = HextoBin(frame.toString().mid(28,2));
            for(length=byte4.size();length<8;length++)
            {
                byte4=zero+byte4;
            }
            if(byte4.midRef(7,1).toInt()==1)
                ui->System_warning->append("Electronic over temperature");

            if(byte4.midRef(6,1).toInt()==1)
                ui->System_warning->append("Sensor trip detected");

            if(byte4.midRef(5,1).toInt()==1)
                ui->System_warning->append("Reference or limits in Rx-PDO's were adjusted to valid values");

            if(byte4.midRef(4,1).toInt()==1)
                ui->System_warning->append("Capacitance discharge failure");

            if(byte4.midRef(3,1).toInt()==1)
                ui->System_warning->append("Cutback limiter active");

            if(byte4.midRef(2,1).toInt()==1)
                ui->System_warning->append("Boot-up sequence not finished");

            if(byte4.midRef(1,1).toInt()==1)
                ui->System_warning->append("At least 1 initialization failed");

            if(byte4.midRef(0,1).toInt()==1)
                ui->System_warning->append("At least 1 device communication failed");

            //Status:
            QString byte5 = HextoBin(frame.toString().mid(31,2));
            for(length=byte5.size();length<8;length++)
            {
                byte5=zero+byte5;
            }
            if(byte5.midRef(7,1).toInt()==1)
                ui->Status->setText("Ready for operation");             

            if(byte5.midRef(6,1).toInt()==1)
                ui->Status->setText("Digital input 1");                        

            if(byte5.midRef(5,1).toInt()==1)
                ui->Status->setText("Digital input 2");            

            if(byte5.midRef(4,1).toInt()==1)
                ui->Status->setText("Digital output 1");

            if(byte5.midRef(3,1).toInt()==1)
                ui->Status->setText("Digital output 2");

            if(byte5.midRef(2,1).toInt()==1)
                ui->Status->setText("At least 1 device communication failed");
        }

        //Receive Tx-PDO2 message
        if(ID_type == 280)
        {
            Add_table_content("TXPDO2",Receive_data_model,frame);
            ui->Actual_torque->setText(frame.toString().mid(16,5));
            ui->Phase_current->setText(frame.toString().mid(22,5));
            ui->DC_voltage->setText(frame.toString().mid(28,5));
            ui->Iq_actual->setText(frame.toString().mid(34,5));
        }

        //Receive Tx-PDO3 message
        if(ID_type == 380)
        {
            Add_table_content("TXPDO3",Receive_data_model,frame);
            ui->Speed->setText(frame.toString().mid(16,5));
            ui->motor_angle->setText(frame.toString().mid(22,5));
            ui->Id_actual->setText(frame.toString().mid(28,5));
            ui->Mech_power->setText(frame.toString().mid(34,5));
        }

        //Receive Tx-PDO4 message
        if(ID_type == 480)
        {
            Add_table_content("TXPDO4",Receive_data_model,frame);
            ui->Motor_temp_1->setText(frame.toString().mid(16,2));
            ui->Motor_temp_2->setText(frame.toString().mid(19,2));
            ui->PCB_temperature->setText(frame.toString().mid(22,2));
            ui->DCB1_temperature->setText(frame.toString().mid(25,2));
            ui->DCB2_temperature->setText(frame.toString().mid(28,2));
            ui->DCB3_temperature->setText(frame.toString().mid(31,2));
            ui->Heatsink_temperature->setText(frame.toString().mid(34,2));
            ui->Hell_sector->setText(frame.toString().mid(37,2));
        }  

        //Receive Tx-SDO message
        if(ID_type == 580)
        {
            QString Command = frame.toString().mid(16,2);
            if(Command=="60")
            {
                Add_table_content("SDOmsg accept",Receive_data_model,frame);
            }
            else if(Command=="80")
            {
                Add_table_content("Server signals error",Receive_data_model,frame);
            }
            else
            {
                Add_table_content("TXSDO",Receive_data_model,frame);
                QString Index = frame.toString().mid(22,2)+frame.toString().mid(19,2);
                QString SubIndex = frame.toString().mid(25,2);
                QString line;
                QString lineIndex;
                QString lineSubindex;
                QStringList split;
                QString Data;
                QString content;
                Data =frame.toString().mid(37,2)+ frame.toString().mid(34,2)+frame.toString().mid(31,2)+frame.toString().mid(28,2);
                for (int j = 0; j < lines.size(); j++)
                {
                    line = lines.at(j);
                    split = line.split(";");//col data
                    if(split.size()>7)
                    {
                        lineIndex = QString::number(QString::fromStdString(lines.at(j).split(";").at(6).toStdString()).toInt(&ok,16),16);
                        lineSubindex =  QString::number(QString::fromStdString(lines.at(j).split(";").at(7).toStdString()).toInt(&ok,16),16);
                        for(int length = lineSubindex.size();length<2;length++)
                        {
                            lineSubindex= "0"+lineSubindex;
                        }
                        //Find the corresponding parameter for the received message in the file
                        if((Index == lineIndex)&&(SubIndex == lineSubindex))
                        {
                            //Convert data
                            if((QString::fromStdString(split.at(3).toStdString())=="DT_U16")||(QString::fromStdString(split.at(3).toStdString())=="DT_U32"))
                            {
                                content = HextoDec(Data);
                            }
                            else if(QString::fromStdString(split.at(3).toStdString())=="DT_F32")
                            {
                                content = HextoFloat(Data);
                            }
                            else if(QString::fromStdString(split.at(3).toStdString())=="DT_I16")
                            {
                                content = HextoDec(Data);
                            }
                            //Show the value in the line edit
                            int pagenumber = QString::fromStdString(split.at(0).toStdString()).toInt();
                            QWidget* wid = stackedWidget->widget(pagenumber);
                            QList<QLineEdit*> lineedit = wid->findChildren<QLineEdit*>();
                            for(int i = 0; i < lineedit.size(); i++)
                            {
                                lineedit.at(i)->setText(QString::number(content.toInt(),10));
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

//Timer
void MainWindow::timerEvent(QTimerEvent *event) {

    const uint COBID = Get_NodeID("ECUID");
    //Send node guard regularlly
    if(nodeguard_flag){
        if(event->timerId() == Timer1) {          
            //Send node guard message
            if (!m_canDevice)
            return;
            else
            m_canDevice->writeFrame(NodeGuard_frame);
        }
    }

    //Send SYNC regularlly
    if(SYNC_flag){
        if(event->timerId() == Timer2)
        {
            //Send SYNC message
            if (!m_canDevice)
            return;
            else
            m_canDevice->writeFrame(SYNC_frame);
        }
    }

    //Send CAN message regularlly
    if(Send_flag)
    {
        if(event->timerId() == Timer3)
        {
            if(!m_canDevice)
                return;
            else
            {
                //Send ECU message
                if(tab_index == 0)
                {                                             
                    m_canDevice->writeFrame(ECU_frame);
                }
                //Send current control message
                else if(tab_index == 1)
                {                  
                    QString Iq_input = ui->Iq_Value_Ca->text();
                    QString Id_input = ui->Id_Value_Ca->text();
                    QString Data = "0100000000000000";

                    const QByteArray payload = QByteArray::fromHex(Data.toLatin1());
                    QCanBusFrame Enable_frame = QCanBusFrame(COBID, payload);
                    m_canDevice->writeFrame(Enable_frame);
                    if ( (Iq_input.isEmpty())&&(Id_input.isEmpty()) )
                    {
                    }
                    else
                    {
                        if(Iq_frame.payload().isEmpty())
                        {
                        }
                        else
                        {
                            m_canDevice->writeFrame(Iq_frame);
                        }
                        if(Id_frame.payload().isEmpty())
                        {
                        }
                        else
                        {                                                     
                            m_canDevice->writeFrame(Id_frame);
                        }
                    }
                }            
            }
        }

    }

}

//Send node guard
void MainWindow::on_NodeGuard_Interval_textChanged(const QString &arg1)
{
    if ( !arg1.isEmpty() )
    {
        ui->checkBox_NodeGuard->setEnabled(true);
    }
    else
    {
        ui->checkBox_NodeGuard->setEnabled(false);
    }
}
void MainWindow::on_checkBox_NodeGuard_stateChanged(int arg1)
{
    nodeguard_flag = arg1;
    //Frame
    QString data = "0000000000000000";
    const uint COBID = Get_NodeID("NodeGuard");
    const QByteArray payload = QByteArray::fromHex(data.toLatin1());
    NodeGuard_frame = QCanBusFrame(COBID, payload);
    //ui->textEdit_2->append(NodeGuard_frame.toString());

    //If the nodeguard has been pressed
    if(nodeguard_flag)
    {
        Add_clear_row("NodeGuard",Send_data_model);
        ui->textEdit_2->append("Start sending NodeGaurd");

        //Start timer to send node guard regularlly
        Timer1 = startTimer(ui->NodeGuard_Interval->text().toInt());
        Add_table_content("NodeGuard",Send_data_model,NodeGuard_frame);
        Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->NodeGuard_Interval->text()));
    }
    else
    {
        //Stop sending node guard
        killTimer(Timer1);
        ui->textEdit_2->setText("Stop NodeGaurd");
    }
}

//Send SYNC
void MainWindow::on_SYNC_interval_textChanged(const QString &arg1)
{
    if ( !arg1.isEmpty() )
    {
        ui->checkBox_SYNC->setEnabled(true);
    }
    else
    {
        ui->checkBox_SYNC->setEnabled(false);
    }
}
void MainWindow::on_checkBox_SYNC_stateChanged(int arg1)
{
    SYNC_flag = arg1;     

    //Frame
    QString data = "0000000000000000";
    const uint SYNCId = ui->SYNC_ID->text().toUInt(nullptr, 16);
    const QByteArray payload = QByteArray::fromHex(data.toLatin1());
    SYNC_frame = QCanBusFrame(SYNCId, payload);

    //If the SYNC has been pressed
    if(SYNC_flag)
    {      
        Add_clear_row("SYNC",Send_data_model);             
        ui->textEdit_2->append("Start sending SYNC");

        //Start timer to send SYNC regularlly
        Timer2 = startTimer(ui->SYNC_interval->text().toInt());
        Add_table_content("SYNC",Send_data_model,SYNC_frame);
        Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->SYNC_interval->text()));
    }
    else
    {

        //Stop sending node guard
        killTimer(Timer2);               
        ui->textEdit_2->setText("Stop SYNC");
    }
}

//Send ECU/Current massage/*
void MainWindow::on_Enable_clicked(bool checked)
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        const uint COBID = Get_NodeID("ECUID");
        Send_flag = true;
        tab_index  = ui->tabWidget->currentIndex();

        //If send buttom has been clicked, then start the timer to send message regularly
        if(Send_flag)
        {
            Timer3 = startTimer(ui->send_interval->text().toInt());
            //Add text to the send table
            //ECU configuration
            if(tab_index== 0)
            {
                Add_clear_row("RXPDO ECU",Send_data_model);
                Add_table_content("RXPDO ECU",Send_data_model,ECU_frame);
                Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->send_interval->text()));
            }
            //Current regulator
            else if(tab_index == 1)
            {
                Add_clear_row("Enable command",Send_data_model);

                //Enable Frame before current regulator frame
                QString Data = "0100000000000000";
                const QByteArray payload = QByteArray::fromHex(Data.toLatin1());
                QCanBusFrame Enable_frame = QCanBusFrame(COBID, payload);
                Add_table_content("Enable command",Send_data_model,Enable_frame);
                Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->send_interval->text()));

                //Current regulator frame
                if(Iq_frame.payload().isEmpty())
                {
                }
                else
                {
                    Add_clear_row("RXPDO_Iq",Send_data_model);
                    Add_table_content("RXPDO_Iq",Send_data_model,Iq_frame);
                    Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->send_interval->text()));
                }

                if(Id_frame.payload().isEmpty())
                {
                }
                else
                {
                    Add_clear_row("RXPDO_Id",Send_data_model);
                    Add_table_content("RXPDO_Id",Send_data_model,Id_frame);
                    Send_data_model->setItem(Send_data_model->rowCount()-1, 3, new QStandardItem(ui->send_interval->text()));
                }
            }
        }
    }
}
//Upload ECU frame
void MainWindow::on_Apply_ECU_value_clicked()
{
    const bool Asymmetric = ui->checkBox->isChecked();//There is no content for Asymmetric mode
    const bool Torque = ui->Torque_control->isChecked();
    const bool Speed = ui->Speed_control->isChecked();
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->setText("Node ID is empty！");
    }
    else
    {
        QString data;
        QString data1;
        QString bit0 = "1";//Enable: 0:disabled; 1:enabled
        QString bit1;//Control mode: 0:Torque mode, 1:Speed mode
        QString bit2 = "0";//Limitation mode: 0:Symmetric; 1:Asymmetric
        //for bit 3+4, which mode will be used?
        //for bit 6, multipurpose digital output  is off or on?
        QString bit34 ="00";//Operation Mode: x0: Auto Mode 01: FOC control 11: BLDC control
        QString bit5 = "0";
        QString bit6 = "0";//Multipurpose digital output: 0: Switch Multipurpose digital output off 1: Switch Multipurpose digital output on
        QString byte0;//Command
        QString byte123 = "000000"; //No use
        QString Message;
        QString Speed_reference;
        QString Speed_max;
        QString Torque_reference;
        QString Torque_max;

        //ECU Torque
        if(Torque)
        {
            bit1 = "0";
            data1 = bit6+bit5+bit34+bit2+bit1+bit0;
            byte0 = BintoHex(data1);
            Torque_reference = QString::number((ui->lineEdit->text().toInt())*100,16).toUpper();
            Speed_max = QString::number((ui->lineEdit_5->text().toInt()),16).toUpper();
            Message = Process_ECU_value(Torque_reference,Speed_max);
        }
        //ECU Speed
        if(Speed)
        {
            bit1 = "1";
            data1 = bit6+bit5+bit34+bit2+bit1+bit0;
            byte0 = BintoHex(data1);
            Speed_reference = QString::number( ui->lineEdit_4->text().toInt(),16).toUpper();
            Torque_max = QString::number((ui->lineEdit_2->text().toInt())*100,16).toUpper();
            Message = Process_ECU_value(Torque_max,Speed_reference);
        }
       data = byte0+byte123+Message;

       //Frame
       const uint COBID =Get_NodeID("ECUID");
       const QByteArray payload = QByteArray::fromHex(data.toLatin1());
       ECU_frame = QCanBusFrame(COBID, payload);
       //ui->textEdit_2->append(ECU_frame.toString());

       //Check the send interval
       if ( ui->send_interval->text().isEmpty() )
       {
           ui->textEdit_2->setText("Message interval is empty");
       }
       else
       {
          ui->Enable->setEnabled(true);
       }
       connect(ui->lineEdit_4, SIGNAL(textChanged(QString)), this, SLOT(Convert_Data(QString)));
    }
}
//Upload current regulator frame
void MainWindow::on_Send_Current_value_clicked()
{
    //COBID
    const uint COBID = Get_NodeID("SDO");

    //Data
    QString Iq_input = ui->Iq_Value_Ca->text();
    QString Id_input = ui->Id_Value_Ca->text();
    QString Bi_Byte0 = "0100011";//Closed loop rotating (current) + D-axis and q-axis current test mode + Test mode disabled
    QString Mode = BintoHex(Bi_Byte0);
    QString Index = "6020";//Index:0x2060
    QString Subindex;//Subindex
    QString Current_value;

    if ( (Iq_input.isEmpty())&&(Id_input.isEmpty()) )
    {
        ui->textEdit_2->append("There is no current value!");
    }
    else
    {
        //Iq
        if(Iq_input.isEmpty())
        {
          ui->textEdit_2->append("No Iq current value!");
        }
        else
        {
           Subindex = "02";
           Current_value = Process_Current_value(Iq_input);
           QString Iq_data = Mode+Index+Subindex+Current_value;

           //Frame
           const QByteArray payload = QByteArray::fromHex(Iq_data.toLatin1());
           Iq_frame = QCanBusFrame(COBID, payload);
           //ui->textEdit_2->append(Iq_frame.toString());
        }

        //Id
        if(Id_input.isEmpty())
        {
            ui->textEdit_2->append("No Id current value!");
        }
        else
        {
            Subindex = "03";
            Current_value = Process_Current_value(Id_input);
            QString Id_data = Mode+Index+Subindex+Current_value;

            //Frame
            const QByteArray payload = QByteArray::fromHex(Id_data.toLatin1());
            Id_frame = QCanBusFrame(COBID, payload);
            //ui->textEdit_2->append(Id_frame.toString());
        }
    }

    if (ui->send_interval->text().isEmpty())
    {
        ui->textEdit_2->setText("Message interval is empty");
    }
    else
    {
       ui->Enable->setEnabled(true);
    }
}

//Input file
void MainWindow::on_choose_file_clicked()
{
    QString filePath;
    bool ok;
    //Save the path of the file
    filePath = QFileDialog::getOpenFileName(this, QStringLiteral("Choose Excel file"), "",QStringLiteral("Excel file(*.xls *.xlsx *.csv)"));
    if(filePath.isEmpty())
        return;
    QFile file(filePath);
    QTextStream textStream(&file);
    QList<QStandardItem*> grandpa;
    QString line;
    QStringList split;
    QStandardItem* ID1;
    QStandardItem* name1;
    //Create new model
    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels(QStringList()<<QStringLiteral("ID")<< QStringLiteral("parameter"));//Create two columns

    if(file.open(QIODevice::ReadOnly))
    {
        while (!textStream.atEnd())
        {
            lines.push_back(textStream.readLine());
        }
        //Go through every row
        for (int j = 0; j < lines.size(); j++)
        {
            line = lines.at(j);
            split = line.split(";");//col data
            if(split.size()>6)
            {
                if(split.at(1).toStdString()=="ParentId")//Record the row number before data row
                {
                    start = j;
                }
                if(j>start)
                {
                    //First data row
                    if(split.at(1).toStdString()=="-1")
                    {
                        name1 = new QStandardItem(QString::fromStdString(split.at(2).toStdString()));
                        ID1 = new QStandardItem(QString::fromStdString(split.at(0).toStdString()));
                        ID1->setToolTip(QString::fromStdString(split.at(0).toStdString()));
                        grandpa.append(ID1);
                        grandpa.append(name1);
                        model->appendRow(grandpa);
                    }
                    else
                    {
                        QList<QStandardItem*> item;
                        QStandardItem* ID = new QStandardItem(QString::fromStdString(split.at(0).toStdString()));
                        QStandardItem* name = new QStandardItem(QString::fromStdString(split.at(2).toStdString()));
                        ID->setToolTip(QString::fromStdString(split.at(0).toStdString()));//Set a "ID" mark to the item
                        item.append(ID);
                        item.append(name);
                        //Scan the parentID row to see if there is item that has same parentID as the "ID" mark
                        QList<QStandardItem *> parent = model->findItems(QString::fromStdString(split.at(1).toStdString()),Qt::MatchContains | Qt::MatchRecursive, 0);

                        //If items have the same parentID as the "ID" mark, then add those items as the "ID" mark item's child
                        for(int i = 0; i < parent.size(); ++i)
                        {
                            if(parent[i]->toolTip() == QString::fromStdString(split.at(1).toStdString()))
                               parent[i]->appendRow(item);
                        }
                    }
                }
            }
        }
    }
    //Add the model into the tree
    ui->treeView->setModel(model);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //Create pages for those parameters
    int parents;
    stackedWidget = new QStackedWidget;
    QSignalMapper* mSignalMapper = new QSignalMapper(this);

    //Add Store pushbuttom to let user store value
    QPushButton *Store = new QPushButton("Store");
    connect(mSignalMapper,SIGNAL(mappedWidget(QWidget*)),this,SLOT(Convert_Data(const QString)));
    ui->gridLayout_27->addWidget(stackedWidget,0, 0);
    ui->gridLayout_27->addWidget(Store,0, 1);

    for(int i = start+1;i < lines.size(); i++)
    {
        line = lines.at(i);
        split = line.split(";");//col data
        if(split.size()>2)
        {
            //If parameter ID is the same as its parentID, just add its name on the page
            if(QString::fromStdString(split.at(0).toStdString()).toInt(&ok)==QString::fromStdString(split.at(1).toStdString()).toInt(&ok))
            {
                parents = QString::fromStdString(split.at(0).toStdString()).toInt(&ok);
                QLabel *name = new QLabel();
                stackedWidget->insertWidget(QString::fromStdString(split.at(0).toStdString()).toInt(&ok),name);
            }
            //If the parameter has ParentID, then insert its page behind its parents's page.
            if(parents == QString::fromStdString(split.at(1).toStdString()).toInt(&ok))
            {
                QLabel *name = new QLabel();
                stackedWidget->insertWidget(QString::fromStdString(split.at(0).toStdString()).toInt(&ok),name);
            }
        }
    }
    //Going through the model to distinguish parents item and children item and add corresponding widgets
    for(int i = 0;i < model->rowCount() ;i++)
    {
        QStandardItem *item = model->item(i);
        QModelIndex x=model->sibling(item->index().row(),0,item->index());
        QStandardItem* nextItem = model->itemFromIndex(x);
        HasChild(nextItem,stackedWidget,mSignalMapper);
    }
    connect(ui->treeView, SIGNAL(clicked(const QModelIndex)), this, SLOT(getTreeClicked(const QModelIndex)));
    connect(this, SIGNAL(page(int)),stackedWidget, SLOT(setCurrentIndex(int)));
    connect(Store, SIGNAL(clicked()),this, SLOT(Store_SDO()));
}

//Add page content for each SDO parameter
void MainWindow::HasChild(QStandardItem *item,QStackedWidget *stackedWidget,QSignalMapper* mSignalMapper)
{
    bool ok;
    QString content = QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(3).toStdString());
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    QModelIndex x=model->sibling(item->index().row(),0,item->index());
    item = model->itemFromIndex(x);
    QScrollArea *Scroll = new QScrollArea();
    QWidget * pWgt = new QWidget;
    QGridLayout* grid_layout = new QGridLayout();
    Scroll->setWidgetResizable(true);
    Q_ASSERT(item);
    QLabel *name = new QLabel(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(2).toStdString())+": "+QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(3).toStdString()));
    grid_layout->addWidget(name,0,0,1,-1);

    //Some parameter didn't have data type in the excel
    if(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(3).toStdString()).isEmpty())
    {
        QLabel *name = new QLabel();
        stackedWidget->insertWidget(item->index().data(0).toInt(&ok),name);
    }
    else
    {
        // If it is a parent item
        if(item->hasChildren())
        {
            //Add widget into the parent item page
           for(int i = 0;i < item->rowCount() ;i++)
           {
               QStandardItem * childitem = item->child(i);
               QModelIndex Child_Index=model->indexFromItem(childitem);
               QLabel *name = new QLabel(QString::fromStdString(lines.at((Child_Index.data(0).toInt(&ok)+start+1)).split(";").at(2).toStdString()));
               QLabel *type = new QLabel(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(3).toStdString()));
               QLabel *Access = new QLabel("Ascess: "+QString::fromStdString(lines.at((Child_Index.data(0).toInt(&ok)+start+1)).split(";").at(5).toStdString()));
               QPushButton *Read = new QPushButton("Read");
               QPushButton *Write = new QPushButton("Write");
               QFrame *line = new QFrame();
               grid_layout->addWidget(name,2*i+1,0);
                grid_layout->addWidget(type,2*i+1,1);
               grid_layout->addWidget(Access,2*i+1,2);
               grid_layout->addWidget(Read,2*i+1,3);
               grid_layout->addWidget(Write,2*i+1,4);
               line->setFrameShape(QFrame::HLine);
               line->setFrameShadow(QFrame::Plain);
               grid_layout->addWidget(line,i*2+2,0,1,4);
               //Connect read and write button in the parents item
               connect(Read, SIGNAL(clicked()),this, SLOT(Read_Multi_SDO()));
               connect(Write, SIGNAL(clicked()),this, SLOT(Write_Multi_SDO()));
           }
           pWgt->setLayout(grid_layout);
           Scroll->setWidget(pWgt);
           stackedWidget->insertWidget(item->index().data(0).toInt(&ok),Scroll);

           //Going through each child item to find out if they are parent of other item
           for(int i = 0;i < item->rowCount() ;i++)//1.inverter->Type and version
           {
               QStandardItem * childitem = item->child(i);
               HasChild(childitem,stackedWidget,mSignalMapper);
           }
       }
        //If it is not a parent item
       else
       {
            //Add weidget into the page
           QLabel *Access = new QLabel("Ascess: ");
           QLabel *Access_value = new QLabel(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(5).toStdString()));
           QPushButton *Read = new QPushButton("Read");
           QLabel *Value = new QLabel("Value: ");
           QLineEdit *Value_content = new QLineEdit();
           QLabel *Max = new QLabel("Max: "+QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(12).toStdString()));
           QLabel *Min = new QLabel("Min: "+QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(11).toStdString()));
           QLabel *Default = new QLabel("Default: "+QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(10).toStdString()));
           QPushButton *Write = new QPushButton("Write");
           QString Des = QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(14).toStdString());
           QTextEdit *Description = new QTextEdit(Des.replace("\\n","\n"));
           grid_layout->addWidget(Description,1,0,1,-1);
           grid_layout->addWidget(Access,2, 0);
           grid_layout->addWidget(Access_value,2, 1);
           grid_layout->addWidget(Read,2, 5);
           grid_layout->addWidget(Value,3, 0);
           grid_layout->addWidget(Value_content,3, 1);
           grid_layout->addWidget(Max,3, 2);
           grid_layout->addWidget(Min,3, 3);
           grid_layout->addWidget(Default,3, 4);
           grid_layout->addWidget(Write,3, 5);
           pWgt->setLayout(grid_layout);
           Scroll->setWidget(pWgt);
           stackedWidget->insertWidget(item->index().data(0).toInt(&ok),Scroll);          
           mSignalMapper->setMapping(Value_content,Value_content);

           //Connect signal for the button and line edit in children item
           connect(Value_content, SIGNAL(textEdited(QString)), this, SLOT(Convert_Data(QString)));
           connect(Write, SIGNAL(clicked()),this, SLOT(Write_SDO()));
           connect(Read, SIGNAL(clicked()),this, SLOT(Read_SDO()));
       }
    }

}

//If user click the tree item, this will return the name of the item
void MainWindow::getTreeClicked(const QModelIndex index)
{
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    QModelIndex x=model->sibling(index.row(),0,index);
    QStandardItem* nextItem = model->itemFromIndex(x);
    bool ok;
    emit page(nextItem->data(0).toInt(&ok));
}

//Read SDO value
//If Read button in parent item was clicked, then read all children's parameters
void MainWindow::Read_Multi_SDO()
{
    int pagenumber = stackedWidget->currentIndex();
    QStandardItem *item;
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    //Find the selected parameter button
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QList<QLabel*> lable = stackedWidget->widget(pagenumber)->findChildren<QLabel*>();
    QList<QPushButton*> pushbutton = stackedWidget->widget(pagenumber)->findChildren<QPushButton*>();

    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        for(int i = 0; i < pushbutton.size(); i++)
         {
            //Find the row location of the button: "i"
             if(pushbutton.at(i)==button)
             {
                for (int j = 0; j < lines.size(); j++)
                {
                    if(lines.at(j).split(";").size()>7)
                    {
                        //Find the selected parameter name in the file
                        if(QString::fromStdString(lines.at(j).split(";").at(2).toStdString())==lable.at(i*3/2+1)->text())
                        {                         
                            //Findout all the children of the parameter and send the read command
                            QString ID = QString::fromStdString(lines.at(j).split(";").at(0).toStdString());
                            QList<QStandardItem *> parent = model->findItems(ID,Qt::MatchExactly | Qt::MatchRecursive, 0);
                            for(int i = 0; i < parent.size(); ++i)
                            {
                                QModelIndex x=model->sibling(parent[i]->index().row(),0,parent[i]->index());
                                item = model->itemFromIndex(x);
                                Read_all_child_value(item);
                            }
                        }
                    }
                }
             }
         }
    }
}
//Read multiple SDO value
void MainWindow::Read_all_child_value(QStandardItem *item)
{
    Q_ASSERT(item);
    //Get the index of the parameter
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    QModelIndex x=model->sibling(item->index().row(),0,item->index());
    item = model->itemFromIndex(x);
    bool ok;

    //Scan every parent item to find out the lowest level item
    if(item->hasChildren())
    {
        for(int i = 0;i < item->rowCount() ;i++)
        {
            QStandardItem * childitem = item->child(i);
            Read_all_child_value(childitem);
        }
    }
    //If the item is the lowest level item, them send their input value to the can bus
    else
    {
        QString Index =QString::number(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(6).toStdString()).toUInt(&ok,16),16);
        QString SubIndex =QString::number(QString::fromStdString(lines.at((item->index().data(0).toInt(&ok)+start+1)).split(";").at(7).toStdString()).toUInt(&ok,16),16);

        //Frame
        QString Message = Process_Read_SDO_Msg(Index,SubIndex);
        const uint COBID = Get_NodeID("SDO");
        const QByteArray payload = QByteArray::fromHex(Message.toLatin1());
        QCanBusFrame SDO = QCanBusFrame(COBID, payload);
        Add_clear_row("SDO_ReadParam",Send_data_model);
        Add_table_content("SDO_ReadParam",Send_data_model,SDO);

        //Send SDO message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(SDO);
    }
}
//If Read button in children item was clicked, then read one parameter
void MainWindow::Read_SDO()
{
    int pagenumber = stackedWidget->currentIndex();
    bool ok;
    QString Index =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(6).toStdString()).toUInt(&ok,16),16);
    QString SubIndex =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(7).toStdString()).toUInt(&ok,16),16);
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        //Frame
        QString Message = Process_Read_SDO_Msg(Index,SubIndex);
        const uint COBID = Get_NodeID("SDO");
        const QByteArray payload = QByteArray::fromHex(Message.toLatin1());
        QCanBusFrame SDO = QCanBusFrame(COBID, payload);
        Add_clear_row("SDO_ReadParam",Send_data_model);
        Add_table_content("SDO_ReadParam",Send_data_model,SDO);

        //Send read SDO message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(SDO);
    }
}

//Write SDO value
//If Write button in parent item was clicked, then write all children's parameters
void MainWindow::Write_Multi_SDO()
{
    int pagenumber = stackedWidget->currentIndex();
    QStandardItem *item;
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    QString ID;
    //stackedWidget->addWidget(stackedWidget->widget(index-1));
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QList<QLabel*> lable = stackedWidget->widget(pagenumber)->findChildren<QLabel*>();
    QList<QPushButton*> pushbutton = stackedWidget->widget(pagenumber)->findChildren<QPushButton*>();

    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        for(int i = 0; i < pushbutton.size(); i++)
         {
            //Find the row location of the button: "i"
             if(pushbutton.at(i)==button)
             {
                for (int j = 0; j < lines.size(); j++)
                {
                    if(lines.at(j).split(";").size()>7)
                    {
                        //Find the selected parameter name in the file
                        if(QString::fromStdString(lines.at(j).split(";").at(2).toStdString())==lable.at((i*3-1)/2)->text())
                        {
                            //Findout all the children of the parameter and send the read command
                             ID = QString::fromStdString(lines.at(j).split(";").at(0).toStdString());
                             QList<QStandardItem *> parent = model->findItems(ID,Qt::MatchExactly | Qt::MatchRecursive, 0);
                             for(int i = 0; i < parent.size(); ++i)
                             {
                                 QModelIndex x=model->sibling(parent[i]->index().row(),0,parent[i]->index());
                                 item = model->itemFromIndex(x);
                                 Write_all_child_value(item);
                             }                     
                        }
                    }
                }
             }
         }
    }
}
//Write multiple SDO value
void MainWindow::Write_all_child_value(QStandardItem *item)
{
    Q_ASSERT(item);
    //Get the index of the parameter
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->treeView->model());
    QModelIndex x=model->sibling(item->index().row(),0,item->index());
    item = model->itemFromIndex(x);
    bool ok;
    int pagenumber = item->index().data(0).toInt(&ok);
    QString Data;
    //Going through all the children item
    if(item->hasChildren())
    {
        for(int i = 0;i < item->rowCount() ;i++)
        {
            QStandardItem * childitem = item->child(i);
            Write_all_child_value(childitem);
        }
    }
    //Send command for the lowest level item
    else
    {
        QWidget* wid = stackedWidget->widget(item->index().data(0).toInt(&ok));
        int index = stackedWidget->indexOf(wid);
        QList<QLineEdit*> line = stackedWidget->widget(index)->findChildren<QLineEdit*>();
        QString Index =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(6).toStdString()).toUInt(&ok,16),16);
        QString SubIndex =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(7).toStdString()).toUInt(&ok,16),16);
        QString Datatype = QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(3).toStdString());
        if(ui->Node->text().isEmpty())
        {
            ui->textEdit_2->append("Please input the node ID");
        }
        else
        {
            //Get value
            for(int i = 0; i < line.size(); i++)
            {
                Data = line.at(i)->text();
            }
            //Whether the value of the parameter has been input
             if(Data.isEmpty())
             {
             }
             else
             {
                 //Frame
                 QString Message = Process_Write_SDO_Msg(Index,SubIndex,Data,Datatype);
                 const uint COBID = Get_NodeID("SDO");
                 const QByteArray payload = QByteArray::fromHex(Message.toLatin1());
                 QCanBusFrame SDO = QCanBusFrame(COBID, payload);
                 Add_clear_row("SDO_WriteParam",Send_data_model);
                 Add_table_content("SDO_WriteParam",Send_data_model,SDO);

                 //Send SDO message
                 if (!m_canDevice)
                 return;
                 else
                 m_canDevice->writeFrame(SDO);
             }
        }
    }
}
//Write one SDO value
void MainWindow::Write_SDO()
{
    int pagenumber = stackedWidget->currentIndex();
    QWidget* wid = stackedWidget->currentWidget();
    int index = stackedWidget->indexOf(wid);
    bool ok;
    QList<QLineEdit*> line = stackedWidget->widget(index)->findChildren<QLineEdit*>();
    QString Index =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(6).toStdString()).toUInt(&ok,16),16);
    QString SubIndex =QString::number(QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(7).toStdString()).toUInt(&ok,16),16);
    QString Datatype = QString::fromStdString(lines.at(pagenumber+start+1).split(";").at(3).toStdString());
    QString Data;

    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        //Get value
        for(int i = 0; i < line.size(); i++)
        {
            Data = line.at(i)->text();
        }

        //Frame
        const uint COBID = Get_NodeID("SDO");
        QString Message = Process_Write_SDO_Msg(Index,SubIndex,Data,Datatype);
        const QByteArray payload = QByteArray::fromHex(Message.toLatin1());
        QCanBusFrame SDO = QCanBusFrame(COBID, payload);
        frame = SDO;
        Add_clear_row("SDO_WriteParam",Send_data_model);
        Add_table_content("SDO_WriteParam",Send_data_model,SDO);

       //Send SDO write message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(SDO);
    }
}

//Store SDO value
void MainWindow::Store_SDO()
{
    if(ui->Node->text().isEmpty())
    {
        ui->textEdit_2->append("Please input the node ID");
    }
    else
    {
        //Frame
        const uint COBID = Get_NodeID("SDO");
        QString Data = "2310100173617665";
        const QByteArray payload = QByteArray::fromHex(Data.toLatin1());
        QCanBusFrame Store_frame = QCanBusFrame(COBID, payload);
        Add_clear_row("SDO-StoreEEPROM",Send_data_model);
        Add_table_content("SDO-StoreEEPROM",Send_data_model,Store_frame);

        //Send SDO message
        if (!m_canDevice)
        return;
        else
        m_canDevice->writeFrame(Store_frame);
    }
}

//Process data
QString MainWindow::Process_SDO_value(QString data)
{
    for(int length = data.size();length<8;length++)
    {
        data="0"+data;
    }
    QString Value = data.mid(6,2)+data.mid(4,2)+data.mid(2,2)+data.mid(0,2);
    return Value;
}
QString MainWindow::Process_Read_SDO_Msg(QString Index,QString Subindex)
{
    QString Command = "40";
    QString Index1 = Index.mid(2,2);
    QString Index2 = Index.mid(0,2);
    for(int length = Subindex.size();length<2;length++)
    {
        Subindex="0"+Subindex;
    }
    QString Data = "00000000";
    QString Message = Command+Index1+Index2+Subindex+Data;
    return Message;
}
QString MainWindow::Process_Write_SDO_Msg(QString Index,QString SubIndex,QString Data,QString Datatype)
{
    QString Command = "23";
    QString Index1 = Index.mid(2,2);
    QString Index2 = Index.mid(0,2);
    for(int length = SubIndex.size();length<2;length++)
    {
        SubIndex="0"+SubIndex;
    }
    QString Value;
    //Convert the input data to Hex according to their data type
    if((Datatype=="DT_U16")||(Datatype=="DT_U32"))
    {
        Value = DectoHex(Data);
        Value = Process_SDO_value(Value);
    }
    else if(Datatype=="DT_F32")
    {
        Value = FloattoHex(Data);
        Value = Process_SDO_value(Value);
    }
    else if(Datatype=="DT_I16")
    {
        Value = QString::number(Data.toInt(),16);
        Value = Process_SDO_value(Value);
    }
    QString Message = Command+Index1+Index2+SubIndex+Value;
    return Message;
}
QString MainWindow::Process_ECU_value(QString Torque,QString Speed)
{
    for(int length=Torque.size();length<4;length++)
    {
        Torque = "0"+Torque;
    }
    for(int length = Speed.size();length<4;length++)
    {
        Speed="0"+Speed;
    }
    QString Message = Torque.right(2)+Torque.left(2)+Speed.right(2)+Speed.left(2);
    return Message;
}
QString MainWindow::Process_Current_value(QString data)
{
    QString value = FloattoHex(data);
    for(int length=value.size();length<8;length++)
    {
        value = "0" + value;
    }
    QString Current_data = value.mid(6,2)+value.mid(4,2)+value.mid(2,2)+value.mid(0,2);
    return Current_data;
}
//Get COBID value
uint MainWindow::Get_NodeID(QString data)
{
    const uint Node = ui->Node->text().toUInt(nullptr, 16);
    QString ID;
    if(data=="ECUID")
    {
        ID = "200";
    }
    else if(data == "SDO")
    {
        ID = "600";
    }
    else if(data == "NodeGuard")
    {
        ID = "700";
    }
    const uint COBID =ID.toUInt(nullptr, 16)+Node;
    return COBID;
}

//Data type conversion
QString MainWindow::BintoHex(QString data)
{

    bool ok;
    int val=data.toInt(&ok,2);    
    data=QString::number(val,10);
    data=data.setNum(val,16);
    data=data.toUpper();
    return data;
}
QString MainWindow::HextoBin(QString data)
{

    bool ok;
    int val=data.toInt(&ok,16);
    data=QString::number(val,10);
    data=data.setNum(val,2);
    return data;
}
QString MainWindow::HextoDec(QString data)
{
    bool ok;
    int val=data.toInt(&ok,16);
    data=QString::number(val,10);
    return data;
}
QString MainWindow::FloattoHex(QString data)
{
    float f = data.toFloat();
    int i = *((int *)&f);
    QString float_value = QString("%1").arg(i, 4, 16, QLatin1Char('0'));
    return float_value;
}
QString MainWindow::DectoHex(QString data)
{
    bool ok;       
    int val=data.toInt(&ok,10);
    data=QString::number(val,10);
    data=data.setNum(val,16);
    data=data.toUpper();
    return data;
}
QString MainWindow::HextoFloat(QString data)
{
    int c = data.toInt(nullptr, 16);
    float d = *(float*)&c;
    QString Float = QString("%1").arg(d);
    return Float;
}

//Show send message
void MainWindow::Add_table_content(QString comment,QStandardItemModel* model,QCanBusFrame frame)
{
    int flag=1;
    quint32 ID = frame.frameId();
    QString CAN_ID = QString::number(ID, 16);
    QString Data_length = QString::number(frame.payload().size(),10);
    QString Data = frame.toString().mid(16);
    int Row = model->rowCount();
    if(Row>0)
    {
        //Add all items in the model to the table
        for(int i = 0;i <Row;i++)
        {
            if(comment == model->data(model->index(i,4)).toString())
            {
                model->item(i, 0)->setText(CAN_ID);
                model->item(i, 1)->setText(Data_length);
                model->item(i, 2)->setText(Data);
                model->item(i, 4)->setText(comment);
                flag = 0;//message has already shown
            }
        }
    }
    //If message is shown: do nothing, otherwise, add content to the table
    if(flag == 0)
    {
    }
    else
    { 
        model->setItem(Row, 0, new QStandardItem(CAN_ID));
        model->setItem(Row, 1, new QStandardItem(Data_length));
        model->setItem(Row, 2, new QStandardItem(Data));
        model->setItem(Row, 4, new QStandardItem(comment));
        flag =1;
    }
}
//Show received message
void MainWindow::Add_Receive_content(QString comment,QStandardItemModel* model,QCanBusFrame frame)
{
    int flag=1;
    quint32 ID = frame.frameId();
    QString CAN_ID = QString::number(ID);
    QString Data_length = QString::number(frame.payload().size());
    QString Data = frame.toString().mid(5);
    for(int i = 0;i < model->rowCount();i++)
    {
        if(comment == model->data(model->index(i,4)).toString())
        {
            model->setItem(i, 0, new QStandardItem(CAN_ID));
            model->setItem(i, 1, new QStandardItem(Data_length));
            model->setItem(i, 2, new QStandardItem(Data));
            model->setItem(i, 4, new QStandardItem(comment));
            flag = 0;//message has already shown
        }
    }

    /*If message is shown: do nothing, otherwise, add content to the table*/
    if(flag == 0)
    {
    }
    else
    {
        model->setItem(model->rowCount(), 0, new QStandardItem(CAN_ID));
        model->setItem(model->rowCount(), 1, new QStandardItem(Data_length));
        model->setItem(model->rowCount(), 2, new QStandardItem(Data));
        model->setItem(model->rowCount(), 4, new QStandardItem(comment));
    }
}
//Clear message in the table
void MainWindow::Add_clear_row(QString comment,QStandardItemModel* model)
{
    int Row = model->rowCount();
    if(Row>0)
    {
        for(int i = 0;i <Row;i++)
        {
            if(comment == model->data(model->index(i,4)).toString())
            {
                model->removeRow(i);
            }
        }
    }
}
//Clear send box
void MainWindow::on_Clear_Send_clicked()
{
    //Clear send text
    for(int i = Send_data_model->rowCount()-1;i>-1;i--)
    {
        Send_data_model->removeRow(i);
    }
}
//Clear received box
void MainWindow::on_Clear_Received_clicked()
{
    //Clear received text
    for(int i = Receive_data_model->rowCount()-1;i>-1;i--)
    {
        Receive_data_model->removeRow(i);
    }
}

