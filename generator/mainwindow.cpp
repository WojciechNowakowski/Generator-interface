#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QString>
#include <QMessageBox>

struct TSerialPortSettings
{
	QString port_number = "COM1";
	uint baud_rate = 9600;
	QSerialPort::DataBits data_bits = QSerialPort::Data8;
	QSerialPort::StopBits stop_bits = QSerialPort::OneStop;
	QSerialPort::Parity parity = QSerialPort::NoParity;
}SerialPortSettings;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_PORTS_info_button_clicked()
{
	const QList<QSerialPortInfo> &infos = QSerialPortInfo::availablePorts();
	QSerialPortInfo info;
	ui->PORTS_info_edit->clear();
	foreach (info, QSerialPortInfo::availablePorts())
	{
		ui->PORTS_info_edit->append("Name: " + info.portName());
		//ui->PORTS_info_edit->append("System Location: " + info.systemLocation());
		ui->PORTS_info_edit->append("Description: " + info.description());
		ui->PORTS_info_edit->append("Manufacturer: " + info.manufacturer());
		if(QSerialPort(info).open(QIODevice::ReadWrite))
		{
			ui->PORTS_info_edit->append("Able to open?: YES");
		}
		else
		{
			ui->PORTS_info_edit->append("Able to open?: NO");
		}
		//ui->PORTS_info_edit->append("Serial Number: " + info.serialNumber());
		//ui->PORTS_info_edit->append("Vendor Identifier: " + info.vendorIdentifier());
		//ui->PORTS_info_edit->append("Product Identifier: " + info.productIdentifier());
	}
}

double calculate_prescaler_period(double f_s, int &prescaler, int &period)
{
	int F_CPU, i = 1, prescalers[7] = {1, 2, 4, 8, 64, 256, 1024};
	double presc_x_top, top, ulamek;

	F_CPU = 32000000;
	prescaler = 1;

	presc_x_top = F_CPU / f_s;
	if (presc_x_top <= 65535)
	{
		top = presc_x_top;
	}
	else
	{
		top = presc_x_top;
		while(top > 65535)
		{
				prescaler = prescalers[i];
				top = F_CPU / (prescaler * f_s);
				i++;
				if (i > 6)
					break;
		}
	}
	prescaler = i;

	ulamek = top - int(top);
	if (ulamek / top > 0)
	{
		ulamek = 100 * ulamek / top;
		/*QMessageBox message;
		message.setWindowTitle("Ostrzeżenie");
		message.setText("Wystąpi niedokładność: " + QString::number(ulamek) + "%");
		message.setIcon(QMessageBox::Warning);
		message.exec();*/
	}

	if (top >= 65536)
	{
		QMessageBox message;
		message.setWindowTitle("Błąd");
		message.setText("Nie można uzyskać zadanej częstotliwości");
		message.setIcon(QMessageBox::Critical);
		message.exec();
		return -1.0;
	}
	period = int(top);
	return ulamek;
}

void Send_data(char* data, qint64 len, QStatusBar *statusBar)
{
	QSerialPort serial;
	serial.setPortName(SerialPortSettings.port_number);
	serial.setBaudRate(SerialPortSettings.baud_rate);
	serial.setDataBits(SerialPortSettings.data_bits);
	serial.setStopBits(SerialPortSettings.stop_bits);
	serial.setFlowControl(QSerialPort::NoFlowControl);
	serial.setParity(SerialPortSettings.parity);
	if (serial.open(QIODevice::ReadWrite))
	{
		qDebug() << "Wysyłanie danych przez port COM";
		qDebug() << "Wysłano bajtów: " << serial.write(data, len);
		serial.waitForBytesWritten(3);
		serial.flush();
		statusBar->showMessage(SerialPortSettings.port_number + " OK");
	}
	serial.close();
}

void MainWindow::on_COM_port_select_currentIndexChanged(const QString &arg1)
{
	SerialPortSettings.port_number = arg1;
}

void MainWindow::on_DATA_bits_select_currentIndexChanged(int index)
{
	switch(index)
	{
		case 0:
			SerialPortSettings.data_bits = QSerialPort::Data5;
		break;
		case 1:
			SerialPortSettings.data_bits = QSerialPort::Data6;
		break;
		case 2:
			SerialPortSettings.data_bits = QSerialPort::Data7;
		break;
		case 3:
			SerialPortSettings.data_bits = QSerialPort::Data8;
		break;
	}
}

void MainWindow::on_STOP_bits_select_currentIndexChanged(int index)
{
	switch(index)
	{
		case 0:
			SerialPortSettings.stop_bits = QSerialPort::OneStop;
		break;
		case 1:
			SerialPortSettings.stop_bits = QSerialPort::TwoStop;
		break;
	}
}

void MainWindow::on_PARITY_select_currentIndexChanged(int index)
{
	switch(index)
	{
		case 0:
			SerialPortSettings.parity = QSerialPort::NoParity;
		break;
		case 1:
			SerialPortSettings.parity = QSerialPort::EvenParity;
		break;
		case 2:
			SerialPortSettings.parity = QSerialPort::OddParity;
		break;
		case 3:
			SerialPortSettings.parity = QSerialPort::SpaceParity;
		break;
		case 4:
			SerialPortSettings.parity = QSerialPort::MarkParity;
		break;
	}
}

void MainWindow::on_BAUD_edit_textChanged(const QString &arg1)
{
	SerialPortSettings.baud_rate = arg1.toInt();
}

void Duty_Check_Send(char ch_no, QCheckBox *checkBox, QLineEdit *edit, QStatusBar *statusBar)
{
	int duty = 0;
	if(checkBox->isChecked())
	{
		duty = edit->text().toInt();
		if(duty <= 0)
		{
			QMessageBox message;
			message.setWindowTitle("Błąd");
			message.setText("Należy wpisać liczbę");
			message.setIcon(QMessageBox::Critical);
			message.exec();
		}
		else
		{
			char tmp[4], duty_l, duty_h;
			duty_l = duty % 256;
			duty_h = duty / 256;
			tmp[0] = 'K';
			tmp[1] = ch_no;
			tmp[2] = duty_l;
			tmp[3] = duty_h;
			qDebug() << "tmp:" << tmp;
			Send_data((char*)tmp, 4, statusBar);
		}
	}
	else
	{
		char tmp[4];
		tmp[0] = 'K';
		tmp[1] = ch_no;
		tmp[2] = 0;
		tmp[3] = 0;
		qDebug() << "tmp:" << tmp;
		Send_data((char*)tmp, 4, statusBar);
	}
}

void MainWindow::on_SEND_data_button_A_clicked()
{
	Duty_Check_Send(1, ui->PWM_A_ON_checkBox, ui->PWM_A_duty_edit, ui->statusBar);
}

void MainWindow::on_SEND_data_button_B_clicked()
{
	Duty_Check_Send(2, ui->PWM_B_ON_checkBox, ui->PWM_B_duty_edit, ui->statusBar);
}

void MainWindow::on_SEND_data_button_C_clicked()
{
	Duty_Check_Send(3, ui->PWM_C_ON_checkBox, ui->PWM_C_duty_edit, ui->statusBar);
}

void MainWindow::on_SEND_data_button_D_clicked()
{
	Duty_Check_Send(4, ui->PWM_D_ON_checkBox, ui->PWM_D_duty_edit, ui->statusBar);
}

void MainWindow::on_Send_frq_Button_clicked()
{
	int period, prescaler;
	char prescaler_c, per_l, per_h;
	double f_s = 50.0;
	f_s = ui->PWM_A_D_frq_edit->text().toDouble();
	qDebug() << "f_s:" << f_s;
	if(f_s <= 0)
	{
		QMessageBox message;
		message.setWindowTitle("Błąd");
		message.setText("Należy wpisać liczbę");
		message.setIcon(QMessageBox::Critical);
		message.exec();
	}
	else
	{
		double ulamek = calculate_prescaler_period(f_s, prescaler, period);
		if(ulamek == -1.0)
		{
			return;
		}
		ui->Accuracy_label->setText("Accuracy: " + QString::number(ulamek) + "%");
		ui->Period_label->setText("Period: " + QString::number(period));
		per_l = period % 256;
		per_h = period / 256;
		qDebug() << "presc = " << prescaler;
		qDebug() << "per_l = " << per_l;
		qDebug() << "per_h = " << per_h;
		prescaler_c = prescaler;
		char tmp[3], tmp2[4];
		tmp[0] = 'F';
		tmp[1] = prescaler_c;
		tmp[2] = 0;
		qDebug() << "tmp:" << tmp;
		Send_data((char*)tmp, 2, ui->statusBar);
		tmp2[0] = 'H';
		tmp2[1] = per_l;
		tmp2[2] = per_h;
		tmp2[3] = 0;
		qDebug() << "tmp2:" << tmp2;
		Send_data((char*)tmp2, 3, ui->statusBar);
	}
}
